# 🏠 Smart Home Face Recognition System

Sistem keamanan pintu pintar dengan face recognition menggunakan **ESP32-CAM** (camera only) + **ESP8266** (controller) + **Flask API** + **React Dashboard**.

## 📖 Documentation

- **[WIRING_GUIDE.md](WIRING_GUIDE.md)** - Complete hardware wiring diagram
- **[ARDUINO_LIBRARIES.md](ARDUINO_LIBRARIES.md)** - Arduino libraries installation guide
- **[SYSTEM_ARCHITECTURE.md](SYSTEM_ARCHITECTURE.md)** - System architecture & data flow

## 🎯 System Overview

```
ESP32-CAM (Camera) → Flask API (Face Recognition) → ESP8266 (Controller)
                                                          ↓
                                              Servo + LED + Buzzer + Sensors
```

**Alur Kerja:**
1. **ESP32-CAM** capture foto wajah
2. Kirim ke **Flask Server** untuk face recognition
3. Flask mengenali wajah dari database Firebase
4. Hasil dikirim ke **ESP8266**
5. ESP8266 buka pintu (servo) jika authorized, atau buzzer jika denied

## 🚀 Features

- ✅ Face Recognition menggunakan `face_recognition` library (dlib)
- ✅ ESP32-CAM hanya untuk kamera (tidak pakai sensor/aktuator)
- ✅ ESP8266 sebagai controller utama (servo, LED, buzzer, DHT, LDR, ultrasonic)
- ✅ Real-time monitoring semua sensor
- ✅ Access logs dengan Firebase Realtime Database
- ✅ Dashboard web interaktif dengan React
- ✅ Manajemen user (register, delete)
- ✅ Auto-lock setelah 5 detik
- ✅ Activity charts dan statistics

## 📋 Prerequisites

### Software
- Python 3.13
- Node.js 18+
- Arduino IDE 1.8.19+
- Firebase Account

### Hardware
- 1x ESP32-CAM (AI-Thinker)
- 1x NodeMCU ESP8266
- 1x Servo Motor SG90
- 2x LED (Red & Green)
- 1x Buzzer
- 1x DHT11 Sensor
- 1x LDR + 10kΩ Resistor
- 1x HC-SR04 Ultrasonic
- Breadboard & Jumper Wires

## 🛠️ Installation

### Step 1: Backend Setup (Flask)

1. **Create virtual environment:**
```powershell
python -m venv .venv
.venv\Scripts\activate
```

2. **Install dependencies:**
```powershell
pip install -r requirements.txt
```

3. **Setup Firebase:**
   - Buat project di [Firebase Console](https://console.firebase.google.com)
   - Enable Realtime Database
   - Download `serviceAccountKey.json` 
   - Letakkan di root folder project
   - Update `databaseURL` di `app_face_recognition.py`

4. **Run Flask server:**
```powershell
python app_face_recognition.py
```

Server akan running di `http://localhost:5000`

### Step 2: Frontend Setup (React)

1. **Navigate to frontend folder:**
```powershell
cd smart-home-recognition
```

2. **Install dependencies:**
```powershell
npm install
```

3. **Start development server:**
```powershell
npm start
```

Frontend akan running di `http://localhost:3000`

### Step 3: Arduino Setup

1. **Install Arduino IDE** (v1.8.19+)

2. **Install Board Support:**
   - ESP32: `https://dl.espressif.com/dl/package_esp32_index.json`
   - ESP8266: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

3. **Install Libraries:**
   - ArduinoJson (v6.x)
   - DHT sensor library (Adafruit)
   - Adafruit Unified Sensor
   - Base64 (untuk ESP32-CAM)

   Lihat [ARDUINO_LIBRARIES.md](ARDUINO_LIBRARIES.md) untuk detail lengkap.

4. **Upload Code:**

   **ESP8266 (Upload Dulu!):**
   ```
   File: esp_code/esp8266_main.ino
   Board: NodeMCU 1.0 (ESP-12E Module)
   Upload Speed: 115200
   
   Update WiFi credentials:
   - const char* ssid = "YOUR_WIFI";
   - const char* password = "YOUR_PASSWORD";
   
   Upload → Catat IP Address dari Serial Monitor!
   ```

   **ESP32-CAM (Upload Kedua):**
   ```
   File: esp_code/esp32cam_face_only.ino
   Board: AI Thinker ESP32-CAM
   Upload Speed: 115200
   
   Update configuration:
   - const char* ssid = "YOUR_WIFI";
   - const char* password = "YOUR_PASSWORD";
   - const char* flaskServerUrl = "http://YOUR_PC_IP:5000/api/recognize";
   - const char* esp8266Ip = "http://ESP8266_IP"; // IP dari step ESP8266
   
   ⚠️ Short IO0 to GND saat upload!
   ```

### Step 4: Hardware Wiring

Lihat [WIRING_GUIDE.md](WIRING_GUIDE.md) untuk diagram lengkap.

**ESP32-CAM:**
- Hanya kamera built-in (tidak ada wiring tambahan)
- Power: 5V 2A adaptor

**ESP8266 Connections:**
```
D0 (GPIO16) → LED Red
D8 (GPIO15) → LED Green
D4 (GPIO2)  → Buzzer
D3 (GPIO0)  → Servo Signal
D1 (GPIO5)  → DHT11 Data
D5 (GPIO14) → Ultrasonic Trig
D6 (GPIO12) → Ultrasonic Echo
A0          → LDR (via 10kΩ resistor)
```

## 📡 API Endpoints

### Flask Server (http://localhost:5000)

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/health` | Health check |
| POST | `/api/recognize` | Face recognition (from ESP32-CAM) |
| POST | `/api/register` | Register new user with face |
| GET | `/api/users` | Get all registered users |
| GET | `/api/user/{id}` | Get user detail |
| PUT | `/api/user/{id}` | Update user info |
| DELETE | `/api/user/{id}` | Delete user |
| GET | `/api/logs` | Get access logs |
| DELETE | `/api/logs/clear` | Clear all logs |
| GET | `/api/config` | Get system configuration |

### ESP8266 Web Server (http://ESP8266_IP)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/face-result` | Receive face recognition result (from ESP32-CAM) |
| GET | `/status` | Get sensor status & door state |
| GET | `/unlock` | Manual door unlock |
| GET | `/lock` | Manual door lock |

## 🔧 Configuration

### 1. Flask Server (`app_face_recognition.py`)

```python
# Face recognition tolerance (0.6 default, lower = more strict)
TOLERANCE = 0.6

# Firebase Database URL
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://your-project.firebaseio.com/'
})
```

### 2. Frontend (`src/services/api.js`)

```javascript
const API_BASE_URL = 'http://localhost:5000/api';
```

### 3. ESP32-CAM (`esp_code/esp32cam_face_only.ino`)

```cpp
// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server URLs
const char* flaskServerUrl = "http://192.168.1.50:5000/api/recognize"; // PC IP
const char* esp8266Ip = "http://192.168.1.100"; // ESP8266 IP

// Capture interval (milliseconds)
const unsigned long CAPTURE_INTERVAL = 3000; // 3 seconds
```

### 4. ESP8266 (`esp_code/esp8266_main.ino`)

```cpp
// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin definitions (sesuaikan jika perlu)
const int servoPin = D3;   // Servo signal
const int buzzPin = D4;    // Buzzer
const int ledRedPin = D0;  // LED Red
const int ledGreenPin = D8; // LED Green
```

## 🎬 How It Works

### Complete Flow

1. **ESP32-CAM** menangkap foto setiap 3 detik
2. Convert image ke **base64** format
3. Kirim ke **Flask Server** via HTTP POST `/api/recognize`
4. Flask melakukan **face recognition** dengan database Firebase
5. Flask return hasil: `{ "authorized": true/false, "user": {...} }`
6. ESP32-CAM forward hasil ke **ESP8266** via HTTP POST `/face-result`
7. **ESP8266** ambil keputusan:
   - ✅ **Authorized** → Servo unlock (90°), LED hijau, auto-lock 5s
   - ❌ **Denied** → Buzzer berbunyi, LED merah blink
8. Log disimpan ke **Firebase RTDB**
9. **React Dashboard** monitoring real-time

### Face Recognition Flow

```
[ESP32-CAM] → Capture Image
     ↓
[Flask API] → Face Detection
     ↓
[Firebase] → Compare with Database
     ↓
[Flask API] → Return Result (authorized/denied)
     ↓
[ESP32-CAM] → Send to ESP8266
     ↓
[ESP8266] → Control Actuators
     ↓
[Servo/LED/Buzzer] → Physical Action
```

## 📊 Firebase Database Structure

```json
{
  "users": {
    "user_20241125120000": {
      "name": "John Doe",
      "email": "john@example.com",
      "phone": "08123456789",
      "face_encoding": [0.123, -0.456, ...],
      "registered_at": "2024-11-25T12:00:00",
      "status": "active",
      "model": "face_recognition"
    }
  },
  "access_logs": {
    "-NxYZ123abc": {
      "timestamp": "2024-11-25T12:05:00",
      "authorized": true,
      "user_id": "user_20241125120000",
      "user_name": "John Doe",
      "confidence": 95.5
    }
  },
  "sensors": {
    "pir": { "motion": false },
    "servo": { "angle": 0, "locked": true },
    "led": { "red": true, "green": false },
    "buzzer": { "active": false }
  }
}
```

## 🎨 Frontend Features

### Dashboard Components
- **CameraFeed**: Live stream dari ESP32-CAM
- **SensorStatus**: Status sensor real-time
- **ActuatorControl**: Kontrol servo, LED, buzzer
- **AccessLog**: History akses masuk/keluar
- **SecurityChart**: Grafik aktivitas
- **UsersList**: Manajemen user

### Available Actions
- ✅ Register new user dengan foto
- ✅ View all registered users
- ✅ Delete user
- ✅ View access logs
- ✅ Real-time sensor monitoring

## 🔐 Security

- Face encodings disimpan di Firebase (tidak menyimpan foto)
- HTTPS recommended untuk production
- Firebase Security Rules harus diatur
- Rate limiting untuk API endpoints

## 🧪 Testing

### 1. Test Flask Server
```powershell
# Run server
python app_face_recognition.py

# Test endpoint (browser or curl)
http://localhost:5000/api/health

# Expected response:
{
  "status": "healthy",
  "model": "face_recognition",
  "timestamp": "..."
}
```

### 2. Test ESP8266
```powershell
# Upload code and open Serial Monitor (115200 baud)
# Expected output:
- WiFi Connected
- IP Address: 192.168.1.100
- Web server started
- System is ready!

# Test web server (browser)
http://192.168.1.100/status
```

### 3. Test ESP32-CAM
```powershell
# Upload code and open Serial Monitor (115200 baud)
# Expected output:
- Camera initialized successfully
- WiFi Connected
- IP Address: 192.168.1.101
- System is ready!
- Capturing image...
```

### 4. Test Full System
1. Register user via web dashboard (http://localhost:3000)
2. Upload foto wajah
3. Letakkan wajah di depan ESP32-CAM
4. Monitor Serial Monitor kedua device
5. Verify: ESP8266 membuka pintu (servo bergerak)

## 🐛 Troubleshooting

### Backend Issues

**Problem:** TensorFlow DLL Error
**Solution:** Sudah fixed! Menggunakan `face_recognition` library (dlib)

**Problem:** Flask server tidak bisa diakses dari ESP32
**Solution:** 
- Check firewall Windows
- Pastikan Flask run di `0.0.0.0` bukan `127.0.0.1`
- Cek IP address dengan `ipconfig`

### Hardware Issues

**Problem:** ESP32-CAM tidak bisa upload
**Solution:**
- Short IO0 ke GND saat upload
- Gunakan adaptor 5V 2A (bukan USB PC)
- Lepas short setelah upload

**Problem:** Servo tidak bergerak
**Solution:**
- Check power supply (minimal 5V 1A)
- Check wiring signal ke D3
- Test manual: `doorServo.write(90);`

**Problem:** Camera capture failed
**Solution:**
- Adaptor power kurang (minimal 2A)
- Frame size terlalu besar (ganti ke VGA)
- Reset ESP32-CAM

**Problem:** ESP8266 tidak terima data dari ESP32-CAM
**Solution:**
- Check IP address ESP8266 di Serial Monitor
- Update IP di code ESP32-CAM
- Pastikan kedua device di network yang sama
- Check endpoint `/face-result`

### Network Issues

**Problem:** WiFi tidak connect
**Solution:**
- SSID dan password benar?
- WiFi harus 2.4GHz (bukan 5GHz)
- Jarak ke router terlalu jauh
- Reset dan upload ulang

**Problem:** ESP32-CAM tidak bisa kirim ke Flask
**Solution:**
- Check firewall PC
- Pastikan Flask server running
- Check IP address PC dengan `ipconfig`
- Test dengan Postman dulu

## 📝 License

MIT License

## 👨‍💻 Author

Smart Home Face Recognition System with ESP32-CAM

## 🤝 Contributing

Pull requests are welcome!

## 📞 Support

Jika ada masalah, silakan buat issue di repository ini.
