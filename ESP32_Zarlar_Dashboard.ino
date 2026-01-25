/* ESP32 WROOM - Zarlar Dashboard Server
 * Author: Fidel Dworp
 * Date: 24 jan 2026
 * 
 * Standalone webserver voor Zarlar dashboard
 * GEEN externe libraries nodig - gebruikt standaard ESP32WebServer!
 * 
 * IP: http://192.168.1.3  mDNS: http://zarlar.local
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ============== CONFIGURATIE ==============
// PAS DEZE AAN VOOR JOUW NETWERK!
const char* WIFI_SSID = "Delannoy";        // ← AANPASSEN!
const char* WIFI_PASS = "kampendaal,34";  // ← AANPASSEN!
const char* MDNS_NAME = "zarlar";          // → http://zarlar.local/

// ESP32 Controller IPs (pas aan indien nodig)
const char* ECO_IP = "192.168.1.99";
const char* TESTROOM_IP = "192.168.1.101";
const char* HVAC_IP = "192.168.1.100";

// ============== GLOBALS ==============
WebServer server(80);

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔══════════════════════════════╗");
  Serial.println("║  ESP32 Zarlar Dashboard v1.0 ║");
  Serial.println("║  24 januari 2026             ║");
  Serial.println("╚══════════════════════════════╝\n");
  
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
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi verbonden!");
    Serial.print("  IP adres: ");
    Serial.println(WiFi.localIP());
    Serial.print("  RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // mDNS
    if (MDNS.begin(MDNS_NAME)) {
      Serial.print("  mDNS: http://");
      Serial.print(MDNS_NAME);
      Serial.println(".local/");
    }
  } else {
    Serial.println("✗ WiFi verbinding mislukt!");
    Serial.println("  Check SSID en wachtwoord in sketch!");
    Serial.println("  Restart ESP32 na aanpassen...");
    while(1) delay(1000); // Stop hier
  }
  
  // Web server setup
  setupWebServer();
  server.begin();
  
  Serial.println("\n╔══════════════════════════════╗");
  Serial.println("║      DASHBOARD READY!        ║");
  Serial.println("╚══════════════════════════════╝");
  Serial.println("\nOpen in browser:");
  Serial.print("  → http://");
  Serial.println(WiFi.localIP());
  Serial.print("  → http://");
  Serial.print(MDNS_NAME);
  Serial.println(".local/\n");
}

// ============== LOOP ==============
void loop() {
  server.handleClient();
  delay(2);
}

// ============== WEB SERVER ==============
void setupWebServer() {
  
  // Root endpoint - Zarlar Dashboard
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getZarlarHTML());
  });
  
  // Info endpoint (voor debugging)
  server.on("/info", HTTP_GET, []() {
    String info = "ESP32 Zarlar Dashboard v1.0\n";
    info += "═══════════════════════════\n";
    info += "IP: " + WiFi.localIP().toString() + "\n";
    info += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    info += "Uptime: " + String(millis()/1000) + " sec\n";
    info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    server.send(200, "text/plain", info);
  });
  
  // 404 handler
  server.onNotFound([]() {
    server.send(404, "text/plain", "404 - Not Found");
  });
}

// ============== HTML GENERATOR ==============
String getZarlarHTML() {
  String html = R"rawliteral(<!DOCTYPE html>
<html lang="nl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Zarlar Dashboard</title>
<style>
* {box-sizing:border-box;margin:0;padding:0}
body {font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;min-height:100vh}
.header {text-align:center;padding:30px 20px;background:rgba(0,0,0,0.2)}
.header h1 {font-size:32px;text-shadow:0 2px 4px rgba(0,0,0,0.3);margin-bottom:10px}
.header p {opacity:0.9;font-size:14px}
.container {max-width:900px;margin:0 auto;padding:20px}
.section {background:rgba(255,255,255,0.95);border-radius:12px;padding:20px;margin-bottom:20px;color:#222;box-shadow:0 4px 12px rgba(0,0,0,0.15)}
.section h2 {margin:0 0 15px 0;color:#667eea;font-size:20px;border-bottom:2px solid #667eea;padding-bottom:10px}
.buttons {display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}
.btn {padding:15px;border:none;border-radius:8px;font-size:15px;font-weight:600;cursor:pointer;transition:all 0.2s;color:#fff}
.btn:hover {transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,0.2)}
.btn:active {transform:translateY(0)}
.btn-photon {background:#007bff}
.btn-photon:hover {background:#0056b3}
.btn-esp {background:#28a745}
.btn-esp:hover {background:#218838}
.btn-hvac {background:#ff6600}
.btn-hvac:hover {background:#e65c00}
#box {background:#fff;border-radius:8px;padding:20px;min-height:200px}
.row {display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid #eee}
.row:last-child {border-bottom:none}
.lbl {font-weight:600;color:#667eea;flex:0 0 140px}
.val {font-family:monospace;font-weight:700;padding:6px 12px;border-radius:6px;background:#f0f0f0;text-align:center;min-width:100px}
.loading {text-align:center;padding:60px 20px;color:#999;font-size:16px}
.spinner {display:inline-block;width:40px;height:40px;border:4px solid #f3f3f3;border-top:4px solid #667eea;border-radius:50%;animation:spin 1s linear infinite;margin-bottom:20px}
@keyframes spin {0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
.error {color:#dc3545;background:#ffe6e6;padding:15px;border-radius:8px;border-left:4px solid #dc3545}
.success {color:#28a745;background:#e6f9e6;padding:15px;border-radius:8px;border-left:4px solid #28a745;margin-bottom:15px}
@media(max-width:600px){.lbl{flex:0 0 100px;font-size:13px}.val{min-width:80px;font-size:13px}.buttons{grid-template-columns:1fr}}
</style>
</head>
<body>
<div class="header">
<h1>🏠 Zarlar Dashboard</h1>
<p>Lokaal Netwerk Monitoring - ESP32 Hosted</p>
</div>

<div class="container">

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
<button class="btn btn-photon" onclick="goPhoton('200033000547373336323230','TESTROOM')">TESTROOM (Photon)</button>
</div>
</div>

<div class="section">
<h2>🔧 ESP32 Controllers</h2>
<div class="buttons">
<button class="btn btn-esp" onclick="goESP32('http://ECO_IP/json','🌞 ECO Boiler')">🌞 ECO Boiler</button>
<button class="btn btn-esp" onclick="goESP32('http://TESTROOM_IP/json','🏠 TESTROOM')">🏠 TESTROOM ESP32</button>
<button class="btn btn-hvac" onclick="goESP32('http://HVAC_IP/log_data','🔥 HVAC')">🔥 HVAC</button>
</div>
</div>

<div class="section">
<h2>📊 Controller Data</h2>
<div id="box">
<div class="loading">
<div class="spinner"></div><br>
Kies een controller hierboven
</div>
</div>
</div>

</div>

<script>
var TOKEN=null;
async function getToken(){
if(TOKEN)return TOKEN;
try{
var r=await fetch('https://controllers-diagnose.filip-delannoy.workers.dev/token');
var d=await r.json();
TOKEN=d.token;
return TOKEN;
}catch(e){console.error("Token error:",e);return null}
}

async function goPhoton(id,name){
var box=document.getElementById('box');
box.innerHTML='<div class="loading"><div class="spinner"></div><br>Laden '+name+'...</div>';
var token=await getToken();
if(!token){box.innerHTML='<div class="error">❌ Geen Particle token</div>';return}
try{
var r=await fetch('https://api.particle.io/v1/devices/'+id+'/JSON_status?access_token='+token,{cache:'no-store'});
if(!r.ok)throw new Error('HTTP '+r.status);
var d=await r.json();
var p=parseResult(d.result||(d.body&&d.body.result)||null);
if(!p)throw new Error('Parse error');
show(box,name,p);
}catch(e){
console.error('Error:',e);
box.innerHTML='<div class="error">❌ Fout: '+e.message+'</div>';
}
}

function parseResult(r){
if(!r)return null;
if(typeof r==='object')return r;
if(typeof r==='string'){
try{return JSON.parse(r)}catch(e){
try{return JSON.parse(decodeURIComponent(r))}catch(e2){return null}
}
}
return null;
}

async function goESP32(url,name){
var box=document.getElementById('box');
box.innerHTML='<div class="loading"><div class="spinner"></div><br>Laden '+name+'...</div>';
try{
var r=await fetch(url,{cache:'no-store'});
if(!r.ok)throw new Error('HTTP '+r.status);
var d=await r.json();
show(box,name,d);
}catch(e){
console.error('Error:',e);
box.innerHTML='<div class="error">❌ Fout: '+e.message+'<br><small>Check netwerk verbinding</small></div>';
}
}

function show(box,name,data){
var h='<div class="success">✓ '+name+' - Verbonden</div>';
for(var k in data){
var v=data[k];
if(typeof v==='object'){try{v=JSON.stringify(v)}catch(e){v='Object'}}
h+='<div class="row"><span class="lbl">'+k+':</span><span class="val">'+v+'</span></div>';
}
box.innerHTML=h;
}
</script>
</body>
</html>
)rawliteral";

  // Vervang IP placeholders
  html.replace("ECO_IP", ECO_IP);
  html.replace("TESTROOM_IP", TESTROOM_IP);
  html.replace("HVAC_IP", HVAC_IP);
  
  return html;
}
