import sqlite3
import uuid
import random
from datetime import datetime, timedelta
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(BASE_DIR, "..", "training.db")

def get_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def seed():
    conn = get_connection()

    users = ["Alice the Strong", "Bob the not as strong"]
    user_ids = []

    # Difficulty-ranges per bruger: (min_start, max_start, min_end, max_end)
    difficulty_ranges = {
        "Alice the Strong":       (6, 7, 9, 10),
        "Bob the not as strong":  (1, 2, 3, 4),
    }

    for username in users:
        conn.execute("INSERT OR IGNORE INTO users (username) VALUES (?)", (username,))
        user_id = conn.execute(
            "SELECT user_id FROM users WHERE username = ?", (username,)
        ).fetchone()["user_id"]
        user_ids.append((user_id, username))
        print(f"Bruger oprettet: {username} (id: {user_id})")

    NUM_SESSIONS = 10  # Antal sessioner per bruger

    for user_id, username in user_ids:
        d_min_start, d_max_start, d_min_end, d_max_end = difficulty_ranges[username]

        for s in range(NUM_SESSIONS):
            session_uuid = str(uuid.uuid4())
            start_time = datetime.now() - timedelta(hours=random.randint(1, 48))
            end_time = start_time + timedelta(minutes=random.randint(10, 60))

            conn.execute(
                "INSERT INTO sessions (session_uuid, user_id, start_time, end_time) VALUES (?, ?, ?, ?)",
                (session_uuid, user_id, start_time.strftime("%Y-%m-%d %H:%M:%S"), end_time.strftime("%Y-%m-%d %H:%M:%S"))
            )
            print(f"  Session oprettet: {session_uuid}")

            # Fremskridt på tværs af sessioner: 0.0 (første) → 1.0 (sidste)
            progress = s / max(NUM_SESSIONS - 1, 1)

            # Basis-difficulty for denne session interpoleret mellem start og slut
            session_base = d_min_start + (d_min_end - d_min_start) * progress

            max_distance = 0
            NUM_DATAPOINTS = 10

            for i in range(NUM_DATAPOINTS):
                distance = round(random.uniform(10, 100), 1)
                max_distance = max(max_distance, distance)

                # Lille variation inden for sessionen (+/- 0.5), clampet til brugerens range
                difficulty = round(
                    max(d_min_start, min(d_max_end, session_base + random.uniform(-0.5, 0.5))),
                    1
                )

                timestamp = (start_time + timedelta(seconds=i * 10)).strftime("%Y-%m-%d %H:%M:%S")
                conn.execute(
                    "INSERT INTO session_data (session_uuid, distance, difficulty, timestamp) VALUES (?, ?, ?, ?)",
                    (session_uuid, distance, difficulty, timestamp)
                )

            conn.execute(
                "UPDATE sessions SET max_distance = ? WHERE session_uuid = ?",
                (max_distance, session_uuid)
            )
            print(f"    {NUM_DATAPOINTS} datapunkter indsat, max_distance: {max_distance}")

    conn.commit()
    conn.close()
    print("\nDatabase seeded!")

if __name__ == "__main__":
    seed()