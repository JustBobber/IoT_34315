import sqlite3
import uuid
import random
from datetime import datetime, timedelta

DB_PATH = "../training.db"

def get_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def seed():
    conn = get_connection()

    # Opret 2 brugere
    users = ["Alice", "Bob"]
    user_ids = []
    for username in users:
        cursor = conn.execute(
            "INSERT OR IGNORE INTO users (username) VALUES (?)", (username,)
        )
        conn.execute("SELECT user_id FROM users WHERE username = ?", (username,))
        user_id = conn.execute(
            "SELECT user_id FROM users WHERE username = ?", (username,)
        ).fetchone()["user_id"]
        user_ids.append(user_id)
        print(f"Bruger oprettet: {username} (id: {user_id})")

    for user_id in user_ids:
        for s in range(2):  # 2 sessioner per bruger
            session_uuid = str(uuid.uuid4())
            start_time = datetime.now() - timedelta(hours=random.randint(1, 48))
            end_time = start_time + timedelta(minutes=random.randint(10, 60))

            conn.execute(
                "INSERT INTO sessions (session_uuid, user_id, start_time, end_time) VALUES (?, ?, ?, ?)",
                (session_uuid, user_id, start_time.strftime("%Y-%m-%d %H:%M:%S"), end_time.strftime("%Y-%m-%d %H:%M:%S"))
            )
            print(f"  Session oprettet: {session_uuid}")

            max_distance = 0
            for i in range(5):  # 5 datapunkter per session
                distance = round(random.uniform(10, 100), 1)
                max_distance = max(max_distance, distance)
                timestamp = (start_time + timedelta(seconds=i * 10)).strftime("%Y-%m-%d %H:%M:%S")
                conn.execute(
                    "INSERT INTO session_data (session_uuid, distance, timestamp) VALUES (?, ?, ?)",
                    (session_uuid, distance, timestamp)
                )

            conn.execute(
                "UPDATE sessions SET max_distance = ? WHERE session_uuid = ?",
                (max_distance, session_uuid)
            )
            print(f"    5 datapunkter indsat, max_distance: {max_distance}")

    conn.commit()
    conn.close()
    print("\nDatabase seeded!")

if __name__ == "__main__":
    seed()