from flask import Flask, render_template, redirect, url_for, request, session
from datetime import datetime
from database import init_db, create_user, start_session, end_session,   \
                     get_session, insert_session_data, get_session_data, \
					 get_all_users, login_user, get_users_sessions

app = Flask(__name__)

app.secret_key = "super hemmelig key"  # skal være der for at kunne køre user sessions
# i tilfælde af vi ikke vil have at user forbliver logget ind, så brug denne metode:
#		import os
#		app.secret_key = os.urandom(24)

@app.route("/")
def index():
	"""
	Index page
	:return: Hvis brugeren er logget ind, returneres index.html med username. Ellers returneres index.html uden bruger.
	"""
	user = session.get("username")
	user_id = session.get("user_id")
	return render_template("index.html", user=user, user_id=user_id)

# ===============================================================
#					Start of user endpoints
# ===============================================================

@app.route("/login")
def login():
	"""
	Login page, her kan logges ind i en eksisterende bruger eller oprettes en ny bruger.
		Der er ikke noget password, da det ikke umiddelbart er nødvendigt
	:return: login.html med alle brugere i databasen.
	"""
	users = get_all_users()
	return render_template("login.html", users=users)

@app.route("/login/create", methods=["POST"])
def login_create_user():
	"""
	Opretter user i databasen hvis der ikke allerede er en user med dette username.
	:return: Sender bruger tilbage til login siden, hvor den nye bruger nu er at finde i listen over brugere.
	"""
	username = request.form.get("username", "").strip()
	if username:
		create_user(username)
	return redirect(url_for("login"))

@app.route("/login/select/<int:user_id>")
def login_select(user_id):
	"""
	Login som en eksisterende user.
	:param user_id: User's id, som findes i databasen. Bruges til at logge ind som denne user.
	:return: Sender brugeren til index siden.
	"""
	username = login_user(user_id)
	if username:
		session["username"] = username
		session["user_id"] = user_id
	return redirect(url_for("index"))

@app.route("/logout")
def logout():
	"""
	Logger user ud ved at fjerne user fra sessionen.
	:return: Sender brugeren tilbage til index siden.
	"""
	session.pop("username", None)
	return redirect(url_for("index"))

# ===============================================================
#					End of user endpoints
# ===============================================================

# ===============================================================
#					Start of user sessions view
# ===============================================================


@app.route("/view_users_sessions/<int:user_id>")
def view_users_sessions(user_id):
	users_sessions = get_users_sessions(user_id)

	# beregner og tilføjer duration af session ud fra start og slut tid.
	sessions_with_duration = []
	for s in users_sessions:
		s = dict(s)
		if s["end_time"]:
			start = datetime.strptime(s["start_time"], "%Y-%m-%d %H:%M:%S")
			end = datetime.strptime(s["end_time"], "%Y-%m-%d %H:%M:%S")
			delta = end - start
			minutes = delta.seconds // 60
			seconds = delta.seconds % 60
			s["duration"] = f"{minutes}m {seconds}s"
		else:
			s["duration"] = "Ikke afsluttet"
		sessions_with_duration.append(s)

	return render_template("users_sessions.html", users_sessions=sessions_with_duration)

@app.route("/session_details/<session_uuid>")
def session_details(session_uuid):
	# TODO: implement some session specific view.
	session_data = get_session_data(session_uuid)
	return render_template("session_details.html", session_data=session_data)

# ===============================================================
#					End of user sessions view
# ===============================================================

# ===============================================================
#			Start of ESP communication and data retrieval
# ===============================================================

@app.route("/start_session", methods=["POST"])
def start_session_endpoint():
	"""
	Opretter en ny trænings session i databasen for den user der er logget ind
	og med en nyt genereret sessions uuid fra esp controlleren.

	:return: 200 hvis der er en user logget ind. Eller err 401 hvis der ikke er en nogen user logget ind.
	"""
	if "username" not in session:
		return {"error": "no user logged in"}, 401

	data = request.get_json()
	print(data)
	session_uuid = data["session_uuid"]
	user_id = session.get("user_id")
	start_session(session_uuid, user_id)
	return {"status": "ok"}, 200


@app.route("/data", methods=["POST"])
def receive_data_from_esp():
	"""
	Indsætter distance i den igangværende trænings session i databasen,
	sessionen identificeres vha. session_uuid, som modtages fra esp'en.
		TODO: bør tjekke at session_uuid faktisk findes i databasen.
	:return: 200
	"""
	data = request.get_json()
	insert_session_data(data["session_uuid"], data["distance"])
	print(data)
	return {"status": "ok"}, 200

@app.route("/end_session", methods=["POST"])
def end_session_endpoint():
	"""
	Afslutter en trænings session med sessions_uuid modtaget fra esp'en
		TODO: bør tjekke at session_uuid faktisk findes i databasen.
	:return: 200
	"""
	data = request.get_json()
	print(data)
	end_session(data["session_uuid"])
	return {"status": "ok"}, 200

# ===============================================================
#			End of ESP communication and data retrieval
# ===============================================================


if __name__ == "__main__":
	init_db()
	print("Server kører på http://0.0.0.0:5050")
	app.run(host="0.0.0.0", port=5050, debug=True)
