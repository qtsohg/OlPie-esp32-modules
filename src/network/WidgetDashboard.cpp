#include "WidgetDashboard.h"

#include "NetworkConfig.h"

namespace espmods::network {

namespace {
String escapeHtml(const String& value) {
  String escaped;
  escaped.reserve(value.length());
  for (char c : value) {
    switch (c) {
      case '&': escaped += F("&amp;"); break;
      case '<': escaped += F("&lt;"); break;
      case '>': escaped += F("&gt;"); break;
      case '\"': escaped += F("&quot;"); break;
      case '\'': escaped += F("&#39;"); break;
      default: escaped += c; break;
    }
  }
  return escaped;
}
}  // namespace

void WidgetDashboard::addButton(const ButtonConfig& button) {
  buttons_.push_back(button);
}

void WidgetDashboard::addSlider(const SliderConfig& slider) {
  sliders_.push_back(slider);
}

void WidgetDashboard::addInput(const InputConfig& input) {
  inputs_.push_back(input);
}

void WidgetDashboard::attach(WebServer& server, const NetworkConfig& config) {
  for (auto& button : buttons_) {
    if (button.id.isEmpty()) continue;
    if (button.endpoint.isEmpty()) {
      button.endpoint = String(F("/api/widgets/button/")) + button.id;
    }
    server.on(button.endpoint.c_str(), HTTP_POST, [this, &server, &button]() {
      if (button.onClick) {
        button.onClick();
      }
      server.send(200, F("application/json"), F("{\"status\":\"ok\"}"));
    });
  }

  for (auto& slider : sliders_) {
    if (slider.id.isEmpty()) continue;
    if (slider.endpoint.isEmpty()) {
      slider.endpoint = String(F("/api/widgets/slider/")) + slider.id;
    }
    server.on(slider.endpoint.c_str(), HTTP_POST, [this, &server, &slider]() {
      String valueArg = server.arg(F("value"));
      float value = valueArg.toFloat();
      slider.value = value;
      if (slider.onChange) {
        slider.onChange(value);
      }
      String response = F("{\"status\":\"ok\",\"value\":");
      response += String(value, 3);
      response += '}';
      server.send(200, F("application/json"), response);
    });
  }

  for (auto& input : inputs_) {
    if (input.id.isEmpty()) continue;
    if (input.endpoint.isEmpty()) {
      input.endpoint = String(F("/api/widgets/input/")) + input.id;
    }
    server.on(input.endpoint.c_str(), HTTP_POST, [this, &server, &input]() {
      String value = server.arg(F("value"));
      input.value = value;
      if (input.onSubmit) {
        input.onSubmit(value);
      }
      server.send(200, F("application/json"), F("{\"status\":\"ok\"}"));
    });
  }

  server.on(F("/"), [this, &server, &config]() {
    server.send(200, F("text/html"), buildIndexHtml(config));
  });
}

String WidgetDashboard::buildIndexHtml(const NetworkConfig& config) const {
  String html;
  html.reserve(8192);

  html += F("<!DOCTYPE html><html><head><meta charset=\"utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />");
  html += F("<title>");
  if (config.deviceHostname) {
    html += escapeHtml(config.deviceHostname);
    html += F(" Controls");
  } else {
    html += F("Device Controls");
  }
  html += F("</title>");
  html += F("<style>body{font-family:'Inter','Segoe UI',sans-serif;background:#10151b;color:#f1f5f9;margin:0;}");
  html += F("header{padding:1.5rem 1.75rem;background:#121b24;border-bottom:1px solid rgba(148,163,184,0.2);position:sticky;top:0;z-index:10;}");
  html += F("h1{margin:0;font-size:1.5rem;font-weight:600;color:#38bdf8;}");
  html += F("main{display:flex;flex-direction:column;gap:1.5rem;padding:1.5rem;}");
  html += F("@media(min-width:900px){main{flex-direction:row;align-items:flex-start;}}");
  html += F("section{background:#121b24;border:1px solid rgba(148,163,184,0.12);border-radius:0.75rem;box-shadow:0 20px 45px -25px rgba(15,23,42,0.6);flex:1;min-width:0;}");
  html += F("section header{padding:1rem 1.5rem;border-bottom:1px solid rgba(148,163,184,0.12);background:transparent;position:static;}");
  html += F("section h2{margin:0;font-size:1.1rem;color:#f8fafc;}");
  html += F("section .content{padding:1.25rem;}");
  html += F("button, input[type='submit']{background:#38bdf8;color:#020617;border:none;border-radius:0.75rem;padding:0.65rem 1.1rem;font-weight:600;cursor:pointer;transition:transform 0.1s ease,box-shadow 0.1s ease;}");
  html += F("button:hover,input[type='submit']:hover{transform:translateY(-1px);box-shadow:0 12px 20px -12px rgba(56,189,248,0.8);}");
  html += F("button:disabled{opacity:0.6;cursor:not-allowed;transform:none;box-shadow:none;}");
  html += F(".widgets{display:grid;gap:1rem;}");
  html += F(".widget-card{background:rgba(15,23,42,0.55);border:1px solid rgba(148,163,184,0.15);border-radius:0.75rem;padding:1rem;display:flex;flex-direction:column;gap:0.75rem;}");
  html += F(".widget-card h3{margin:0;font-size:1rem;color:#f8fafc;}");
  html += F(".widget-desc{margin:0;color:#94a3b8;font-size:0.85rem;}");
  html += F(".slider-value{font-weight:600;color:#38bdf8;}");
  html += F("input[type='range']{width:100%;accent-color:#38bdf8;}");
  html += F("input[type='text']{width:100%;padding:0.6rem 0.75rem;border-radius:0.65rem;border:1px solid rgba(148,163,184,0.25);background:rgba(15,23,42,0.4);color:#f8fafc;}");
  html += F("label{font-weight:500;color:#e2e8f0;}");
  html += F(".logs{font-family:'JetBrains Mono','Fira Code','Courier New',monospace;background:#0f172a;border-radius:0.75rem;border:1px solid rgba(148,163,184,0.1);min-height:360px;max-height:600px;overflow:auto;padding:1rem;white-space:pre-wrap;word-break:break-word;}");
  html += F(".status{color:#94a3b8;font-size:0.85rem;margin-top:0.4rem;}");
  html += F(".log-controls{display:flex;gap:0.6rem;margin-top:1rem;flex-wrap:wrap;}");
  html += F(".toast{position:fixed;bottom:1.5rem;right:1.5rem;background:#0f172a;border:1px solid rgba(148,163,184,0.22);padding:0.85rem 1.1rem;border-radius:0.65rem;box-shadow:0 25px 45px -20px rgba(15,23,42,0.7);font-size:0.85rem;color:#e2e8f0;display:none;}");
  html += F(".toast.show{display:block;}");
  html += F("</style></head><body>");

  html += F("<header><h1>");
  if (config.deviceHostname) {
    html += escapeHtml(config.deviceHostname);
  } else {
    html += F("Device Dashboard");
  }
  html += F("</h1><div class=\"status\" id=\"status\">Loading...</div></header>");

  html += F("<main>");
  html += F("<section><header><h2>Live Logs</h2></header><div class=\"content\"><pre id=\"logs\" class=\"logs\">Loading logs...</pre>");
  html += F("<div class=\"log-controls\">");
  html += F("<button id=\"clearLogs\">Clear Display</button>");
  html += F("<button id=\"downloadLogs\">Download Logs</button>");
  html += F("<button id=\"autoScrollBtn\" class=\"auto-scroll\">Auto-scroll: ON</button>");
  html += F("</div></div></section>");

  html += F("<section><header><h2>Controls</h2></header><div class=\"content widgets\">");

  for (const auto& button : buttons_) {
    if (button.id.isEmpty()) continue;
    html += F("<div class=\"widget-card\"><div><h3>");
    html += escapeHtml(button.label);
    html += F("</h3>");
    if (button.description.length() > 0) {
      html += F("<p class=\"widget-desc\">");
      html += escapeHtml(button.description);
      html += F("</p>");
    }
    html += F("</div><button data-widget=\"button\" data-endpoint=\"");
    html += button.endpoint;
    html += F("\" data-id=\"");
    html += button.id;
    html += F("\">");
    html += escapeHtml(button.label);
    html += F("</button></div>");
  }

  for (const auto& slider : sliders_) {
    if (slider.id.isEmpty()) continue;
    html += F("<div class=\"widget-card\"><h3>");
    html += escapeHtml(slider.label);
    html += F("</h3><div><label for=\"");
    html += slider.id;
    html += F("\">Value: <span class=\"slider-value\" id=\"");
    html += slider.id;
    html += F("-value\">");
    html += String(slider.value, 2);
    html += F("</span></label></div>");
    html += F("<input type=\"range\" id=\"");
    html += slider.id;
    html += F("\" min=\"");
    html += String(slider.min, 2);
    html += F("\" max=\"");
    html += String(slider.max, 2);
    html += F("\" step=\"");
    html += String(slider.step, 2);
    html += F("\" value=\"");
    html += String(slider.value, 2);
    html += F("\" data-widget=\"slider\" data-endpoint=\"");
    html += slider.endpoint;
    html += F("\" data-id=\"");
    html += slider.id;
    html += F("\" /></div>");
  }

  for (const auto& input : inputs_) {
    if (input.id.isEmpty()) continue;
    html += F("<div class=\"widget-card\"><h3>");
    html += escapeHtml(input.label);
    html += F("</h3><form data-widget=\"input\" data-endpoint=\"");
    html += input.endpoint;
    html += F("\" data-id=\"");
    html += input.id;
    html += F("\"><input type=\"text\" name=\"value\" value=\"");
    html += escapeHtml(input.value);
    html += F("\" placeholder=\"");
    html += escapeHtml(input.placeholder);
    html += F("\" /><input type=\"submit\" value=\"Submit\" /></form></div>");
  }

  html += F("</div></section>");
  html += F("</main>");
  html += F("<div class=\"toast\" id=\"toast\"></div>");

  html += F("<script>");
  html += F("const logsEl=document.getElementById('logs');const statusEl=document.getElementById('status');const autoScrollBtn=document.getElementById('autoScrollBtn');const toast=document.getElementById('toast');let autoScroll=true;let lastUpdateTime=new Date();");
  html += F("function showToast(message){toast.textContent=message;toast.classList.add('show');setTimeout(()=>toast.classList.remove('show'),1800);} ");
  html += F("function updateStatus(){const now=new Date();const diff=Math.round((now-lastUpdateTime)/1000);statusEl.textContent=`Last update: ${diff}s ago | Auto-refresh: ON`;}");
  html += F("async function fetchLogs(){try{const response=await fetch('/logs',{cache:'no-cache'});if(!response.ok){throw new Error(`HTTP ${response.status}`);}const shouldPin=autoScroll&&Math.abs(logsEl.scrollTop+logsEl.clientHeight-logsEl.scrollHeight)<8;const text=await response.text();logsEl.textContent=text;if(shouldPin){logsEl.scrollTop=logsEl.scrollHeight;}lastUpdateTime=new Date();updateStatus();}catch(err){statusEl.textContent=`Error: ${err.message}`;}}");
  html += F("document.getElementById('clearLogs').addEventListener('click',()=>{logsEl.textContent='Logs cleared (display only)';});");
  html += F("document.getElementById('downloadLogs').addEventListener('click',()=>{const logContent=logsEl.textContent;const blob=new Blob([logContent],{type:'text/plain'});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download=`device-logs-${new Date().toISOString().slice(0,19).replace(/:/g,'-')}.txt`;document.body.appendChild(a);a.click();document.body.removeChild(a);URL.revokeObjectURL(url);});");
  html += F("autoScrollBtn.addEventListener('click',()=>{autoScroll=!autoScroll;autoScrollBtn.textContent=`Auto-scroll: ${autoScroll?'ON':'OFF'}`;autoScrollBtn.classList.toggle('auto-scroll',autoScroll);});");
  html += F("setInterval(updateStatus,1000);setInterval(fetchLogs,1000);fetchLogs();");

  html += F("document.querySelectorAll('[data-widget=\"button\"]').forEach(btn=>{btn.addEventListener('click',async()=>{btn.disabled=true;try{const response=await fetch(btn.dataset.endpoint,{method:'POST'});if(!response.ok){throw new Error('Request failed');}showToast(`${btn.textContent.trim()} triggered`);}catch(err){showToast(err.message);}finally{btn.disabled=false;}});});");
  html += F("document.querySelectorAll('[data-widget=\"slider\"]').forEach(slider=>{const valueEl=document.getElementById(`${slider.dataset.id}-value`);slider.addEventListener('input',()=>{valueEl.textContent=parseFloat(slider.value).toFixed(2);});slider.addEventListener('change',async()=>{try{const response=await fetch(slider.dataset.endpoint,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`value=${encodeURIComponent(slider.value)}`});if(!response.ok){throw new Error('Request failed');}showToast(`${slider.dataset.id} → ${parseFloat(slider.value).toFixed(2)}`);}catch(err){showToast(err.message);}});});");
  html += F("document.querySelectorAll('[data-widget=\"input\"]').forEach(form=>{form.addEventListener('submit',async(event)=>{event.preventDefault();const formData=new FormData(form);const value=formData.get('value');try{const response=await fetch(form.dataset.endpoint,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`value=${encodeURIComponent(value)}`});if(!response.ok){throw new Error('Request failed');}showToast(`${form.dataset.id} updated`);}catch(err){showToast(err.message);}});});");

  html += F("</script></body></html>");
  return html;
}

}  // namespace espmods::network

