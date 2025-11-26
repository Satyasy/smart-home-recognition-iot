# 📷 ESP32-CAM Setup & Streaming Guide

## 🎯 Overview

ESP32-CAM akan menjalankan:
1. **Camera streaming** di port 81 (`http://192.168.5.86:81/stream`)
2. **Face recognition** - capture & kirim ke Flask backend
3. **HTTP API** di port 80 untuk control

## 📦 Hardware Requirements

- **ESP32-CAM AI-Thinker** (1x)
- **FTDI Programmer** atau **USB to TTL** untuk upload
- **Jumper wires** (Female-Female)
- **Power supply 5V** (minimal 500mA, recommended 1A)

## 🔌 Wiring for Programming

### ESP32-CAM to FTDI Programmer

```
ESP32-CAM        FTDI
━━━━━━━━━━━━━━━━━━━━━━━
5V       ----->  VCC (5V)
GND      ----->  GND
U0R (RX) ----->  TX
U0T (TX) ----->  RX
```

### Programming Mode

**Untuk masuk mode programming:**
1. Connect **GPIO 0** to **GND** dengan jumper
2. Power on ESP32-CAM
3. Upload code
4. **Remove jumper** setelah upload
5. Press **Reset button**

```
Programming Mode:
GPIO 0 ----[JUMPER]---- GND

Normal Mode:
GPIO 0             (no connection)
```

## 📥 Upload Code

### Step 1: Install Arduino IDE & ESP32 Board

```
1. Download Arduino IDE: https://www.arduino.cc/en/software
2. Install ESP32 Board Package:
   - File → Preferences
   - Additional Board URLs:
     https://dl.espressif.com/dl/package_esp32_index.json
   - Tools → Board → Boards Manager
   - Search "ESP32" → Install "ESP32 by Espressif Systems"
```

### Step 2: Install Libraries

Install via Library Manager (Tools → Manage Libraries):
- **ArduinoJson** by Benoit Blanchon (v6.x)
- **ESP32Servo** by Kevin Harrington (optional, tidak dipakai di ESP32-CAM)
- **base64** by Densaugeo

### Step 3: Configure ESP32-CAM Code

Edit `esp32_face_recognition.ino`:

```cpp
// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Static IP Configuration
IPAddress local_IP(192, 168, 5, 86);   // ESP32-CAM IP
IPAddress gateway(192, 168, 5, 1);     // Your router IP
IPAddress subnet(255, 255, 255, 0);

// Server Configuration
const char* flaskServer = "http://192.168.5.221:5000";  // Backend
const char* esp8266Server = "http://192.168.5.250";     // ESP8266
```

### Step 4: Arduino IDE Settings

**Board Configuration:**
```
Tools → Board → ESP32 Arduino → AI Thinker ESP32-CAM
Tools → Upload Speed → 115200
Tools → Flash Frequency → 80MHz
Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)
Tools → Core Debug Level → None
Tools → Port → (your COM port)
```

### Step 5: Upload Process

```
1. Connect GPIO 0 to GND (programming mode)
2. Connect FTDI to PC
3. Click Upload button in Arduino IDE
4. Wait for "Connecting........"
5. Press Reset button on ESP32-CAM jika stuck di "Connecting"
6. Wait until "Hard resetting via RTS pin..."
7. Remove GPIO 0 to GND jumper
8. Press Reset button
9. Done!
```

**Expected upload output:**
```
Connecting........_____....._____
Chip is ESP32D0WDQ6 (revision 1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: xx:xx:xx:xx:xx:xx
Uploading stub...
Running stub...
Stub running...
Writing at 0x00001000... (100 %)
Wrote 3072 bytes (138 compressed) at 0x00001000 in 0.0 seconds
...
Hard resetting via RTS pin...
```

## ✅ Verify Upload Success

### Serial Monitor Check

1. Open Serial Monitor (Ctrl+Shift+M)
2. Set baud rate: **115200**
3. Press Reset button on ESP32-CAM

**Expected output:**
```
=================================
ESP32-CAM Face Recognition
=================================
PSRAM found
Camera initialized!
Connecting to WiFi: YourSSID
.....
✅ WiFi Connected!
📡 IP: 192.168.5.86
📶 RSSI: -45 dBm
✅ HTTP server started on port 80
Starting stream server on port: 81
✅ Stream server started successfully
=================================
System Ready!
Stream URL: http://192.168.5.86:81/stream
Web Interface: http://192.168.5.86
=================================
```

### Network Test

```powershell
# Test ping
ping 192.168.5.86

# Test web interface (browser)
http://192.168.5.86

# Test camera stream (browser)
http://192.168.5.86:81/stream
```

## 🎥 Camera Stream Testing

### Method 1: Browser

Open in browser:
```
http://192.168.5.86:81/stream
```

Should show live MJPEG stream from camera.

### Method 2: VLC Media Player

```
1. Open VLC
2. Media → Open Network Stream
3. URL: http://192.168.5.86:81/stream
4. Play
```

### Method 3: Dashboard

```
1. Open React dashboard: http://localhost:3000
2. Camera feed component should show live stream
3. If offline, click "Refresh" button
```

## 🔧 Camera Stream in Dashboard

### Auto-Detection

Dashboard akan otomatis load stream dari:
```javascript
const camIp = process.env.REACT_APP_ESP32_CAM_IP || '192.168.5.86';
const streamUrl = `http://${camIp}:81/stream`;
```

### Manual Configuration

Edit `.env`:
```env
REACT_APP_ESP32_CAM_IP=192.168.5.86
```

Restart React dev server:
```bash
# Stop current server (Ctrl+C)
npm start
```

### Stream Display

Camera feed component will show:
- ✅ **LIVE** indicator when streaming
- ⚠️ **CONNECTING** during connection
- ❌ **OFFLINE** if no connection
- Refresh button untuk reconnect
- Stream resolution: 640x480

## 🤖 Face Recognition Flow

### Automatic Mode (Default)

```
1. ESP32-CAM captures frame every 3 seconds
2. Converts to base64
3. Sends POST request to Flask backend (/api/recognize)
4. Backend processes face recognition
5. If face matched:
   - ESP32-CAM sends unlock to ESP8266
   - Door unlocks
   - Green LED on
6. If not matched:
   - Trigger alert
   - Red LED stays on
   - Buzzer beeps
```

### Manual Capture

From dashboard or web interface:
```javascript
// Dashboard button "Capture & Recognize"
fetch('http://192.168.5.86/capture')
  .then(response => response.json())
  .then(data => console.log(data));
```

## 📊 API Endpoints

### ESP32-CAM HTTP API (Port 80)

#### GET /
Web interface dengan live stream

#### GET /status
```json
{
  "camera": "online",
  "ip": "192.168.5.86",
  "stream_url": "http://192.168.5.86:81/stream",
  "rssi": -45
}
```

#### GET /capture
Manually trigger face recognition
```json
{
  "success": true,
  "message": "Face recognition initiated"
}
```

### Camera Stream (Port 81)

#### GET /stream
MJPEG stream
- Format: multipart/x-mixed-replace; boundary=frame
- Resolution: 640x480 (VGA)
- FPS: ~15-30 (depends on network)
- CORS enabled

## 🐛 Troubleshooting

### Upload Failed

**Problem:** "Failed to connect to ESP32"

**Solutions:**
1. **Check wiring:** TX ↔ RX must be swapped
2. **GPIO 0 to GND:** Must be connected during upload
3. **Press Reset:** When Arduino IDE shows "Connecting..."
4. **Reduce upload speed:** Tools → Upload Speed → 115200
5. **Try different USB port**
6. **Check FTDI power:** 5V vs 3.3V (use 5V)

### Camera Not Initialized

**Serial shows:** `Camera init failed: 0x105`

**Solutions:**
1. **Check camera connection:** OV2640 module properly seated
2. **Power issue:** Use external 5V 1A supply
3. **PSRAM not found:** Some ESP32-CAM don't have PSRAM
   ```cpp
   // Change in code:
   if(psramFound()){
     config.frame_size = FRAMESIZE_VGA;  // Lower resolution
   ```

### WiFi Not Connecting

**Serial shows:** Infinite dots `.....................`

**Solutions:**
1. **2.4GHz only:** ESP32-CAM doesn't support 5GHz WiFi
2. **SSID/Password:** Check credentials
3. **Try DHCP:** Comment out static IP config
   ```cpp
   // if (!WiFi.config(local_IP, gateway, subnet)) {
   //   Serial.println("Static IP Failed");
   // }
   ```

### Stream Not Loading in Dashboard

**Symptoms:** Camera shows "Offline" or blank

**Check:**
1. **Ping test:** `ping 192.168.5.86`
2. **Direct stream:** Open `http://192.168.5.86:81/stream` in browser
3. **CORS:** ESP32-CAM sends CORS headers
4. **Firewall:** Allow port 81
5. **Network:** ESP32-CAM & PC on same network

**Fix in code:**
```cpp
// In stream_handler:
httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
```

### Stream Slow or Laggy

**Solutions:**
1. **Lower resolution:**
   ```cpp
   config.frame_size = FRAMESIZE_QVGA;  // 320x240
   ```

2. **Increase JPEG quality (lower number = better quality but slower):**
   ```cpp
   config.jpeg_quality = 15;  // Try 12-20
   ```

3. **Check WiFi signal:**
   ```cpp
   Serial.println(WiFi.RSSI());  // Should be > -70 dBm
   ```

4. **Reduce capture interval:**
   ```cpp
   const unsigned long CAPTURE_INTERVAL = 5000;  // 5 seconds
   ```

### Face Recognition Always Fails

**Check:**
1. **Backend running:** Flask on port 5000
2. **User registered:** At least 1 face in database
3. **Lighting:** Good lighting conditions
4. **Distance:** 30-100cm from camera
5. **Network:** ESP32-CAM can reach Flask server

**Test recognition manually:**
```bash
# Capture image from ESP32-CAM
http://192.168.5.86:81/stream

# Test backend
curl -X POST http://192.168.5.221:5000/api/recognize \
  -H "Content-Type: application/json" \
  -d '{"image": "data:image/jpeg;base64,..."}'
```

### ESP32-CAM Resets Frequently

**Causes:**
1. **Brownout:** Insufficient power
2. **Overheating:** Poor ventilation
3. **Bad power supply:** Use quality 5V 1A adapter

**Solutions:**
```cpp
// Disable brownout detector (already in code)
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
```

Add capacitor: 100µF between 5V and GND on ESP32-CAM

## 🎨 Camera Settings Adjustment

Edit in `initCamera()`:

```cpp
sensor_t * s = esp_camera_sensor_get();

// Brightness: -2 to 2 (0 = default)
s->set_brightness(s, 0);

// Contrast: -2 to 2 (0 = default)  
s->set_contrast(s, 0);

// Saturation: -2 to 2 (0 = default)
s->set_saturation(s, 0);

// White balance: 0 = disable, 1 = enable
s->set_whitebal(s, 1);

// Exposure control: 0 = disable, 1 = enable
s->set_exposure_ctrl(s, 1);

// Horizontal mirror: 0 = disable, 1 = enable
s->set_hmirror(s, 0);

// Vertical flip: 0 = disable, 1 = enable
s->set_vflip(s, 0);
```

## 📏 Resolution Options

```cpp
// Available frame sizes:
FRAMESIZE_QVGA    // 320x240   - Fastest, lowest quality
FRAMESIZE_CIF     // 400x296
FRAMESIZE_VGA     // 640x480   - Recommended for streaming
FRAMESIZE_SVGA    // 800x600
FRAMESIZE_XGA     // 1024x768
FRAMESIZE_SXGA    // 1280x1024
FRAMESIZE_UXGA    // 1600x1200 - Highest quality, slowest

// Set in camera_config_t:
config.frame_size = FRAMESIZE_VGA;
```

## 🚀 Testing Checklist

- [ ] Upload successful
- [ ] Serial Monitor shows "System Ready!"
- [ ] WiFi connected dengan IP 192.168.5.86
- [ ] Ping successful
- [ ] Web interface accessible (`http://192.168.5.86`)
- [ ] Stream accessible (`http://192.168.5.86:81/stream`)
- [ ] Dashboard shows camera feed
- [ ] Face recognition captures working
- [ ] Backend receives images
- [ ] Door unlocks when face recognized

## 💡 Pro Tips

1. **Power:** Always use good 5V 1A power supply
2. **Lighting:** Face recognition works best in good lighting
3. **Distance:** Keep face 30-100cm from camera
4. **Angle:** Face camera directly, avoid side angles
5. **WiFi:** Keep ESP32-CAM close to router for stable stream
6. **Heat:** ESP32-CAM gets warm during streaming (normal)
7. **PSRAM:** AI-Thinker model has 4MB PSRAM (check Serial Monitor)

## 🔄 Stream URL Summary

```
Web Interface:  http://192.168.5.86
Camera Stream:  http://192.168.5.86:81/stream
Status API:     http://192.168.5.86/status
Capture API:    http://192.168.5.86/capture

Dashboard:      http://localhost:3000 (auto-loads stream)
```

## 🎯 Integration with Dashboard

Dashboard `CameraFeed.jsx` will:
1. Auto-detect ESP32-CAM from `.env`
2. Load stream via `<img>` tag
3. Show connection status
4. Auto-refresh on error
5. Display face recognition results
6. Show ultrasonic sensor data

Stream will be visible in real-time di dashboard! 📺✨

---

**Quick Test:**
```bash
# Terminal 1: Backend
python app_face_recognition.py

# Terminal 2: Frontend  
cd smart-home-recognition && npm start

# Terminal 3: ESP32-CAM Serial Monitor
# Check IP: 192.168.5.86
# Stream: http://192.168.5.86:81/stream

# Browser: http://localhost:3000
# Should see live camera feed! 🎥
```
