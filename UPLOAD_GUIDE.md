# 📤 Upload Arduino Code ke ESP8266

## 📋 Prerequisites

### 1. Install Arduino IDE
Download dari: https://www.arduino.cc/en/software

### 2. Install ESP8266 Board Package
1. Buka Arduino IDE
2. File → Preferences
3. Di "Additional Board Manager URLs", tambahkan:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Tools → Board → Boards Manager
5. Cari "esp8266" → Install "ESP8266 by ESP8266 Community"

### 3. Install Required Libraries
Tools → Manage Libraries, install:
- **ArduinoJson** (by Benoit Blanchon) - untuk JSON parsing
- **ESP8266WiFi** (included with board package)
- **ESP8266WebServer** (included with board package)

## 🔧 Upload Test Server ke ESP8266

### Step 1: Konfigurasi Code

Edit file `esp8266_test_server.ino`:

```cpp
// Ganti dengan WiFi credentials Anda
const char* ssid = "YOUR_WIFI_SSID";        
const char* password = "YOUR_WIFI_PASSWORD"; 

// Pastikan IP sesuai dengan jaringan Anda
IPAddress local_IP(192, 168, 5, 250);  // Target IP
IPAddress gateway(192, 168, 5, 1);     // Router IP
IPAddress subnet(255, 255, 255, 0);
```

**Cara cek Gateway router:**
```powershell
# Windows
ipconfig

# Cari "Default Gateway" di adapter WiFi aktif
# Example: 192.168.5.1 atau 192.168.1.1
```

### Step 2: Connect ESP8266

1. Hubungkan ESP8266 ke PC via USB
2. Install CH340 driver jika belum (untuk NodeMCU clone)
   - Download: http://www.wch.cn/downloads/CH341SER_EXE.html

### Step 3: Configure Arduino IDE

**Board Settings:**
- Tools → Board → ESP8266 Boards → **NodeMCU 1.0 (ESP-12E Module)**
- Tools → Upload Speed → **115200**
- Tools → CPU Frequency → **80 MHz**
- Tools → Flash Size → **4MB (FS:2MB OTA:~1019KB)**
- Tools → Port → **COM3** (atau port ESP8266 Anda)

**Cara cek Port:**
1. Cabut ESP8266 → lihat port yang tersedia
2. Colok ESP8266 → port baru yang muncul adalah port ESP8266

### Step 4: Upload Code

1. Open `esp8266_test_server.ino` di Arduino IDE
2. Klik **Verify** (✓) untuk compile
3. Jika sukses, klik **Upload** (→)
4. Tunggu hingga selesai (muncul "Done uploading")

**Expected output saat upload:**
```
Sketch uses 292184 bytes (28%) of program storage space.
Global variables use 27564 bytes (33%) of dynamic memory.
Uploading...
Writing at 0x00000000... (100%)
Hard resetting via RTS pin...
```

### Step 5: Verify Upload Success

1. Buka **Serial Monitor** (Ctrl+Shift+M)
2. Set baud rate: **115200**
3. Press **Reset button** di ESP8266

**Expected Serial Monitor output:**
```
=================================
ESP8266 Test Server
=================================
Connecting to WiFi: YourSSID
......
✅ WiFi Connected!
📡 IP Address: 192.168.5.250
🌐 Gateway: 192.168.5.1
📶 Signal Strength: -45 dBm
🚀 HTTP Server Started!
=================================

Test endpoints:
  http://192.168.5.250/sensor
  http://192.168.5.250/status
```

## ✅ Verify Connection

### Test 1: Ping
```powershell
ping 192.168.5.250
```
Expected: Reply dengan time < 10ms

### Test 2: Browser
Buka di browser:
```
http://192.168.5.250
```
Expected: HTML page "ESP8266 Test Server"

### Test 3: API Endpoint
```
http://192.168.5.250/sensor
```
Expected: JSON response
```json
{
  "temperature": 25.5,
  "humidity": 65.0,
  "light": 450,
  "distance": 15.5
}
```

### Test 4: Dashboard
1. Buka React dashboard: `http://localhost:3000`
2. Klik tombol **"Test ESP8266"**
3. Check console (F12) untuk hasil test
4. Status indicator harus **"ESP8266: Connected"** (hijau)

## 🐛 Troubleshooting Upload

### Error: "espcomm_open failed"
**Cause:** Port tidak bisa diakses

**Solution:**
1. Close Serial Monitor
2. Disconnect/reconnect USB
3. Try different USB port
4. Install CH340 driver

### Error: "Timed out waiting for packet header"
**Cause:** ESP8266 tidak masuk upload mode

**Solution:**
1. Hold **Flash button** (GPIO0)
2. Press **Reset button**
3. Release Reset, then Flash
4. Click Upload

### Error: "Board not found on COM port"
**Cause:** Wrong port atau driver issue

**Solution:**
```powershell
# Check COM ports
mode
# atau
Get-WmiObject Win32_SerialPort | Select-Object Name,DeviceID
```

Install driver CH340: http://www.wch.cn/downloads/CH341SER_EXE.html

### WiFi tidak connect
**Serial Monitor shows:** `Connecting to WiFi: YourSSID........` (infinite dots)

**Solution:**
1. Check SSID dan password benar
2. ESP8266 hanya support **2.4GHz WiFi** (tidak support 5GHz)
3. Pastikan WiFi tidak hidden
4. Try DHCP mode (comment out static IP):
   ```cpp
   // if (!WiFi.config(local_IP, gateway, subnet)) {
   //   Serial.println("Static IP Failed to configure");
   // }
   ```

### IP Address berbeda dari yang diset
**Serial shows:** `📡 IP Address: 192.168.5.XXX` (bukan 250)

**Cause:** Static IP gagal, menggunakan DHCP

**Solution:**
1. Check gateway IP benar
2. Pastikan IP 192.168.5.250 belum digunakan device lain
3. Restart router
4. Atau gunakan IP yang didapat DHCP:
   - Update di `.env`:
     ```env
     REACT_APP_ESP8266_IP=192.168.5.XXX
     ```
   - Restart React dev server

## 🔄 Update Code

Jika perlu update code:
1. Edit `.ino` file
2. Click Upload di Arduino IDE
3. Tunggu upload selesai
4. ESP8266 akan auto-restart
5. Check Serial Monitor untuk verify

## 📊 Monitor HTTP Requests

Serial Monitor akan menampilkan setiap request:
```
[HTTP] GET /sensor - 200 OK
[HTTP] GET /status - 200 OK
[HTTP] POST /unlock - Door UNLOCKED
[HTTP] POST /lock - Door LOCKED
```

Jika dashboard connect, akan ada request setiap 2 detik:
```
[HTTP] GET /sensor - 200 OK
[HTTP] GET /status - 200 OK
[HTTP] GET /sensor - 200 OK
[HTTP] GET /status - 200 OK
...
```

## 🚀 Next Steps

Setelah test server berfungsi:
1. ✅ Verify dashboard connect ke ESP8266
2. ✅ Test unlock/lock dari dashboard
3. ✅ Test PIN verification
4. 📦 Replace dengan full code + sensors (DHT11, LDR, HC-SR04, Servo, LCD)
5. 📦 Upload code ke ESP32-CAM untuk face recognition

## 📝 Notes

- Code ini **hanya untuk testing** koneksi
- Menggunakan **mock data**, bukan sensor real
- Setelah dashboard connect, bisa lanjut ke implementasi sensor actual
- Full sensor code akan ada di folder `esp8266_full_code/`
