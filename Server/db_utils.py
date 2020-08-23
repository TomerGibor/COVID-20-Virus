import json
from typing import Dict, List, Union, Any
from dataclasses import asdict
from functools import wraps

from consts import Client


def singleton(cls):
    """ An implementation of singleton using decorator. """
    instances = {}

    @wraps(cls)
    def wrapper(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]

    return wrapper


class ClientEncoder(json.JSONEncoder):
    """ Custom class to encode client in order to dump to json file. """

    def default(self, client: Client) -> Dict[str, Union[int, str, List[str]]]:
        """ Called in case json can't serialize object. """
        return asdict(client)


class ClientDecoder(json.JSONDecoder):
    def __init__(self):
        super().__init__(object_hook=self.dict_to_client)

    @staticmethod
    def dict_to_client(obj: Any) -> Union[Client, Any]:
        """ Called for every json object. """
        if isinstance(obj, dict) and 'commands' in obj:
            return Client(**obj)
        return obj


@singleton
class DBHandler:
    """ Class to handle DB related tasks. """

    DB_NAME = 'db.json'
    MAX_COMMANDS = 10

    def __init__(self):
        self._load_db()

    def _load_db(self) -> None:
        with open(self.DB_NAME, 'r') as file:
            self._db = json.load(file, cls=ClientDecoder)

    def update_db(self) -> None:
        with open(self.DB_NAME, 'w') as file:
            json.dump(self._db, file, cls=ClientEncoder)

    def initialize_new_client(self, mac_address: str, datetime: str):
        self._db[mac_address] = Client(last_datetime=datetime)
        self.update_db()

    def retrieve_commands_from_db(self, mac_address: str) -> List[str]:
        commands = self._db[mac_address].commands[:self.MAX_COMMANDS]
        # remove retrieved commands from db
        self._db[mac_address].commands = self._db[mac_address].commands[self.MAX_COMMANDS:]
        return commands

    def add_commands(self, mac_address: str, commands: List[str]) -> None:
        self._db[mac_address].commands += commands
        self.update_db()

    def serialize_db(self) -> Dict[str, Dict[str, Union[int, str, List[str]]]]:
        serialized_db = {}
        for mac_address, client in self._db.items():
            serialized_db[mac_address] = asdict(client)
        return serialized_db

    @property
    def db(self) -> Dict[str, Client]:
        return self._db
