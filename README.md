# Anomaly Detector

A real-time log anomaly detector with a C++ detection engine and a Python dashboard. It tails server logs, flags suspicious patterns using a sliding-window analysis, and streams reports to a live web dashboard over a length-prefixed TCP socket.

## What it detects

- **High request volume from a single IP** — flags IPs sending requests faster than a configurable speed threshold
- **Directory scanning / brute-force attacks** — flags spikes in 404 responses
- **Error rate spikes** — flags unusual bursts of `ERROR`-level log entries

## How it works

```
log file → Reader → Parser → Detector → Reporter → SocketServer ──TCP──▶ SocketClient → ReportBuilder → dashboard
```

**C++ engine (`cpp/`)**
- `io_manager` — tails the log file, tracking a byte offset in `storage/.reader_offset` so it resumes correctly across restarts, even while the file is being actively written
- `parser` — schema-driven log parser built on RE2. The field schema is given as a label string (e.g. `(ip)(method)(status)`), parsed once at construction time into an ordered list of field names; `parse()` then builds RE2 capture args from that schema and runs a single `FullMatchN` per log line, returning a field-name → value map. Changing what a log format looks like is a one-line schema change, not a rewrite of the parsing logic.
- `detector` — maintains a fixed-size sliding window over recent logs; computes per-IP, per-error-type, and per-404 request speed to detect bursts, using a function-pointer state machine (`do_insert` → `process`) to fill the window efficiently before switching to steady-state sliding
- `reporter` — serializes flagged logs to escaped JSON
- `socket_server` — pushes reports to connected clients over TCP using 4-byte length-prefixed framing

**Python dashboard (`dashboard/`)**
- `socket_client.py` — connects to the C++ socket server, reconnects automatically on drop, and reads length-prefixed messages
- `report_builder.py` — renders each report into an HTML page and updates the dashboard homepage; all report data is HTML-escaped on output
- `server.py` — serves the dashboard over HTTP, with path-traversal protection on static file/report requests

## Running it

**Build and start the detector:**
```bash
mkdir build && cd build
cmake ../cpp
cmake --build . -t run
```

**Start the dashboard:**
```bash
cd dashboard
python3 main.py
```

## Testing with the testbed

The `testbed/` folder contains a throwaway HTTP server and a traffic generator for exercising the detector against directory-scan and flood patterns, without needing real traffic.

⚠️ `testbed/server.py` is a separate, local-only test server — it trusts a client-supplied `X-Test-IP` header to simulate requests from different IPs. This is intentional for testing, but this pattern should never be used outside the testbed.

**Start the test server**
```bash
cd testbed
python3 server.py
```

**Generate test traffic**
```bash
cd testbed
python3 attacker.py
```

Point the C++ detector at `logs/server.log` (or wherever `server.py`'s logging is configured to write) to see anomalies get flagged and reported to the dashboard in real time.

Then open `http://127.0.0.1:8080`.

## Notes

This is a prototype — built to explore streaming log analysis and a minimal report pipeline end-to-end, not hardened for production traffic. A few deliberate design choices worth knowing about:

- The sliding window doesn't decrement `sus_count` immediately on window slide, by design — this lets short bursts after a delay still trigger detection rather than being diluted by the window average
- The parser's field schema is declared once (as a label string) and reused across every call to `parse()`, so adding or reordering fields doesn't require touching the parsing logic itself
- Report data is escaped for both JSON (C++ side) and HTML (Python side) since it originates from untrusted request logs
- Static file and report serving validate that resolved paths stay within their intended directory, to prevent path traversal via crafted URLs