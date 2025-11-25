/**
 * ESP32-CAM - Face Recognition Only
 * 
 * Fungsi:
 * 1. Capture foto dari kamera
 * 2. Kirim ke Flask server untuk face recognition
 * 3. Kirim hasil (authorized/denied) ke ESP8266 via Serial/HTTP
 * 
 * Hardware:
 * - ESP32-CAM (AI-Thinker)
 * - Hanya menggunakan port kamera built-in
 * 
 * Komunikasi:
 * - WiFi ke Flask Server (Face Recognition API)
 * - HTTP POST ke ESP8266 (kirim hasil recognition)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

// ===== WiFi Configuration =====
const char* ssid = "Zfox";
const char* password = "asdfg1234";

// ===== Server Configuration =====
const char* flaskServerUrl = "http://192.168.50.221:5000/api/recognize";  // Flask API
const char* esp8266Ip = "http://192.168.50.250";  // IP ESP8266 (ganti sesuai IP ESP8266)

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

// ===== Timing Variables =====
unsigned long lastCaptureTime = 0;
const unsigned long CAPTURE_INTERVAL = 3000; // Capture setiap 3 detik

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("ESP32-CAM Face Recognition System");
  Serial.println("=================================\n");
  
  // Disable brownout detector untuk stabilitas
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Initialize camera
  if (initCamera()) {
    Serial.println("[OK] Camera initialized successfully!");
  } else {
    Serial.println("[ERROR] Camera initialization failed!");
    while(1); // Stop jika kamera gagal
  }
  
  // Connect to WiFi
  connectWiFi();
  
  Serial.println("\n[READY] System is ready!");
  Serial.println("Waiting for face detection...\n");
}

void loop() {
  // Capture dan recognize face setiap interval
  if (millis() - lastCaptureTime > CAPTURE_INTERVAL) {
    lastCaptureTime = millis();
    
    Serial.println("-----------------------------");
    
    // Cek jarak dulu ke ESP8266
    if (shouldCapture()) {
      Serial.println("Object detected < 30cm - Capturing image...");
      // Capture dan proses face recognition
      recognizeAndSendToESP8266();
    } else {
      Serial.println("No object within range - Skipping capture");
    }
  }
  
  delay(100);
}

// ===== Initialize Camera =====
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
  
  // Pengaturan kualitas gambar
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;  // 640x480 (cukup untuk face recognition)
    config.jpeg_quality = 10;  // 0-63, semakin kecil semakin bagus
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }
  
  return true;
}

// ===== Check Distance from ESP8266 Ultrasonic Sensor =====
bool shouldCapture() {
  if(WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  HTTPClient http;
  String url = String(esp8266Ip) + "/check-distance";
  http.begin(url);
  http.setTimeout(3000); // 3 second timeout
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    
    // Parse JSON response
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      float distance = doc["distance"] | 999.0;
      bool shouldCapture = doc["should_capture"] | false;
      
      Serial.printf("Distance: %.2f cm - Should capture: %s\n", distance, shouldCapture ? "YES" : "NO");
      
      http.end();
      return shouldCapture;
    }
  } else {
    Serial.printf("[ERROR] Failed to get distance: %d\n", httpResponseCode);
  }
  
  http.end();
  return false; // Default: jangan capture jika gagal komunikasi
}

// ===== Connect to WiFi =====
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] WiFi Connection Failed!");
  }
}

// ===== Capture, Recognize, and Send to ESP8266 =====
void recognizeAndSendToESP8266() {
  // 1. Capture foto
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("[ERROR] Camera capture failed!");
    return;
  }
  
  Serial.printf("Image captured: %d bytes\n", fb->len);
  
  // 2. Convert to base64
  String imageBase64 = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb); // Release memory
  
  // 3. Kirim ke Flask server untuk face recognition
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(flaskServerUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(15000); // 15 second timeout
    
    // Create JSON payload
    DynamicJsonDocument doc(imageBase64.length() + 100);
    doc["image"] = imageBase64;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.println("Sending to Flask server...");
    
    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Flask Response: " + response);
      
      // Parse response
      DynamicJsonDocument responseDoc(1024);
      DeserializationError error = deserializeJson(responseDoc, response);
      
      if (!error) {
        bool authorized = responseDoc["authorized"] | false;
        String userName = responseDoc["user"]["name"] | "Unknown";
        float confidence = responseDoc["user"]["confidence"] | 0.0;
        
        // 4. Kirim hasil ke ESP8266
        if (authorized) {
          Serial.printf("[SUCCESS] Face Recognized: %s (%.2f%%)\n", userName.c_str(), confidence);
          sendToESP8266(true, userName, confidence);
        } else {
          Serial.println("[DENIED] Face not recognized!");
          sendToESP8266(false, "Unknown", 0.0);
        }
      } else {
        Serial.println("[ERROR] Failed to parse response");
      }
    } else {
      Serial.printf("[ERROR] HTTP Error: %d\n", httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("[ERROR] WiFi not connected!");
  }
}

// ===== Send Result to ESP8266 =====
void sendToESP8266(bool authorized, String userName, float confidence) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Endpoint ESP8266 untuk menerima hasil face recognition
    String url = String(esp8266Ip) + "/face-result";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["authorized"] = authorized;
    doc["name"] = userName;
    doc["confidence"] = confidence;
    doc["timestamp"] = millis();
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.println("Sending to ESP8266: " + jsonPayload);
    
    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("ESP8266 Response: " + response);
    } else {
      Serial.printf("[ERROR] Failed to send to ESP8266: %d\n", httpResponseCode);
    }
    
    http.end();
  }
}
