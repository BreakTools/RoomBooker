"""Aiosqlite database interfacing code for the Room Booker backend.

Written by Mervin van Brakel, 2025.
"""

from __future__ import annotations

import logging
from pathlib import Path

import aiosqlite

from data_models import Booking, OverlappingBookingError, Room, RoomNameTakenError

logger = logging.getLogger("database_interface")
DATABASE_PATH = Path(__file__).parent / "room_bookings.db"
_database_connection = None


async def _get_database_connection():
    """Retrieves the database connection. If it doesn't exist, it creates it.
    It's a bit ugly. Can't think of anything better right now that works async."""
    global _database_connection  # noqa: PLW0603
    if _database_connection is None:
        _database_connection = await aiosqlite.connect(
            DATABASE_PATH, check_same_thread=False
        )
    return _database_connection


async def configure_database() -> None:
    """Configures the database with the necessary tables."""
    database = await _get_database_connection()
    await database.execute("""
        CREATE TABLE IF NOT EXISTS bookings (
            id INTEGER PRIMARY KEY,
            room_id INTEGER NOT NULL,
            start_time INTEGER NOT NULL,
            end_time INTEGER NOT NULL,
            name TEXT NOT NULL,
            user_id TEXT NOT NULL,
            user_name TEXT NOT NULL
        )
    """)
    await database.execute("""
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        )
    """)


async def create_booking(new_booking: Booking) -> None:
    """Creates a booking in the database.

    Args:
        booking: The booking to create.

    Raises:
        OverlappingBookingError: If the booking overlaps with another booking.
    """
    overlapping_booking = await _check_for_overlapping_booking(new_booking)
    if overlapping_booking is not None:
        error_message = f"A booking called '{overlapping_booking.name}' by {overlapping_booking.user_name} already exists during that time slot!"
        raise OverlappingBookingError(error_message)

    logger.info(
        "Adding booking %s by %s to the database.",
        new_booking.name,
        new_booking.user_name,
    )

    new_booking.end_time -= 1  # Little sneaky -1 to prevent overlap problems

    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        """
        INSERT INTO bookings (room_id, start_time, end_time, name, user_id, user_name)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (
            new_booking.room_id,
            new_booking.start_time,
            new_booking.end_time,
            new_booking.name,
            new_booking.user_id,
            new_booking.user_name,
        ),
    )
    await database.commit()
    await cursor.close()


async def delete_booking(booking_id: int) -> None:
    """Deletes a booking from the database.

    Args:
        booking id: The id of the booking to delete.
    """
    booking = await get_booking_by_id(booking_id)
    logger.info(
        "Deleting booking %s by %s from the database.",
        booking.name,
        booking.user_name,
    )

    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("DELETE FROM bookings WHERE id = ?", (booking.id,))
    await database.commit()
    await cursor.close()


async def extend_booking(booking: Booking, extension_time_in_seconds: int) -> None:
    """Extends the end time of a booking.

    Args:
        booking: The booking to extend.
        extension_time_in_seconds: The amount of seconds to extend the booking by.

    Raises:
        OverlappingBookingError: If the changed booking time overlaps with another booking.
    """
    new_end_time = booking.end_time + extension_time_in_seconds

    # This is a bit silly and quirky innit
    check_booking = Booking(
        id=booking.id,
        room_id=booking.room_id,
        start_time=booking.end_time + 1,
        end_time=new_end_time,
        name=booking.name,
        user_id=booking.user_id,
        user_name=booking.user_name,
    )

    overlapping_booking = await _check_for_overlapping_booking(check_booking)
    if overlapping_booking is not None:
        error_message = f"Extending the booking would overlap with another booking called '{overlapping_booking.name}' by {overlapping_booking.user_name}!"
        raise OverlappingBookingError(error_message)

    logger.info(
        "Extending booking %s by %s in the database by %s seconds.",
        booking.name,
        booking.user_name,
        extension_time_in_seconds,
    )

    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        "UPDATE bookings SET end_time = ? WHERE id = ?",
        (new_end_time, booking.id),
    )
    await database.commit()
    await cursor.close()


async def prepend_booking(booking: Booking, prepended_time_in_seconds: int) -> None:
    """Prepends the start time of a booking.

    Args:
        booking: The booking to prepend.
        prepended_time_in_seconds: The amount of seconds to prepend the booking by.

    Raises:
        OverlappingBookingError: If the changed booking time overlaps with another booking.
    """
    new_start_time = booking.start_time - prepended_time_in_seconds

    # Same silly logic as before
    check_booking = Booking(
        id=booking.id,
        room_id=booking.room_id,
        start_time=new_start_time,
        end_time=booking.start_time - 1,
        name=booking.name,
        user_id=booking.user_id,
        user_name=booking.user_name,
    )

    overlapping_booking = await _check_for_overlapping_booking(check_booking)
    if overlapping_booking is not None:
        error_message = f"Prepending the booking would overlap with another booking called '{overlapping_booking.name}' by {overlapping_booking.user_name}!"
        raise OverlappingBookingError(error_message)

    logger.info(
        "Prepending booking %s by %s in the database by %s seconds.",
        booking.name,
        booking.user_name,
        prepended_time_in_seconds,
    )

    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        "UPDATE bookings SET start_time = ? WHERE id = ?",
        (new_start_time, booking.id),
    )
    await database.commit()
    await cursor.close()


async def get_booking_by_id(booking_id: int) -> Booking | None:
    """Gets a booking by its ID.

    Args:
        booking_id: The ID of the booking to get.

    Returns:
        The booking, None if it doesn't exist.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("SELECT * FROM bookings WHERE id = ?", (booking_id,))
    fetched_booking = await cursor.fetchone()
    await cursor.close()

    return await _get_booking_from_cursor_response(fetched_booking)


async def get_all_current_and_upcoming_bookings() -> list[Booking]:
    """Gets all current and upcoming bookings. Used for admins.

    Returns:
        A list of bookings.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        "SELECT * FROM bookings WHERE start_time >= strftime('%s', 'now')"
    )
    fetched_bookings = await cursor.fetchall()
    await cursor.close()

    return [
        await _get_booking_from_cursor_response(booking) for booking in fetched_bookings
    ]


async def get_current_and_upcoming_bookings_for_user_id(
    user_id: str, fetch_recently_ended: bool = False
) -> list[Booking]:
    """Gets the current and upcoming bookings for a user.

    Args:
        user_id: The user ID to get the bookings for.
        fetch_recently_ended: Whether to also fetch bookings that have recently ended in the last hour.

    Returns:
        A list of bookings.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()

    if fetch_recently_ended:
        await cursor.execute(
            """
            SELECT * FROM bookings WHERE user_id = ? AND (end_time > strftime('%s', 'now') OR end_time > strftime('%s', 'now', '-1 hour')) ORDER BY start_time ASC
            """,
            (user_id,),
        )
    else:
        await cursor.execute(
            """
            SELECT * FROM bookings WHERE user_id = ? AND end_time > strftime('%s', 'now') ORDER BY start_time ASC
            """,
            (user_id,),
        )

    fetched_bookings = await cursor.fetchall()
    await cursor.close()

    return [
        await _get_booking_from_cursor_response(booking) for booking in fetched_bookings
    ]


async def create_room(room: Room) -> None:
    """Creates a room in the database.

    Args:
        room: The room to create.

    Raises:
        RoomNameTakenError: If the room name is already taken.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("SELECT * FROM rooms WHERE name = ?", (room.name,))
    fetched_room = await cursor.fetchone()

    if fetched_room is not None:
        error_message = f"Can't create room: Room name '{room.name}' is already taken!"
        logger.info(
            "User tried requesting room name %s, but it is already taken.", room.name
        )
        raise RoomNameTakenError(error_message)

    logger.info("Adding room %s to the database.", room.name)
    await cursor.execute("INSERT INTO rooms (name) VALUES (?)", (room.name,))
    await database.commit()
    await cursor.close()


async def delete_room(room_id: int) -> None:
    """Deletes a room (and its bookings) from the database.

    Args:
        room_id: The id of the room to delete.
    """
    room = await get_room_by_id(room_id)
    logger.info("Deleting room %s from the database.", room.name)

    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("DELETE FROM rooms WHERE id = ?", (room.id,))
    await cursor.execute("DELETE FROM bookings WHERE room_id = ?", (room.id,))
    await database.commit()
    await cursor.close()


async def get_room_by_id(room_id: int) -> Room | None:
    """Gets a room by its ID.

    Args:
        room_id: The ID of the room to get.

    Returns:
        The room, None if it doesn't exist.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("SELECT * FROM rooms WHERE id = ?", (room_id,))
    fetched_room = await cursor.fetchone()
    await cursor.close()

    return await _get_room_from_cursor_response(fetched_room)


async def get_all_rooms() -> list[Room]:
    """Gets all rooms.

    Returns:
        A list of rooms.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute("SELECT * FROM rooms")
    fetched_rooms = await cursor.fetchall()
    await cursor.close()

    return [await _get_room_from_cursor_response(room) for room in fetched_rooms]


async def get_current_booking_for_room(room: Room) -> Booking | None:
    """Gets the current booking for a room.

    Args:
        Room: The room to get the booking for.

    Returns:
        The current booking or None if there is none.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        """
        SELECT * FROM bookings
        WHERE room_id = ? AND start_time <= strftime('%s', 'now') AND end_time >= strftime('%s', 'now')
        """,
        (room.id,),
    )
    fetched_booking = await cursor.fetchone()
    await cursor.close()

    return await _get_booking_from_cursor_response(fetched_booking)


async def get_upcoming_bookings_for_room(
    room: Room,
    search_end_time: int,
    amount_of_upcoming_bookings: int,
) -> list[Booking]:
    """Gets the upcoming bookings for a room. Will always return the amount of bookings specified.
    If there are less bookings than the amount specified, the list will be padded with None.

    Args:
        Room: The room to get the bookings for.
        search_end_time: The end time to search to.
        amount_of_upcoming_bookings: The amount of upcoming bookings to get.

    Returns:
        A list of bookings.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        """
        SELECT * FROM bookings
        WHERE room_id = ? AND start_time >= strftime('%s', 'now') AND start_time <= ?
        ORDER BY start_time ASC
        LIMIT ?
        """,
        (room.id, search_end_time, amount_of_upcoming_bookings),
    )
    fetched_bookings = await cursor.fetchall()
    await cursor.close()

    bookings = [
        await _get_booking_from_cursor_response(booking) for booking in fetched_bookings
    ]
    bookings.extend([None] * (amount_of_upcoming_bookings - len(bookings)))

    return bookings


async def get_coming_week_bookings_for_room(room: Room) -> list[Booking]:
    """Gets the bookings for a room from the start of the current day to one week after the start of today.

    Args:
        Room: The room to get the bookings for.

    Returns:
        A list of bookings.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        """
        SELECT * FROM bookings
        WHERE room_id = ? AND start_time >= strftime('%s', 'now', 'start of day')
        AND start_time < strftime('%s', 'now', 'start of day', '+7 days')
        ORDER BY start_time ASC
        """,
        (room.id,),
    )
    fetched_bookings = await cursor.fetchall()
    await cursor.close()

    return [
        await _get_booking_from_cursor_response(booking) for booking in fetched_bookings
    ]


async def _check_for_overlapping_booking(new_booking: Booking) -> Booking | None:
    """Gets all bookings that overlap with the given booking.

    Args:
        booking: The booking to check for overlaps.

    Returns:
        The overlapping booking or None if there is none.
    """
    database = await _get_database_connection()
    cursor = await database.cursor()
    await cursor.execute(
        """
        SELECT * FROM bookings
        WHERE room_id = ? AND NOT (start_time >= ? OR end_time <= ?)
        """,
        (new_booking.room_id, new_booking.end_time, new_booking.start_time),
    )
    fetched_booking = await cursor.fetchone()

    if fetched_booking:
        return await _get_booking_from_cursor_response(fetched_booking)

    return None


async def _get_booking_from_cursor_response(cursor_response) -> Booking | None:
    """Gets a booking from a cursor response.

    Args:
        cursor_response: The cursor response to get the booking from.

    Returns:
        The booking, None if the cursor response is empty.
    """
    if cursor_response is None:
        return None

    return Booking(
        id=cursor_response[0],
        room_id=cursor_response[1],
        start_time=cursor_response[2],
        end_time=cursor_response[3],
        name=cursor_response[4],
        user_id=cursor_response[5],
        user_name=cursor_response[6],
    )


async def _get_room_from_cursor_response(cursor_response) -> Room | None:
    """Gets a room from a cursor response.

    Args:
        cursor_response: The cursor response to get the room from.

    Returns:
        The room, None if the cursor response is empty.
    """
    if cursor_response is None:
        return None

    return Room(id=cursor_response[0], name=cursor_response[1])
