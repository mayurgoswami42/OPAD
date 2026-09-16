const elem_stream = document.querySelector('#stream');

const elem_spark_rate = document.querySelector('#sparkRate');
const elem_spark_scan = document.querySelector('#sparkScan');
const elem_spark_error = document.querySelector('#sparkError');

const elem_rate_count = document.querySelector('#rateCount');
const elem_scan_count = document.querySelector('#scanCount');
const elem_error_count = document.querySelector('#errorCount');

const elem_uptime = document.querySelector("#uptime");
const elem_offset = document.querySelector("#byteOffset");
const elem_speed = document.querySelector("#parseRate");

const elem_socket_status = document.querySelector("#connDot");

const w = 260, h = 36;
const event_source = new EventSource('/events');
var prev_uptime = -1;
var up_seconds = 0;
var connection_alive = false;
var connection_update = false;

let scan_data = Array(30).fill(0);
let error_data = Array(30).fill(0);
let rate_data = Array(30).fill(0);

let data_kinds = {"DIRECTORY_ATTACK" : "scan", "MAX_REQUESTS_FROM_A_IP" : "rate", "SPIKE_ERROR_RATE" : "error"};

let format_time = seconds => {
    return new Date(seconds * 1000).toISOString().slice(11, 19);
};

let slide_data = (json_obj) => {
    let scan_speed = parseFloat(json_obj["scan_speed"]).toFixed(2);
    elem_scan_count.innerHTML = scan_speed + "<sub> req/s</sub>";
    scan_data.push(scan_speed);
    scan_data.shift();
    
    let error_speed = parseFloat(json_obj["error_speed"]).toFixed(2);
    elem_error_count.innerHTML = error_speed + "<sub> req/s</sub>";
    error_data.push(error_speed);
    error_data.shift();

    let rate_speed = parseFloat(json_obj["rate_speed"]).toFixed(2);
    elem_rate_count.innerHTML = rate_speed + "<sub> req/s</sub>";
    rate_data.push(rate_speed);
    rate_data.shift();

    up_seconds = json_obj["uptime"];

    elem_offset.innerHTML = json_obj["offset"];
    elem_speed.innerHTML = json_obj["tool_speed"] + " logs/s";
};

function escape_html(s){
  return s.replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
}

function build_payload(obj) {
  const rows = Object.entries(obj).map(([k, v]) => {
    let v_class = 'v';
    if (k === 'ip') v_class += ' hl-ip';
    if (k === 'status') v_class += ' hl-status';
    if (k === 'method') v_class += ' hl-method';

    const vStr = typeof v === 'string' ? `"${escape_html(v)}"` : v;
    return ` <span class="k">"${k}"</span><span class="punct">: </span><span class="${v_class}">${vStr}</span>`;
  });

  return `<span class="punct">{</span>\n${rows.join(',\n')}\n<span class="punct">}</span>`;
}

let prepend_element = data => {
    let div = document.createElement('div');
    div.className = "ticket";
    div.dataset.kind = data_kinds[data["anomaly_type"]];
    div.innerHTML = `<div class="ticket-head">
        <span class="severity-tag ${div.dataset.kind}">${data["anomaly_type"]}</span>
        <span class="ticket-type">${data["anomaly_type"]}</span>
        <span class="ticket-date">${data["date"]}</span>
        <span class="ticket-time">${data["time"]}</span>
    </div>
    <div class="ticket-ip">${data["user_ip"]}</div>
    <div class="payload">${build_payload(data)}
    </div>`;

    elem_stream.prepend(div);
}

event_source.onmessage = event => {
    connection_alive = true;
    const data = JSON.parse(event.data);
    if (data.anomaly_type !== undefined)
    {
        prepend_element(data);
    }
    else
    {
        slide_data(data);
    }
};

let get_tag = data => {
    let min = Math.min(...data);
    const max = Math.max(...data, 1);
    const step = w/(data.length-1);
    let points = data.map((v, i) => `${(i * step).toFixed(1)}, ${(h - (v/max) * h * 0.9 - 2).toFixed(1)}`).join(' ');
    let tags = `<polyline points="${points}" fill="none" stroke="$COLOR$" stroke-width="1.6" stroke-linejoin="round" stroke-linecap="round" opacity="0.85"/>
    <polygon points="0,${h} ${points} ${w},${h}" fill="$COLOR$" opacity="0.08"></polygon>`;

    return tags;
};

setInterval(() => {
    if (!connection_alive)
    {
        elem_socket_status.classList.remove("pulse");
        elem_socket_status.classList.add("off");
        connection_update = false;
    }
    
    if (connection_alive && !connection_update)
    {
        elem_socket_status.classList.remove("off");
        elem_socket_status.classList.add("pulse");
        connection_update = true;
    }

    let rate_tags = get_tag(rate_data);
    let error_tags = get_tag(error_data);
    let scan_tags = get_tag(scan_data);
    
    elem_spark_rate.innerHTML = rate_tags.replaceAll("$COLOR$", "#B8860B");
    elem_spark_scan.innerHTML = scan_tags.replaceAll("$COLOR$", "#B54834");
    elem_spark_error.innerHTML = error_tags.replaceAll("$COLOR$", "#8B2E2E");

    if (prev_uptime == up_seconds)
    {
        connection_alive = false;
    }

    elem_uptime.innerHTML = format_time(up_seconds);

    prev_uptime = up_seconds;
}, 1000);