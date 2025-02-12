"""REST API using FastAPI and uvicorn that the displays will fetch their information from.

Written by Mervin van Brakel, 2025."""

from __future__ import annotations

import logging
import os

import pytz
import uvicorn
from dotenv import load_dotenv
from fastapi import FastAPI, HTTPException

import database_interfacing
import utility
from data_models import Booking

load_dotenv()

logger = logging.getLogger("display_api")

app = FastAPI()

if not os.getenv("DISPLAY_API_PORT"):
    error_message = "DISPLAY_API_PORT environment variable not set!"
    raise ValueError(error_message)

if not os.getenv("DISPLAY_API_PORT").isdigit():
    error_message = "DISPLAY_API_PORT should be a port number!"
    raise ValueError(error_message)


@app.get("/rooms/{room_id}/{timezone_id}")
async def get_room_display_data(room_id: int, timezone_id: str):
    """Gets the display data for a room. Was having troubles with the "/" character,
    apparently URL-encoding doesn't work nicely here because Uvicorn decodes it before
    it gets to FastAPI or something.

    Args:
        room_id: The ID of the room to get the display data for.
        timezone_id: The IANA ID of the display, where the / is an & instead lol.
    """
    room = await database_interfacing.get_room_by_id(room_id)
    if not room:
        raise HTTPException(status_code=404, detail="ERROR: Room ID not found.")

    timezone = timezone_id.replace("&", "/")
    if timezone not in pytz.all_timezones:
        raise HTTPException(
            status_code=400, detail="ERROR: Invalid system timezone ID."
        )

    logger.debug("Getting display data for room %s in timezone %s.", room_id, timezone)

    day_end_time = await utility.get_day_end_time_from_timezone(pytz.timezone(timezone))
    current_booking = await database_interfacing.get_current_booking_for_room(room)
    upcoming_bookings = await database_interfacing.get_upcoming_bookings_for_room(
        room, day_end_time, 3
    )

    return {
        "current_booking": await booking_to_sendable_dict(current_booking),
        "first_upcoming_booking": await booking_to_sendable_dict(upcoming_bookings[0]),
        "second_upcoming_booking": await booking_to_sendable_dict(upcoming_bookings[1]),
        "third_upcoming_booking": await booking_to_sendable_dict(upcoming_bookings[2]),
    }


async def booking_to_sendable_dict(booking: Booking) -> dict:
    """Constructs and returns the booking dict for display.

    Args:
        booking: The booking to construct the dict for.

    Returns:
        The constructed booking dict. Empty when no booking was provided.
    """
    if booking is None:
        return {
            "id": "",
            "name": "",
            "user": "",
            "start_time": 0,
            "end_time": 0,
        }

    return {
        "id": str(booking.id),
        "name": booking.name,
        "user": booking.user_name,
        "start_time": booking.start_time,
        "end_time": booking.end_time,
    }


async def start_display_api():
    """Starts the display API in an asyncio compatible way."""
    config = uvicorn.Config(
        "display_api:app",
        host="0.0.0.0",
        port=int(os.getenv("DISPLAY_API_PORT")),
        log_level="critical",
    )
    server = uvicorn.Server(config)
    await server.serve()
