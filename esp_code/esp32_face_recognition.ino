/**
 * ESP32-CAM Smart Door Lock with Face Recognition
 * 
 * Hardware:
 * - ESP32-CAM (AI-Thinker)
 * - PIR Motion Sensor
 * - Servo Motor SG90
 * - LED Red & Green
 * - Buzzer
 * 
 * Connections:
 * PIR Sensor -> GPIO 13
 * Servo Motor -> GPIO 12
 * LED Red -> GPIO 2
 * LED Green -> GPIO 4
 * Buzzer -> GPIO 14
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <base64.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server Configuration
const char* serverUrl = "http://YOUR_SERVER_IP:5000/api/recognize";
const char* firebaseUrl = "https://iot-rc-ef82d-default-rtdb.asia-southeast1.firebasedatabase.app";

// Pin Definitions
#define PIR_PIN 13
#define SERVO_PIN 12
#define LED_RED 2
#define LED_GREEN 4
#define BUZZER_PIN 14

// Camera Model: AI-Thinker
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

Servo doorServo;
bool doorLocked = true;
unsigned long lastMotionTime = 0;
const unsigned long MOTION_COOLDOWN = 5000; // 5 seconds

void setup() {
  Serial.begin(115200);
  
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initial state
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize servo
  doorServo.attach(SERVO_PIN);
  lockDoor();
  
  // Initialize camera
  initCamera();
  
  // Connect to WiFi
  connectWiFi();
  
  Serial.println("System Ready!");
}

void loop() {
  // Check PIR sensor
  bool motionDetected = digitalRead(PIR_PIN);
  
  if (motionDetected && (millis() - lastMotionTime > MOTION_COOLDOWN)) {
    Serial.println("Motion detected!");
    lastMotionTime = millis();
    
    // Update Firebase sensor data
    updateSensorData(true);
    
    // Capture and recognize face
    recognizeFace();
  }
  
  delay(100);
}

void initCamera() {
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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  Serial.println("Camera initialized!");
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void recognizeFace() {
  // Capture photo
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  // Convert to base64
  String imageBase64 = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  
  // Send to server
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["image"] = imageBase64;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
      
      // Parse response
      StaticJsonDocument<512> responseDoc;
      deserializeJson(responseDoc, response);
      
      bool authorized = responseDoc["authorized"];
      const char* userName = responseDoc["user"]["name"];
      float confidence = responseDoc["user"]["confidence"];
      
      if (authorized) {
        Serial.printf("Access Granted! Welcome %s (%.2f%%)\n", userName, confidence);
        unlockDoor();
      } else {
        Serial.println("Access Denied!");
        triggerAlert();
      }
    } else {
      Serial.printf("HTTP Error: %d\n", httpResponseCode);
    }
    
    http.end();
  }
}

// ===== Send Unlock Command to ESP8266 =====
void unlockDoorViaESP8266(const char* userName) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String unlockUrl = String(esp8266Server) + "/unlock";
    
    http.begin(unlockUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Send unlock with face recognition
    DynamicJsonDocument doc(256);
    doc["method"] = "face";
    doc["user"] = userName;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      Serial.println("[ESP8266] Door unlocked via ESP8266");
    } else {
      Serial.printf("[ESP8266] Unlock failed: %d\n", httpResponseCode);
    }
    
    http.end();
  }
}

// ===== Trigger Alert via ESP8266 =====
void triggerAlertViaESP8266() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String buzzerUrl = String(esp8266Server) + "/buzzer";
    
    http.begin(buzzerUrl);
    http.addHeader("Content-Type", "application/json");
    
    DynamicJsonDocument doc(128);
    doc["active"] = true;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    http.POST(jsonPayload);
    http.end();
    
    Serial.println("[ESP8266] Alert triggered");
  }
}

// ===== Camera Stream Handler =====
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
  if(res != ESP_OK){
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 11);
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  return res;
}

// ===== Start Camera Stream Server =====
void startCameraStream() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  Serial.print("Starting stream server on port: ");
  Serial.println(config.server_port);
  
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("✅ Stream server started successfully");
  }
}

// ===== HTTP Handlers =====
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Face Recognition</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial; 
      margin: 20px; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      text-align: center;
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
    h1 { margin-top: 0; }
    .status { 
      padding: 15px; 
      background: rgba(76, 175, 80, 0.3); 
      border-radius: 10px; 
      margin: 15px 0;
      border: 2px solid rgba(76, 175, 80, 0.5);
    }
    img { 
      width: 100%; 
      max-width: 640px; 
      border-radius: 10px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.3);
    }
    .button {
      background: rgba(33, 150, 243, 0.8);
      color: white;
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-size: 16px;
      margin: 10px;
      transition: all 0.3s;
    }
    .button:hover {
      background: rgba(33, 150, 243, 1);
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(33, 150, 243, 0.4);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>📷 ESP32-CAM Face Recognition</h1>
    <div class="status">✅ Camera Online</div>
    
    <h2>🎥 Live Stream</h2>
    <img src="http://)";
  
  html += WiFi.localIP().toString();
  html += R"(:81/stream" alt="Camera Stream">
    
    <div>
      <button class="button" onclick="location.reload()">🔄 Refresh</button>
      <button class="button" onclick="capture()">📸 Capture & Recognize</button>
    </div>
    
    <p style="margin-top: 30px; opacity: 0.8;">
      <strong>Stream URL:</strong><br>
      http://)";
  html += WiFi.localIP().toString();
  html += R"(:81/stream
    </p>
  </div>
  
  <script>
    function capture() {
      fetch('/capture')
        .then(response => response.json())
        .then(data => {
          alert(data.message);
        });
    }
  </script>
</body>
</html>
  )";
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", html);
}

void handleCapture() {
  recognizeFace();
  
  DynamicJsonDocument doc(256);
  doc["success"] = true;
  doc["message"] = "Face recognition initiated";
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handleStatus() {
  DynamicJsonDocument doc(256);
  doc["camera"] = "online";
  doc["ip"] = WiFi.localIP().toString();
  doc["stream_url"] = "http://" + WiFi.localIP().toString() + ":81/stream";
  doc["rssi"] = WiFi.RSSI();
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

// ===== Start Web Server =====
void startWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/capture", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("✅ HTTP server started on port 80");
}
