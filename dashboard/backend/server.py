from http.server import BaseHTTPRequestHandler, HTTPServer
import posixpath
from urllib.parse import unquote
from collections import deque
import os

import time

HOST: str = "127.0.0.1"
PORT: int = 8080

SOURCE_DIR: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE_PATH: str = os.path.join(SOURCE_DIR, "frontend")

class RequestHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.report_queue = deque()
        super().__init__(*args, **kwargs)
    
    def __safe_join(self, base_dir: str, url_path: str) -> str | None:
        decoded_path: str = unquote(url_path) # decode percent encoding before checking

        normalized = posixpath.normpath(decoded_path)

        if normalized.startswith("..") or "/../" in normalized or normalized == "..":
            return None

        normalized: str = normalized.lstrip("/")

        candidate = os.path.abspath(os.path.join(base_dir, normalized))

        if not (candidate == base_dir or candidate.startswith(base_dir + os.sep)):
            return None

        return candidate

    def __send_response_404(self):
        self.send_response(404)
        self.end_headers()

    def __send_page_404(self, path: str) -> None:
        _file = open(f"{SOURCE_DIR}/templates/page_not_found.html", "r")
        page: str = _file.read()
        _file.close()
        page = page.replace("$INVALID_PATH$", path)
        self.__send_page(404, 'text/html', message=page)

    def __send_page(self, status, type: str, message: str | None = None, file: str | None = None) -> None:
        self.send_response(status)
        self.send_header('Content-type', type)
        self.end_headers()

        content: bytes = b''

        if message:
            content = message.encode()
        elif file:
            _file = open(file, "rb")
            content = _file.read()
            _file.close()

        try:
            self.wfile.write(content)
        except Exception as e:
            print(e)

    def do_GET(self) -> None:
        path: str = self.path.split('?')[0]

        if path == '/':
            self.__send_page(200, 'text/html', file = f"{BASE_PATH}/index.html")
        elif path == '/events':
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream')
            self.send_header('Cache-Control', 'no-cache')
            self.send_header('Connection', 'keep-alive')
            self.end_headers()

            try:
                while True:
                    if len(self.report_queue) > 0:
                        task_str = self.report_queue[0]
                        self.wfile.write(task_str.encode('utf-8'))
                        self.wfile.flush()

                        time.sleep(1)

            except (ConnectionAbortedError, BrokenPipeError) as e:
                print(f'[SERVER::ERROR] Client disconnected: {e}')
                return

        elif path.endswith(".css"):
            _path: str | None = self.__safe_join(f"{BASE_PATH}/styles", path)
            if (_path and os.path.isfile(_path)):
                self.__send_page(200, 'text/css', file=_path)
            else:self.__send_response_404()

        elif path.endswith(".js"):
            _path: str | None = self.__safe_join(f"{BASE_PATH}/scripts", path)
            if (_path and os.path.isfile(_path)):
                self.__send_page(200, 'text/js', file=_path)
            else:self.__send_response_404()

        elif path.startswith("/report_"):
            _path: str | None = self.__safe_join(f"{BASE_PATH}/reports", path)
            if (_path and os.path.isfile(_path)):
                self.__send_page(200, 'text/html', file=_path)
            else:self.__send_page_404(path)

        else:
            self.__send_page_404(path)

def run() -> None:
    server = HTTPServer((HOST, PORT), RequestHandler)
    print(f"Server is running on [{HOST}:{PORT}]")
    server.serve_forever()
    server.server_close()