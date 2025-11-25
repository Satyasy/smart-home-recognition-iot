# Arduino Libraries Required

## ESP32-CAM Libraries

### Core Libraries (Built-in with ESP32 Board)
- esp_camera.h
- WiFi.h
- soc/soc.h
- soc/rtc_cntl_reg.h

### External Libraries (Install via Arduino Library Manager)
1. **ArduinoJson** (by Benoit Blanchon)
   - Version: 6.x atau lebih baru
   - Install: Arduino IDE → Tools → Manage Libraries → Search "ArduinoJson"

2. **HTTPClient** (Built-in with ESP32)
   - Tidak perlu install manual

3. **Base64** (by Arturo Guadalupi)
   - Install: Library Manager → Search "Base64 by Arturo Guadalupi"
   - Alternative: Use built-in base64 dari mbedtls

### Board Manager
- **ESP32 Board Support**
  - URL: https://dl.espressif.com/dl/package_esp32_index.json
  - Install: File → Preferences → Additional Board Manager URLs
  - Tools → Board → Boards Manager → Search "ESP32" → Install

---

## ESP8266 Libraries

### Core Libraries (Built-in with ESP8266 Board)
- ESP8266WiFi.h
- ESP8266WebServer.h

### External Libraries (Install via Arduino Library Manager)
1. **ArduinoJson** (by Benoit Blanchon)
   - Version: 6.x
   - Install: Library Manager → Search "ArduinoJson"

2. **Servo** (Built-in)
   - Sudah include di ESP8266 core

3. **DHT sensor library** (by Adafruit)
   - Install: Library Manager → Search "DHT sensor library"
   - Dependencies: Juga install "Adafruit Unified Sensor"

4. **LiquidCrystal I2C** (by Frank de Brabander)
   - Install: Library Manager → Search "LiquidCrystal I2C"
   - Version: 1.1.2 atau lebih baru
   - Purpose: Control LCD I2C 16x2 display

### Board Manager
- **ESP8266 Board Support**
  - URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Install: File → Preferences → Additional Board Manager URLs
  - Tools → Board → Boards Manager → Search "ESP8266" → Install

---

## Installation Steps

### 1. Install Arduino IDE
- Download dari: https://www.arduino.cc/en/software
- Versi minimal: 1.8.19 atau Arduino IDE 2.x

### 2. Install Board Support

**Untuk ESP32:**
```
File → Preferences
Additional Board Manager URLs:
https://dl.espressif.com/dl/package_esp32_index.json

Tools → Board → Boards Manager
Search: "ESP32"
Install: "ESP32 by Espressif Systems"
```

**Untuk ESP8266:**
```
File → Preferences
Additional Board Manager URLs:
http://arduino.esp8266.com/stable/package_esp8266com_index.json

Tools → Board → Boards Manager
Search: "ESP8266"
Install: "esp8266 by ESP8266 Community"
```

### 3. Install Libraries

**Via Library Manager (Recommended):**
```
Tools → Manage Libraries

Search and Install:
1. ArduinoJson (v6.21.0 or newer)
2. DHT sensor library (v1.4.4 or newer)
3. Adafruit Unified Sensor (v1.1.9 or newer)
4. Base64 (v1.4.0 or newer) - for ESP32-CAM
```

**Via Manual Installation:**
```
1. Download library ZIP
2. Sketch → Include Library → Add .ZIP Library
3. Select downloaded ZIP file
```

### 4. Board Configuration

**ESP32-CAM:**
```
Tools → Board → ESP32 Arduino → AI Thinker ESP32-CAM
Tools → CPU Frequency → 240MHz
Tools → Flash Frequency → 80MHz
Tools → Flash Mode → QIO
Tools → Flash Size → 4MB (32Mb)
Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)
Tools → Core Debug Level → None
Tools → Upload Speed → 115200
```

**ESP8266 NodeMCU:**
```
Tools → Board → ESP8266 Boards → NodeMCU 1.0 (ESP-12E Module)
Tools → CPU Frequency → 80MHz
Tools → Flash Size → 4MB (FS:2MB OTA:~1019KB)
Tools → Upload Speed → 115200
Tools → Erase Flash → Only Sketch
```

---

## Library Dependencies Matrix

| Library | ESP32-CAM | ESP8266 | Version | Purpose |
|---------|-----------|---------|---------|---------|
| WiFi | ✅ Built-in | ✅ Built-in | - | WiFi connection |
| ArduinoJson | ✅ Required | ✅ Required | 6.x | JSON parsing |
| esp_camera | ✅ Built-in | ❌ N/A | - | Camera control |
| Base64 | ✅ Required | ❌ N/A | 1.4+ | Image encoding |
| HTTPClient | ✅ Built-in | ✅ Built-in | - | HTTP requests |
| ESP8266WebServer | ❌ N/A | ✅ Built-in | - | Web server |
| Servo | ❌ N/A | ✅ Built-in | - | Servo control |
| DHT | ❌ N/A | ✅ Required | 1.4+ | DHT sensor |
| LiquidCrystal_I2C | ❌ N/A | ✅ Required | 1.1.2+ | LCD I2C display |
| Wire | ❌ N/A | ✅ Built-in | - | I2C communication |

---

## Common Issues & Solutions

### Issue: "esp_camera.h: No such file or directory"
**Solution:** Install ESP32 board support via Board Manager

### Issue: "ArduinoJson.h: No such file or directory"
**Solution:** Install ArduinoJson library via Library Manager

### Issue: "DHT.h: No such file or directory"
**Solution:** 
1. Install "DHT sensor library" by Adafruit
2. Install "Adafruit Unified Sensor" (dependency)

### Issue: "Base64.h: No such file or directory"
**Solution:** Install Base64 library by Arturo Guadalupi

### Issue: "LiquidCrystal_I2C.h: No such file or directory"
**Solution:** 
1. Install "LiquidCrystal I2C" by Frank de Brabander
2. Restart Arduino IDE setelah instalasi

### Issue: LCD tidak menampilkan apa-apa
**Solution:**
1. Scan alamat I2C dengan i2c_scanner sketch
2. Ubah alamat di code dari 0x27 ke 0x3F (atau sebaliknya)
3. Putar potensiometer di belakang LCD untuk adjust brightness
4. Cek koneksi SDA/SCL ke pin yang benar

### Issue: Upload error "Serial port not found"
**Solution:**
- Install CP2102 or CH340 USB driver
- Check cable connection
- Try different USB port
- For ESP32-CAM: Short IO0 to GND during upload

### Issue: "A fatal error occurred: Failed to connect"
**Solution:**
- ESP32-CAM: Press Reset button, short IO0 to GND
- ESP8266: Press Flash button while uploading
- Reduce upload speed to 115200

### Issue: Out of memory during compilation
**Solution:**
- Use "Huge APP" partition scheme for ESP32
- Reduce JSON buffer size
- Disable debug output

---

## Verification

### Test ESP32-CAM Libraries
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "Base64.h"

void setup() {
  Serial.begin(115200);
  Serial.println("All libraries loaded successfully!");
}

void loop() {}
```

### Test ESP8266 Libraries
```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void setup() {
  Serial.begin(115200);
  Serial.println("All libraries loaded successfully!");
}

void loop() {}
```

---

## Additional Resources

- ESP32-CAM Documentation: https://docs.espressif.com/
- ESP8266 Arduino Core: https://arduino-esp8266.readthedocs.io/
- ArduinoJson Documentation: https://arduinojson.org/
- DHT Sensor Guide: https://learn.adafruit.com/dht

---

## Update Libraries

Untuk update library ke versi terbaru:
```
Tools → Manage Libraries
Click "Update" pada library yang ada update baru
```

⚠️ **Note:** Selalu backup code sebelum update library, karena API bisa berubah.
