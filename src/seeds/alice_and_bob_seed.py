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

    settings = {
        "Alice the Strong": {
            "start_distance": 35,
            "end_distance": 87,
            "start_difficulty": 6,
            "end_difficulty": 9,
        },
        "Bob the not as strong": {
            "start_distance": 15,
            "end_distance": 55,
            "start_difficulty": 2,
            "end_difficulty": 5,
        },
    }

    NUM_SESSIONS = 10
    NUM_DATAPOINTS = 200

    NOISE_DISTANCE = 10.0      # random variation in distance
    NOISE_DIFFICULTY = 0.7    # random variation in difficulty
    NOISE_TREND = 2.0         # random variation in session-to-session progress

    for username in users:
        conn.execute("INSERT OR IGNORE INTO users (username) VALUES (?)", (username,))
        user_id = conn.execute(
            "SELECT user_id FROM users WHERE username = ?", (username,)
        ).fetchone()["user_id"]
        user_ids.append((user_id, username))
        print(f"Bruger oprettet: {username} (id: {user_id})")

    for user_id, username in user_ids:
        cfg = settings[username]

        for s in range(NUM_SESSIONS):
            session_uuid = str(uuid.uuid4())

            start_time = datetime.now() - timedelta(days=NUM_SESSIONS - s)
            start_time += timedelta(minutes=random.randint(0, 120))
            end_time = start_time + timedelta(minutes=random.randint(10, 35))

            conn.execute(
                "INSERT INTO sessions (session_uuid, user_id, start_time, end_time) VALUES (?, ?, ?, ?)",
                (
                    session_uuid,
                    user_id,
                    start_time.strftime("%Y-%m-%d %H:%M:%S"),
                    end_time.strftime("%Y-%m-%d %H:%M:%S"),
                ),
            )

            print(f"  Session oprettet: {session_uuid}")

            session_progress = s / max(NUM_SESSIONS - 1, 1)

            session_target_distance = (
                cfg["start_distance"]
                + (cfg["end_distance"] - cfg["start_distance"]) * session_progress
                + random.uniform(-NOISE_TREND, NOISE_TREND)
            )

            session_base_difficulty = (
                cfg["start_difficulty"]
                + (cfg["end_difficulty"] - cfg["start_difficulty"]) * session_progress
            )

            max_distance = 0
            difficulties = []

            for i in range(NUM_DATAPOINTS):
                point_progress = i / max(NUM_DATAPOINTS - 1, 1)

                # Distance improves within each session
                warmup_factor = 0.75 + 0.25 * point_progress

                distance = (
                    session_target_distance * warmup_factor
                    + random.uniform(-NOISE_DISTANCE, NOISE_DISTANCE)
                )

                distance = max(0, round(distance, 1))
                max_distance = max(max_distance, distance)

                difficulty = (
                    session_base_difficulty
                    + 0.7 * point_progress
                    + random.uniform(-NOISE_DIFFICULTY, NOISE_DIFFICULTY)
                )

                difficulty = max(1, min(10, round(difficulty)))
                difficulties.append(difficulty)

                timestamp = (start_time + timedelta(seconds=i * 10)).strftime(
                    "%Y-%m-%d %H:%M:%S"
                )

                conn.execute(
                    """
                    INSERT INTO session_data
                    (session_uuid, distance, difficulty, timestamp)
                    VALUES (?, ?, ?, ?)
                    """,
                    (session_uuid, distance, difficulty, timestamp),
                )

            average_difficulty = round(sum(difficulties) / len(difficulties), 1)

            conn.execute(
                """
                UPDATE sessions
                SET max_distance = ?, average_difficulty = ?
                WHERE session_uuid = ?
                """,
                (max_distance, average_difficulty, session_uuid),
            )

            print(
                f"    {NUM_DATAPOINTS} datapunkter indsat, "
                f"max_distance: {max_distance}, "
                f"avg_difficulty: {average_difficulty}"
            )

    conn.commit()
    conn.close()
    print("\nDatabase seeded!")

if __name__ == "__main__":
    seed()
