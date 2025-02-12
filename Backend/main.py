"""Main file that should be ran to start the Room Booker backend.

Written by Mervin van Brakel, 2025."""

import asyncio
import logging

import database_interfacing
from display_api import start_display_api
from logging_config import setup_logging
from slack_bot import start_slack_bot

setup_logging()

logger = logging.getLogger(__name__)


async def main():
    """Starts the Room Booker backend."""
    logger.info("Starting Room Booker backend!")
    await database_interfacing.configure_database()
    await asyncio.gather(start_display_api(), start_slack_bot())


if __name__ == "__main__":
    asyncio.run(main())
