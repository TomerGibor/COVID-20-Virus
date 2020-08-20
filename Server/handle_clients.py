from db_utils import DBHandler
from fastapi import Request
from driveapi import upload_file_from_existing_file, create_folder
from typing import List, Dict
from consts import HTTPHeaders, DBIdentifiers


NO_COMMANDS = ['no_commands']
TEMP_FILE = 'temp'
db_handler = DBHandler()


def handle_first_command(request: Request) -> Dict[str, List[str]]:
    mac = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    if db_handler.db.get(mac) is None:
        # new client
        create_new_client(mac, datetime)
        return {'commands': NO_COMMANDS}
    else:
        # existing client
        return get_commands(mac, datetime)


def handle_command_request(request: Request) -> Dict[str, List[str]]:
    mac = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    return get_commands(mac, datetime)


async def upload_file(request: Request, filename: str, db_identifier: DBIdentifiers,
                      file_type: str, remove_nulls: bool = False) -> None:
    if db_identifier not in DBIdentifiers.__members__.values():
        return  # illegal identifier
    mac = request.headers[HTTPHeaders.MAC]
    await _write_to_temp_file(request, remove_nulls)
    title = _build_title(mac, filename, file_type, db_identifier)
    _increase_identifier(mac, db_identifier)
    upload_file_from_existing_file(title=title,
                                   src_filename=TEMP_FILE,
                                   mime_type=request.headers[HTTPHeaders.CONTENT_TYPE],
                                   parent=mac)


def get_commands(mac: str, datetime: str) -> Dict[str, List[str]]:
    db_handler.db[mac].last_datetime = datetime
    db_handler.update_db()
    commands = db_handler.retrieve_commands_from_db(mac) or NO_COMMANDS
    return {'commands': commands}


def create_new_client(mac: str, datetime: str) -> None:
    db_handler.initialize_new_client(mac, datetime)
    create_folder(mac)  # create new Google Drive folder for the new client


async def _write_to_temp_file(request: Request, remove_nulls: bool) -> None:
    data = await request.body()
    if remove_nulls:
        data = data.replace(b'\x00', b'')
    with open(TEMP_FILE, 'wb') as f:
        f.write(data)


def _build_title(mac: str, filename: str, file_type: str, db_identifier: DBIdentifiers) -> str:
    return f'{filename}{getattr(db_handler.db[mac], db_identifier)}.{file_type}'


def _increase_identifier(mac: str, db_identifier: DBIdentifiers) -> None:
    setattr(db_handler.db[mac], db_identifier,
            getattr(db_handler.db[mac], db_identifier) + 1)
    db_handler.update_db()


