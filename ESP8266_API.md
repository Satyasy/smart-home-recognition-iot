# ESP8266 API Endpoints

ESP8266 harus menyediakan HTTP REST API berikut untuk komunikasi dengan Dashboard.

## Base URL
```
http://192.168.5.250
```

## 📡 Endpoints yang Diperlukan

### 1. GET /sensor
**Deskripsi:** Mendapatkan data dari semua sensor

**Response:**
```json
{
  "temperature": 25.5,
  "humidity": 65.0,
  "light": 450,
  "distance": 15.5
}
```

**Fields:**
- `temperature` (float): Suhu dari DHT11 dalam Celsius
- `humidity` (float): Kelembaban dari DHT11 dalam persen
- `light` (int): Nilai LDR (0-1023)
- `distance` (float): Jarak dari HC-SR04 dalam cm

---

### 2. GET /status
**Deskripsi:** Mendapatkan status semua aktuator

**Response:**
```json
{
  "servo": {
    "angle": 0,
    "locked": true
  },
  "led": {
    "red": true,
    "green": false
  },
  "buzzer": {
    "active": false
  },
  "lcd": {
    "line1": "Door LOCKED",
    "line2": "Waiting..."
  }
}
```

---

### 3. POST /unlock
**Deskripsi:** Buka kunci pintu dengan PIN

**Request Body:**
```json
{
  "pin": "0000"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Door unlocked",
  "servo": {
    "angle": 90,
    "locked": false
  }
}
```

**Actions:**
- Gerakkan servo ke 90° (unlocked position)
- LED Red OFF, LED Green ON
- Update LCD: "Door UNLOCKED"
- Set status locked = false

---

### 4. POST /lock
**Deskripsi:** Kunci pintu

**Response:**
```json
{
  "success": true,
  "message": "Door locked",
  "servo": {
    "angle": 0,
    "locked": true
  }
}
```

**Actions:**
- Gerakkan servo ke 0° (locked position)
- LED Red ON, LED Green OFF
- Update LCD: "Door LOCKED"
- Set status locked = true

---

### 5. POST /led
**Deskripsi:** Kontrol LED

**Request Body:**
```json
{
  "color": "red",
  "state": true
}
```

**Parameters:**
- `color`: "red" atau "green"
- `state`: true (ON) atau false (OFF)

**Response:**
```json
{
  "success": true,
  "led": {
    "red": true,
    "green": false
  }
}
```

---

### 6. POST /buzzer
**Deskripsi:** Kontrol buzzer

**Request Body:**
```json
{
  "state": true
}
```

**Response:**
```json
{
  "success": true,
  "buzzer": {
    "active": true
  }
}
```

---

### 7. POST /lcd
**Deskripsi:** Update LCD Display

**Request Body:**
```json
{
  "line1": "Door UNLOCKED",
  "line2": "User: Admin"
}
```

**Response:**
```json
{
  "success": true,
  "lcd": {
    "line1": "Door UNLOCKED",
    "line2": "User: Admin"
  }
}
```

---

## 🔧 Contoh Implementasi Arduino (ESP8266)

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin definitions
#define DHT_PIN D1
#define LDR_PIN A0
#define TRIG_PIN D5
#define ECHO_PIN D6
#define SERVO_PIN D7
#define LED_RED_PIN D2
#define LED_GREEN_PIN D3
#define BUZZER_PIN D4

// Components
DHT dht(DHT_PIN, DHT11);
Servo doorServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

// State variables
bool isLocked = true;
int servoAngle = 0;
String lcdLine1 = "Door LOCKED";
String lcdLine2 = "Waiting...";

void setup() {
  Serial.begin(115200);
  
  // Initialize components
  dht.begin();
  doorServo.attach(SERVO_PIN);
  lcd.init();
  lcd.backlight();
  
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initial state
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, LOW);
  doorServo.write(0);
  updateLCD(lcdLine1, lcdLine2);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\\nConnected! IP: " + WiFi.localIP().toString());
  
  // Setup routes
  server.on("/sensor", HTTP_GET, handleGetSensor);
  server.on("/status", HTTP_GET, handleGetStatus);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/lock", HTTP_POST, handleLock);
  server.on("/led", HTTP_POST, handleLED);
  server.on("/buzzer", HTTP_POST, handleBuzzer);
  server.on("/lcd", HTTP_POST, handleLCD);
  
  server.enableCORS(true);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleGetSensor() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int light = analogRead(LDR_PIN);
  float distance = getDistance();
  
  String json = "{";
  json += "\\"temperature\\":" + String(temp) + ",";
  json += "\\"humidity\\":" + String(hum) + ",";
  json += "\\"light\\":" + String(light) + ",";
  json += "\\"distance\\":" + String(distance);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleGetStatus() {
  String json = "{";
  json += "\\"servo\\":{\\"angle\\":" + String(servoAngle) + ",\\"locked\\":" + (isLocked ? "true" : "false") + "},";
  json += "\\"led\\":{\\"red\\":" + (digitalRead(LED_RED_PIN) ? "true" : "false") + ",\\"green\\":" + (digitalRead(LED_GREEN_PIN) ? "true" : "false") + "},";
  json += "\\"buzzer\\":{\\"active\\":" + (digitalRead(BUZZER_PIN) ? "true" : "false") + "},";
  json += "\\"lcd\\":{\\"line1\\":\\"" + lcdLine1 + "\\",\\"line2\\":\\"" + lcdLine2 + "\\"}";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleUnlock() {
  // PIN verification bisa ditambahkan di sini
  servoAngle = 90;
  isLocked = false;
  doorServo.write(servoAngle);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, HIGH);
  
  String json = "{\\"success\\":true,\\"message\\":\\"Door unlocked\\"}";
  server.send(200, "application/json", json);
}

void handleLock() {
  servoAngle = 0;
  isLocked = true;
  doorServo.write(servoAngle);
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, LOW);
  
  String json = "{\\"success\\":true,\\"message\\":\\"Door locked\\"}";
  server.send(200, "application/json", json);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void updateLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcdLine1 = line1;
  lcdLine2 = line2;
}
```

---

## 📝 Testing Endpoints

### Menggunakan curl:

```bash
# Get sensor data
curl http://192.168.5.250/sensor

# Get status
curl http://192.168.5.250/status

# Unlock door
curl -X POST -H "Content-Type: application/json" -d '{"pin":"0000"}' http://192.168.5.250/unlock

# Lock door
curl -X POST http://192.168.5.250/lock

# Control LED
curl -X POST -H "Content-Type: application/json" -d '{"color":"green","state":true}' http://192.168.5.250/led

# Control buzzer
curl -X POST -H "Content-Type: application/json" -d '{"state":true}' http://192.168.5.250/buzzer

# Update LCD
curl -X POST -H "Content-Type: application/json" -d '{"line1":"Hello","line2":"World"}' http://192.168.5.250/lcd
```

---

## ⚡ Pin Connections

### DHT11 (Temperature & Humidity)
- VCC → 3.3V
- GND → GND
- DATA → D1

### LDR (Light Sensor)
- One leg → 3.3V
- Other leg → A0 + 10kΩ resistor to GND

### HC-SR04 (Ultrasonic)
- VCC → 5V
- GND → GND
- TRIG → D5
- ECHO → D6

### Servo SG90
- VCC → 5V (external)
- GND → GND
- Signal → D7

### LEDs
- Red LED → D2 (with 220Ω resistor)
- Green LED → D3 (with 220Ω resistor)

### Buzzer
- Positive → D4
- Negative → GND

### LCD I2C 16x2
- VCC → 5V
- GND → GND
- SDA → D2 (GPIO4)
- SCL → D1 (GPIO5)

---

**Note:** Pastikan CORS enabled di ESP8266 agar Dashboard bisa mengakses API!
