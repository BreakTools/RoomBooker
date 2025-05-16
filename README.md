# Room Booker
A simple room booking solution using a Slack bot and a small display! Here's a GIF that shows you everything you need to know:
<p align="center">
  <img src="https://github.com/user-attachments/assets/365dddd8-1cd7-4efb-ba06-0cf1d86ae926" />
</p>


# Why use Room Booker?
Does the following situation sound familiar to you?
- Your company uses Slack for their internal communication.
- You have one or more meeting rooms that are not well managed.
- A system like Joan or Logitech Tap is far too expensive.
- You like hosting things locally and being in control of your data.

Then Room Booker is the perfect free and open source solution for you!

# Fully customizable!
You can customize the look of Room Booker to match the style of your company. Every font, color and image can be individually changed.
<p align="center">
  <img src="https://github.com/user-attachments/assets/cc791ef2-5176-4e25-8605-9546fb0ecc56" />
</p>


# User friendly!
It's fast and easy to use on both desktop and mobile platforms. Here's an example booking through the Slack app:
<p align="center">
  <img src="https://github.com/user-attachments/assets/48ae8b05-ffed-468f-a930-84294908dc7d" />
</p>



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
- Go to the Basic Information tab, find App Level Tokens and click on Create Token and Scopes. Give your token a name and add the `connections:write` scope. You'll now see the App Level token, which you'll need later.

Now you need to run the Docker image.
- Pull the backend Docker image by running `docker pull ghcr.io/breaktools/room-booker-backend:{version}`, where {version} should be replaced by the version you're using (e.g. 1.0.1).
- Run the backend by running `docker run -e SLACK_BOT_TOKEN="{bot_token}" -e SLACK_APP_TOKEN="{app_token}" -e DISPLAY_API_PORT="{api_port}" -t  ghcr.io/breaktools/room-booker-backend:{version}`, where {bot_token}, {app_token} and {version} should be replaced by the values you've gotten from previous steps and {api_port} should be replaced by the port you wish to run the backend on.

Your backend is now running! You can try it by opening Slack, clicking on the Room Booker bot and adding a room.

## Running the display software
Running the display software is simple!
- Download the display software from the releases tab here on this GitHub page. Download the .AppImage file for Linux machines and the .exe file for Windows machines (the .exe file is an installer which will install the software for you). 
- Open the display software and click on the BreakTools logo (or use Alt+S), this will open the settings menu.
- Fill in the API address of your backend, it should look something like `http://145.90.27.19:5014`, and click on the restart program button.
That's it! There's a large amount of other settings available in that menu so be sure to tweak them all to your liking and make them match your organization's branding!
