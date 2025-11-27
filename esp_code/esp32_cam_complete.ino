/*
 * ESP32-CAM Face Recognition Smart Door Lock
 * Complete working version with all required includes
 * 
 * Hardware: ESP32-CAM AI-Thinker
 * IP Address: 192.168.5.86
 * Ports: 80 (HTTP API), 81 (MJPEG Stream)
 * 
 * Features:
 * - MJPEG camera streaming on port 81
 * - Face recognition via Flask backend
 * - Web server with status API
 * - CORS enabled for React dashboard
 * - Commands ESP8266 for door control
 */

// ============================
// INCLUDES
// ============================
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================
// WIFI CONFIGURATION
// ============================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Static IP Configuration
IPAddress local_IP(192, 168, 5, 86);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// ============================
// SERVER CONFIGURATION
// ============================
const char* flaskServer = "http://192.168.5.221:5000";
const char* esp8266Server = "http://192.168.5.250";

WebServer server(80);        // HTTP API server
httpd_handle_t stream_httpd = NULL;  // MJPEG stream server

// ============================
// CAMERA PINS (AI-THINKER)
// ============================
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

// ============================
// GLOBAL VARIABLES
// ============================
unsigned long lastFaceRecognitionTime = 0;
const unsigned long FACE_RECOGNITION_INTERVAL = 5000; // 5 seconds

bool cameraInitialized = false;
String lastRecognitionResult = "No recognition yet";

// ============================
// CAMERA INITIALIZATION
// ============================
bool initCamera() {
  Serial.println("[CAMERA] Initializing...");
  
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
  
  // High quality for face recognition
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    Serial.println("[CAMERA] PSRAM found - using high quality");
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("[CAMERA] No PSRAM - using standard quality");
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAMERA] Init failed: 0x%x\n", err);
    return false;
  }
  
  // Get sensor settings
  sensor_t * s = esp_camera_sensor_get();
  if (s == NULL) {
    Serial.println("[CAMERA] Sensor get failed");
    return false;
  }
  
  // Sensor optimizations
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  
  Serial.println("[CAMERA] Initialized successfully");
  return true;
}

// ============================
// MJPEG STREAM HANDLER
// ============================
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  
  static int64_t last_frame = 0;
  if (!last_frame) {
    last_frame = esp_timer_get_time();
  }
  
  res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
  if (res != ESP_OK) {
    return res;
  }
  
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[STREAM] Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          Serial.println("[STREAM] JPEG compression failed");
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
    
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, 
        "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 13);
    }
    
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    
    if (res != ESP_OK) {
      break;
    }
    
    int64_t fr_end = esp_timer_get_time();
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    
    // Limit to ~10 FPS
    if (frame_time < 100) {
      delay(100 - frame_time);
    }
  }
  
  return res;
}

// ============================
// START CAMERA STREAM SERVER
// ============================
void startCameraStream() {
  Serial.println("[STREAM] Starting MJPEG stream server on port 81...");
  
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  config.ctrl_port = 32768;
  
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("[STREAM] Server started successfully");
    Serial.println("[STREAM] Access at: http://192.168.5.86:81/stream");
  } else {
    Serial.println("[STREAM] Failed to start server");
  }
}

// ============================
// FACE RECOGNITION
// ============================
void recognizeFace() {
  Serial.println("[RECOGNITION] Capturing image...");
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[RECOGNITION] Camera capture failed");
    lastRecognitionResult = "Camera error";
    return;
  }
  
  Serial.printf("[RECOGNITION] Image captured: %d bytes\n", fb->len);
  
  HTTPClient http;
  http.begin(String(flaskServer) + "/api/recognize");
  http.addHeader("Content-Type", "image/jpeg");
  http.setTimeout(10000); // 10 second timeout
  
  Serial.println("[RECOGNITION] Sending to Flask backend...");
  int httpCode = http.POST(fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
  
  if (httpCode > 0) {
    Serial.printf("[RECOGNITION] HTTP Response: %d\n", httpCode);
    String response = http.getString();
    Serial.println("[RECOGNITION] Response: " + response);
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      bool recognized = doc["recognized"];
      const char* name = doc["name"];
      float confidence = doc["confidence"];
      
      if (recognized) {
        Serial.printf("[RECOGNITION] ✓ Recognized: %s (%.2f%%)\n", name, confidence * 100);
        lastRecognitionResult = String(name) + " (" + String((int)(confidence * 100)) + "%)";
        unlockDoorViaESP8266(name);
      } else {
        Serial.println("[RECOGNITION] ✗ Face not recognized");
        lastRecognitionResult = "Unknown face";
        triggerAlertViaESP8266();
      }
    } else {
      Serial.println("[RECOGNITION] JSON parse error");
      lastRecognitionResult = "Parse error";
    }
  } else {
    Serial.printf("[RECOGNITION] HTTP Error: %s\n", http.errorToString(httpCode).c_str());
    lastRecognitionResult = "Connection error";
  }
  
  http.end();
}

// ============================
// UNLOCK DOOR VIA ESP8266
// ============================
void unlockDoorViaESP8266(const char* personName) {
  Serial.printf("[ESP8266] Sending unlock command for: %s\n", personName);
  
  HTTPClient http;
  http.begin(String(esp8266Server) + "/unlock");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<256> doc;
  doc["method"] = "face";
  doc["name"] = personName;
  
  String requestBody;
  serializeJson(doc, requestBody);
  
  Serial.println("[ESP8266] Request: " + requestBody);
  
  int httpCode = http.POST(requestBody);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("[ESP8266] Response (%d): %s\n", httpCode, response.c_str());
  } else {
    Serial.printf("[ESP8266] Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

// ============================
// TRIGGER ALERT VIA ESP8266
// ============================
void triggerAlertViaESP8266() {
  Serial.println("[ESP8266] Triggering alert...");
  
  HTTPClient http;
  http.begin(String(esp8266Server) + "/buzzer");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<128> doc;
  doc["duration"] = 500;
  
  String requestBody;
  serializeJson(doc, requestBody);
  
  int httpCode = http.POST(requestBody);
  
  if (httpCode > 0) {
    Serial.printf("[ESP8266] Alert triggered (%d)\n", httpCode);
  } else {
    Serial.printf("[ESP8266] Alert error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

// ============================
// WEB SERVER HANDLERS
// ============================
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleRoot() {
  Serial.println("[HTTP] GET / - Root page");
  
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Face Recognition</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 20px; background: #f0f0f0; }
    .container { background: white; padding: 20px; border-radius: 10px; max-width: 800px; margin: 0 auto; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .status { padding: 10px; border-radius: 5px; margin: 10px 0; }
    .status.online { background: #d4edda; color: #155724; }
    .camera-feed { width: 100%; max-width: 640px; border: 2px solid #ddd; border-radius: 5px; margin: 20px 0; }
    .button { background: #007bff; color: white; border: none; padding: 12px 24px; font-size: 16px; border-radius: 5px; cursor: pointer; margin: 5px; }
    .button:hover { background: #0056b3; }
    .info { background: #e7f3ff; padding: 15px; border-radius: 5px; margin: 10px 0; text-align: left; }
    .info p { margin: 5px 0; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎥 ESP32-CAM Face Recognition</h1>
    
    <div class="status online">
      ✓ Camera Online - Ready for Face Recognition
    </div>
    
    <h2>Live Camera Feed</h2>
    <img class="camera-feed" src="http://192.168.5.86:81/stream" alt="Camera Stream">
    
    <div>
      <button class="button" onclick="location.reload()">🔄 Refresh</button>
    </div>
    
    <div class="info">
      <h3>System Information</h3>
      <p><strong>IP Address:</strong> 192.168.5.86</p>
      <p><strong>Stream Port:</strong> 81</p>
      <p><strong>API Port:</strong> 80</p>
      <p><strong>Flask Backend:</strong> 192.168.5.221:5000</p>
      <p><strong>ESP8266 Control:</strong> 192.168.5.250</p>
      <p><strong>Last Recognition:</strong> )" + lastRecognitionResult + R"(</p>
    </div>
    
    <div class="info">
      <h3>API Endpoints</h3>
      <p><strong>GET /status</strong> - Camera status</p>
      <p><strong>POST /capture</strong> - Trigger face recognition</p>
      <p><strong>GET /stream</strong> - MJPEG stream (port 81)</p>
    </div>
  </div>
</body>
</html>
  )";
  
  enableCORS();
  server.send(200, "text/html", html);
}

void handleCapture() {
  Serial.println("[HTTP] POST /capture - Manual capture triggered");
  
  recognizeFace();
  
  StaticJsonDocument<256> doc;
  doc["success"] = true;
  doc["message"] = "Face recognition triggered";
  doc["result"] = lastRecognitionResult;
  
  String response;
  serializeJson(doc, response);
  
  enableCORS();
  server.send(200, "application/json", response);
}

void handleStatus() {
  Serial.println("[HTTP] GET /status - Status check");
  
  StaticJsonDocument<512> doc;
  doc["status"] = "online";
  doc["camera"] = cameraInitialized ? "ready" : "error";
  doc["ip"] = WiFi.localIP().toString();
  doc["stream_url"] = "http://192.168.5.86:81/stream";
  doc["last_recognition"] = lastRecognitionResult;
  doc["uptime"] = millis() / 1000;
  
  String response;
  serializeJson(doc, response);
  
  enableCORS();
  server.send(200, "application/json", response);
}

void handleOptions() {
  enableCORS();
  server.send(204);
}

void startWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_POST, handleCapture);
  server.on("/status", HTTP_GET, handleStatus);
  
  // CORS preflight
  server.on("/capture", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("[HTTP] Web server started on port 80");
  Serial.println("[HTTP] Access at: http://192.168.5.86");
}

// ============================
// SETUP
// ============================
void setup() {
  // Disable brownout detector
  WRITE_PERM_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("ESP32-CAM Face Recognition System");
  Serial.println("=================================\n");
  
  // Initialize camera
  cameraInitialized = initCamera();
  if (!cameraInitialized) {
    Serial.println("[ERROR] Camera initialization failed!");
    Serial.println("[ERROR] System halted. Please check connections and restart.");
    while (1) {
      delay(1000);
    }
  }
  
  // Configure WiFi
  Serial.println("[WIFI] Configuring static IP...");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("[WIFI] Static IP configuration failed");
  }
  
  // Connect to WiFi
  Serial.print("[WIFI] Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected!");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n[WIFI] Connection failed!");
    Serial.println("[ERROR] System halted. Please check WiFi credentials.");
    while (1) {
      delay(1000);
    }
  }
  
  // Start servers
  startCameraStream();
  startWebServer();
  
  Serial.println("\n=================================");
  Serial.println("System Ready!");
  Serial.println("=================================");
  Serial.println("Camera Stream: http://192.168.5.86:81/stream");
  Serial.println("Web Interface: http://192.168.5.86");
  Serial.println("Status API: http://192.168.5.86/status");
  Serial.println("=================================\n");
}

// ============================
// MAIN LOOP
// ============================
void loop() {
  server.handleClient();
  
  // Auto face recognition every 5 seconds
  if (millis() - lastFaceRecognitionTime > FACE_RECOGNITION_INTERVAL) {
    lastFaceRecognitionTime = millis();
    recognizeFace();
  }
  
  delay(10);
}
