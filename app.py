from flask import Flask, render_template, jsonify

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html', title='Welcome to Flask', message='Hello, Flask!')

@app.route('/getData')
def data():
    data = {
        "temperature": "29.10",
        "humidity": "55",
        "smoke": "65"
    }
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)