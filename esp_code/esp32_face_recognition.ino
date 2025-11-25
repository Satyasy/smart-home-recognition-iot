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

void unlockDoor() {
  Serial.println("Unlocking door...");
  doorLocked = false;
  
  // Control actuators
  doorServo.write(90); // Unlock position
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  
  // Update Firebase
  updateFirebase("servo/angle", 90);
  updateFirebase("servo/locked", false);
  updateFirebase("led/red", false);
  updateFirebase("led/green", true);
  
  // Auto lock after 5 seconds
  delay(5000);
  lockDoor();
}

void lockDoor() {
  Serial.println("Locking door...");
  doorLocked = true;
  
  // Control actuators
  doorServo.write(0); // Lock position
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  
  // Update Firebase
  updateFirebase("servo/angle", 0);
  updateFirebase("servo/locked", true);
  updateFirebase("led/red", true);
  updateFirebase("led/green", false);
}

void triggerAlert() {
  Serial.println("Alert triggered!");
  
  // Activate buzzer
  for(int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  
  // Update Firebase
  updateFirebase("buzzer/active", true);
  delay(1000);
  updateFirebase("buzzer/active", false);
}

void updateSensorData(bool motion) {
  updateFirebase("pir/motion", motion);
}

void updateFirebase(String path, bool value) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(firebaseUrl) + "/sensors/" + path + ".json";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String payload = value ? "true" : "false";
    http.PUT(payload);
    http.end();
  }
}

void updateFirebase(String path, int value) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(firebaseUrl) + "/sensors/" + path + ".json";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String payload = String(value);
    http.PUT(payload);
    http.end();
  }
}
