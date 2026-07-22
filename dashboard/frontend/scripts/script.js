const spark_rate = document.querySelector('#sparkRate');
const spark_scan = document.querySelector('#sparkScan');
const spark_error = document.querySelector('#sparkError');

const w = 260, h = 36;
const event_source = new EventSource('/events');

let scan_data = Array(30).fill(0);
let error_data = Array(30).fill(0);
let rate_data = Array(30).fill(0);

let slide_data = (json_obj) => {
    scan_data.push(parseFloat(json_obj["scan_speed"]));
    scan_data.shift();
    error_data.push(parseFloat(json_obj["error_speed"]));
    error_data.shift();
    rate_data.push(parseFloat(json_obj["rate_speed"]));
    rate_data.shift();
};

event_source.onmessage = event => {
    const data = JSON.parse(event.data);
    if (data.command === "reload")
    {
        console.log(true);
        location.reload();
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