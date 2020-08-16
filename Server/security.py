import secrets
import hashlib
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBasic, HTTPBasicCredentials

security = HTTPBasic()
# admin (replace with your own username)
USERNAME = '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918'
# 123456 (replace with your own password)
PASSWD = '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92'


def get_current_username(credentials: HTTPBasicCredentials = Depends(security)) -> str:
    hashed_username = sha256_hash(credentials.username)
    hashed_passwd = sha256_hash(credentials.password)
    correct_username = secrets.compare_digest(hashed_username, USERNAME)
    correct_password = secrets.compare_digest(hashed_passwd, PASSWD)
    if not (correct_username and correct_password):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect username or password",
            headers={"WWW-Authenticate": "Basic"},
        )
    return credentials.username


def sha256_hash(string: str) -> str:
    return hashlib.sha256(string.encode('utf-8')).hexdigest()