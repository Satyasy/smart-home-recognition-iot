# 🔌 ESP8266 Wiring Diagram

## 📦 Required Components

### Main Components
- **ESP8266 NodeMCU** (1x)
- **DHT11** - Temperature & Humidity sensor (1x)
- **LDR** - Light Dependent Resistor (1x)
- **HC-SR04** - Ultrasonic Distance sensor (1x)
- **Servo SG90** - 180° servo motor (1x)
- **LCD I2C 16x2** - LCD with I2C module (1x)
- **LED Red** 5mm (1x)
- **LED Green** 5mm (1x)
- **Buzzer** Active 5V (1x)

### Passive Components
- **Resistor 10kΩ** (1x) - for LDR voltage divider
- **Resistor 220Ω** (2x) - for LED current limiting
- **Breadboard** (1x)
- **Jumper wires** (Male-Male, Male-Female)
- **Power supply 5V** (untuk servo eksternal - optional)

## 🔧 Pin Connections

### ESP8266 NodeMCU Pinout
```
                    +-----------------+
                    |    NodeMCU      |
      3V3 ----      | [ ]         [ ] |  ---- Vin (5V)
      GND ----      | [ ]         [ ] |  ---- GND
      RST ----      | [ ]         [ ] |  ---- RST
       A0 ----LDR   | [ ]         [ ] |  D0
       D0 ----      | [ ]         [ ] |  D1 ---- LCD SDA
       D1 ----      | [ ]         [ ] |  D2 ---- LCD SCL
       D2 ----      | [ ]         [ ] |  D3 ---- Buzzer
       D3 ----      | [ ]         [ ] |  D4 ---- DHT11
       D4 ----      | [ ]         [ ] |  3V3
      GND ----      | [ ]         [ ] |  D5 ---- HC-SR04 Trig
       D5 ----      | [ ]         [ ] |  D6 ---- HC-SR04 Echo
       D6 ----      | [ ]         [ ] |  D7 ---- Servo
       D7 ----      | [ ]         [ ] |  D8
       D8 ----      | [ ]   USB   [ ] |  RX
       RX ----      | [ ]         [ ] |  TX
       TX ----      | [ ]         [ ] |  GND
      GND ----      | [ ]         [ ] |  3V3
                    +-----------------+
```

## 📝 Detailed Wiring

### 1. DHT11 Temperature & Humidity Sensor
```
DHT11          ESP8266
━━━━━━━━━━━━━━━━━━━━━━━
VCC    ----->  3.3V
DATA   ----->  D4
GND    ----->  GND
```

### 2. LDR (Light Sensor)
```
Circuit:
   3.3V ----[ LDR ]----+---- A0
                       |
                   [ 10kΩ ]
                       |
                      GND
```

**Explanation:** Voltage divider circuit
- Light: LDR resistance ↓ → Voltage at A0 ↑
- Dark: LDR resistance ↑ → Voltage at A0 ↓

### 3. HC-SR04 Ultrasonic Sensor
```
HC-SR04        ESP8266
━━━━━━━━━━━━━━━━━━━━━━━
VCC    ----->  5V (Vin pin)
TRIG   ----->  D5
ECHO   ----->  D6
GND    ----->  GND
```

**⚠️ Important:** HC-SR04 Echo pin outputs 5V, but ESP8266 is 3.3V tolerant on most pins. For safety, use voltage divider:
```
ECHO ----[ 1kΩ ]----+---- D6
                    |
                [ 2kΩ ]
                    |
                   GND
```

### 4. Servo SG90 (Door Lock)
```
Servo SG90     ESP8266
━━━━━━━━━━━━━━━━━━━━━━━
Brown/Black --> GND
Red        ---> 5V (Vin or external power)
Orange     ---> D7
```

**⚠️ Power Warning:**
- Servo can draw up to 500mA when moving
- ESP8266 USB power may not be sufficient
- **Recommended:** Use external 5V power supply for servo
  - Connect servo VCC to external 5V
  - Connect servo GND to external GND **AND** ESP8266 GND (common ground)
  - Servo signal stays connected to D7

**External Power Setup:**
```
External 5V PSU
    (+) ----> Servo VCC
    (-) ----> Servo GND + ESP8266 GND (common ground)

ESP8266 D7 ----> Servo Signal
```

### 5. LCD I2C 16x2
```
LCD I2C        ESP8266
━━━━━━━━━━━━━━━━━━━━━━━
GND    ----->  GND
VCC    ----->  5V (Vin)
SDA    ----->  D2 (GPIO4)
SCL    ----->  D1 (GPIO5)
```

**Note:** I2C default address biasanya `0x27` atau `0x3F`. Jika LCD tidak muncul, scan I2C address dengan code scanner.

### 6. LED Red & Green
```
LED Red                ESP8266
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Anode (+) --[ 220Ω ]---> D1
Cathode (-)  ---------->  GND

LED Green              ESP8266
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Anode (+) --[ 220Ω ]---> D2
Cathode (-)  ---------->  GND
```

**LED Polarity:**
- Long leg = Anode (+) = connects to resistor → GPIO
- Short leg = Cathode (-) = connects to GND

### 7. Buzzer (Active)
```
Buzzer         ESP8266
━━━━━━━━━━━━━━━━━━━━━━━
(+)    ----->  D3
(-)    ----->  GND
```

**Buzzer Types:**
- **Active Buzzer:** Has built-in oscillator, beeps when powered (recommended)
- **Passive Buzzer:** Needs PWM signal to make sound

## 🎨 Complete Breadboard Layout

```
                  ESP8266 NodeMCU
                  ┌─────────────┐
                  │             │
     DHT11────────┤ D4      Vin ├────── 5V Rail
                  │             │
     HC-SR04      │             │
       Trig───────┤ D5       D1 ├────── LED Red (+ 220Ω)
       Echo───────┤ D6       D2 ├────── LED Green (+ 220Ω)
                  │             │
     Servo────────┤ D7       D3 ├────── Buzzer (+)
                  │             │
     LCD I2C      │             │
       SDA────────┤ D2 (GPIO4)  │
       SCL────────┤ D1 (GPIO5)  │
                  │             │
     LDR──────────┤ A0      GND ├────── GND Rail
                  │             │
                  └─────────────┘

Power Rails:
  5V Rail  ← Vin pin or External 5V PSU
  GND Rail ← Multiple GND connections (common ground)
```

## 🔋 Power Considerations

### Power Requirements
| Component | Voltage | Current (mA) | Notes |
|-----------|---------|-------------|-------|
| ESP8266   | 3.3V    | 80-170      | Internal regulator from 5V |
| DHT11     | 3.3V    | 0.3-2.5     | Low power |
| HC-SR04   | 5V      | 15          | During measurement |
| Servo     | 5V      | 100-500     | **High current when moving!** |
| LCD       | 5V      | 20-80       | Depends on backlight |
| LEDs      | 3.3V    | 20 each     | With 220Ω resistor |
| Buzzer    | 5V      | 30          | Active buzzer |

**Total Peak Current:** ~900mA

### Power Options

#### Option 1: USB Power Only (Not Recommended)
```
USB Cable (5V 500mA) → NodeMCU Vin
```
- ⚠️ May cause brownout when servo moves
- ⚠️ ESP8266 may reset unexpectedly
- ✅ OK for testing without servo

#### Option 2: External 5V Power Supply (Recommended)
```
5V 2A PSU → Breadboard 5V Rail
         → Connect to NodeMCU Vin
         → Common GND
```
- ✅ Stable power for all components
- ✅ Servo won't cause voltage drops
- ✅ Recommended for production use

#### Option 3: Dual Power (Best)
```
USB 5V → ESP8266 Vin (for ESP + sensors)
External 5V 2A → Servo VCC (dedicated servo power)
                → Common GND with ESP8266
```
- ✅ Isolates servo power from ESP
- ✅ Most stable configuration
- ✅ Prevents servo noise affecting ESP

## 🧪 Testing Checklist

### Pre-Power Check
- [ ] Double-check all connections
- [ ] Verify polarity (LEDs, buzzer, DHT11)
- [ ] Check no short circuits between 5V and GND
- [ ] Ensure common ground for all components
- [ ] Servo has adequate power supply

### Power-On Tests
1. **LED Test:** Red LED should light up (door locked state)
2. **LCD Test:** LCD backlight on, displays "Initializing..."
3. **WiFi Test:** LCD shows "WiFi Connected" + IP address
4. **Buzzer Test:** Single beep on startup
5. **Servo Test:** Should move to lock position (0°)

### Serial Monitor Check
```
=================================
ESP8266 Smart Door Lock
=================================
Connecting to WiFi: YourSSID
.....
✅ WiFi Connected!
📡 IP: 192.168.5.250
📶 RSSI: -45 dBm
🚀 HTTP Server Started!
=================================

Default PIN: 0000
System Ready!
```

### Sensor Tests
Open Serial Monitor and watch for:
```
[HTTP] GET /sensor
Temperature: 25.5°C
Humidity: 65.0%
Light: 450
Distance: 15.5 cm
```

### Web Interface Test
1. Open browser: `http://192.168.5.250`
2. Should see sensor values
3. Door status: LOCKED
4. Sensor data updates automatically

### Dashboard Test
1. Open React dashboard: `http://localhost:3000`
2. Status: "ESP8266: Connected" (green)
3. Click "Unlock with PIN"
4. Enter PIN: `0000`
5. Servo should rotate to 90°
6. Green LED lights up
7. LCD shows "Door UNLOCKED"
8. Buzzer double-beep
9. After 5 seconds: auto-lock

## 🐛 Troubleshooting

### LCD not displaying
- **Check I2C address:** Try 0x27 or 0x3F
- **Check connections:** SDA to D2, SCL to D1
- **Try I2C scanner code** to detect address

### Servo not moving / ESP resets when servo moves
- **Insufficient power:** Use external 5V supply
- **Add capacitor:** 100-470µF across servo power pins
- **Common ground:** Ensure ESP and servo share GND

### DHT11 reads NaN
- **Wait time:** DHT11 needs 2 seconds between reads
- **Check wiring:** VCC to 3.3V, Data to D4, GND to GND
- **Add pull-up resistor:** 4.7kΩ between VCC and Data pin (optional)

### HC-SR04 distance always 999.9
- **Power:** Needs 5V on VCC
- **Echo protection:** Consider voltage divider for Echo pin
- **Max range:** HC-SR04 max range is ~400cm
- **Timeout:** Code has 30ms timeout

### WiFi won't connect
- **2.4GHz only:** ESP8266 doesn't support 5GHz
- **SSID/Password:** Check credentials in code
- **Static IP conflict:** Another device using 192.168.5.250?
- **Try DHCP:** Comment out static IP config

## 📸 Photos Reference

**Component Identification:**
- **DHT11:** Blue/White sensor with 3-4 pins, has grid pattern
- **HC-SR04:** Two "eyes" (ultrasonic transducers)
- **Servo SG90:** Small blue/orange motor with 3 wires
- **LCD I2C:** 16x2 character display with I2C backpack (4 pins)
- **LDR:** Round photoresistor (looks like mini LED)

**NodeMCU Orientation:**
- USB port at bottom
- "NodeMCU" text readable
- Pin labels visible

## ✅ Final Verification
- [ ] All LEDs working
- [ ] LCD displays text
- [ ] Buzzer makes sound
- [ ] Servo moves smoothly
- [ ] Sensors reading values
- [ ] Web interface accessible
- [ ] Dashboard connects
- [ ] PIN unlock works
- [ ] Auto-lock after 5 seconds

Once all checks pass, system is ready for use! 🎉
