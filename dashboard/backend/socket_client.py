from typing import Callable

import socket
import time
import struct

class SocketClient:
    def __init__(self, on_report: Callable[[str], None], send_speed: Callable[[str], None], host: str, port: int):
        self.host: str = host
        self.port: int = port
        self.on_report: Callable[[str], None] = on_report
        self.send_speed: Callable[[str], None] = send_speed
        self.running = False

    def __distinguish(self, data: str):
        if 'speed' in data:
            self.send_speed(data)
        else:
            self.on_report(data)

    def stop(self):
        self.running = False

    def __receive(self, sock: socket.socket, n: int) -> bytes | None:
        buf: bytes = b""
        while (len(buf) < n):
            chunk: bytes = sock.recv(n - len(buf))
            if not chunk:
                return None
            buf += chunk
        return buf

    def run(self):
        self.running = True
        while self.running:
            try:
                self.__connect_and_listen()
            except (ConnectionResetError, ConnectionRefusedError, OSError) as e:
                print(f"[socket_client] connection lost {e}, retrying in 3s")
                time.sleep(3)

    def __connect_and_listen(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((self.host, self.port))
            print(f"[socket_client] connected to {self.host}:{self.port}")

            while self.running:
                header = self.__receive(s, 4)
                if header is None:
                    print("[socket_client] connection closed")
                    break

                (msg_len,) = struct.unpack("!I", header)

                payload = self.__receive(s, msg_len)

                if payload is None:
                    print("[socket_client] connection closed mid message")
                    break

                try:
                    text = payload.decode()
                except UnicodeDecodeError:
                    continue

                self.__distinguish(text)
                s.sendall(b"Data received")