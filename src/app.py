from flask import Flask, render_template, redirect, url_for, request, session
from database import init_db, create_user, start_session, end_session,   \
                     get_session, insert_session_data, get_session_data, \
					 get_all_users, login_user, get_users_sessions


app = Flask(__name__)

app.secret_key = "super hemmelig key"  # skal være der for at kunne køre user sessions


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

@app.route("/view_users_sessions/<int:user_id>")
def view_users_sessions(user_id):
	users_sessions = get_users_sessions(user_id)
	return render_template("users_sessions.html", users_sessions=users_sessions)

@app.route("/view_session")
def view_session():
	# TODO: implement some session specific view.
	pass
	# return render_template("view_session.html")

@app.route("/data", methods=["POST"])
def receive_data_from_esp():
	# TODO: implement me..
	pass
	if "username" not in session:
		return {"error": "no user logged in"}, 401  # sender err tilbage til esp der så skal stoppe logning.
													# det er bare et forslag, ved ikke helt om det skal være sådan.


if __name__ == "__main__":
	init_db()
	print("Server kører på http://0.0.0.0:5050")
	app.run(host="0.0.0.0", port=5050, debug=True)
