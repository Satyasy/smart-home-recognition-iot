/**
 * ESP32-CAM Smart Door Lock - Dashboard Ready
 * 
 * Fungsi:
 * 1. Live MJPEG stream untuk dashboard (port 80/stream)
 * 2. Face recognition endpoint untuk dashboard (/recognize)
 * 3. Status endpoint untuk connection test (/status)
 * 4. Komunikasi dengan ESP8266 untuk unlock door
 * 
 * Dashboard akan:
 * - Polling /status setiap 2 detik (connection test)
 * - Display stream dari /stream
 * - Trigger recognition via POST /recognize
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

// ===== WiFi Configuration =====
const char* ssid = "YOUR_WIFI_SSID";        // ← GANTI dengan WiFi Anda
const char* password = "YOUR_WIFI_PASSWORD"; // ← GANTI dengan password WiFi

// Static IP Configuration
IPAddress local_IP(192, 168, 5, 86);     // ESP32-CAM IP
IPAddress gateway(192, 168, 5, 1);       // Router IP
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// ===== Server Configuration =====
const char* flaskServerUrl = "http://192.168.5.221:5000/api/recognize";
const char* esp8266Ip = "http://192.168.5.250";

// ===== Camera Model: AI-Thinker ESP32-CAM =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== Flash LED Pin =====
#define FLASH_LED_PIN     4  // GPIO 4 for flash LED

// ===== Web Server =====
WebServer server(80);

// ===== Camera Stream Handler =====
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ===== Global Variables =====
int flashBrightness = 200; // 0-255, default bright untuk better recognition
String lastRecognitionResult = "No recognition yet";
unsigned long lastRecognitionTime = 0;

// ===== Auto Recognition =====
bool autoRecognitionEnabled = true;
unsigned long lastAutoRecognitionTime = 0;
const unsigned long AUTO_RECOGNITION_INTERVAL = 5000; // 5 seconds
const float AUTO_RECOGNITION_THRESHOLD = 0.50; // 50% confidence

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=============================================");
  Serial.println("ESP32-CAM Smart Door Lock - Dashboard Ready");
  Serial.println("=============================================\n");
  
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Initialize flash LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  Serial.println("[OK] Flash LED initialized!");
  
  // Initialize camera
  if (initCamera()) {
    Serial.println("[OK] Camera initialized!");
  } else {
    Serial.println("[ERROR] Camera init failed!");
    while(1) {
      delay(1000);
    }
  }
  
  // Connect WiFi with static IP
  connectWiFi();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] Cannot start without WiFi!");
    while(1) {
      delay(1000);
    }
  }
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/recognize", HTTP_POST, handleRecognizeFace);
  server.on("/flash", HTTP_GET, handleFlashControl);
  
  // CORS preflight
  server.on("/capture", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_OPTIONS, handleOptions);
  server.on("/recognize", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("[OK] HTTP server started");
  
  Serial.println("\n=============================================");
  Serial.println("[READY] ESP32-CAM is ready!");
  Serial.println("=============================================");
  Serial.printf("Stream URL: http://%s/stream\n", WiFi.localIP().toString().c_str());
  Serial.printf("Status URL: http://%s/status\n", WiFi.localIP().toString().c_str());
  Serial.printf("Recognize URL: http://%s/recognize\n", WiFi.localIP().toString().c_str());
  Serial.println("=============================================\n");
}

void loop() {
  server.handleClient();
  
  // Auto-recognition setiap 5 detik
  if (autoRecognitionEnabled && (millis() - lastAutoRecognitionTime >= AUTO_RECOGNITION_INTERVAL)) {
    lastAutoRecognitionTime = millis();
    performAutoRecognition();
  }
  
  delay(10);
}

// ===== Camera Initialization =====
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Quality settings (optimized for stable streaming & recognition)
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;  // 320x240 - smaller, more stable
    config.jpeg_quality = 10;  // Better quality for recognition
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    Serial.println("[CAMERA] PSRAM found - using high quality");
  } else {
    config.frame_size = FRAMESIZE_QVGA;  // 320x240
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("[CAMERA] No PSRAM - using standard quality");
  }
  
  // Init camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAMERA] Init failed: 0x%x\n", err);
    return false;
  }
  
  // Camera settings - optimized for face recognition
  sensor_t* s = esp_camera_sensor_get();
  if (s == NULL) {
    Serial.println("[CAMERA] Sensor get failed");
    return false;
  }
  
  s->set_brightness(s, 1);     // Slight brightness increase
  s->set_contrast(s, 1);       // Slight contrast increase
  s->set_saturation(s, 0);     // Normal saturation
  s->set_whitebal(s, 1);       // Auto white balance ON
  s->set_awb_gain(s, 1);       // Auto white balance gain ON
  s->set_wb_mode(s, 0);        // Auto white balance mode
  s->set_exposure_ctrl(s, 1);  // Auto exposure ON
  s->set_aec2(s, 1);           // Auto exposure algorithm ON
  s->set_gain_ctrl(s, 1);      // Auto gain ON
  s->set_agc_gain(s, 0);       // Auto gain ceiling
  s->set_gainceiling(s, (gainceiling_t)2);  // Gain ceiling 4x
  s->set_bpc(s, 1);            // Black pixel correction ON
  s->set_wpc(s, 1);            // White pixel correction ON
  s->set_raw_gma(s, 1);        // Gamma correction ON
  s->set_lenc(s, 1);           // Lens correction ON
  s->set_hmirror(s, 0);        // Horizontal mirror OFF
  s->set_vflip(s, 0);          // Vertical flip OFF
  s->set_dcw(s, 1);            // Downscale ON
  s->set_colorbar(s, 0);       // Color bar OFF
  
  return true;
}

// ===== WiFi Connection with Static IP =====
void connectWiFi() {
  Serial.println("[WIFI] Configuring static IP...");
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("[WIFI] Static IP configuration failed");
  }
  
  Serial.print("[WIFI] Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false); // Disable WiFi sleep for better performance
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected!");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n[ERROR] WiFi connection failed!");
    Serial.println("[ERROR] Please check SSID and password!");
  }
}

// ===== CORS Helper =====
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ===== Web Handlers =====
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Dashboard Ready</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial; 
            text-align: center; 
            margin: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container {
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            padding: 20px;
            border-radius: 15px;
            max-width: 800px;
            margin: 0 auto;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
        }
        img { 
            max-width: 100%; 
            border: 2px solid white;
            border-radius: 10px;
            box-shadow: 0 4px 16px rgba(0,0,0,0.3);
        }
        button { 
            padding: 12px 24px; 
            font-size: 16px; 
            margin: 10px;
            background: rgba(33, 150, 243, 0.8);
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s;
        }
        button:hover {
            background: rgba(33, 150, 243, 1);
            transform: translateY(-2px);
        }
        .status {
            background: rgba(76, 175, 80, 0.3);
            padding: 15px;
            border-radius: 10px;
            margin: 15px 0;
            border: 2px solid rgba(76, 175, 80, 0.5);
        }
        .info {
            background: rgba(255,255,255,0.1);
            padding: 15px;
            border-radius: 10px;
            margin: 15px 0;
            text-align: left;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📷 ESP32-CAM Dashboard Ready</h1>
        <div class="status">✅ Camera Online & Connected</div>
        
        <h2>🎥 Live Stream</h2>
        <img src="/stream" alt="Camera Stream">
        
        <div>
            <button onclick="location.reload()">🔄 Refresh</button>
            <button onclick="testRecognition()">🔍 Test Recognition</button>
        </div>
        
        <div class="info">
            <h3>System Information</h3>
            <p><strong>IP:</strong> )rawliteral";
  html += WiFi.localIP().toString();
  html += R"rawliteral(</p>
            <p><strong>Stream:</strong> /stream</p>
            <p><strong>Status:</strong> /status (for dashboard polling)</p>
            <p><strong>Recognize:</strong> POST /recognize (for face recognition)</p>
            <p><strong>Last Result:</strong> )rawliteral";
  html += lastRecognitionResult;
  html += R"rawliteral(</p>
        </div>
    </div>
    
    <script>
        function testRecognition() {
            fetch('/recognize', {method: 'POST'})
                .then(r => r.json())
                .then(data => {
                    if (data.recognized) {
                        alert('✅ Recognized: ' + data.name + ' (' + (data.confidence * 100).toFixed(1) + '%)');
                    } else {
                        alert('❌ Face not recognized');
                    }
                })
                .catch(e => alert('Error: ' + e));
        }
    </script>
</body>
</html>
)rawliteral";
  
  enableCORS();
  server.send(200, "text/html", html);
}

void handleStream() {
  WiFiClient client = server.client();
  
  // CORS headers for browser access
  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.printf("Content-Type: %s\r\n", _STREAM_CONTENT_TYPE);
  client.println("Connection: close");
  client.println("Cache-Control: no-cache");
  client.println();
  
  Serial.println("[STREAM] Client connected");
  
  unsigned long lastFrameTime = 0;
  const unsigned long frameDelay = 100; // 10 fps for stability
  
  while (client.connected()) {
    // Frame rate control
    if (millis() - lastFrameTime < frameDelay) {
      delay(5);
      continue;
    }
    lastFrameTime = millis();
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[ERROR] Failed to get frame");
      delay(100);
      continue;
    }
    
    // Check frame size
    if (fb->len > 30000) {
      Serial.println("[WARNING] Frame too large, skipping");
      esp_camera_fb_return(fb);
      continue;
    }
    
    // Send MJPEG frame in chunks
    client.print(_STREAM_BOUNDARY);
    client.printf(_STREAM_PART, fb->len);
    
    // Write in smaller chunks (2KB at a time)
    const size_t chunkSize = 2048;
    size_t remaining = fb->len;
    size_t offset = 0;
    bool writeSuccess = true;
    
    while (remaining > 0 && client.connected()) {
      size_t toWrite = (remaining > chunkSize) ? chunkSize : remaining;
      size_t written = client.write(fb->buf + offset, toWrite);
      
      if (written != toWrite) {
        writeSuccess = false;
        break;
      }
      
      offset += written;
      remaining -= written;
      delay(1);
    }
    
    esp_camera_fb_return(fb);
    
    if (!writeSuccess || !client.connected()) {
      break;
    }
  }
  
  Serial.println("[STREAM] Client disconnected");
}

void handleCapture() {
  Serial.println("[MANUAL] Capture requested");
  
  enableCORS();
  
  // Turn on flash for capture
  analogWrite(FLASH_LED_PIN, flashBrightness);
  delay(100);
  
  camera_fb_t* fb = esp_camera_fb_get();
  
  // Turn off flash
  digitalWrite(FLASH_LED_PIN, LOW);
  
  if (!fb) {
    server.send(500, "application/json", "{\"success\":false,\"message\":\"Camera capture failed\"}");
    return;
  }
  
  // Encode to base64
  String imageBase64 = base64::encode(fb->buf, fb->len);
  
  Serial.printf("[MANUAL] Image captured: %d bytes (base64: %d)\n", fb->len, imageBase64.length());
  
  // Return captured image as base64
  DynamicJsonDocument doc(imageBase64.length() + 200);
  doc["success"] = true;
  doc["message"] = "Image captured successfully";
  doc["image"] = imageBase64;
  doc["size"] = fb->len;
  doc["width"] = fb->width;
  doc["height"] = fb->height;
  
  String response;
  serializeJson(doc, response);
  
  esp_camera_fb_return(fb);
  
  server.send(200, "application/json", response);
}

// ===== Status Endpoint (untuk dashboard polling) =====
void handleStatus() {
  enableCORS();
  
  DynamicJsonDocument doc(512);
  doc["status"] = "online";
  doc["device"] = "ESP32-CAM";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000; // seconds
  doc["stream_url"] = "http://" + WiFi.localIP().toString() + "/stream";
  doc["free_heap"] = ESP.getFreeHeap();
  doc["camera"] = "ready";
  doc["last_recognition"] = lastRecognitionResult;
  doc["flash_brightness"] = flashBrightness;
  
  String response;
  serializeJson(doc, response);
  
  Serial.println("[HTTP] GET /status - Dashboard polling");
  server.send(200, "application/json", response);
}

void handleFlashControl() {
  enableCORS();
  
  if (server.hasArg("brightness")) {
    int brightness = server.arg("brightness").toInt();
    if (brightness >= 0 && brightness <= 255) {
      flashBrightness = brightness;
      
      DynamicJsonDocument doc(128);
      doc["success"] = true;
      doc["brightness"] = flashBrightness;
      doc["message"] = "Flash brightness set to " + String(flashBrightness);
      
      String response;
      serializeJson(doc, response);
      
      server.send(200, "application/json", response);
      Serial.printf("[FLASH] Brightness set to: %d\n", flashBrightness);
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"Brightness must be 0-255\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing brightness parameter\"}");
  }
}

void handleOptions() {
  enableCORS();
  server.send(204);
}

// ===== Handle Face Recognition dari Dashboard =====
void handleRecognizeFace() {
  enableCORS();
  
  Serial.println("[RECOGNIZE] Face recognition requested from dashboard");
  
  // Turn on flash for capture
  analogWrite(FLASH_LED_PIN, flashBrightness);
  delay(100);
  
  camera_fb_t* fb = esp_camera_fb_get();
  
  // Turn off flash
  digitalWrite(FLASH_LED_PIN, LOW);
  
  if (!fb) {
    Serial.println("[ERROR] Camera capture failed");
    server.send(500, "application/json", "{\"success\":false,\"message\":\"Camera capture failed\"}");
    return;
  }
  
  Serial.printf("[RECOGNIZE] Image captured: %d bytes\n", fb->len);
  
  // Send to Flask for recognition
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(flaskServerUrl);
    http.addHeader("Content-Type", "image/jpeg");
    http.setTimeout(15000); // 15 seconds
    
    Serial.println("[RECOGNIZE] Sending to Flask backend...");
    int httpResponseCode = http.POST(fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("[RECOGNIZE] Flask response (%d): %s\n", httpResponseCode, response.c_str());
      
      DynamicJsonDocument responseDoc(1024);
      if (deserializeJson(responseDoc, response) == DeserializationError::Ok) {
        bool recognized = responseDoc["recognized"] | false;
        String name = responseDoc["name"] | "Unknown";
        float confidence = responseDoc["confidence"] | 0.0;
        
        if (recognized) {
          Serial.printf("[SUCCESS] ✅ Face Recognized: %s (%.2f%%)\n", name.c_str(), confidence * 100);
          lastRecognitionResult = name + " (" + String((int)(confidence * 100)) + "%)";
          lastRecognitionTime = millis();
          
          // Send unlock command to ESP8266
          sendUnlockToESP8266(name, confidence);
          
          // Return success to dashboard
          DynamicJsonDocument resultDoc(512);
          resultDoc["success"] = true;
          resultDoc["recognized"] = true;
          resultDoc["name"] = name;
          resultDoc["confidence"] = confidence;
          resultDoc["message"] = "Face recognized: " + name;
          
          String resultJson;
          serializeJson(resultDoc, resultJson);
          server.send(200, "application/json", resultJson);
        } else {
          Serial.println("[DENIED] ❌ Face not recognized");
          lastRecognitionResult = "Unknown face";
          
          // Return failure to dashboard
          DynamicJsonDocument resultDoc(512);
          resultDoc["success"] = true;
          resultDoc["recognized"] = false;
          resultDoc["message"] = "Face not recognized";
          
          String resultJson;
          serializeJson(resultDoc, resultJson);
          server.send(200, "application/json", resultJson);
        }
      } else {
        Serial.println("[ERROR] Failed to parse Flask response");
        lastRecognitionResult = "Parse error";
        server.send(500, "application/json", "{\"success\":false,\"message\":\"Failed to parse response\"}");
      }
    } else {
      Serial.printf("[ERROR] Flask HTTP error: %d\n", httpResponseCode);
      lastRecognitionResult = "Connection error";
      server.send(500, "application/json", "{\"success\":false,\"message\":\"Failed to connect to Flask backend\"}");
    }
    
    http.end();
  } else {
    esp_camera_fb_return(fb);
    server.send(500, "application/json", "{\"success\":false,\"message\":\"WiFi not connected\"}");
  }
}

// ===== Auto Recognition Function =====
void performAutoRecognition() {
  Serial.println("[AUTO] Starting auto face recognition...");
  
  // Capture image with flash
  analogWrite(FLASH_LED_PIN, flashBrightness);
  delay(100);
  
  camera_fb_t* fb = esp_camera_fb_get();
  digitalWrite(FLASH_LED_PIN, LOW);
  
  if (!fb) {
    Serial.println("[AUTO] Failed to capture image");
    return;
  }
  
  Serial.printf("[AUTO] Image captured: %d bytes\n", fb->len);
  
  // Send to Flask for recognition
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(flaskServerUrl);
    http.addHeader("Content-Type", "image/jpeg");
    http.setTimeout(10000); // 10 seconds
    
    int httpResponseCode = http.POST(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      
      DynamicJsonDocument responseDoc(1024);
      if (deserializeJson(responseDoc, response) == DeserializationError::Ok) {
        bool recognized = responseDoc["recognized"] | false;
        String name = responseDoc["name"] | "Unknown";
        float confidence = responseDoc["confidence"] | 0.0;
        
        // Check if confidence meets threshold (50%)
        if (recognized && confidence >= AUTO_RECOGNITION_THRESHOLD) {
          Serial.printf("[AUTO] ✅ Face Recognized: %s (%.2f%%)\n", name.c_str(), confidence * 100);
          lastRecognitionResult = name + " (" + String((int)(confidence * 100)) + "%)";
          lastRecognitionTime = millis();
          
          // Send unlock command
          sendUnlockToESP8266(name, confidence);
        } else if (recognized) {
          Serial.printf("[AUTO] ⚠️ Face detected but confidence too low: %s (%.2f%% < 50%%)\n", name.c_str(), confidence * 100);
        } else {
          Serial.println("[AUTO] ❌ No face recognized");
        }
      }
    } else {
      Serial.printf("[AUTO] Flask HTTP error: %d\n", httpResponseCode);
    }
    
    http.end();
  }
}

// ===== Send Unlock Command to ESP8266 =====
void sendUnlockToESP8266(String userName, float confidence) {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("[ESP8266] WiFi not connected, cannot send unlock");
    return;
  }
  
  HTTPClient http;
  String url = String(esp8266Ip) + "/unlock";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  
  DynamicJsonDocument doc(512);
  doc["method"] = "face";
  doc["name"] = userName;
  doc["confidence"] = confidence;
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  Serial.println("[ESP8266] Sending unlock command: " + jsonPayload);
  int httpCode = http.POST(jsonPayload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("[ESP8266] ✅ Unlock command sent (HTTP %d)\n", httpCode);
    Serial.println("[ESP8266] Response: " + response);
  } else {
    Serial.printf("[ESP8266] ❌ Failed to send unlock command: %d\n", httpCode);
  }
  
  http.end();
}
