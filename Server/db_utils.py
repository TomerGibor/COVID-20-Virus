import json

DB_NAME = 'db.json'


def load_db() -> dict:
    with open(DB_NAME, 'r') as file:
        return json.load(file)


def write_db(db: dict) -> None:
    with open(DB_NAME, 'w') as file:
        json.dump(db, file)