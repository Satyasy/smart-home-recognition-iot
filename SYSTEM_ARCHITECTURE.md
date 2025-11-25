# System Architecture & Data Flow

## Overview

Sistem Smart Home Face Recognition ini terdiri dari 3 komponen utama yang bekerja secara terintegrasi:

1. **ESP32-CAM** - Face recognition camera
2. **ESP8266** - Sensor & Actuator controller
3. **Flask Server** - AI/ML face recognition engine
4. **React Dashboard** - Web monitoring interface
5. **Firebase RTDB** - Real-time database

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     SMART HOME SYSTEM                           │
└─────────────────────────────────────────────────────────────────┘

                        ┌──────────────┐
                        │   Internet   │
                        └──────┬───────┘
                               │
            ┌──────────────────┼──────────────────┐
            │                  │                  │
            ▼                  ▼                  ▼
    ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
    │  ESP32-CAM   │   │ Flask Server │   │   Firebase   │
    │  (Camera)    │──▶│(Face Recog.) │──▶│   (RTDB)     │
    └──────┬───────┘   └──────┬───────┘   └──────┬───────┘
           │                  │                   │
           │ HTTP POST        │ REST API          │
           │ (Face Result)    │                   │
           │                  │                   │
           ▼                  │                   │
    ┌──────────────┐          │                   │
    │   ESP8266    │          │                   │
    │ (Controller) │◀─────────┘                   │
    └──────┬───────┘                              │
           │                                       │
           │                                       │
    ┌──────┴───────┬─────────┬─────────┬─────────┤
    ▼              ▼         ▼         ▼         ▼
┌────────┐   ┌────────┐ ┌────────┐ ┌────────┐ ┌──────────┐
│ Servo  │   │  LED   │ │Buzzer  │ │  DHT11 │ │   LDR    │
│(Door)  │   │ R & G  │ │(Alert) │ │(Temp)  │ │ (Light)  │
└────────┘   └────────┘ └────────┘ └────────┘ └──────────┘

                           ┌──────────────┐
                           │    React     │
                           │  Dashboard   │◀────┐
                           └──────────────┘     │
                                  ▲              │
                                  │              │
                                  └──────────────┘
                                  WebSocket / REST API
```

---

## Data Flow Sequence

### Scenario 1: Face Recognition Success (Door Unlock)

```
┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│ESP32-CAM │     │  Flask   │     │ ESP8266  │     │ Firebase │
└────┬─────┘     └────┬─────┘     └────┬─────┘     └────┬─────┘
     │                │                │                │
     │ 1. Capture     │                │                │
     │    Image       │                │                │
     │◄───────────────┤                │                │
     │                │                │                │
     │ 2. POST        │                │                │
     │    /recognize  │                │                │
     │    (base64)    │                │                │
     ├───────────────►│                │                │
     │                │                │                │
     │                │ 3. Face        │                │
     │                │    Recognition │                │
     │                │    Processing  │                │
     │                │                │                │
     │                │ 4. Check       │                │
     │                │    Database    │                │
     │                ├────────────────┼───────────────►│
     │                │◄───────────────┼────────────────┤
     │                │                │                │
     │ 5. Response    │                │                │
     │    {authorized:│                │                │
     │     true}      │                │                │
     │◄───────────────┤                │                │
     │                │                │                │
     │ 6. POST        │                │                │
     │    /face-result│                │                │
     │    to ESP8266  │                │                │
     ├────────────────┼───────────────►│                │
     │                │                │                │
     │                │                │ 7. Unlock Door │
     │                │                │    - Servo 90° │
     │                │                │    - LED Green │
     │                │                │    - Buzzer OFF│
     │                │                │                │
     │                │                │ 8. Log Access  │
     │                │                ├───────────────►│
     │                │                │                │
     │                │                │ 9. Auto Lock   │
     │                │                │    (5 seconds) │
     │                │                │                │
```

### Scenario 2: Face Recognition Failed (Alert)

```
┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│ESP32-CAM │     │  Flask   │     │ ESP8266  │     │ Firebase │
└────┬─────┘     └────┬─────┘     └────┬─────┘     └────┬─────┘
     │                │                │                │
     │ 1. Capture     │                │                │
     │    Image       │                │                │
     │                │                │                │
     │ 2. POST        │                │                │
     │    /recognize  │                │                │
     ├───────────────►│                │                │
     │                │                │                │
     │                │ 3. Face        │                │
     │                │    Recognition │                │
     │                │    Processing  │                │
     │                │                │                │
     │ 5. Response    │                │                │
     │    {authorized:│                │                │
     │     false}     │                │                │
     │◄───────────────┤                │                │
     │                │                │                │
     │ 6. POST        │                │                │
     │    /face-result│                │                │
     │    (denied)    │                │                │
     ├────────────────┼───────────────►│                │
     │                │                │                │
     │                │                │ 7. Trigger     │
     │                │                │    Alert       │
     │                │                │    - Buzzer ON │
     │                │                │    - LED Red   │
     │                │                │    Blink       │
     │                │                │                │
     │                │                │ 8. Log Failed  │
     │                │                │    Access      │
     │                │                ├───────────────►│
```

---

## Component Details

### 1. ESP32-CAM Module

**Responsibilities:**
- Capture foto dari kamera
- Convert image ke base64
- Send ke Flask server via HTTP POST
- Terima response face recognition
- Forward hasil ke ESP8266

**Endpoints Called:**
- `POST http://FLASK_IP:5000/api/recognize`
- `POST http://ESP8266_IP/face-result`

**Operating Mode:**
- Continuous capture setiap 3 detik
- Automatic face detection
- No sensors, no actuators

**Code Location:**
`esp_code/esp32cam_face_only.ino`

---

### 2. Flask Server (Face Recognition Engine)

**Responsibilities:**
- Terima image base64 dari ESP32-CAM
- Face detection & encoding
- Compare dengan database (Firebase)
- Return authorization result
- Log access attempts

**API Endpoints:**
```
POST   /api/recognize     - Face recognition
POST   /api/register      - Register new user
GET    /api/users         - Get all users
GET    /api/user/{id}     - Get user detail
PUT    /api/user/{id}     - Update user
DELETE /api/user/{id}     - Delete user
GET    /api/logs          - Get access logs
DELETE /api/logs/clear    - Clear logs
GET    /api/health        - Health check
GET    /api/config        - Get configuration
```

**Technology:**
- Python 3.13
- Flask web framework
- face_recognition library (dlib)
- Firebase Admin SDK

**Code Location:**
`app_face_recognition.py`

---

### 3. ESP8266 Controller

**Responsibilities:**
- Terima hasil face recognition dari ESP32-CAM
- Kontrol semua actuators (servo, LED, buzzer)
- Baca semua sensors (DHT, LDR, ultrasonic)
- Expose web server untuk monitoring
- Auto-lock door setelah 5 detik

**Web Server Endpoints:**
```
POST /face-result  - Receive recognition result
GET  /status       - Get sensor status
GET  /unlock       - Manual unlock
GET  /lock         - Manual lock
```

**Sensors:**
- DHT11 - Temperature & Humidity
- LDR - Light intensity
- HC-SR04 - Distance measurement

**Actuators:**
- Servo SG90 - Door lock (0° = locked, 90° = unlocked)
- LED Red - Door locked indicator
- LED Green - Door unlocked indicator
- Buzzer - Alert for unauthorized access

**Code Location:**
`esp_code/esp8266_main.ino`

---

### 4. Firebase Realtime Database

**Data Structure:**
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
    "temperature": 28.5,
    "humidity": 65.0,
    "light": 850,
    "distance": 45.2,
    "door_locked": false
  }
}
```

**Usage:**
- Store registered users with face encodings
- Log all access attempts (success & failed)
- Store real-time sensor data
- Sync with web dashboard

---

### 5. React Dashboard

**Features:**
- Real-time monitoring
- User management (register, delete)
- Access logs visualization
- Sensor data charts
- Manual door control
- System health status

**Pages:**
- Dashboard - Main overview
- Users - User management
- Logs - Access history
- Settings - Configuration

**Code Location:**
`smart-home-recognition/src/`

---

## Network Configuration

### IP Address Setup

```
Network: 192.168.1.0/24 (example)

Devices:
- Router:        192.168.1.1
- PC/Laptop:     192.168.1.50  (Flask Server)
- ESP8266:       192.168.1.100 (DHCP or Static)
- ESP32-CAM:     192.168.1.101 (DHCP or Static)
- Smartphone:    192.168.1.xxx (Access dashboard)
```

### Port Configuration

```
Flask Server:    http://192.168.1.50:5000
ESP8266 Server:  http://192.168.1.100:80
React Dashboard: http://192.168.1.50:3000
Firebase:        https://project.firebaseio.com
```

---

## Communication Protocols

### 1. ESP32-CAM → Flask Server
- Protocol: HTTP POST
- Format: JSON
- Content: Base64 encoded image
- Timeout: 15 seconds

### 2. Flask Server → Firebase
- Protocol: Firebase Admin SDK
- Authentication: Service Account Key
- Operations: Read/Write to RTDB

### 3. ESP32-CAM → ESP8266
- Protocol: HTTP POST
- Format: JSON
- Content: Recognition result
- Retry: 3 attempts

### 4. ESP8266 → Client (Dashboard)
- Protocol: HTTP GET/POST
- Format: JSON
- Content: Sensor data, status

### 5. React Dashboard → Flask/Firebase
- Protocol: REST API / WebSocket
- Format: JSON
- Real-time updates via polling

---

## Security Considerations

1. **Network Security**
   - Use WPA2 WiFi encryption
   - Change default passwords
   - Implement MAC filtering (optional)

2. **API Security**
   - Add API key authentication
   - Rate limiting
   - HTTPS for production

3. **Face Recognition**
   - Adjust tolerance (0.6 default)
   - Store only encodings, not images
   - Implement liveness detection (future)

4. **Firebase Security**
   - Configure security rules
   - Restrict read/write access
   - Enable authentication

---

## Performance Metrics

### ESP32-CAM
- Capture time: ~200ms
- Processing time: depends on Flask
- Memory usage: ~50KB per image
- Power consumption: ~180mA @ 5V

### Flask Server
- Recognition time: 1-3 seconds
- CPU usage: Medium
- Memory: ~200MB
- Concurrent requests: 1 (can be scaled)

### ESP8266
- Response time: <100ms
- Sensor read: ~50ms each
- Power consumption: ~70mA @ 5V

---

## Future Enhancements

1. **Add MQTT Protocol** - For better real-time communication
2. **Implement OTA Updates** - Remote firmware updates
3. **Add More Sensors** - Fingerprint, RFID, PIR motion
4. **Mobile App** - Native Android/iOS app
5. **Edge AI** - Run face recognition on ESP32
6. **Notification System** - Push notifications on events
7. **Video Recording** - Save video on unauthorized access
8. **Multi-Language** - Support for multiple languages
9. **Voice Control** - Integration with Alexa/Google Home
10. **Battery Backup** - UPS for power failure

---

## Maintenance

### Regular Tasks
- Check sensor calibration (monthly)
- Update firmware (quarterly)
- Clean camera lens (weekly)
- Backup database (daily)
- Test actuators (weekly)
- Review access logs (daily)

### Troubleshooting Checklist
- [ ] WiFi connection stable?
- [ ] Power supply adequate?
- [ ] All sensors responding?
- [ ] Camera capture working?
- [ ] Face recognition accurate?
- [ ] Servo movement smooth?
- [ ] LED indicators working?
- [ ] Buzzer functioning?
- [ ] Firebase connected?
- [ ] Dashboard accessible?
