from typing import Union

from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive
from pydrive.files import GoogleDriveFile

g_login = GoogleAuth()
g_login.LocalWebserverAuth()  # Creates local webserver and auto handles authentication
drive = GoogleDrive(g_login)  # Make GoogleDrive instance with authenticated GoogleAuth instance
BASE_FOLDER_NAME = 'Clients'
base_folder = None


def setup_driveapi() -> None:
    """ Creates the base folder if it doesn't already exist.
    MUST call this function before any other functions from this module.
    """
    global base_folder
    if (folder := _get_folder_by_title(BASE_FOLDER_NAME)) is not None:
        # folder already exists
        base_folder = folder
    else:
        # folder needs to be created
        base_folder = _create_base_folder()


def create_folder(title: str) -> None:
    """
    Create folder inside base folder.
    Args:
        title (str): The title of the folder that will be created.
    """
    folder = drive.CreateFile({'title': title,
                               'mimeType': 'application/vnd.google-apps.folder',
                               'parents': [base_folder]})
    folder.Upload()


def upload_file_from_existing_file(title: str, src_filename: str, mime_type: str,
                                   parent_folder_title: str) -> None:
    """
    Creates file in parent folder (if exists such folder) and copies
      the content of the file from an existing file.
    Args:
        title (str): The title of the file to be created.
        src_filename (str): The title of the source file.
        mime_type (str): The MIME type of the file.
        parent_folder_title (str): The parent folder of the file in
          which the file will be saved.
    """
    folder = _get_folder_by_title(parent_folder_title)
    if folder is not None:
        file = drive.CreateFile({'title': title,
                                 'parents': [folder],
                                 'mimeType': mime_type})
        file.SetContentFile(src_filename)
        file.Upload()


def _get_folder_by_title(title: str) -> Union[GoogleDriveFile, None]:
    folders = drive.ListFile(
        {
            'q': f"title='{title}'"
                 f" and mimeType='application/vnd.google-apps.folder'"
                 f" and trashed=false"
        }
    ).GetList()
    if not folders:
        return None
    return folders[0]  # Return first folder in folders list


def _create_base_folder() -> GoogleDriveFile:
    folder = drive.CreateFile({'title': BASE_FOLDER_NAME,
                               'mimeType': 'application/vnd.google-apps.folder'})
    folder.Upload()
    return folder


