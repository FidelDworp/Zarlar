/* ESP32 WROOM - Zarlar Dashboard
 * Author: Fidel Dworp
 * Date: 24 jan 2026
 * 
 * Standalone webserver die het Zarlar dashboard host
 * Verbindt met Particle Photon controllers + ESP32 controllers
 * 
 * ACCESS: http://192.168.1.X/
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// ============== CONFIGURATIE ==============
// PAS DEZE AAN!
const char* WIFI_SSID = "Delannoy";        // ← onze login
const char* WIFI_PASS = "kampendaal,34";   // ← onze login
const char* MDNS_NAME = "zarlar";          // → http://zarlar.local/

// ESP32 Controller IPs (pas aan indien nodig)
const char* ECO_IP =      "192.168.1.99";
const char* TESTROOM_IP = "192.168.1.101";
const char* HVAC_IP =     "192.168.1.100";

// ============== GLOBALS ==============
AsyncWebServer server(80);

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 Zarlar Dashboard ===");
  Serial.println("Version 1.0 - 24 jan 2026");
  
  // WiFi verbinden
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("Verbinden met WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi verbonden!");
    Serial.print("IP adres: ");
    Serial.println(WiFi.localIP());
    Serial.print("URL: http://");
    Serial.println(WiFi.localIP());
    
    // mDNS
    if (MDNS.begin(MDNS_NAME)) {
      Serial.print("mDNS: http://");
      Serial.print(MDNS_NAME);
      Serial.println(".local/");
    }
  } else {
    Serial.println("\n✗ WiFi verbinding mislukt!");
    Serial.println("Check SSID en wachtwoord!");
  }
  
  // Web server setup
  setupWebServer();
  server.begin();
  
  Serial.println("\n=== Dashboard READY ===");
  Serial.println("Open in browser:");
  Serial.print("  http://");
  Serial.println(WiFi.localIP());
  Serial.print("  http://");
  Serial.print(MDNS_NAME);
  Serial.println(".local/");
}

// ============== LOOP ==============
void loop() {
  delay(100);
  // Webserver draait asynchroon
}

// ============== WEB SERVER ==============
void setupWebServer() {
  
  // Root endpoint - Zarlar Dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = getZarlarHTML();
    request->send(200, "text/html", html);
  });
  
  // Info endpoint
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    String info = "ESP32 Zarlar Dashboard v1.0\n";
    info += "IP: " + WiFi.localIP().toString() + "\n";
    info += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    info += "Uptime: " + String(millis()/1000) + " sec\n";
    request->send(200, "text/plain", info);
  });
  
  // 404
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });
}

// ============== HTML GENERATOR ==============
String getZarlarHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="nl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Zarlar Dashboard - ESP32</title>
<style>
  body{margin:0;font-family:Arial,Helvetica,sans-serif;background:linear-gradient(135deg, #667eea 0%, #764ba2 100%);color:#fff;min-height:100vh}
  .header{text-align:center;padding:30px 20px;background:rgba(0,0,0,0.2)}
  .header h1{margin:0;font-size:32px;text-shadow:0 2px 4px rgba(0,0,0,0.3)}
  .header p{margin:10px 0 0 0;opacity:0.9;font-size:14px}
  .container{max-width:900px;margin:0 auto;padding:20px}
  .section{background:rgba(255,255,255,0.95);border-radius:12px;padding:20px;margin-bottom:20px;color:#222;box-shadow:0 4px 12px rgba(0,0,0,0.15)}
  .section h2{margin:0 0 15px 0;color:#667eea;font-size:20px;border-bottom:2px solid #667eea;padding-bottom:10px}
  .buttons{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:12px}
  .btn{padding:15px;background:#667eea;color:#fff;border:none;border-radius:8px;font-size:16px;cursor:pointer;font-weight:600;transition:all 0.3s}
  .btn:hover{background:#5568d3;transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,0.2)}
  .btn:active{transform:translateY(0)}
  .btn-photon{background:#007bff}
  .btn-photon:hover{background:#0056b3}
  .btn-esp{background:#28a745}
  .btn-esp:hover{background:#218838}
  .btn-hvac{background:#ff6600}
  .btn-hvac:hover{background:#e65c00}
  #box{background:#fff;border-radius:8px;padding:20px;min-height:200px}
  .row{display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid #eee}
  .row:last-child{border-bottom:none}
  .lbl{font-weight:600;color:#667eea;flex:0 0 140px}
  .val{font-family:monospace;font-weight:700;padding:6px 12px;border-radius:6px;background:#f0f0f0;text-align:center;min-width:100px}
  .loading{text-align:center;padding:60px 20px;color:#999;font-size:18px}
  .loading-spinner{display:inline-block;width:40px;height:40px;border:4px solid #f3f3f3;border-top:4px solid #667eea;border-radius:50%;animation:spin 1s linear infinite;margin-bottom:20px}
  @keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
  .error{color:#dc3545;background:#ffe6e6;padding:15px;border-radius:8px;border-left:4px solid #dc3545}
  .success{color:#28a745;background:#e6f9e6;padding:15px;border-radius:8px;border-left:4px solid #28a745}
  @media(max-width:600px){.lbl{flex:0 0 100px;font-size:13px}.val{min-width:80px;font-size:13px}}
</style>
</head>
<body>
  <div class="header">
    <h1>🏠 Zarlar Dashboard</h1>
    <p>Lokaal Netwerk Monitoring - ESP32 Hosted</p>
  </div>
  
  <div class="container">
    
    <!-- Particle Photon Controllers -->
    <div class="section">
      <h2>☁️ Particle Photon Controllers</h2>
      <div class="buttons">
        <button class="btn btn-photon" onclick="goPhoton('30002c000547343233323032','BandB')">BandB</button>
        <button class="btn btn-photon" onclick="goPhoton('5600420005504b464d323520','Badkamer')">Badkamer</button>
        <button class="btn btn-photon" onclick="goPhoton('420035000e47343432313031','Inkom')">Inkom</button>
        <button class="btn btn-photon" onclick="goPhoton('310017001647373335333438','Keuken')">Keuken</button>
        <button class="btn btn-photon" onclick="goPhoton('33004f000e504b464d323520','Wasplaats')">Wasplaats</button>
        <button class="btn btn-photon" onclick="goPhoton('210042000b47343432313031','Eetplaats')">Eetplaats</button>
        <button class="btn btn-photon" onclick="goPhoton('410038000547353138383138','Zitplaats')">Zitplaats</button>
        <button class="btn btn-photon" onclick="goPhoton('200033000547373336323230','TESTROOM Photon')">TESTROOM (Photon)</button>
      </div>
    </div>
    
    <!-- ESP32 Controllers -->
    <div class="section">
      <h2>🔧 ESP32 Controllers</h2>
      <div class="buttons">
        <button class="btn btn-esp" onclick="goESP32('http://ECO_IP/json','ECO Boiler')">🌞 ECO Boiler</button>
        <button class="btn btn-esp" onclick="goESP32('http://TESTROOM_IP/json','TESTROOM ESP32')">🏠 TESTROOM ESP32</button>
        <button class="btn btn-hvac" onclick="goESP32('http://HVAC_IP/log_data','HVAC')">🔥 HVAC</button>
      </div>
    </div>
    
    <!-- Display Box -->
    <div class="section">
      <h2>📊 Data</h2>
      <div id="box">
        <div class="loading">
          <div class="loading-spinner"></div><br>
          Kies een controller hierboven
        </div>
      </div>
    </div>
    
  </div>

<script>
// ============================================================================
// PARTICLE API
// ============================================================================
var PARTICLE_TOKEN = null;

async function getToken() {
  if (PARTICLE_TOKEN) return PARTICLE_TOKEN;
  try {
    var res = await fetch('https://controllers-diagnose.filip-delannoy.workers.dev/token');
    var data = await res.json();
    PARTICLE_TOKEN = data.token;
    return PARTICLE_TOKEN;
  } catch (e) {
    console.error("Token fetch failed:", e);
    return null;
  }
}

async function goPhoton(deviceId, name) {
  var box = document.getElementById('box');
  box.innerHTML = '<div class="loading"><div class="loading-spinner"></div><br>Laden ' + name + '...</div>';
  
  var token = await getToken();
  if (!token) {
    box.innerHTML = '<div class="error">❌ Geen toegang tot Particle token</div>';
    return;
  }
  
  try {
    var resp = await fetch('https://api.particle.io/v1/devices/' + deviceId + '/JSON_status?access_token=' + token, {cache:'no-store'});
    if (!resp.ok) throw new Error('HTTP ' + resp.status);
    var d = await resp.json();
    var parsed = safeParseResult(d.result || (d.body && d.body.result) || null);
    if (!parsed) throw new Error('Kan JSON niet parsen');
    displayData(box, name, parsed);
  } catch(err) {
    console.error('Photon error:', err);
    box.innerHTML = '<div class="error">❌ Fout: ' + err.message + '</div>';
  }
}

function safeParseResult(resultField) {
  if (resultField === undefined || resultField === null) return null;
  if (typeof resultField === 'object') return resultField;
  if (typeof resultField === 'string') {
    try { return JSON.parse(resultField); }
    catch(e) { 
      try { return JSON.parse(decodeURIComponent(resultField)); } 
      catch(e2) { return null; } 
    }
  }
  return null;
}

// ============================================================================
// ESP32 API
// ============================================================================
async function goESP32(url, name) {
  var box = document.getElementById('box');
  box.innerHTML = '<div class="loading"><div class="loading-spinner"></div><br>Laden ' + name + '...</div>';
  
  try {
    var resp = await fetch(url, {cache: 'no-store'});
    if (!resp.ok) throw new Error('HTTP ' + resp.status);
    var data = await resp.json();
    displayData(box, name, data);
  } catch(err) {
    console.error('ESP32 error:', err);
    box.innerHTML = '<div class="error">❌ Fout: ' + err.message + '<br><small>Zorg dat je op het thuisnetwerk bent</small></div>';
  }
}

// ============================================================================
// DISPLAY
// ============================================================================
function displayData(boxElement, name, data) {
  var html = '<div class="success">✓ ' + name + ' - Verbonden</div>';
  html += '<div style="margin-top:20px">';
  
  for (var key in data) {
    var value = data[key];
    if (typeof value === 'object') {
      try { value = JSON.stringify(value); }
      catch(e) { value = 'Object'; }
    }
    html += '<div class="row"><span class="lbl">' + key + ':</span><span class="val">' + value + '</span></div>';
  }
  
  html += '</div>';
  boxElement.innerHTML = html;
}

// Auto-refresh elke 30 seconden (optioneel - kan je uitzetten)
// setInterval(function() {
//   var lastButton = document.querySelector('.btn:focus');
//   if (lastButton) lastButton.click();
// }, 30000);
</script>
</body>
</html>
)rawliteral";

  // Vervang IP placeholders met echte IPs
  html.replace("ECO_IP", ECO_IP);
  html.replace("TESTROOM_IP", TESTROOM_IP);
  html.replace("HVAC_IP", HVAC_IP);
  
  return html;
}
