import secrets
import hashlib
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBasic, HTTPBasicCredentials

security = HTTPBasic()
USERNAME = '1c74dc99fb9d3ad9c7daedb4f7f1f3e54c07f31d49e71acb9de681df1eff9455'
PASSWD = 'caa11353428317ddf54eccd2f209f1e71cb8b658e1026c2370aa6d5da4cc268f'


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