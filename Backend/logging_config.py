"""Logging configuration for the Room Booker backend.

Written by Mervin van Brakel, 2025.
"""

import logging
import logging.config

LOGGING_CONFIG = {
    "version": 1,
    "disable_existing_loggers": False,
    "formatters": {
        "standard": {"format": "%(asctime)s [%(levelname)s] %(name)s: %(message)s"},
    },
    "handlers": {
        "console": {
            "level": "DEBUG",
            "class": "logging.StreamHandler",
            "formatter": "standard",
        },
        "file": {
            "level": "INFO",
            "class": "logging.FileHandler",
            "filename": "room_booker.log",
            "formatter": "standard",
        },
    },
    "loggers": {
        "": {"handlers": ["console", "file"], "level": "INFO", "propagate": True},
        "database_interface": {
            "handlers": ["console", "file"],
            "level": "INFO",
            "propagate": False,
        },
        "slack_bot": {
            "handlers": ["console", "file"],
            "level": "INFO",
            "propagate": False,
        },
        "display_api": {
            "handlers": ["console", "file"],
            "level": "INFO",
            "propagate": False,
        },
    },
}


def setup_logging():
    logging.config.dictConfig(LOGGING_CONFIG)
