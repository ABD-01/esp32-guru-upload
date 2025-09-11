from flask import Flask, request, Response, render_template
import os, time, subprocess, requests
from pathlib import Path

app = Flask(__name__)
UPLOAD_DIR = Path("./coredumps")
UPLOAD_DIR.mkdir(exist_ok=True)

@app.route('/')
def index():
    files = sorted(UPLOAD_DIR.glob("coredump-*.elf"), reverse=True)
    file_data = [
        {"name": f.name, "timestamp": time.ctime(f.stat().st_mtime), "size": f.stat().st_size}
        for f in files
    ]
    return render_template("index.html", files=file_data, title="Coredumps")

@app.route('/upload', methods=['POST'])
def upload():
    timestamp = int(time.time())
    filename = f"coredump-{timestamp}.elf"
    filepath = UPLOAD_DIR / filename
    with open(filepath, "wb") as f:
        f.write(request.get_data())
    return f"Saved {filename}\n", 200

@app.route('/coredump/<filename>')
def show_coredump(filename):
    filepath = UPLOAD_DIR / filename
    if not filepath.exists():
        return "File not found", 404
    try:
        result = subprocess.run(
            ["idf.py", "coredump-info", "--core", str(filepath)],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
        )
        output = result.stdout
    except Exception as e:
        output = f"Error running idf.py: {e}"
    return render_template("coredump.html", filename=filename, output=output, title="Coredump")

@app.route('/esp_server')
def esp_server():
    return render_template("esp_server.html", title="ESP Server")

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=8080)
