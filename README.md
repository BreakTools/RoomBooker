# Room Booker
A simple room booking solution using a Slack bot and a small display! Here's a 3 second GIF that shows you everything you need to know:

(GIF will go here)

# Why use Room Booker?
Does the following situation sound familiar to you?
- Your company uses Slack for their internal communication.
- You have one or more meeting rooms that are not well managed.
- A system like Joan or Logitech Tap is far too expensive.
- You like hosting things locally and being in control of your data.

Then Room Booker is the perfect free and open source solution for you!

# Requirements for running Room Booker
To start using Room Booker you'll need the following things:
- A Slack workspace.
- A place on your local network to run a Docker container.
- A small display that you can mount next to a room (those 7-inch 1024x600 ones work well).
- A computer running Linux (preferred) or Windows that you can permanently hook up to the small display. It doesn't need much, so something like a small NUC works. I'm currently working on adding Raspberry Pi support so stay tuned for that!


# Installation instructions
## Running the backend
First you'll need to create a Slack bot and add it to your organization.
- Go to https://api.slack.com/apps, log into your organization and create a bot using the following manifest:
```json
{
    "display_information": {
        "name": "Room Booker",
        "description": "Book a room using Room Booker!",
        "background_color": "#FFFFFF"
    },
    "features": {
        "app_home": {
            "home_tab_enabled": true,
            "messages_tab_enabled": false,
            "messages_tab_read_only_enabled": true
        },
        "bot_user": {
            "display_name": "Room Booker",
            "always_online": false
        }
    },
    "oauth_config": {
        "scopes": {
            "bot": [
                "users:read"
            ]
        }
    },
    "settings": {
        "event_subscriptions": {
            "bot_events": [
                "app_home_opened"
            ]
        },
        "interactivity": {
            "is_enabled": true
        },
        "org_deploy_enabled": false,
        "socket_mode_enabled": true,
        "token_rotation_enabled": false
    }
}
```
- Go to the Install App tab and install the app to your organization. On this page you'll also find the Bot User OAuth Token which you'll need later.
- Go to the Basic Information tab, fidn App Level Tokens and click on Create Token and Scopes. Give your token a name and add the `connections:write` scope. You'll now see the App Level token, which you'll need later.
- Pull the backend Docker image by running `docker pull ghcr.io/breaktools/room-booker-backend:{version}`, where {version} should be replaced by the version you're using (e.g. 1.0.1).
- Run the backend by running `docker run -e SLACK_BOT_TOKEN="{bot_token}" -e SLACK_APP_TOKEN="{app_token}" -e DISPLAY_API_PORT="{api_port}" -t  ghcr.io/breaktools/room-booker-backend:VERSION`, where {bot_token}, {app_token} and {api_port} should be replaced by the values you've gotten from previous steps and the port you wish to run the backend on.

Your backend is now running! You can try it by opening Slack, clicking on the Room Booker bot and adding a room.

