# ESP8266 Wiring Diagram - Complete dengan Relay

## 📌 Pin Configuration Summary

| Component | ESP8266 Pin | GPIO | Notes |
|-----------|-------------|------|-------|
| **DHT11** | D7 | GPIO13 | Temperature & Humidity sensor |
| **LDR** | A0 | ADC | Light sensor (analog) |
| **HC-SR04 Trigger** | D5 | GPIO14 | Ultrasonic distance sensor |
| **HC-SR04 Echo** | D6 | GPIO12 | Ultrasonic distance sensor |
| **Servo SG90** | D3 | GPIO0 | Door lock servo motor |
| **Buzzer** | D4 | GPIO2 | Active buzzer |
| **LED** | D0 | GPIO16 | Status LED (single) |
| **Relay** | TX | GPIO1 | Relay module for lamp control |
| **LCD SDA** | D1 | GPIO5 | I2C LCD data |
| **LCD SCL** | D2 | GPIO4 | I2C LCD clock |

---

## 🔌 Detailed Wiring

### 1. DHT11 Temperature & Humidity Sensor
```
DHT11         ESP8266
-----         --------
VCC    →      3.3V
GND    →      GND
DATA   →      D7 (GPIO13)
```
**Notes**: Add 10kΩ pull-up resistor between VCC and DATA

---

### 2. LDR (Light Sensor)
```
LDR Circuit:
VCC (3.3V) → LDR → A0 → 10kΩ Resistor → GND
```
**Notes**: Voltage divider circuit for analog reading

---

### 3. HC-SR04 Ultrasonic Sensor
```
HC-SR04       ESP8266
-------       --------
VCC    →      5V (jika tersedia) atau 3.3V
GND    →      GND
TRIG   →      D5 (GPIO14)
ECHO   →      D6 (GPIO12) + voltage divider jika pakai 5V
```
**IMPORTANT**: Jika HC-SR04 pakai 5V, ECHO pin harus pakai voltage divider:
```
ECHO → 1kΩ → D6 (GPIO12)
             ↓
            2kΩ → GND
```

---

### 4. Servo SG90 (Door Lock)
```
Servo         ESP8266
-----         --------
VCC (Red)    →  5V (External Power recommended)
GND (Brown)  →  GND
Signal (Orange) → D3 (GPIO0)
```
**IMPORTANT**: 
- Servo perlu power eksternal 5V (battery/adapter)
- Jangan pakai power dari ESP8266 (tidak cukup current)
- Ground HARUS sama antara ESP8266 dan power supply servo

---

### 5. Active Buzzer
```
Buzzer        ESP8266
------        --------
VCC (+)   →   D4 (GPIO2)
GND (-)   →   GND
```
**Notes**: Add NPN transistor + 1kΩ resistor untuk drive buzzer jika perlu

---

### 6. LED (Status Indicator)
```
ESP8266       LED        Resistor      GND
--------      ---        --------      ---
D0 (GPIO16) → Anode(+) → 220Ω → Cathode(-) → GND
```
**Notes**: 
- Single LED untuk status (ON = locked/red, OFF = unlocked)
- Bisa ganti dengan 2 LED (Red + Green) dengan common cathode

---

### 7. Relay Module (for Lamp Control) ⭐ NEW
```
Relay Module      ESP8266
------------      --------
VCC        →      5V atau 3.3V (tergantung relay)
GND        →      GND
IN (Signal) →     TX (GPIO1)

Lamp Connection:
220V AC → Relay COM → Relay NO → Lamp → Neutral
```

**⚠️ IMPORTANT - GPIO1 (TX) Conflict**:
- GPIO1 adalah TX pin (Serial output)
- Saat upload code, GPIO1 akan output Serial data
- **Solusi**:
  1. **Lepas kabel relay saat upload code**
  2. Atau gunakan pin lain (misal D8/GPIO15)
  3. Code sudah disable Serial setelah boot

**Alternative Pin untuk Relay**:
```cpp
// Jika GPIO1 (TX) bermasalah, ganti dengan:
const int relayPin = D8;  // GPIO15 - lebih aman
```

**Relay Safety**:
- Gunakan relay module dengan optocoupler (isolasi)
- Pastikan relay rating cukup untuk beban lamp
- HATI-HATI dengan 220V AC!

---

### 8. LCD I2C 16x2
```
LCD I2C       ESP8266
-------       --------
VCC    →      5V atau 3.3V
GND    →      GND
SDA    →      D1 (GPIO5)
SCL    →      D2 (GPIO4)
```
**Notes**: 
- I2C address default: 0x27 (bisa juga 0x3F)
- Cek dengan I2C scanner jika LCD tidak muncul

---

## 🔋 Power Supply Recommendations

### Option 1: Single 5V Power Supply
```
5V Adapter (2A minimum)
   ├─→ ESP8266 VIN pin (built-in voltage regulator to 3.3V)
   ├─→ Servo VCC (via separate cable)
   ├─→ Relay Module VCC
   ├─→ HC-SR04 VCC
   └─→ LCD VCC

⚠️ Common GND for ALL components!
```

### Option 2: Dual Power Supply (Recommended)
```
3.3V/5V Power:
   ├─→ ESP8266: 3.3V
   ├─→ DHT11, LDR, LED, Buzzer: 3.3V
   └─→ I2C LCD: 3.3V or 5V

Separate 5V/6V (2A):
   ├─→ Servo Motor
   ├─→ Relay Module
   └─→ HC-SR04 (optional)

⚠️ Common GND between both supplies!
```

---

## 🧪 Testing Checklist

### Before Powering On:
- [ ] Check all VCC connections (no shorts to GND)
- [ ] Check all GND connections are common
- [ ] **Relay wire disconnected for first upload**
- [ ] Servo has separate power supply
- [ ] LCD I2C address is correct (0x27 or 0x3F)

### After Power On:
- [ ] ESP8266 boots and connects to WiFi
- [ ] LCD displays "Initializing..."
- [ ] LED lights up (door locked state)
- [ ] Serial monitor shows IP address
- [ ] Can access web interface at IP address

### Component Tests (via Dashboard):
- [ ] Temperature & Humidity reading updates
- [ ] Light sensor value changes
- [ ] Ultrasonic distance reading updates
- [ ] Servo moves when unlock/lock
- [ ] Buzzer sounds when alert
- [ ] LED changes with door status
- [ ] **Lamp turns ON/OFF with relay** ⭐
- [ ] LCD updates with status messages

---

## ⚡ GPIO1 (TX) Relay - Known Issues & Solutions

### Problem:
GPIO1 (TX) outputs Serial data during:
- Boot sequence
- Code upload
- Serial.print() calls

### Solutions:

#### Solution 1: Disconnect During Upload
```
1. Disconnect relay wire from TX pin
2. Upload code
3. Wait for upload to complete
4. Reconnect relay wire
5. Press ESP8266 reset button
```

#### Solution 2: Use Different Pin (Recommended)
Update code to use safer pin:
```cpp
// Change from:
const int relayPin = 1;  // GPIO1 (TX) - problematic

// To:
const int relayPin = D8; // GPIO15 - safer option

// Wiring:
ESP8266 D8 → Relay IN
```

#### Solution 3: Disable Serial in Code
Code already includes:
```cpp
Serial.begin(115200);
delay(100);
Serial.flush();
// Serial remains active for debugging
```

**Best Practice**: Use D8 (GPIO15) instead of TX (GPIO1)

---

## 📊 Pin Usage Table

| Pin | GPIO | Function | Can Use? |
|-----|------|----------|----------|
| TX | 1 | Serial TX / Relay | ⚠️ Conflict with Serial |
| RX | 3 | Serial RX | ❌ Reserved |
| D0 | 16 | LED | ✅ OK (no PWM) |
| D1 | 5 | LCD SDA | ✅ OK |
| D2 | 4 | LCD SCL | ✅ OK |
| D3 | 0 | Servo | ⚠️ Boot mode (OK if released) |
| D4 | 2 | Buzzer | ⚠️ Boot mode (OK if released) |
| D5 | 14 | HC-SR04 TRIG | ✅ OK |
| D6 | 12 | HC-SR04 ECHO | ✅ OK |
| D7 | 13 | DHT11 | ✅ OK |
| D8 | 15 | **Available** | ✅ Best for Relay |
| A0 | ADC | LDR | ✅ OK (analog only) |

**Recommended Pin Change**:
```cpp
const int relayPin = D8;  // GPIO15 - USE THIS instead of GPIO1
```

---

## 🔥 Safety Warnings

### High Voltage (Relay):
- ⚠️ **220V AC can KILL**
- Use proper insulation
- Double-check wiring before powering on
- Use relay module with optocoupler isolation
- Keep 220V wires away from low voltage circuit

### Power Supply:
- ⚠️ Don't overload ESP8266 3.3V regulator
- Use separate power for servo (high current)
- Common GND is MANDATORY
- Add capacitors (100µF) near servo for noise filtering

### Component Ratings:
- ⚠️ Relay must match lamp wattage
- Typical relay: 10A @ 250V AC (max 2200W)
- Calculate: Watts = Voltage × Current
- Add 20% safety margin

---

## 🛠️ Troubleshooting

### Relay doesn't work:
1. Check relay module LED (should light when active)
2. Test with different GPIO pin (D8 recommended)
3. Check relay module voltage (3.3V vs 5V)
4. Measure voltage on relay IN pin
5. Try manual control via dashboard

### Lamp doesn't turn on:
1. Check relay LED is ON when lamp should be ON
2. Test lamp directly with 220V (bypass relay)
3. Check relay COM → NO connection
4. Ensure 220V is connected to COM terminal
5. Check relay contact rating matches lamp wattage

### ESP8266 resets when relay activates:
1. Power supply not enough current
2. Add bulk capacitor (1000µF) to power supply
3. Use separate power for relay
4. Check common GND connection

---

## ✅ Final Configuration

Update your ESP8266 code with this safer configuration:

```cpp
// RECOMMENDED PIN CONFIGURATION
const int dhtPin = D7;     // GPIO13 - DHT11
const int ldrPin = A0;     // ADC - LDR
const int trigPin = D5;    // GPIO14 - Ultrasonic trigger
const int echoPin = D6;    // GPIO12 - Ultrasonic echo
const int servoPin = D3;   // GPIO0 - Servo
const int buzzPin = D4;    // GPIO2 - Buzzer
const int ledPin = D0;     // GPIO16 - LED
const int relayPin = D8;   // GPIO15 - Relay ⭐ CHANGED from GPIO1

// LCD I2C
// SDA = D1 (GPIO5)
// SCL = D2 (GPIO4)
```

This configuration avoids TX/RX pins and uses safer GPIO pins! 🎯
