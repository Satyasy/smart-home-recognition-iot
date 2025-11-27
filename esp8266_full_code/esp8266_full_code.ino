
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ArduinoJson.h>
  #include <DHT.h>
  #include <Servo.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>

  // ===== WiFi Configuration =====
  const char* ssid = "Zfox";        
  const char* password = "asdfg1234"; 

  // ===== Static IP Configuration =====
  IPAddress local_IP(192, 168, 5, 250);
  IPAddress gateway(192, 168, 5, 1);
  IPAddress subnet(255, 255, 255, 0);

  // ===== Pin Configuration (Updated sesuai wiring) =====
  #define DHT_PIN D7        // GPIO13 - DHT11 data pin
  #define DHT_TYPE DHT11
  #define LDR_PIN A0        // ADC - LDR analog pin
  #define TRIG_PIN D5       // GPIO14 - HC-SR04 trigger
  #define ECHO_PIN D6       // GPIO12 - HC-SR04 echo
  #define SERVO_PIN D3      // GPIO0 - Servo signal
  #define BUZZER_PIN D4     // GPIO2 - Buzzer
  #define LED_RED_PIN D0        // GPIO16 - LED (single LED)
  #define LED_GREEN_PIN D8
  #define RELAY_PIN 1       // GPIO1 (TX) - Relay for lamp
  // Note: GPIO1 (TX) can be used as GPIO after Serial is disabled
  // LCD I2C: SDA=D1 (GPIO5), SCL=D2 (GPIO4)

  // ===== Objects =====
  ESP8266WebServer server(80);
  DHT dht(DHT_PIN, DHT_TYPE);
  Servo doorServo;
  LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns, 2 rows
                                      // SDA=D1 (GPIO5), SCL=D2 (GPIO4)

  // ===== Relay State =====
  bool lampOn = false;

  // ===== PIN Configuration =====
  const String DEFAULT_PIN = "0000";
  String currentPin = DEFAULT_PIN;

  // ===== Door State =====
  bool doorLocked = true;
  unsigned long unlockTime = 0;
  const unsigned long AUTO_LOCK_DELAY = 5000; // 5 seconds

  // ===== Sensor Data =====
  struct {
    float temperature = 0;
    float humidity = 0;
    int light = 0;
    float distance = 0;
  } sensorData;

  // ===== CORS Headers =====
  void enableCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  }

  // ===== OPTIONS Handler =====
  void handleOptions() {
    enableCORS();
    server.send(204);
  }

  // ===== Sensor Functions =====
  float readDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 999.9; // No object detected
    
    float distance = duration * 0.034 / 2; // Speed of sound = 340 m/s
    return distance;
  }

  int readLight() {
    int rawValue = analogRead(LDR_PIN);
    // Convert to 0-1023 range (ESP8266 ADC is 10-bit)
    return rawValue;
  }

  void updateSensorData() {
    // Read DHT11
    sensorData.temperature = dht.readTemperature();
    sensorData.humidity = dht.readHumidity();
    
    // Check if DHT readings failed
    if (isnan(sensorData.temperature)) sensorData.temperature = 0;
    if (isnan(sensorData.humidity)) sensorData.humidity = 0;
    
    // Read LDR
    sensorData.light = readLight();
    
    // Read HC-SR04
    sensorData.distance = readDistance();
  }

  // ===== Actuator Functions =====
  void unlockDoor() {
    doorLocked = false;
    doorServo.write(180); // Unlock position
    
    digitalWrite(LED_RED_PIN, LOW);   // RED LED OFF
    digitalWrite(LED_GREEN_PIN, HIGH); // GREEN LED ON (unlocked)
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door UNLOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Welcome!");
    
    // Buzzer beep untuk konfirmasi
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    
    unlockTime = millis();
    
    Serial.println("[DOOR] UNLOCKED");
  }

  void lockDoor() {
    doorLocked = true;
    doorServo.write(0); // Lock position
    
    digitalWrite(LED_RED_PIN, HIGH);  // RED LED ON (locked)
    digitalWrite(LED_GREEN_PIN, LOW);  // GREEN LED OFF
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Scan to enter");
    
    // Single beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    
    Serial.println("[DOOR] LOCKED");
  }

  // ===== Relay/Lamp Control =====
  void turnLampOn() {
    lampOn = true;
    digitalWrite(RELAY_PIN, HIGH); // Relay ON (lamp on)
    Serial.println("[LAMP] ON");
  }

  void turnLampOff() {
    lampOn = false;
    digitalWrite(RELAY_PIN, LOW); // Relay OFF (lamp off)
    Serial.println("[LAMP] OFF");
  }

  void toggleLamp() {
    if (lampOn) {
      turnLampOff();
    } else {
      turnLampOn();
    }
  }

  void triggerAlert() {
    digitalWrite(LED_RED_PIN, HIGH);   // RED LED ON for alert
    digitalWrite(LED_GREEN_PIN, LOW);  // GREEN LED OFF
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERT!");
    lcd.setCursor(0, 1);
    lcd.print("Access Denied");
    
    // Buzzer alert pattern - SHORTENED to avoid blocking
    for (int i = 0; i < 3; i++) {  // Changed from 3 to 2 beeps
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);  // Reduced from 200ms
      digitalWrite(BUZZER_PIN, LOW);
      delay(80);   // Reduced from 100ms
    }
    
    delay(1000);  // Reduced from 2000ms (2 seconds to 1 second)
    
    if (doorLocked) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Door LOCKED");
      lcd.setCursor(0, 1);
      lcd.print("Scan to enter");
    }
    
    Serial.println("[ALERT] Triggered");
  }

  // ===== HTTP Handlers =====

  // POST /alert - Trigger alert dari dashboard
  void handleAlert() {
    Serial.println("[HTTP] POST /alert - Triggering alert");
    
    triggerAlert();
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["message"] = "Alert triggered";
    
    String json;
    serializeJson(response, json);
    
    enableCORS();
    server.send(200, "application/json", json);
  }

  // GET /sensor
  void handleSensor() {
    updateSensorData();
    
    StaticJsonDocument<256> doc;
    doc["temperature"] = sensorData.temperature;
    doc["humidity"] = sensorData.humidity;
    doc["light"] = sensorData.light;
    doc["distance"] = sensorData.distance;
    
    String json;
    serializeJson(doc, json);
    
    enableCORS();
    server.send(200, "application/json", json);
    
    Serial.println("[HTTP] GET /sensor");
  }

  // GET /status
  void handleStatus() {
    StaticJsonDocument<512> doc;
    
    doc["servo"]["angle"] = doorLocked ? 0 : 90;
    doc["servo"]["locked"] = doorLocked;
    
    doc["led"]["red"] = digitalRead(LED_RED_PIN);
    doc["led"]["green"] = digitalRead(LED_GREEN_PIN);
    
    doc["buzzer"]["active"] = false; // Buzzer is momentary
    
    doc["relay"]["lamp"] = lampOn; // Lamp status
    
    // Get current LCD text (simplified)
    doc["lcd"]["line1"] = doorLocked ? "Door LOCKED" : "Door UNLOCKED";
    doc["lcd"]["line2"] = doorLocked ? "Scan to enter" : "Welcome!";
    
    String json;
    serializeJson(doc, json);
    
    enableCORS();
    server.send(200, "application/json", json);
    
    Serial.println("[HTTP] GET /status");
  }

  // POST /unlock
  void handleUnlock() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      
      if (error) {
        enableCORS();
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }
      
      // Handle Face Recognition unlock
      if (doc.containsKey("method") && doc["method"] == "face") {
        String userName = doc["name"] | "Unknown";
        float confidence = doc["confidence"] | 0.0;
        
        Serial.print("[HTTP] POST /unlock - FACE RECOGNITION: ");
        Serial.print(userName);
        Serial.print(" (");
        Serial.print(confidence * 100);
        Serial.println("%)");
        
        unlockDoor();
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        response["message"] = "Door unlocked by face recognition";
        response["user"] = userName;
        response["confidence"] = confidence;
        response["servo"]["angle"] = 180;
        response["servo"]["locked"] = false;
        
        String json;
        serializeJson(response, json);
        
        enableCORS();
        server.send(200, "application/json", json);
        return;
      }
      
      // Handle PIN unlock
      if (doc.containsKey("pin")) {
        String receivedPin = doc["pin"].as<String>();
        
        Serial.print("[HTTP] POST /unlock - PIN: ");
        Serial.println(receivedPin);
        
        // Verify PIN
        if (receivedPin == currentPin) {
          unlockDoor();
          
          StaticJsonDocument<256> response;
          response["success"] = true;
          response["message"] = "Door unlocked";
          response["servo"]["angle"] = 180;
          response["servo"]["locked"] = false;
          
          String json;
          serializeJson(response, json);
          
          enableCORS();
          server.send(200, "application/json", json);
          return;
        } else {
          // Wrong PIN - Send response IMMEDIATELY before triggering alert
          StaticJsonDocument<256> response;
          response["success"] = false;
          response["message"] = "Invalid PIN";
          
          String json;
          serializeJson(response, json);
          
          enableCORS();
          server.send(403, "application/json", json);
          
          Serial.println("[HTTP] POST /unlock - WRONG PIN");
          
          // IMPORTANT: Trigger alert AFTER sending response to avoid timeout
          delay(10); // Small delay to ensure response is sent
          triggerAlert();
          
          return;
        }
      }
    }
    
    enableCORS();
    server.send(400, "application/json", "{\"error\":\"Invalid request - missing method or pin\"}");
  }

  // POST /lock
  void handleLock() {
    lockDoor();
    
    StaticJsonDocument<256> doc;
    doc["success"] = true;
    doc["message"] = "Door locked";
    doc["servo"]["angle"] = 0;
    doc["servo"]["locked"] = true;
    
    String json;
    serializeJson(doc, json);
    
    enableCORS();
    server.send(200, "application/json", json);
    
    Serial.println("[HTTP] POST /lock");
  }

  // POST /led
  void handleLED() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      
      if (!error) {
        if (doc.containsKey("red")) {
          digitalWrite(LED_RED_PIN, doc["red"] ? HIGH : LOW);
        }
        if (doc.containsKey("green")) {
          digitalWrite(LED_GREEN_PIN, doc["green"] ? HIGH : LOW);
        }
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        response["led"]["red"] = digitalRead(LED_RED_PIN);
        response["led"]["green"] = digitalRead(LED_GREEN_PIN);
        
        String json;
        serializeJson(response, json);
        
        enableCORS();
        server.send(200, "application/json", json);
        
        Serial.println("[HTTP] POST /led");
        return;
      }
    }
    
    enableCORS();
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }

  // POST /buzzer
  void handleBuzzer() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      
      if (!error && doc.containsKey("active")) {
        bool active = doc["active"];
        
        if (active) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(200);
          digitalWrite(BUZZER_PIN, LOW);
        }
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        
        String json;
        serializeJson(response, json);
        
        enableCORS();
        server.send(200, "application/json", json);
        
        Serial.println("[HTTP] POST /buzzer");
        return;
      }
    }
    
    enableCORS();
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }

  // POST /lamp - Control relay lamp
  void handleLamp() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      
      if (!error && doc.containsKey("state")) {
        String state = doc["state"].as<String>();
        
        if (state == "on") {
          turnLampOn();
        } else if (state == "off") {
          turnLampOff();
        } else if (state == "toggle") {
          toggleLamp();
        }
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        response["lamp"] = lampOn ? "on" : "off";
        response["message"] = lampOn ? "Lamp turned ON" : "Lamp turned OFF";
        
        String json;
        serializeJson(response, json);
        
        enableCORS();
        server.send(200, "application/json", json);
        
        Serial.println("[HTTP] POST /lamp - " + String(lampOn ? "ON" : "OFF"));
        return;
      }
    }
    
    enableCORS();
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }

  // POST /lcd
  void handleLCD() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      
      if (!error) {
        lcd.clear();
        
        if (doc.containsKey("line1")) {
          String line1 = doc["line1"].as<String>();
          lcd.setCursor(0, 0);
          lcd.print(line1);
        }
        
        if (doc.containsKey("line2")) {
          String line2 = doc["line2"].as<String>();
          lcd.setCursor(0, 1);
          lcd.print(line2);
        }
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        
        String json;
        serializeJson(response, json);
        
        enableCORS();
        server.send(200, "application/json", json);
        
        Serial.println("[HTTP] POST /lcd");
        return;
      }
    }
    
    enableCORS();
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }

  // Root endpoint
  void handleRoot() {
    String html = R"(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP8266 Smart Door Lock</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { 
        font-family: Arial; 
        margin: 20px; 
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
      }
      .container { 
        background: rgba(255,255,255,0.1); 
        backdrop-filter: blur(10px);
        padding: 20px; 
        border-radius: 15px; 
        max-width: 600px; 
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
      .sensor-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 10px;
        margin: 20px 0;
      }
      .sensor-box {
        background: rgba(255,255,255,0.1);
        padding: 15px;
        border-radius: 10px;
        text-align: center;
      }
      .sensor-value {
        font-size: 24px;
        font-weight: bold;
        margin: 10px 0;
      }
      .door-status {
        text-align: center;
        padding: 20px;
        background: )";
    
    html += doorLocked ? "rgba(244, 67, 54, 0.3)" : "rgba(76, 175, 80, 0.3)";
    html += R"(;
        border-radius: 10px;
        margin: 20px 0;
        font-size: 24px;
        font-weight: bold;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>🏠 Smart Door Lock System</h1>
      <div class="status">✅ ESP8266 Online</div>
      
      <div class="door-status">
        🚪 Door Status: )";
    
    html += doorLocked ? "LOCKED 🔒" : "UNLOCKED 🔓";
    html += R"(
      </div>
      
      <h2>📊 Sensor Data</h2>
      <div class="sensor-grid">
        <div class="sensor-box">
          <div>🌡️ Temperature</div>
          <div class="sensor-value">)";
    html += String(sensorData.temperature, 1);
    html += R"(°C</div>
        </div>
        <div class="sensor-box">
          <div>💧 Humidity</div>
          <div class="sensor-value">)";
    html += String(sensorData.humidity, 1);
    html += R"(%</div>
        </div>
        <div class="sensor-box">
          <div>💡 Light</div>
          <div class="sensor-value">)";
    html += String(sensorData.light);
    html += R"(</div>
        </div>
        <div class="sensor-box">
          <div>📏 Distance</div>
          <div class="sensor-value">)";
    html += String(sensorData.distance, 1);
    html += R"( cm</div>
        </div>
      </div>
      
      <p style="text-align: center; margin-top: 30px; opacity: 0.8;">
        <strong>IP:</strong> )";
    html += WiFi.localIP().toString();
    html += R"(<br>
        <strong>Signal:</strong> )";
    html += String(WiFi.RSSI());
    html += R"( dBm
      </p>
    </div>
    
    <script>
      // Auto refresh setiap 5 detik
      setTimeout(() => location.reload(), 5000);
    </script>
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
    Serial.println("ESP8266 Smart Door Lock");
    Serial.println("=================================");
    
    // Disable Serial for GPIO1 (TX) to use as relay pin
    Serial.begin(115200);
    delay(100);
    Serial.flush();
    // Serial.end(); // Keep serial for debugging, but relay on GPIO1 may conflict
    
    // Initialize pins
    pinMode(LED_RED_PIN, OUTPUT);    // RED LED
    pinMode(LED_GREEN_PIN, OUTPUT);  // GREEN LED
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);      // Relay for lamp
    
    // Initial state - door locked, lamp off
    digitalWrite(LED_RED_PIN, HIGH);   // RED LED ON (locked)
    digitalWrite(LED_GREEN_PIN, LOW);  // GREEN LED OFF
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);      // Lamp OFF
    
    // Initialize DHT
    dht.begin();
    
    // Initialize Servo
    doorServo.attach(SERVO_PIN);
    doorServo.write(0); // Lock position
    
    // Initialize LCD
    Wire.begin(D2, D1); // SDA, SCL untuk NodeMCU
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    
    // Configure static IP
    if (!WiFi.config(local_IP, gateway, subnet)) {
      Serial.println("Static IP Failed");
    }
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    lcd.setCursor(0, 1);
    lcd.print("WiFi...");
    
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
      Serial.print("📡 IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("📶 RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Connected");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      delay(2000);
    } else {
      Serial.println();
      Serial.println("❌ WiFi FAILED!");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Failed!");
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
    server.on("/lamp", HTTP_POST, handleLamp);     // NEW: Lamp control
    server.on("/alert", HTTP_POST, handleAlert);   // NEW: Alert trigger
    server.on("/lcd", HTTP_POST, handleLCD);
    
    // Handle OPTIONS for CORS
    server.on("/sensor", HTTP_OPTIONS, handleOptions);
    server.on("/status", HTTP_OPTIONS, handleOptions);
    server.on("/unlock", HTTP_OPTIONS, handleOptions);
    server.on("/lock", HTTP_OPTIONS, handleOptions);
    server.on("/led", HTTP_OPTIONS, handleOptions);
    server.on("/buzzer", HTTP_OPTIONS, handleOptions);
    server.on("/lamp", HTTP_OPTIONS, handleOptions); // NEW: CORS for lamp
    server.on("/alert", HTTP_OPTIONS, handleOptions); // NEW: CORS for alert
    server.on("/lcd", HTTP_OPTIONS, handleOptions);
    
    // Start server
    server.begin();
    Serial.println("🚀 HTTP Server Started!");
    Serial.println("=================================");
    Serial.println();
    
    // Set initial LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Scan to enter");
    
    // Welcome beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    
    Serial.println("Default PIN: " + currentPin);
    Serial.println("System Ready!");
  }

  // ===== Loop =====
  void loop() {
    server.handleClient();
    
    // Auto-lock check
    if (!doorLocked && (millis() - unlockTime > AUTO_LOCK_DELAY)) {
      Serial.println("[AUTO] Locking door after timeout");
      lockDoor();
    }
    
    // Update sensor data periodically
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate > 2000) {
      lastSensorUpdate = millis();
      updateSensorData();
    }
  }
