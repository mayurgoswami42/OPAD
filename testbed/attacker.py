import random
import requests
from threading import Thread

lines = list()
ips = list()

state = [True, False]

# with open("../logs/server.log", "r") as file:
#     print(len(file.readlines()))

with open("wordlist.txt", "r") as file:
    lines = [line.strip() for line in file]

with open("ips.txt", "r") as file:
    ips = [ip.strip() for ip in file]

def send_requests(random_ips = False, random_dir = False):
    for _ in range(1000000):
        head = {"X-Test-IP" : ips[0]}
        line = ''
        if random_ips:
            head = {"X-Test-IP" : random.choice(ips)}

        if random_dir:
            line = random.choice(lines)

        response = requests.get(f"http://192.168.43.57:8000/{line}", headers=head)
        print(response)

threads = []

send_requests(False, True)

# for i in range(100):
#     threads.append(Thread(target=send_requests, args=(random.choice(state), random.choice(state))))
#     threads[-1].start()
#     threads[-1].join()