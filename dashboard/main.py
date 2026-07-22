from threading import Thread

from backend import SocketClient, ReportBuilder, run, add_task

rep_builder: ReportBuilder = ReportBuilder()

sock_client: SocketClient = SocketClient(rep_builder.save_reports, add_task, '127.0.0.1', 5555)

socket_thread: Thread = Thread(target=sock_client.run, daemon=True)
socket_thread.start()

try:
    run()
except KeyboardInterrupt: ...
finally: sock_client.stop()