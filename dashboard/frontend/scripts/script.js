const stream_div = document.querySelector('#stream');

const spark_rate = document.querySelector('#sparkRate');
const spark_scan = document.querySelector('#sparkScan');
const spark_error = document.querySelector('#sparkError');

const w = 260, h = 36;
const event_source = new EventSource('/events');

let scan_data = Array(30).fill(0);
let error_data = Array(30).fill(0);
let rate_data = Array(30).fill(0);

let data_kinds = {"DIRECTORY_ATTACK" : "scan", "MAX_REQUESTS_FROM_A_IP" : "rate", "SPIKE_ERROR_RATE" : "error"};

let slide_data = (json_obj) => {
    scan_data.push(parseFloat(json_obj["scan_speed"]));
    scan_data.shift();
    error_data.push(parseFloat(json_obj["error_speed"]));
    error_data.shift();
    rate_data.push(parseFloat(json_obj["rate_speed"]));
    rate_data.shift();
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

    stream_div.prepend(div);
}

event_source.onmessage = event => {
    const data = JSON.parse(event.data);
    console.log(data);
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
    let rate_tags = get_tag(rate_data);
    let error_tags = get_tag(error_data);
    let scan_tags = get_tag(scan_data);
    
    spark_rate.innerHTML = rate_tags.replaceAll("$COLOR$", "#B8860B");
    spark_scan.innerHTML = scan_tags.replaceAll("$COLOR$", "#B54834");
    spark_error.innerHTML = error_tags.replaceAll("$COLOR$", "#8B2E2E");

}, 1000);