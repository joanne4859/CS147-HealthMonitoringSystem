from flask import Flask
from flask import request
app = Flask(__name__)

@app.route("/")
def hello():
    render_template("index.html", result = request.args.get("var"))
    print(request.args.get("var"))