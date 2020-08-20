from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive

g_login = GoogleAuth()
g_login.LocalWebserverAuth()
drive = GoogleDrive(g_login)


def create_folder(title: str) -> None:
    folder = drive.CreateFile({'title': title, 'mimeType': 'application/vnd.google-apps.folder'})
    folder.Upload()


def upload_file_from_existing_file(title: str, src_filename: str, mime_type: str, parent: str) -> None:
    folders = drive.ListFile(
        {
            'q': f"title='{parent}' and mimeType='application/vnd.google-apps.folder' and trashed=false"}
    ).GetList()
    for folder in folders:
        if folder['title'] == parent:
            file = drive.CreateFile({'title': title,
                                     'parents': [{'id': folder['id']}],
                                     'mimeType': mime_type})
            file.SetContentFile(src_filename)
            file.Upload()
