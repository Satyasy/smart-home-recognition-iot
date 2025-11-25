# Wiring Diagram & Setup Guide

## Hardware Requirements

### ESP32-CAM Module
- 1x ESP32-CAM (AI-Thinker)
- 1x FTDI Programmer (untuk upload code)
- Camera built-in (tidak perlu tambahan)

### ESP8266 Module
- 1x NodeMCU ESP8266
- 1x Servo Motor SG90
- 2x LED (Red & Green)
- 1x Buzzer
- 1x DHT11 Temperature & Humidity Sensor
- 1x LDR (Light Dependent Resistor)
- 1x Ultrasonic Sensor HC-SR04
- 1x LCD I2C 16x2 (Address 0x27 atau 0x3F)
- Resistor 10kΩ (untuk LDR)
- Resistor 220Ω x2 (untuk LED)
- Breadboard dan kabel jumper

## Wiring Diagram

### ESP32-CAM (Hanya Kamera)
```
ESP32-CAM:
┌─────────────────────┐
│  ESP32-CAM Module   │
│                     │
│  Built-in Camera    │ → Capture foto
│                     │
│  WiFi Antenna       │ → Komunikasi dengan Flask & ESP8266
│                     │
│  Power: 5V          │
│  GND                │
└─────────────────────┘

Power Supply:
- VCC → 5V
- GND → GND

⚠️ PENTING:
- Gunakan adaptor 5V minimal 2A
- Jangan hubungkan pin lain (kecuali untuk programming)
```

### ESP8266 (Sensor & Aktuator)
```
NodeMCU ESP8266:
┌─────────────────────────────────────────┐
│         NodeMCU ESP8266                 │
├─────────────────────────────────────────┤
│                                         │
│  D0 (GPIO16)  → LED Red     (+Resistor)│
│  D8 (GPIO15)  → LED Green   (+Resistor)│
│  D4 (GPIO2)   → Buzzer      (+)        │
│                                         │
│  D3 (GPIO0)   → Servo Signal (Orange)  │
│                                         │
│  D7 (GPIO13)  → DHT11 Data             │
│                                         │
│  D5 (GPIO14)  → Ultrasonic Trig        │
│  D6 (GPIO12)  → Ultrasonic Echo        │
│                                         │
│  D2 (GPIO4)   → LCD I2C SDA            │
│  D1 (GPIO5)   → LCD I2C SCL            │
│                                         │
│  A0 (ADC)     → LDR (via 10kΩ divider) │
│                                         │
│  3.3V         → DHT11 VCC              │
│  5V (VIN)     → Servo VCC + LCD VCC    │
│  GND          → All GND (Black)        │
└─────────────────────────────────────────┘
```

### Detailed Component Connections

#### 1. Servo Motor (Pintu)
```
Servo SG90:
- Wire Coklat  (GND)    → ESP8266 GND
- Wire Merah   (VCC)    → ESP8266 VIN (5V)
- Wire Orange  (Signal) → ESP8266 D3 (GPIO0)
```

#### 2. LED Red (Pintu Terkunci)
```
LED Red:
- Anode (+)  → ESP8266 D0 (GPIO16)
- Cathode (-) → 220Ω Resistor → GND
```

#### 3. LED Green (Pintu Terbuka)
```
LED Green:
- Anode (+)  → ESP8266 D8 (GPIO15)
- Cathode (-) → 220Ω Resistor → GND
```

#### 4. Buzzer (Alert)
```
Buzzer:
- Positive (+) → ESP8266 D4 (GPIO2)
- Negative (-) → GND
```

#### 5. DHT11 (Temperature & Humidity)
```
DHT11:
- VCC  → ESP8266 3.3V
- Data → ESP8266 D7 (GPIO13) - DIPINDAH dari D1 karena D1 dipakai I2C SCL
- GND  → ESP8266 GND
```

#### 6. LCD I2C 16x2 (Display)
```
LCD I2C 16x2:
- VCC → ESP8266 5V (VIN)
- GND → ESP8266 GND
- SDA → ESP8266 D2 (GPIO4)
- SCL → ESP8266 D1 (GPIO5)

⚠️ CATATAN:
- Alamat I2C biasanya 0x27 atau 0x3F
- Jika LCD tidak muncul, scan alamat I2C dengan sketch i2c_scanner
- Pastikan backlight sudah nyala (putar potensiometer di bagian belakang LCD)
```

#### 7. Ultrasonic HC-SR04 (Distance)
```
HC-SR04:
- VCC  → ESP8266 VIN (5V)
- Trig → ESP8266 D5 (GPIO14)
- Echo → ESP8266 D6 (GPIO12)
- GND  → ESP8266 GND
```

#### 8. LDR (Light Sensor)
```
Voltage Divider Circuit:
VCC (3.3V) ─┬─ 10kΩ Resistor ─┬─ GND
            │                  │
            └─ LDR ────────────┤
                               │
                               └─ ESP8266 A0

Skema:
3.3V → 10kΩ → A0 → LDR → GND
```

## Power Supply Recommendations

### ESP32-CAM
- Voltage: 5V DC
- Current: Minimal 2A (untuk stabilitas camera)
- **Jangan gunakan USB computer** (arus tidak cukup)
- Gunakan adaptor dedicated 5V 2A

### ESP8266
- Voltage: 5V via USB atau adaptor
- Current: 1A sudah cukup
- Bisa pakai power bank atau adaptor USB

## Upload Code Guide

### ESP32-CAM Programming
```
Wiring untuk Upload Code:
FTDI Programmer → ESP32-CAM
- TX  → U0R (RX)
- RX  → U0T (TX)
- GND → GND
- 5V  → 5V
- GND → IO0 (short saat upload)

Steps:
1. Hubungkan FTDI ke ESP32-CAM
2. Short IO0 ke GND
3. Tekan tombol Reset di ESP32-CAM
4. Upload code dari Arduino IDE
5. Lepas short IO0 dari GND
6. Tekan Reset lagi
7. Buka Serial Monitor (115200 baud)
```

### ESP8266 Programming
```
NodeMCU ESP8266:
- Langsung colok USB ke komputer
- Tidak perlu jumper tambahan
- Upload langsung dari Arduino IDE
```

## Network Configuration

### WiFi Setup
```cpp
// Di kedua device (ESP32-CAM & ESP8266)
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";
```

### IP Address Configuration

1. **Upload code ESP8266 dulu**
2. **Buka Serial Monitor ESP8266**
3. **Catat IP Address yang muncul** (contoh: 192.168.1.100)
4. **Update code ESP32-CAM**:
```cpp
const char* esp8266Ip = "http://192.168.1.100"; // Ganti dengan IP ESP8266
```
5. **Upload code ESP32-CAM**

### Flask Server Configuration

1. **Cari IP Address PC/Laptop Anda**
```powershell
ipconfig  # Windows
ifconfig  # Linux/Mac
```

2. **Update di ESP32-CAM**:
```cpp
const char* flaskServerUrl = "http://192.168.1.50:5000/api/recognize"; // Ganti IP
```

3. **Jalankan Flask Server**:
```powershell
python app_face_recognition.py
```

## Testing Sequence

### 1. Test Flask Server
```bash
# Akses dari browser:
http://localhost:5000/api/health

# Expected response:
{
  "status": "healthy",
  "model": "face_recognition",
  "timestamp": "2024-11-25T..."
}
```

### 2. Test ESP8266
```bash
# Buka Serial Monitor ESP8266 (115200 baud)
# Harus muncul:
- WiFi Connected
- IP Address: 192.168.1.xxx
- Web server started
- System is ready!

# Test endpoint:
http://192.168.1.100/status  # Ganti dengan IP ESP8266
```

### 3. Test ESP32-CAM
```bash
# Buka Serial Monitor ESP32-CAM (115200 baud)
# Harus muncul:
- Camera initialized successfully
- WiFi Connected
- IP Address: 192.168.1.xxx
- System is ready!
```

### 4. Test Full System
1. Letakkan wajah di depan ESP32-CAM
2. ESP32-CAM capture foto
3. Kirim ke Flask server
4. Flask recognize face
5. ESP32-CAM kirim hasil ke ESP8266
6. ESP8266 buka pintu (servo) atau buzzer

## Troubleshooting

### ESP32-CAM tidak bisa upload
- Short IO0 ke GND saat upload
- Gunakan adaptor 5V 2A (bukan USB PC)
- Lepas short setelah upload selesai

### ESP8266 sensor tidak terbaca
- Check wiring dengan multimeter
- Pastikan voltage supply cukup
- Test sensor satu per satu

### Servo tidak bergerak
- Pastikan power supply 5V cukup (minimal 1A)
- Check wiring signal ke D3
- Test dengan servo.write(90) manual

### Camera capture failed
- Adaptor 5V kurang kuat (minimal 2A)
- Frame size terlalu besar (ganti ke VGA)
- PSRAM tidak terdeteksi

### WiFi tidak connect
- SSID dan password benar?
- Jarak ke router terlalu jauh
- Channel WiFi 2.4GHz only (bukan 5GHz)

### Face recognition tidak akurat
- Pencahayaan kurang bagus
- Jarak wajah terlalu jauh/dekat (30-50cm ideal)
- Register ulang dengan foto yang lebih baik
- Adjust TOLERANCE di Flask server (default 0.6)

## Safety Tips

⚠️ **PENTING:**
- Jangan hubungkan servo langsung ke 3.3V (gunakan 5V)
- Gunakan resistor untuk LED (220Ω)
- Jangan short power pins
- Test tanpa power dulu (dry run)
- Gunakan breadboard yang bagus
- Check polaritas LED dan buzzer
- Jangan upload code saat device powered dari external supply

## Next Steps

Setelah semua bekerja:
1. Test face recognition dengan berbagai user
2. Register user baru via web dashboard
3. Monitor access logs di Firebase
4. Adjust servo angle jika perlu (0° locked, 90° unlocked)
5. Fine-tune sensor thresholds
6. Add more features (fingerprint, RFID, dll)
