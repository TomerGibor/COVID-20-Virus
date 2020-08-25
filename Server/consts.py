from dataclasses import dataclass, field
from typing import List
from enum import Enum


class MIMETypesExtensions(str, Enum):
    PNG = 'image/png', 'png'
    BMP = 'image/bmp', 'bmp'
    TXT = 'text/plain', 'txt'

    def __new__(cls, mime_type, file_extension):
        obj = str.__new__(cls)
        obj._value_ = mime_type
        cls._value2member_map_[file_extension] = obj
        obj.file_extension = file_extension
        return obj

    @classmethod
    def has_value(cls, value):
        return value in cls._value2member_map_


class DBIdentifiers(str, Enum):
    SCREENSHOT = 'screenshots_taken'
    WEBCAM = 'webcam_shots_taken'
    KEYLOG = 'keylogs_received'

    @classmethod
    def has_value(cls, value):
        return value in cls._value2member_map_


class HTTPHeaders(str, Enum):
    DATETIME = 'Date'
    MAC = 'Mac-Address'
    CONTENT_TYPE = 'Content-Type'


@dataclass
class Client:
    commands: List[str] = field(default_factory=list)
    last_datetime: str = None
    screenshots_taken: int = 0
    webcam_shots_taken: int = 0
    keylogs_received: int = 0

