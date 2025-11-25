/**
 * ESP8266 - Smart Home Controller
 * 
 * Fungsi:
 * 1. Kontrol semua sensor (DHT, LDR, Ultrasonic)
 * 2. Kontrol semua aktuator (Servo, LED, Buzzer)
 * 3. Terima hasil face recognition dari ESP32-CAM
 * 4. Buka pintu jika wajah dikenali, buzzer jika tidak
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SOUND_SPEED 0.034

// ===== WiFi Configuration =====
const char* ssid = "Zfox";
const char* password = "asdfg1234";

// ===== Web Server =====
ESP8266WebServer server(80);

// ===== Pin Definitions =====
// I2C LCD (SDA=D2/GPIO4, SCL=D1/GPIO5) - menggunakan pin I2C default ESP8266
// Note: DHT dipindah ke pin lain karena D1 dipakai SCL

// DHT sensor (dipindah dari D1 ke D7)
const int dhtPin = D7;     // GPIO13

// Output
const int buzzPin = D4;    // GPIO2
const int ledRedPin  = D0; // GPIO16
const int ledGreenPin = D8; // GPIO15

// LDR sensor
const int ldrPin = A0;     // ADC pin

// Ultrasonic
const int trigPin = D5;    // GPIO14
const int echoPin = D6;    // GPIO12

// Servo
const int servoPin = D3;   // GPIO0

// ===== DHT Configuration =====
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// ===== LCD Configuration =====
// LCD I2C address biasanya 0x27 atau 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== Global Variables =====
Servo doorServo;
long duration;
float distanceCm;
bool doorLocked = true;
String lastRecognizedUser = "";
float lastConfidence = 0.0;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n==============================");
  Serial.println("ESP8266 Smart Home Controller");
  Serial.println("==============================\n");

  // Initialize pins
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ldrPin, INPUT);

  // Initial state - Door locked
  digitalWrite(ledRedPin, HIGH);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(buzzPin, LOW);

  // Initialize devices
  doorServo.attach(servoPin);
  doorServo.write(0); // Locked position
  
  dht.begin();
  
  // Initialize LCD I2C
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  
  // Connect to WiFi
  connectWiFi();
  
  // Setup web server routes
  setupWebServer();
  
  Serial.println("[READY] System is ready!");
  Serial.println("Waiting for commands...\n");
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
    Serial.println("Gunakan IP ini di ESP32-CAM!");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door: LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Waiting...");
  } else {
    Serial.println("\n[ERROR] WiFi Connection Failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
  }
}

// ===== Setup Web Server =====
void setupWebServer() {
  // Endpoint untuk menerima hasil face recognition dari ESP32-CAM
  server.on("/face-result", HTTP_POST, handleFaceResult);
  
  // Endpoint untuk status sensor
  server.on("/status", HTTP_GET, handleStatus);
  
  // Endpoint untuk ESP32-CAM cek jarak sebelum capture
  server.on("/check-distance", HTTP_GET, handleCheckDistance);
  
  // Endpoint untuk kontrol manual
  server.on("/unlock", HTTP_GET, handleUnlock);
  server.on("/lock", HTTP_GET, handleLock);
  
  server.begin();
  Serial.println("[OK] Web server started!");
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Read sensors periodically
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 2000) { // Every 2 seconds
    lastSensorRead = millis();
    readAllSensors();
  }
  
  delay(10);
}

// ===== Handle Face Recognition Result from ESP32-CAM =====
void handleFaceResult() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("\n[RECEIVED] Face recognition result:");
    Serial.println(body);
    
    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error) {
      bool authorized = doc["authorized"];
      String userName = doc["name"] | "Unknown";
      float confidence = doc["confidence"] | 0.0;
      
      lastRecognizedUser = userName;
      lastConfidence = confidence;
      
      if (authorized && confidence >= 70.0) {
        Serial.printf("[AUTHORIZED] Welcome %s (%.2f%%)\n", userName.c_str(), confidence);
        unlockDoor();
      } else if (authorized && confidence < 70.0) {
        Serial.printf("[LOW ACCURACY] %s (%.2f%%) - Rejected\n", userName.c_str(), confidence);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Low Accuracy");
        lcd.setCursor(0, 1);
        lcd.print(String(confidence, 1) + "% < 70%");
        triggerAlert();
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door: LOCKED");
      } else {
        Serial.println("[DENIED] Access denied!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access DENIED");
        lcd.setCursor(0, 1);
        lcd.print("Unknown Face");
        triggerAlert();
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door: LOCKED");
      }
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      Serial.println("[ERROR] Failed to parse JSON");
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

// ===== Handle Status Request =====
void handleStatus() {
  StaticJsonDocument<512> doc;
  
  doc["door_locked"] = doorLocked;
  doc["last_user"] = lastRecognizedUser;
  doc["last_confidence"] = lastConfidence;
  
  // Sensor readings
  doc["temperature"] = dht.readTemperature();
  doc["humidity"] = dht.readHumidity();
  doc["light"] = analogRead(ldrPin);
  doc["distance"] = distanceCm;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

// ===== Handle Check Distance (untuk ESP32-CAM) =====
void handleCheckDistance() {
  // Baca ultrasonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * SOUND_SPEED / 2;
  
  StaticJsonDocument<128> doc;
  doc["distance"] = distance;
  doc["should_capture"] = (distance > 0 && distance < 30);
  
  String response;
  serializeJson(doc, response);
  
  Serial.printf("Distance check: %.2f cm - Capture: %s\n", distance, (distance < 30 ? "YES" : "NO"));
  
  server.send(200, "application/json", response);
}

// ===== Handle Manual Unlock =====
void handleUnlock() {
  Serial.println("[MANUAL] Manual unlock triggered");
  unlockDoor();
  server.send(200, "application/json", "{\"status\":\"unlocked\"}");
}

// ===== Handle Manual Lock =====
void handleLock() {
  Serial.println("[MANUAL] Manual lock triggered");
  lockDoor();
  server.send(200, "application/json", "{\"status\":\"locked\"}");
}

// ===== Read All Sensors =====
void readAllSensors() {
  Serial.println("-----------------------------");
  
  // Ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  
  // DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  Serial.print("Temperature: ");
  if (!isnan(temperature)) {
    Serial.print(temperature);
    Serial.println(" °C");
  } else {
    Serial.println("ERROR");
  }
  
  Serial.print("Humidity: ");
  if (!isnan(humidity)) {
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("ERROR");
  }
  
  // LDR sensor
  int ldrValue = analogRead(ldrPin);
  Serial.print("Light (LDR): ");
  Serial.print(ldrValue);
  
  if (ldrValue < 300) {
    Serial.println(" -> SANGAT GELAP");
  } else if (ldrValue < 500) {
    Serial.println(" -> GELAP");
  } else if (ldrValue < 700) {
    Serial.println(" -> REDUP");
  } else {
    Serial.println(" -> TERANG");
  }
  
  Serial.print("Door Status: ");
  Serial.println(doorLocked ? "LOCKED" : "UNLOCKED");
  Serial.println("-----------------------------");
}

// ===== Unlock Door =====
void unlockDoor() {
  doorLocked = false;
  
  Serial.println("[ACTION] Unlocking door...");
  
  // Tampilkan di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  if (lastRecognizedUser.length() > 11) {
    lcd.print(lastRecognizedUser.substring(0, 11));
  } else {
    lcd.print(lastRecognizedUser);
  }
  
  // Servo ke posisi unlock (180 derajat)
  doorServo.write(180);
  
  // LED hijau nyala, merah mati
  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledGreenPin, HIGH);
  
  // Buzzer off
  digitalWrite(buzzPin, LOW);
  
  Serial.println("[SUCCESS] Door unlocked!");
  
  // Auto lock setelah 5 detik
  delay(2000);
  
  // Tampilkan akurasi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Accuracy:");
  lcd.setCursor(0, 1);
  lcd.print(String(lastConfidence, 1) + "%");
  
  delay(3000);
  lockDoor();
}

// ===== Lock Door =====
void lockDoor() {
  doorLocked = true;
  
  Serial.println("[ACTION] Locking door...");
  
  // Servo ke posisi lock (0 derajat)
  doorServo.write(0);
  
  // LED merah nyala, hijau mati
  digitalWrite(ledRedPin, HIGH);
  digitalWrite(ledGreenPin, LOW);
  
  Serial.println("[SUCCESS] Door locked!");
}

// ===== Trigger Alert (Buzzer) =====
void triggerAlert() {
  Serial.println("[ALERT] Access denied - triggering buzzer!");
  
  // LED merah berkedip dengan buzzer
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzPin, HIGH);
    digitalWrite(ledRedPin, HIGH);
    delay(200);
    digitalWrite(buzzPin, LOW);
    digitalWrite(ledRedPin, LOW);
    delay(200);
  }
  
  // Kembalikan LED merah menyala (locked state)
  digitalWrite(ledRedPin, HIGH);
  
  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Door: LOCKED");
}