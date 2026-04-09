from flask import Flask, render_template, redirect, url_for, request, session
from database import init_db, create_user, start_session, end_session,   \
                     get_session, insert_session_data, get_session_data, \
					 get_all_users, login_user


app = Flask(__name__)

app.secret_key = "super hemmelig key"

@app.route("/")
def index():
	user = session.get("username")
	return render_template("index.html", user=user)

@app.route("/login")
def login():
	users = get_all_users()
	return render_template("login.html", users=users)

@app.route("/login/create", methods=["POST"])
def login_create_user():
	username = request.form.get("username", "").strip()
	if username:
		create_user(username)
	return redirect(url_for("login"))

@app.route("/login/select/<int:user_id>")
def login_select(user_id):
	username = login_user(user_id)
	if username:
		session["username"] = username
	return redirect(url_for("index"))

@app.route("/logout")
def logout():
	session.pop("username", None)
	return redirect(url_for("index"))

@app.route("/view_session")
def view_session():
	# TODO: implement some session specific view.
	pass
	# return render_template("view_session.html")


@app.route("/data", methods=["POST"])
def receive_data_from_esp():
	# TODO: implement me..
	pass


if __name__ == "__main__":
	init_db()
	print("Server kører på http://0.0.0.0:5050")
	app.run(host="0.0.0.0", port=5050, debug=True)
