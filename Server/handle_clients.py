from db_utils import DBHandler
from fastapi import Request
from driveapi import upload_file_from_file, create_folder
from typing import List, Dict
from consts import HTTPHeaders

NO_COMMANDS = ['no_commands']
TEMP_FILE = 'temp'
db_handler = DBHandler()


def handle_first_command(request: Request) -> Dict[str, List[str]]:
    mac = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    if db_handler.db.get(mac) is None:
        # new client
        db_handler.initialize_new_client(mac, datetime)
        create_folder(mac)  # create new Google Drive folder for the new client
        return {'commands': NO_COMMANDS}
    else:
        # existing client
        db_handler.db[mac].last_datetime = datetime
        db_handler.update_db()
        commands = db_handler.retrieve_commands_from_db(mac) or NO_COMMANDS
        return {'commands': commands}


def handle_command_request(request: Request) -> Dict[str, List[str]]:
    mac = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    db_handler.db[mac].last_datetime = datetime
    db_handler.update_db()
    commands = db_handler.retrieve_commands_from_db(mac) or NO_COMMANDS
    return {'commands': commands}


async def upload_file(request: Request, filename: str, db_identifier: str,
                      file_type: str, remove_nulls: bool = False) -> None:
    data = await request.body()
    if remove_nulls:
        data = data.replace(b'\x00', b'')
    mac = request.headers[HTTPHeaders.MAC]
    title = f'{filename}{getattr(db_handler.db[mac], db_identifier)}.{file_type}'
    with open(TEMP_FILE, 'wb') as f:
        f.write(data)
    setattr(db_handler.db[mac], db_identifier, getattr(db_handler.db[mac],
                                                       db_identifier) + 1)
    db_handler.update_db()
    upload_file_from_file(title=title,
                          src_filename=TEMP_FILE,
                          mime_type=request.headers[HTTPHeaders.CONTENT_TYPE],
                          parent=mac)
