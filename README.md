# OPAD

> **Optimized Portable Anomaly Detector**

A high-performance, portable, real-time log anomaly detection platform featuring a C++ detection engine and a Python dashboard. OPAD tails server logs, detects suspicious activity using sliding-window analysis, and streams reports to a live web dashboard over a length-prefixed TCP socket.

## Features

- Real-time log monitoring
- High-performance C++23 detection engine
- Portable architecture
- Sliding-window anomaly detection
- Live HTML dashboard
- Schema-driven RE2 log parser
- TCP streaming between detector and dashboard
- Automatic recovery from restarts

## What it detects

- **High request volume from a single IP** — flags IPs sending requests faster than a configurable threshold
- **Directory scanning / brute-force attacks** — detects bursts of 404 responses
- **Error rate spikes** — detects unusual bursts of `ERROR` log entries

## Architecture

```text
Log File
    │
    ▼
 Reader
    │
    ▼
 Parser
    │
    ▼
 Detector
    │
    ▼
 Reporter
    │
    ▼
Socket Server
    │
 Length-Prefixed TCP
    │
    ▼
Socket Client
    │
    ▼
Report Builder
    │
    ▼
 Dashboard
```

## Components

### C++ Engine (`cpp/`)

#### `io_manager`

Continuously tails the log file while tracking its byte offset in `storage/.reader_offset`, allowing OPAD to resume processing after restarts without rereading previously processed logs.

#### `parser`

A schema-driven log parser built on **Google RE2**.

Instead of hardcoding field extraction, OPAD accepts a field schema such as

```text
(ip)(method)(status)
```

The schema is parsed once during construction into an ordered list of field names. During parsing, the parser dynamically constructs the RE2 capture arguments and performs a single `RE2::FullMatchN()` call to produce a field-name → value map.

Changing a log format only requires updating the schema instead of rewriting parsing logic.

#### `detector`

Maintains a fixed-size sliding window over recent log entries.

It tracks:

- request rate per IP
- 404 response frequency
- error-rate spikes

A function-pointer state machine

```text
do_insert → process
```

is used to efficiently transition from the initial window-fill phase into steady-state processing.

#### `reporter`

Serializes detected anomalies into escaped JSON before transmission.

#### `socket_server`

Streams reports to connected clients using 4-byte length-prefixed TCP framing.

---

### Python Dashboard (`dashboard/`)

#### `socket_client.py`

Maintains a persistent connection to the C++ engine, automatically reconnecting if the connection drops.

#### `report_builder.py`

Converts anomaly reports into HTML pages while escaping all user-controlled data before rendering.

#### `server.py`

Serves the dashboard through a lightweight HTTP server with path traversal protection for both static assets and generated reports.

## Running OPAD

### Build and start the detector

```bash
mkdir build
cd build

cmake ../cpp
cmake --build . -t run
```

### Start the dashboard

```bash
cd dashboard
python3 main.py
```

## Testing

The `testbed/` directory contains a disposable HTTP server together with a traffic generator for exercising OPAD without requiring production traffic.

### Start the test server

```bash
cd testbed
python3 server.py
```

### Generate attack traffic

```bash
cd testbed
python3 attacker.py
```

Point the detector at the generated log file (typically `logs/server.log`) to observe anomalies being detected and streamed to the dashboard.

Open:

```text
http://127.0.0.1:8080
```

## Security Notes

The test server intentionally trusts the client-supplied `X-Test-IP` header to simulate requests from different IP addresses.

This behavior exists **only** to simplify testing and must never be used in a production deployment.

## Design Notes

OPAD is a prototype focused on exploring high-performance streaming log analysis rather than being a production SIEM.

Notable implementation details include:

- Schema-driven parsing that separates log structure from parsing logic.
- Fixed-memory sliding-window analysis.
- Function-pointer state machine for efficient initialization.
- Length-prefixed TCP protocol between the detection engine and dashboard.
- JSON escaping in C++ and HTML escaping in Python to safely handle untrusted log data.
- Path validation to prevent directory traversal attacks.
- Resume support using persisted byte offsets for uninterrupted log processing after restarts.

## Tech Stack

- **C++23**
- **Python 3**
- **Google RE2**
- **CMake**
- **TCP Sockets**
- **Sliding Window Algorithms**

---

**OPAD (Optimized Portable Anomaly Detector)** is a portable, real-time log anomaly detection platform built to demonstrate efficient streaming analysis, modular architecture, and secure end-to-end report generation.
