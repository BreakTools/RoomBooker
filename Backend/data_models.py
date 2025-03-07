"""Data models for the Room Booker backend.

Written by Mervin van Brakel, 2025."""

from dataclasses import dataclass


@dataclass
class Booking:
    """Booking object used across the backend."""

    id: int
    room_id: int
    start_time: int
    end_time: int
    name: str
    user_id: str
    user_name: str


@dataclass
class Room:
    """Room object used across the backend."""

    id: int
    name: str


@dataclass
class BookingDay:
    """BookingDay object used across the backend."""

    date_text: str
    bookings: list[Booking]


class OverlappingBookingError(Exception):
    """Exception for when a booking overlaps with another booking."""


class IncorrectTimeError(Exception):
    """Exception for when a booking has incorrect timing."""


class RoomNameTakenError(Exception):
    """Exception for when a room name is already taken."""
