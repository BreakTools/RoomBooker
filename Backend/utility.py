"""Utility functions for the Room Booker backend.

Written by Mervin van Brakel, 2025."""

from __future__ import annotations

from datetime import datetime

import pytz  # noqa: TC002

from data_models import Booking, BookingDay


async def divide_bookings_into_days_by_timezone(
    bookings: list[Booking], user_timezone: pytz.tzinfo
) -> list[BookingDay]:
    """Divides a list of bookings into a list of BookingDay objects, based on the user's timezone.

    Args:
        bookings: The list of bookings to divide.
        user_timezone: The user's timezone.

    Returns:
        A list of BookingDay objects, where each object represents a day with its bookings.
    """
    bookings_by_day = {}
    for booking in bookings:
        booking_date_text = await get_date_string_from_timestamp(
            booking.start_time, user_timezone
        )

        if booking_date_text not in bookings_by_day:
            bookings_by_day[booking_date_text] = []

        bookings_by_day[booking_date_text].append(booking)

    return [
        BookingDay(date_text, bookings)
        for date_text, bookings in bookings_by_day.items()
    ]


async def get_time_string_from_timestamp(
    timestamp: int, user_timezone: pytz.tzinfo
) -> str:
    """Gets a time string from a timestamp, based on the user's timezone.
    I use a sneaky little +1 to fix the -1 used in the database which prevents overlapping.

    Args:
        timestamp: The timestamp to get the time string from.
        user_timezone: The user's timezone.

    Returns:
        The time string.
    """
    return datetime.fromtimestamp(timestamp + 1, user_timezone).strftime("%H:%M")


async def get_date_string_from_timestamp(
    timestamp: int, user_timezone: pytz.tzinfo
) -> str:
    """Gets a date string from a timestamp, based on the user's timezone.

    Args:
        timestamp: The timestamp to get the date string from.
        user_timezone: The user's timezone.

    Returns:
        The date string.
    """
    return datetime.fromtimestamp(timestamp, user_timezone).strftime("%d-%m-%Y")


async def get_day_end_time_from_timezone(
    user_timezone: pytz.tzinfo,
) -> int:
    """Gets the end time (23:59) of the day, based on the user's timezone.

    Args:
        user_timezone: The user's timezone.

    Returns:
        The end time of the day in timestamp format.
    """
    today = datetime.now(user_timezone)
    return int(today.replace(hour=23, minute=59, second=59).timestamp())


async def get_new_unix_start_end_time_from_booking_and_time_strings(
    booking: Booking, start_time: str, end_time: str, user_timezone: pytz.tzinfo
) -> tuple[int, int]:
    """Gets the start and end time of a booking in Unix timestamp format. Bit of a wonky function
    but Slack's time pickers only return data in HH:MM format. We do have the booking data so from
    that we can determine the date of the booking and then combine that with the time strings to
    create a Unix timestamp.

    Args:
        booking: The booking to get the start and end time for.
        start_time: The new start time of the booking.
        end_time: The new end time of the booking.
        user_timezone: The user's timezone.

    Returns:
        The start and end time of the booking in Unix timestamp format.
    """
    booking_date = datetime.fromtimestamp(booking.start_time, user_timezone).strftime(
        "%d-%m-%Y"
    )

    new_start_datetime = datetime.strptime(
        f"{booking_date} {start_time}", "%d-%m-%Y %H:%M"
    )
    new_end_datetime = datetime.strptime(f"{booking_date} {end_time}", "%d-%m-%Y %H:%M")

    start_time = int(user_timezone.localize(new_start_datetime).timestamp())
    end_time = int(user_timezone.localize(new_end_datetime).timestamp())

    return start_time, end_time
