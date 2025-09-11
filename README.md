# esp32-guru-upload
Upload Guru Meditation crashdumps from an ESP32 to a Flask server endpoint.

ESP32 firmware captures crashdumps from the dedicated coredump partition.

Crashdumps are uploaded via HTTP POST to a Flask server.

## How To

### 1. Open in Codespaces

This repository is preconfigured with a development container.
Open it in GitHub Codespaces to get ESP-IDF and Python environment ready.

### 2. Start ESP32 Firmware in QEMU

Launch the emulated ESP32 firmware with port forwarding:

```bash
cd esp32-guru-upload
get_idf   # setup ESP-IDF environment
idf.py qemu --qemu-extra-args "-nic user,hostfwd=tcp::8081-:8080"
```

For more insight on qemu visit my [Blog](https://abd-01.github.io/posts/2025-07-15-QEMU/)

### 3. Start the Flask Server

In another terminal:

```bash
cd esp32-guru-upload
get_idf   # setup ESP-IDF environment
python endpoint_app.py
```

* Flask server will run at `http://localhost:8080/`.

### 4. Access the Web UI

Visit:
[http://localhost:8080/](http://localhost:8080/) or github codespaces provided URL.

## License

GNU Affero General Public License v3.0 (AGPL-3.0). See [LICENSE](LICENSE) for details.