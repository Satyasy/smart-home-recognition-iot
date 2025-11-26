/*
 * ESP8266 Simple Web Server Test
 * 
 * Test code untuk verify koneksi ESP8266 dengan dashboard
 * Upload code ini untuk test dasar sebelum implementasi full sensor
 * 
 * Hardware: ESP8266 NodeMCU
 * Port: 80 (HTTP)
 * 
 * Endpoints yang disediakan:
 * - GET /sensor      : Return mock sensor data
 * - GET /status      : Return mock actuator status
 * - POST /unlock     : Simulate unlock door
 * - POST /lock       : Simulate lock door
 * - POST /led        : Control LED simulation
 * - POST /buzzer     : Control buzzer simulation
 * - POST /lcd        : Update LCD text simulation
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ===== WiFi Configuration =====
const char* ssid = "YOUR_WIFI_SSID";        // Ganti dengan SSID WiFi Anda
const char* password = "YOUR_WIFI_PASSWORD"; // Ganti dengan password WiFi

// ===== Static IP Configuration (Optional) =====
IPAddress local_IP(192, 168, 5, 250);  // IP yang diinginkan
IPAddress gateway(192, 168, 5, 1);     // Gateway router
IPAddress subnet(255, 255, 255, 0);    // Subnet mask

// ===== Web Server =====
ESP8266WebServer server(80);

// ===== Simulated State =====
struct {
  float temperature = 25.5;
  float humidity = 65.0;
  int light = 450;
  float distance = 15.5;
} sensorData;

struct {
  int servoAngle = 0;
  bool doorLocked = true;
  bool ledRed = true;
  bool ledGreen = false;
  bool buzzerActive = false;
  String lcdLine1 = "Door LOCKED";
  String lcdLine2 = "Waiting...";
} actuatorState;

// ===== CORS Headers =====
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ===== OPTIONS Handler (untuk CORS preflight) =====
void handleOptions() {
  enableCORS();
  server.send(204); // No content
}

// ===== GET /sensor =====
void handleSensor() {
  StaticJsonDocument<256> doc;
  
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["light"] = sensorData.light;
  doc["distance"] = sensorData.distance;
  
  String json;
  serializeJson(doc, json);
  
  enableCORS();
  server.send(200, "application/json", json);
  
  Serial.println("[HTTP] GET /sensor - 200 OK");
}

// ===== GET /status =====
void handleStatus() {
  StaticJsonDocument<512> doc;
  
  doc["servo"]["angle"] = actuatorState.servoAngle;
  doc["servo"]["locked"] = actuatorState.doorLocked;
  
  doc["led"]["red"] = actuatorState.ledRed;
  doc["led"]["green"] = actuatorState.ledGreen;
  
  doc["buzzer"]["active"] = actuatorState.buzzerActive;
  
  doc["lcd"]["line1"] = actuatorState.lcdLine1;
  doc["lcd"]["line2"] = actuatorState.lcdLine2;
  
  String json;
  serializeJson(doc, json);
  
  enableCORS();
  server.send(200, "application/json", json);
  
  Serial.println("[HTTP] GET /status - 200 OK");
}

// ===== POST /unlock =====
void handleUnlock() {
  // Simulate unlock
  actuatorState.doorLocked = false;
  actuatorState.servoAngle = 90;
  actuatorState.ledRed = false;
  actuatorState.ledGreen = true;
  actuatorState.lcdLine1 = "Door UNLOCKED";
  
  StaticJsonDocument<256> doc;
  doc["success"] = true;
  doc["message"] = "Door unlocked";
  doc["servo"]["angle"] = actuatorState.servoAngle;
  doc["servo"]["locked"] = actuatorState.doorLocked;
  
  String json;
  serializeJson(doc, json);
  
  enableCORS();
  server.send(200, "application/json", json);
  
  Serial.println("[HTTP] POST /unlock - Door UNLOCKED");
}

// ===== POST /lock =====
void handleLock() {
  // Simulate lock
  actuatorState.doorLocked = true;
  actuatorState.servoAngle = 0;
  actuatorState.ledRed = true;
  actuatorState.ledGreen = false;
  actuatorState.lcdLine1 = "Door LOCKED";
  actuatorState.lcdLine2 = "Waiting...";
  
  StaticJsonDocument<256> doc;
  doc["success"] = true;
  doc["message"] = "Door locked";
  doc["servo"]["angle"] = actuatorState.servoAngle;
  doc["servo"]["locked"] = actuatorState.doorLocked;
  
  String json;
  serializeJson(doc, json);
  
  enableCORS();
  server.send(200, "application/json", json);
  
  Serial.println("[HTTP] POST /lock - Door LOCKED");
}

// ===== POST /led =====
void handleLED() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error) {
      if (doc.containsKey("red")) {
        actuatorState.ledRed = doc["red"];
      }
      if (doc.containsKey("green")) {
        actuatorState.ledGreen = doc["green"];
      }
      
      StaticJsonDocument<256> response;
      response["success"] = true;
      response["led"]["red"] = actuatorState.ledRed;
      response["led"]["green"] = actuatorState.ledGreen;
      
      String json;
      serializeJson(response, json);
      
      enableCORS();
      server.send(200, "application/json", json);
      
      Serial.printf("[HTTP] POST /led - Red:%d Green:%d\n", 
                    actuatorState.ledRed, actuatorState.ledGreen);
      return;
    }
  }
  
  enableCORS();
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

// ===== POST /buzzer =====
void handleBuzzer() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error && doc.containsKey("active")) {
      actuatorState.buzzerActive = doc["active"];
      
      StaticJsonDocument<256> response;
      response["success"] = true;
      response["buzzer"]["active"] = actuatorState.buzzerActive;
      
      String json;
      serializeJson(response, json);
      
      enableCORS();
      server.send(200, "application/json", json);
      
      Serial.printf("[HTTP] POST /buzzer - Active:%d\n", actuatorState.buzzerActive);
      return;
    }
  }
  
  enableCORS();
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

// ===== POST /lcd =====
void handleLCD() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error) {
      if (doc.containsKey("line1")) {
        actuatorState.lcdLine1 = doc["line1"].as<String>();
      }
      if (doc.containsKey("line2")) {
        actuatorState.lcdLine2 = doc["line2"].as<String>();
      }
      
      StaticJsonDocument<256> response;
      response["success"] = true;
      response["lcd"]["line1"] = actuatorState.lcdLine1;
      response["lcd"]["line2"] = actuatorState.lcdLine2;
      
      String json;
      serializeJson(response, json);
      
      enableCORS();
      server.send(200, "application/json", json);
      
      Serial.println("[HTTP] POST /lcd");
      Serial.printf("  Line1: %s\n", actuatorState.lcdLine1.c_str());
      Serial.printf("  Line2: %s\n", actuatorState.lcdLine2.c_str());
      return;
    }
  }
  
  enableCORS();
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

// ===== Root endpoint =====
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP8266 Test Server</title>
  <style>
    body { font-family: Arial; margin: 40px; background: #f0f0f0; }
    .container { background: white; padding: 20px; border-radius: 8px; max-width: 600px; }
    h1 { color: #333; }
    .status { padding: 10px; background: #4CAF50; color: white; border-radius: 4px; }
    .endpoint { margin: 10px 0; padding: 10px; background: #f9f9f9; border-left: 3px solid #2196F3; }
    code { background: #e0e0e0; padding: 2px 6px; border-radius: 3px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔧 ESP8266 Test Server</h1>
    <div class="status">✅ Server is running!</div>
    <h2>Available Endpoints:</h2>
    <div class="endpoint">
      <strong>GET /sensor</strong><br>
      Returns sensor data (temperature, humidity, light, distance)
    </div>
    <div class="endpoint">
      <strong>GET /status</strong><br>
      Returns actuator status (servo, LED, buzzer, LCD)
    </div>
    <div class="endpoint">
      <strong>POST /unlock</strong><br>
      Unlock door simulation
    </div>
    <div class="endpoint">
      <strong>POST /lock</strong><br>
      Lock door simulation
    </div>
    <div class="endpoint">
      <strong>POST /led</strong><br>
      Control LED: <code>{"red": true, "green": false}</code>
    </div>
    <div class="endpoint">
      <strong>POST /buzzer</strong><br>
      Control buzzer: <code>{"active": true}</code>
    </div>
    <div class="endpoint">
      <strong>POST /lcd</strong><br>
      Update LCD: <code>{"line1": "Text 1", "line2": "Text 2"}</code>
    </div>
    <p><strong>IP Address:</strong> )";
  
  html += WiFi.localIP().toString();
  html += R"(</p>
  </div>
</body>
</html>
  )";
  
  enableCORS();
  server.send(200, "text/html", html);
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=================================");
  Serial.println("ESP8266 Test Server");
  Serial.println("=================================");
  
  // Configure static IP (comment out untuk DHCP)
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP Failed to configure");
  }
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi Connected!");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("🌐 Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("📶 Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println();
    Serial.println("❌ WiFi Connection FAILED!");
    Serial.println("Please check SSID and password");
    return;
  }
  
  // Setup HTTP endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/sensor", HTTP_GET, handleSensor);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/lock", HTTP_POST, handleLock);
  server.on("/led", HTTP_POST, handleLED);
  server.on("/buzzer", HTTP_POST, handleBuzzer);
  server.on("/lcd", HTTP_POST, handleLCD);
  
  // Handle OPTIONS for CORS preflight
  server.on("/sensor", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_OPTIONS, handleOptions);
  server.on("/unlock", HTTP_OPTIONS, handleOptions);
  server.on("/lock", HTTP_OPTIONS, handleOptions);
  server.on("/led", HTTP_OPTIONS, handleOptions);
  server.on("/buzzer", HTTP_OPTIONS, handleOptions);
  server.on("/lcd", HTTP_OPTIONS, handleOptions);
  
  // Start server
  server.begin();
  Serial.println("🚀 HTTP Server Started!");
  Serial.println("=================================");
  Serial.println();
  Serial.println("Test endpoints:");
  Serial.print("  http://");
  Serial.print(WiFi.localIP());
  Serial.println("/sensor");
  Serial.print("  http://");
  Serial.print(WiFi.localIP());
  Serial.println("/status");
  Serial.println();
}

// ===== Loop =====
void loop() {
  server.handleClient();
  
  // Simulate sensor changes (optional)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    
    // Random sensor variations
    sensorData.temperature = 24.0 + random(0, 40) / 10.0;
    sensorData.humidity = 60.0 + random(0, 100) / 10.0;
    sensorData.light = 400 + random(0, 200);
    sensorData.distance = 10.0 + random(0, 200) / 10.0;
  }
}
