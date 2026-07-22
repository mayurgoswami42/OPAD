from http.server import HTTPServer, BaseHTTPRequestHandler
from threading import Thread
import logging
import time
import os
import sys
from urllib.parse import parse_qs

HOST = "192.168.43.57"
PORT = 8000

TEST_MODE = True

main_page = 'login.html'

logging.basicConfig(
    filename="../logs/server.log",
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s"
)

def getType(filePath:str):
    i = 0
    while i < len(filePath) and filePath[i] != '.': i+=1
    ext = ""
    for j in range(len(filePath) - i -1):
        ext += filePath[i+j+1]
    
    return ext

class LogType:
    DEBUG = "DEBUG"
    INFO = "INFO"
    WARN = "WARN"
    ERROR = "ERROR"
    FETAL = "FETAL"

class RequestHandler(BaseHTTPRequestHandler):
    def __send_response(self, status: int, header: str, message: str | None = None, file: str | None = None):
        encoded_msg = b""
        if message:
            encoded_msg = message.encode('utf-8')
        else:
            encoded_msg = open(file, "rb").read()

        self.send_response(status)
        self.send_header('Content-type', header)
        self.end_headers()

        try:
            self.wfile.write(encoded_msg)
        except Exception as e:
            print(e)
    
    def do_POST(self):
        content_len = int(self.headers.get('Content-Length', 0))
        data = self.rfile.read(content_len)
        try:
            dict_data = parse_qs(data)
            if (dict_data[b'username'][0] == b'admin' and dict_data[b'password'][0] == b'admin'):
                global main_page
                main_page = 'index.html'
                self.send_response(302)
                self.send_header('Location', 'index.html')
                self.end_headers()
            else:
                self.__send_response(200, 'text/html', file="frontend/login.html")
        except:
            self.__send_response(400, 'text/html', file="frontend/error.html")

    def do_GET(self):
        path = self.path.split('?')[0]
        if path == '/':
            self.__send_response(200, 'text/html', "Home page")
        elif path == '/login':
            self.__send_response(200, "text/html", "Login page")
        else:
            self.__send_response(404, "text/html", "File Not Found")

    def log_message(self, format, *args):
        client_ip = self.address_string()
        if TEST_MODE: client_ip = self.headers.get("X-Test-IP", client_ip)
        logging.info(f"{client_ip} {format % args}")
        

def watchOut(delay, server):
    path = os.path.abspath(__file__)
    xmtime = os.path.getmtime(path)
    while (True):
        time.sleep(delay)
        mtime = os.path.getmtime(path)
        if (xmtime != mtime):
            print("Updates recorded!")
            server.shutdown()
            print("Server is Stopped!")
            return

def runServer():
    server = HTTPServer((HOST, PORT), RequestHandler)

    watcher = Thread(target=watchOut, kwargs={"delay": 1, "server": server}, daemon=True)
    watcher.start()
    
    print(f"Server is running at [http://{HOST}:{PORT}]...")
    server.serve_forever()
    server.server_close()

def main():
    runServer()
    print(f"Restarting server in 1 second")
    time.sleep(1)
    os.execv(sys.executable, [sys.executable] + sys.argv)

if __name__ == "__main__":
    main()