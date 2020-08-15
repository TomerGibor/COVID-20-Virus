A virus for Windows written in C with a server that manages the clients.

A shortcut to the executable file is intended to be install on a target computer in folder "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp" for it to be ran automatically on startup (inside the folder to which the shortcut leads, the escapi.dll file should be places as well). The malware includes options to take screenshots and webcam shots (if a webcam is present and not in use), log keyboard, and execute commands in cmd. The clients are controlled by a server that serves them commands upon request (by default - one minute delay between requests) and saves the pictures and logfiles on Google Drive.

The server and client communicate via HTTP requests and responses.

Possible commands are:
start_keylogging - starts to log keyboard and saves it to a file locally.
send_keylogging - sends the logged data to the server which saves it to Google Drive.
stop_keylogging - stops to log keyboard.
take_webcam_capture - if webcam is present and not in use, takes picture from webcam and sends it to the server which saves it to Google Drive. Otherwise, does nothing.
take_screenshot - takes screenshot and sends it to the server which saves it to Google Drive.
execute `command` - executes `command` in cmd.
stop_execution - stops the program execution.
no_commands - does nothing (used in case the server wants to tell a client that it has no commands for the moment).

For the data received from clients to be saved in Google Drive, one would have to enable the Drive API and fill in the credentials in file credentials.json (`Enable the Drive API` button in: https://developers.google.com/drive/api/v3/quickstart/python#step_1_turn_on_the , choose project name, desktop app, download credentials).
One might choose to erase file credentials.json, and then a link would be opened in one's browser in which one would need to sign into their gmail account.
In both cases, one would have to fill in the client_id and client_secret in file settings.yaml.

Commands can be added to a client by surfing to / (on server), in which a form will be presented containing all mac-addresses (clients are identified by their mac-address) and a box with space to enter one or more on the commands mentioned above.

Users are encouraged to upload the server to a hosting website (such as heroku). All the file required by heroku are already supplied in this repository(such as requirements.txt, Procfile and runtime.txt).

Please be responsible with using this code. Tomer Gibor is not liable by any means to any damaged caused by anyone who used this code.
