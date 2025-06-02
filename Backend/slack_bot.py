"""Slack bot code for the Room Booker.

Written by Mervin van Brakel, 2025.
"""

import copy
import json
import logging
import os
from pathlib import Path

import pytz
from dotenv import load_dotenv
from slack_bolt.adapter.socket_mode.async_handler import AsyncSocketModeHandler
from slack_bolt.async_app import AsyncApp

import database_interfacing
import utility
from data_models import (
    Booking,
    BookingDay,
    IncorrectTimeError,
    OverlappingBookingError,
    Room,
    RoomNameTakenError,
)

load_dotenv()

logger = logging.getLogger("slack_bot")


if not os.getenv("SLACK_BOT_TOKEN"):
    error_message = "SLACK_BOT_TOKEN environment variable not set!"
    raise ValueError(error_message)

if not os.getenv("SLACK_APP_TOKEN"):
    error_message = "SLACK_APP_TOKEN environment variable not set!"
    raise ValueError(error_message)


PATH_TO_MODAL_FOLDER = Path(__file__).parent / "slack_modals"

app = AsyncApp(token=os.getenv("SLACK_BOT_TOKEN"))
loaded_modals = {}


@app.event("app_home_opened")
async def send_home_tab(client, event) -> None:
    """Sends the home tab view to the user.

    Args:
        client: The Slack client.
        event: The Slack event.
    """
    app_home_modal = await get_modal("app_home")
    user_info = await client.users_info(user=event["user"])

    if not user_info["user"]["is_admin"]:
        app_home_modal["blocks"] = app_home_modal["blocks"][:-5]  # Admin options delete

    app_home_modal["blocks"][0]["text"]["text"] = (
        f"Hey there <@{user_info['user']['id']}> :wave:"
    )

    all_rooms = await database_interfacing.get_all_rooms()

    if not all_rooms:
        app_home_modal["blocks"].insert(
            2,
            await get_text_block(
                "Hmmm... It would appear your Slack Admin has not yet configured any rooms!"
            ),
        )
        await client.views_publish(user_id=event["user"], view=app_home_modal)
        return

    room_blocks = []
    for room in all_rooms:
        room_blocks.extend(await get_bookable_room_modal(room))
        room_blocks.append({"type": "divider"})

    app_home_modal["blocks"][3:3] = room_blocks

    await client.views_publish(user_id=event["user"], view=app_home_modal)


@app.action("book_clicked")
async def send_booking_modal(ack, body, client) -> None:
    """Sends the booking modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()

    room = await database_interfacing.get_room_by_id(int(body["actions"][0]["value"]))
    booking_modal = await get_modal("booking")
    booking_modal["blocks"][0]["text"]["text"] = (
        f"Fill in the following form to book: *{room.name}*"
    )
    booking_modal["private_metadata"] = str(room.id)

    await client.views_open(trigger_id=body["trigger_id"], view=booking_modal)


@app.action("show_bookings_clicked")
async def send_booking_list_modal(ack, body, client) -> None:
    """Sends the booking list modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()
    room = await database_interfacing.get_room_by_id(int(body["actions"][0]["value"]))
    upcoming_bookings = await database_interfacing.get_coming_week_bookings_for_room(
        room
    )

    if not upcoming_bookings:
        await client.views_open(
            trigger_id=body["trigger_id"],
            view=await get_error_modal("There are no upcoming bookings in the next 7 days for this room!"),
        )
        return

    user_info = await client.users_info(user=body["user"]["id"])
    user_timezone = pytz.timezone(user_info["user"]["tz"])

    days = await utility.divide_bookings_into_days_by_timezone(
        upcoming_bookings, user_timezone
    )
    list_modal = await get_modal("empty")
    list_modal["blocks"] = [
        await get_text_block(f"These are the coming week's bookings for *{room.name}*:")
    ]

    list_modal["blocks"].extend(await get_booking_blocks_from_days(days, user_timezone))

    await client.views_open(trigger_id=body["trigger_id"], view=list_modal)


@app.action("unbook_clicked")
async def send_unbooking_modal(ack, body, client) -> None:
    """Sends the unbooking modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()
    user_id = body["user"]["id"]
    user_info = await client.users_info(user=user_id)

    if user_info["user"]["is_admin"]:
        bookings = await database_interfacing.get_all_current_and_upcoming_bookings()
    else:
        bookings = (
            await database_interfacing.get_current_and_upcoming_bookings_for_user_id(
                user_id
            )
        )

    bookings = await database_interfacing.get_current_and_upcoming_bookings_for_user_id(
        user_id
    )

    if not bookings:
        await client.views_open(
            trigger_id=body["trigger_id"],
            view=await get_error_modal(
                "You have no current or upcoming bookings to remove!"
            ),
        )
        return

    unbooking_modal = await get_modal("unbooking")
    unbooking_modal["blocks"][0]["element"][
        "options"
    ] = await bookings_to_slack_options(bookings, user_info["user"]["tz"])

    await client.views_open(trigger_id=body["trigger_id"], view=unbooking_modal)


@app.action("change_time_clicked")
async def send_change_time_modal(ack, body, client) -> None:
    """Sends the change time modal to the user."""
    await ack()

    user_id = body["user"]["id"]
    bookings = await database_interfacing.get_current_and_upcoming_bookings_for_user_id(
        user_id, fetch_recently_ended=True
    )

    if not bookings:
        await client.views_open(
            trigger_id=body["trigger_id"],
            view=await get_error_modal("You have no bookings to change the time for!"),
        )
        return

    user_info = await client.users_info(user=user_id)
    user_timezone = user_info["user"]["tz"]

    options = await bookings_to_slack_options(bookings, user_timezone)

    change_time_booking_select_modal = await get_modal("change_time_booking_select")
    change_time_booking_select_modal["blocks"][0]["element"]["options"] = options
    change_time_booking_select_modal["blocks"][0]["element"]["initial_option"] = (
        options[0]
    )

    await client.views_open(
        trigger_id=body["trigger_id"], view=change_time_booking_select_modal
    )


@app.action("show_rooms_clicked")
async def send_room_list_modal(ack, body, client) -> None:
    """Sends the room list modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()

    all_rooms = await database_interfacing.get_all_rooms()
    if not all_rooms:
        await client.views_open(
            trigger_id=body["trigger_id"],
            view=await get_error_modal(
                "Hmmm... It appears there are no rooms to show!"
            ),
        )
        return

    list_modal = await get_modal("empty")
    list_modal["blocks"] = [
        await get_text_block(
            "These are all the existing rooms and their IDs. You'll need the ID for setting up the booking displays."
        )
    ]
    list_modal["blocks"] += [
        await get_text_block(f"*{room.id}*: {room.name}") for room in all_rooms
    ]
    await client.views_open(trigger_id=body["trigger_id"], view=list_modal)


@app.action("create_room_clicked")
async def send_room_creation_modal(ack, body, client) -> None:
    """Sends the room creation modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()
    await client.views_open(
        trigger_id=body["trigger_id"], view=await get_modal("create_room")
    )


@app.action("delete_room_clicked")
async def send_room_deletion_modal(ack, body, client) -> None:
    """Sends the room deletion modal to the user.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
    """
    await ack()

    all_rooms = await database_interfacing.get_all_rooms()

    if not all_rooms:
        await client.views_open(
            trigger_id=body["trigger_id"],
            view=await get_error_modal("There are no rooms to delete!"),
        )
        return

    room_deletion_modal = await get_modal("delete_room")
    room_deletion_modal["blocks"][0]["element"]["options"] = [
        await get_option(room.name, str(room.id)) for room in all_rooms
    ]
    await client.views_open(trigger_id=body["trigger_id"], view=room_deletion_modal)


@app.view("new_booking")
async def process_booking(ack, body, client, view) -> None:
    """Processes a user booking from the booking modal.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
        view: Slack view
    """
    start_time = int(
        view["state"]["values"]["booking_start_time"]["booking_start_time_select"][
            "selected_date_time"
        ]
    )
    duration = int(
        view["state"]["values"]["booking_duration"]["booking_duration_select"][
            "selected_option"
        ]["value"]
    )
    user_info = await client.users_info(user=body["user"]["id"])
    booking = Booking(
        id=None,
        room_id=int(view["private_metadata"]),
        start_time=start_time,
        end_time=start_time + duration * 60,
        name=view["state"]["values"]["booking_name"]["booking_name_input"]["value"],
        user_id=body["user"]["id"],
        user_name=user_info["user"]["real_name"],
    )

    try:
        await database_interfacing.create_booking(booking)
    except OverlappingBookingError as error_message:
        errors = {"booking_start_time": str(error_message)}
        await ack(response_action="errors", errors=errors)
        return

    await ack()

    room = await database_interfacing.get_room_by_id(booking.room_id)
    success_modal = await get_modal("empty")
    readable_start_time = await utility.get_time_string_from_timestamp(
        booking.start_time, pytz.timezone(user_info["user"]["tz"])
    )
    readable_end_time = await utility.get_time_string_from_timestamp(
        booking.end_time, pytz.timezone(user_info["user"]["tz"])
    )

    if await utility.is_booking_time_very_late(
        booking, pytz.timezone(user_info["user"]["tz"])
    ):
        success_modal["blocks"].append(
            await get_text_block(
                f":warning: Your booking time is very late, are you okay?! You've booked *{room.name}* from *{readable_start_time}* to *{readable_end_time}*! :white_check_mark:"
            )
        )
    else:
        success_modal["blocks"].append(
            await get_text_block(
                f"You've successfully booked *{room.name}* from *{readable_start_time}* to *{readable_end_time}*! :white_check_mark:"
            )
        )
    await client.views_open(trigger_id=body["trigger_id"], view=success_modal)


@app.view("unbooking")
async def process_unbooking(ack, body, client, view) -> None:
    """Processes a user unbooking from the unbooking modal.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
        view: Slack view
    """
    selected_booking_id = int(
        view["state"]["values"]["unbooking_select"]["unbooking_select_selection"][
            "selected_option"
        ]["value"]
    )
    await database_interfacing.delete_booking(selected_booking_id)
    await ack()

    success_modal = await get_modal("empty")
    success_modal["blocks"].append(
        await get_text_block(
            "Your booking was successfully removed! :white_check_mark:"
        )
    )
    await client.views_open(trigger_id=body["trigger_id"], view=success_modal)


@app.view("change_time_booking_select")
async def process_change_time_booking_select(ack, body, client, view) -> None:
    """Processes a change time booking selection by opening a new modal where the user can
    select their new times.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
        view: Slack view
    """
    await ack()

    user_id = body["user"]["id"]

    selected_booking_id = int(
        view["state"]["values"]["booking_select"]["change_time_booking_selection"][
            "selected_option"
        ]["value"]
    )
    booking = await database_interfacing.get_booking_by_id(selected_booking_id)

    user_info = await client.users_info(user=user_id)
    user_timezone = user_info["user"]["tz"]

    change_time_time_select_modal = await get_modal("change_time_time_select")
    change_time_time_select_modal["blocks"][0]["text"]["text"] = (
        f"Select the new start and end time for the booking *{booking.name}*."
    )

    start_time_string = await utility.get_time_string_from_timestamp(
        booking.start_time, pytz.timezone(user_timezone)
    )
    change_time_time_select_modal["blocks"][1]["element"]["initial_time"] = (
        start_time_string
    )
    change_time_time_select_modal["blocks"][1]["element"]["timezone"] = user_timezone

    end_time_string = await utility.get_time_string_from_timestamp(
        booking.end_time, pytz.timezone(user_timezone)
    )
    change_time_time_select_modal["blocks"][2]["element"]["initial_time"] = (
        end_time_string
    )
    change_time_time_select_modal["blocks"][2]["element"]["timezone"] = user_timezone

    change_time_time_select_modal["private_metadata"] = str(booking.id)

    await client.views_open(
        trigger_id=body["trigger_id"], view=change_time_time_select_modal
    )


@app.view("change_time_time_select")
async def process_change_time_time_select(ack, body, client, view) -> None:
    """Parses the newly selected start and end times, validates them and updates the booking."""
    selected_booking_id = int(view["private_metadata"])

    selected_start_time_string = view["state"]["values"]["start_time_select"][
        "start_time_select_selection"
    ]["selected_time"]

    selected_end_time_string = view["state"]["values"]["end_time_select"][
        "end_time_select_selection"
    ]["selected_time"]

    user_timezone = view["state"]["values"]["start_time_select"][
        "start_time_select_selection"
    ]["timezone"]

    booking = await database_interfacing.get_booking_by_id(selected_booking_id)

    (
        start_time,
        end_time,
    ) = await utility.get_new_unix_start_end_time_from_booking_and_time_strings(
        booking,
        selected_start_time_string,
        selected_end_time_string,
        pytz.timezone(user_timezone),
    )

    try:
        await database_interfacing.change_booking_time(booking, start_time, end_time)
    except IncorrectTimeError as error_message:
        errors = {"end_time_select": str(error_message)}
        await ack(response_action="errors", errors=errors)
        return

    except OverlappingBookingError as error_message:
        errors = {"end_time_select": str(error_message)}
        await ack(response_action="errors", errors=errors)
        return

    await ack()

    success_modal = await get_modal("empty")
    user_info = await client.users_info(user=body["user"]["id"])
    old_readable_start_time = await utility.get_time_string_from_timestamp(
        booking.start_time, pytz.timezone(user_info["user"]["tz"])
    )
    old_readable_end_time = await utility.get_time_string_from_timestamp(
        booking.end_time, pytz.timezone(user_info["user"]["tz"])
    )
    new_readable_start_time = await utility.get_time_string_from_timestamp(
        start_time, pytz.timezone(user_info["user"]["tz"])
    )
    new_readable_end_time = await utility.get_time_string_from_timestamp(
        end_time, pytz.timezone(user_info["user"]["tz"])
    )
    success_modal["blocks"].append(
        await get_text_block(
            f"Your booking *{booking.name}* has been successfully changed from *{old_readable_start_time} - {old_readable_end_time}* to *{new_readable_start_time} - {new_readable_end_time}*! :white_check_mark:"
        )
    )
    await client.views_open(trigger_id=body["trigger_id"], view=success_modal)


@app.view("create_room")
async def process_create_room(ack, view) -> None:
    """Processes a new room from the create room modal.

    Args:
        ack: Slack acknowledge
        view: Slack view
    """
    new_room = Room(
        id=None, name=view["state"]["values"]["create_room_input"]["room_name"]["value"]
    )

    try:
        await database_interfacing.create_room(new_room)
    except RoomNameTakenError as error_message:
        errors = {"create_room_input": str(error_message)}
        await ack(response_action="errors", errors=errors)
        return

    await ack()


@app.view("delete_room")
async def process_delete_room(ack, view) -> None:
    """Processes a room removal from the delete room modal.

    Args:
        ack: Slack acknowledge
        body: Slack body
        client: Slack client
        view: Slack view
    """
    selected_room_id = int(
        view["state"]["values"]["delete_room_select"]["delete_room_select_selection"][
            "selected_option"
        ]["value"]
    )
    await database_interfacing.delete_room(selected_room_id)
    await ack()


async def load_modals() -> None:
    """Loads the modals into memory."""
    global loaded_modals  # noqa: PLW0603
    loaded_modals = {
        modal.stem: json.loads(modal.read_text())
        for modal in PATH_TO_MODAL_FOLDER.iterdir()
        if modal.suffix == ".json"
    }


async def get_modal(modal_name: str) -> dict:
    """Returns the modal with the given name.

    Args:
        modal_name: The name of the modal to return.

    Returns:
        The modal dict of the given name.
    """
    return copy.deepcopy(loaded_modals[modal_name])


async def get_option(text: str, value: str) -> dict:
    """Returns a Slack option dict for use in modals.

    Args:
        text: The text of the option.
        value: The value of the option.

    Returns:
        The option dict.
    """
    if len(text) > 75:  # Slack max option length
        text = text[:72] + "..."

    return {
        "text": {"type": "plain_text", "text": text, "emoji": False},
        "value": value,
    }


async def get_text_block(text: str) -> dict:
    """Returns the given string in the structure of a markdown formatted Slack text block.

    Args:
        text: The text to display in the block.

    Returns:
        The Slack text block dict.
    """
    return {
        "type": "section",
        "text": {"type": "mrkdwn", "text": text},
    }


async def get_error_modal(error_message: str) -> dict:
    """Returns a modal dict with an error message.

    Args:
        error_message: The error message to display.

    Returns:
        The error modal dict.
    """
    error_modal = await get_modal("error_message")
    error_modal["blocks"][0]["text"]["text"] = error_message
    return error_modal


async def get_bookable_room_modal(room: Room) -> list[dict]:
    """Returns a list of two block elements that show the room name and the
    action button for showing bookings and booking the room.

    Args:
        room: The room to display in the modal.

    Returns:
        The two block elements.
    """
    room_name_block = await get_text_block(f"*{room.name}*")

    actions_block = await get_modal("bookable_room_actions")
    actions_block["elements"][0]["value"] = str(room.id)
    actions_block["elements"][1]["value"] = str(room.id)

    return [room_name_block, actions_block]


async def get_booking_blocks_from_days(
    days: list[BookingDay], user_timezone: pytz.tzinfo
) -> list[dict]:
    """Gets a formatted list of blocks for the booking list modal.
    Consists of headers for each day and the bookings for that day.

    Args:
        days: The list of BookingDay objects.
        user_timezone: The user's timezone.

    Returns:
        The list of blocks for the booking list modal.
    """
    blocks = []
    for day in days:
        blocks.append({"type": "divider"})

        blocks.append(
            {
                "type": "header",
                "text": {
                    "type": "plain_text",
                    "text": day.date_text,
                },
            }
        )

        for booking in day.bookings:
            readable_start_time = await utility.get_time_string_from_timestamp(
                booking.start_time, user_timezone
            )
            readable_end_time = await utility.get_time_string_from_timestamp(
                booking.end_time, user_timezone
            )
            blocks.append(
                {
                    "type": "section",
                    "text": {
                        "type": "mrkdwn",
                        "text": f"*{readable_start_time} - {readable_end_time}*: *{booking.name}* by <@{booking.user_id}>",
                    },
                }
            )

    return blocks


async def bookings_to_slack_options(
    bookings: list[Booking], user_timezone: pytz.tzinfo
) -> list[dict]:
    """Gets a list of Slack options for the given bookings.

    Args:
        bookings: The booking to show as options.
        user_timezone: The user's timezone.

    Returns:
        The formatted string.
    """
    booking_options = []

    for booking in bookings:
        booking_start_time = await utility.get_time_string_from_timestamp(
            booking.start_time, pytz.timezone(user_timezone)
        )
        booking_end_time = await utility.get_time_string_from_timestamp(
            booking.end_time, pytz.timezone(user_timezone)
        )
        booking_date = await utility.get_date_string_from_timestamp(
            booking.start_time, pytz.timezone(user_timezone)
        )
        room_name = (await database_interfacing.get_room_by_id(booking.room_id)).name
        booking_string = f"{booking.name} | {booking_date} {booking_start_time}-{booking_end_time} | {room_name}"
        booking_options.append(await get_option(booking_string, str(booking.id)))

    return booking_options


async def start_slack_bot() -> None:
    """Starts the Slack Bolt app in an asyncio compatible way.."""
    await load_modals()
    async_handler = AsyncSocketModeHandler(
        app,
        os.getenv("SLACK_APP_TOKEN"),
    )
    await async_handler.start_async()
