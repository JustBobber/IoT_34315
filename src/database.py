import sqlite3
import uuid
import os
from contextlib import contextmanager

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(BASE_DIR, ".", "training.db")


@contextmanager
def get_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    except Exception:
        conn.rollback()
        raise
    finally:
        conn.close()


def init_db():
    with get_connection() as conn:
        conn.executescript("""
            CREATE TABLE IF NOT EXISTS users (
                user_id     INTEGER PRIMARY KEY AUTOINCREMENT,
                username    TEXT UNIQUE NOT NULL,
                created_at  TEXT NOT NULL DEFAULT (datetime('now'))
            );

            CREATE TABLE IF NOT EXISTS sessions (
                session_uuid  TEXT PRIMARY KEY,
                start_time    TEXT NOT NULL DEFAULT (datetime('now')),
                end_time      TEXT,
                user_id       INTEGER REFERENCES users(id),
                max_distance  REAL
            );

            CREATE TABLE IF NOT EXISTS session_data (
                id            INTEGER PRIMARY KEY AUTOINCREMENT,
                session_uuid  TEXT REFERENCES sessions(session_uuid),
                timestamp     TEXT NOT NULL DEFAULT (datetime('now')),
                distance      REAL
            );
        """)


# --- Users ---

def create_user(username):
    # TODO: fix så appen ikke chrashser hvis vi adder en user med username der allerede findes.
    with get_connection() as conn:
        conn.execute("INSERT INTO users (username) VALUES (?)", (username,))
        return conn.execute("SELECT * FROM users WHERE username = ?", (username,)).fetchone()

def get_all_users():
    with get_connection() as conn:
        return conn.execute("SELECT * FROM users").fetchall()

def login_user(user_id):
    with get_connection() as conn:
        row = conn.execute("SELECT username FROM users where user_id = ?", (user_id, )).fetchone()
        if row:
            return row["username"]


# --- Sessions ---

def start_session(user_id):
    session_uuid = str(uuid.uuid4())
    with get_connection() as conn:
        conn.execute(
            "INSERT INTO sessions (session_uuid, user_id) VALUES (?, ?)",
            (session_uuid, user_id)
        )
    return session_uuid


def end_session(session_uuid):
    with get_connection() as conn:
        conn.execute(
            "UPDATE sessions SET end_time = datetime('now') WHERE session_uuid = ?",
            (session_uuid,)
        )


def get_session(session_uuid):
    with get_connection() as conn:
        return conn.execute(
            "SELECT * FROM sessions WHERE session_uuid = ?",
            (session_uuid,)
        ).fetchone()


def get_users_sessions(user_id):
    with get_connection() as conn:
        return conn.execute(
            "SELECT * FROM sessions WHERE user_id = ?",
            (user_id,)
        ).fetchall()


# --- Session data ---

def insert_session_data(session_uuid, distance, pressure):
    with get_connection() as conn:
        conn.execute(
            """INSERT INTO session_data (session_uuid, distance, pressure)
               VALUES (?, ?, ?)""",
            (session_uuid, distance, pressure)
        )
        conn.execute(
            """UPDATE sessions SET
                max_pressure = MAX(COALESCE(max_pressure, 0), ?),
                max_distance = MAX(COALESCE(max_distance, 0), ?)
               WHERE session_uuid = ?""",
            (pressure, distance, session_uuid)
        )


def get_session_data(session_uuid):
    with get_connection() as conn:
        return conn.execute(
            "SELECT * FROM session_data WHERE session_uuid = ? ORDER BY timestamp",
            (session_uuid,)
        ).fetchall()


if __name__ == "__main__":
    init_db()
    print("database initialized")
