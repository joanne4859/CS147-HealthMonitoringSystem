from flask import Flask, jsonify
from flask import request
app = Flask(__name__)

stats = {"temp": 0, "bpm": 0}

@app.route("/")
def hello():
    return render_template("index.html", result = [stats["temp"], stats["bpm"]])

@app.route("/update_step", methods=["GET"])
def update_step():
    bpm = request.args.get("bpm")
    temp = request.args.get("temp")
    if bpm is not None:
        stats["bpm"] = bpm 
    else:
        bpm = stats["bpm"] 
    if temp is not None:
        stats["temp"] = temp
    else:
        temp = stats["temp"]
    return jsonify({"message": "Data updated successfully", "temp": temp, "bpm": bpm})