from flask import Flask
from flask import request
app = Flask(__name__)

@app.route("/")
def hello():
    return render_template("index.html", result = [request.args.get("temp"), request.args.get("steps")])