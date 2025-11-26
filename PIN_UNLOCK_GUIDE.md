# 🔐 PIN Unlock Quick Start Guide

## ✅ Prerequisites

Sebelum test PIN unlock, pastikan:
- ✅ ESP8266 test server sudah connected (hijau di dashboard)
- ✅ Backend Flask running
- ✅ Frontend React running
- ✅ Browser console tidak ada error (F12)

## 🚀 Test PIN Unlock - Step by Step

### Step 1: Verify Connection Status

1. Buka dashboard: `http://localhost:3000`
2. Check status indicators di header:
   ```
   Backend: Connected ✅ (hijau)
   ESP8266: Connected ✅ (hijau)
   ```
3. Jika salah satu merah, troubleshoot dulu:
   - **Backend merah**: Check Flask running di port 5000
   - **ESP8266 merah**: Check Arduino code uploaded & WiFi connected

### Step 2: Open PIN Modal

1. Klik button **"🔓 Unlock with PIN"** di dashboard
2. Modal popup dengan numpad muncul
3. Current door status: **LOCKED 🔒**

### Step 3: Enter PIN

Default PIN: **0000**

**Method 1: Click Numpad**
- Klik angka 0, 0, 0, 0 pada numpad
- Watch PIN dots fill up: ● ● ● ●

**Method 2: Keyboard**
- Type: 0, 0, 0, 0
- Works dengan numpad atau angka biasa

### Step 4: Verify PIN

1. Klik button **"Verify PIN"**
2. Request dikirim ke:
   - Backend: `POST /api/verify-pin` → verify PIN valid
   - ESP8266: `POST /unlock` → unlock servo

### Step 5: Watch the Action! 🎬

**What Happens:**

#### Dashboard (React)
- ✅ Success message: "Door unlocked successfully!"
- ✅ Status changes: LOCKED 🔒 → UNLOCKED 🔓
- ✅ Access log baru muncul
- ✅ Modal closes automatically

#### ESP8266 Hardware (if wired)
1. **Servo** rotates: 0° → 90° (unlocked position)
2. **LED**:
   - Red OFF
   - Green ON
3. **LCD** displays:
   ```
   Door UNLOCKED
   Welcome!
   ```
4. **Buzzer**: Double beep
   - Beep! (100ms)
   - Pause (50ms)
   - Beep! (100ms)

#### Serial Monitor (ESP8266)
```
[HTTP] POST /unlock - PIN: 0000
[DOOR] UNLOCKED
```

#### Browser Console (F12)
```javascript
Sending PIN to backend: 0000
Backend verify PIN response: {success: true, message: "PIN valid"}
Sending unlock command to ESP8266...
ESP8266 unlock response: {success: true, message: "Door unlocked"}
Door unlocked successfully!
```

### Step 6: Auto-Lock

After 5 seconds, door automatically locks:

#### ESP8266 Hardware
1. **Servo** rotates back: 90° → 0° (locked)
2. **LED**:
   - Red ON
   - Green OFF
3. **LCD** displays:
   ```
   Door LOCKED
   Scan to enter
   ```
4. **Buzzer**: Single beep (200ms)

#### Serial Monitor
```
[AUTO] Locking door after timeout
[DOOR] LOCKED
```

#### Dashboard
- Status changes: UNLOCKED 🔓 → LOCKED 🔒

## 🎯 Testing Scenarios

### Scenario 1: Correct PIN ✅

**Input:** 0000
**Expected:**
- ✅ Success message
- ✅ Door unlocks
- ✅ Green LED on
- ✅ Double beep
- ✅ Auto-lock after 5 seconds

### Scenario 2: Wrong PIN ❌

**Input:** 1234 (not registered)
**Expected:**
- ❌ Error message: "Invalid PIN"
- ❌ Door stays locked
- ❌ Red LED stays on
- ❌ Buzzer alert pattern (3x beeps)
- ❌ LCD shows "ALERT! Access Denied"

### Scenario 3: Empty PIN ❌

**Input:** (nothing)
**Expected:**
- ❌ Verify button disabled
- ❌ Cannot submit

### Scenario 4: Multiple Attempts

**Attempt 1:** Wrong PIN → Alert
**Attempt 2:** Correct PIN → Success
**Result:** Works correctly, no lockout

## 🧪 Testing with Mock Data (No Hardware)

Jika hardware belum wired:

1. **Enable Test Mode**:
   - Click button **"Test Mode"** di dashboard header
   - Button turns yellow: **"Test Mode ON"**

2. **Test PIN Unlock**:
   - Enter PIN: 0000
   - Click Verify
   - Dashboard shows success (using mock response)
   - Servo state changes (simulated)
   - LED state changes (simulated)

3. **What's Simulated**:
   - ✅ Backend PIN verification (real)
   - ✅ ESP8266 unlock response (mocked)
   - ✅ Sensor data (mocked)
   - ✅ Dashboard UI updates (real)

## 📊 Monitoring & Debugging

### Browser Console (F12)

**Normal Flow:**
```javascript
🔐 PIN Modal opened
PIN entered: ●●●●
Sending PIN to backend: 0000
Backend verify PIN response: {success: true, message: "PIN valid"}
Sending unlock command to ESP8266...
ESP8266 unlock response: {success: true, message: "Door unlocked", servo: {angle: 90, locked: false}}
Door unlocked successfully!
Waiting for auto-lock in 5 seconds...
[AUTO] Locking door
```

**Error Flow:**
```javascript
🔐 PIN Modal opened
PIN entered: ●●●●
Sending PIN to backend: 1234
Backend verify PIN response: {success: false, message: "Invalid PIN"}
Invalid PIN code. Please try again.
```

### Network Tab (F12)

**Check Requests:**

1. **Backend PIN Verify**:
   ```
   POST http://192.168.5.221:5000/api/verify-pin
   Status: 200 OK
   Response: {success: true, message: "PIN valid"}
   ```

2. **ESP8266 Unlock**:
   ```
   POST http://192.168.5.250/unlock
   Status: 200 OK
   Response: {success: true, message: "Door unlocked"}
   ```

### Serial Monitor (ESP8266)

**Expected Output:**
```
[HTTP] GET /sensor - 200 OK
[HTTP] GET /status - 200 OK
[HTTP] POST /unlock - PIN: 0000
[DOOR] UNLOCKED
[HTTP] GET /sensor - 200 OK
[HTTP] GET /status - 200 OK
...
[AUTO] Locking door after timeout
[DOOR] LOCKED
```

## 🐛 Troubleshooting PIN Unlock

### Issue 1: "Backend: Disconnected"

**Symptoms:** Cannot verify PIN, modal doesn't open

**Solution:**
```bash
# Check Flask running
python app_face_recognition.py

# Should see:
# * Running on http://192.168.5.221:5000

# Test endpoint
curl http://192.168.5.221:5000/api/health
```

### Issue 2: "ESP8266: Disconnected"

**Symptoms:** PIN verified but door doesn't unlock

**Solution:**
1. Check ESP8266 Serial Monitor
2. Verify IP: 192.168.5.250
3. Test endpoint: `http://192.168.5.250/status`
4. Upload Arduino code jika belum

### Issue 3: PIN Always Invalid

**Symptoms:** Even 0000 returns "Invalid PIN"

**Check:**
1. **Backend**:
   ```python
   # app_face_recognition.py
   # Check PIN verification logic
   @app.route('/api/verify-pin', methods=['POST'])
   ```

2. **Firebase** (if used):
   ```json
   {
     "pins": {
       "0000": {
         "user": "Admin",
         "created": "2024-01-01"
       }
     }
   }
   ```

3. **Test API directly**:
   ```bash
   curl -X POST http://192.168.5.221:5000/api/verify-pin \
   -H "Content-Type: application/json" \
   -d '{"pin":"0000"}'
   ```

### Issue 4: Servo Not Moving

**Symptoms:** PIN verified, no physical action

**Possible Causes:**
1. **Test mode active**: Disable test mode
2. **Mock code uploaded**: Use `esp8266_full_code.ino`
3. **Servo not wired**: Check D7 connection
4. **No power**: Servo needs external 5V 2A
5. **Code issue**: Check `handleUnlock()` function

**Solution:**
```cpp
// Verify this code exists in handleUnlock():
unlockDoor();
doorServo.write(90); // Should move to 90°
```

### Issue 5: Auto-Lock Not Working

**Symptoms:** Door unlocks but never locks back

**Check:**
1. **Serial Monitor**: Should show `[AUTO] Locking door after timeout`
2. **Delay setting**:
   ```cpp
   const unsigned long AUTO_LOCK_DELAY = 5000; // 5 seconds
   ```
3. **Loop running**:
   ```cpp
   void loop() {
     server.handleClient(); // Must have this
     // Auto-lock check here
   }
   ```

### Issue 6: Modal Doesn't Close After Success

**Check:**
1. Browser console for errors
2. `PinModal.jsx` close handler:
   ```javascript
   if (result.success) {
     onClose(); // Modal should close
   }
   ```

## 📋 PIN Management

### Add New PIN

**Method 1: Firebase**
```json
{
  "pins": {
    "0000": { "user": "Admin" },
    "1234": { "user": "Family" },
    "5678": { "user": "Guest" }
  }
}
```

**Method 2: Code**
```cpp
// esp8266_full_code.ino
String validPins[] = {"0000", "1234", "5678"};
```

### Change Default PIN

```cpp
// esp8266_full_code.ino
const String DEFAULT_PIN = "1234"; // Change here
```

### Delete PIN

Firebase: Remove entry from `pins` node

## ✅ Success Checklist

Before testing PIN unlock, verify:

- [ ] Flask backend running (port 5000)
- [ ] React frontend running (port 3000)
- [ ] ESP8266 uploaded with code
- [ ] ESP8266 connected to WiFi
- [ ] Dashboard shows both green indicators
- [ ] Browser console has no errors
- [ ] Network tab shows successful requests
- [ ] Serial Monitor shows ESP8266 logs

## 🎥 Expected Demo Flow

1. **Initial State**:
   - Dashboard shows: Door LOCKED 🔒
   - LED: Red ON, Green OFF
   - LCD: "Door LOCKED / Scan to enter"

2. **User Action**:
   - Click "Unlock with PIN"
   - Enter PIN: 0000
   - Click "Verify PIN"

3. **System Response**:
   - Backend verifies PIN ✅
   - ESP8266 unlocks servo
   - Dashboard updates to UNLOCKED 🔓
   - LED: Red OFF, Green ON
   - LCD: "Door UNLOCKED / Welcome!"
   - Buzzer: Beep-beep

4. **Auto-Lock**:
   - Wait 5 seconds
   - Servo locks automatically
   - Dashboard updates to LOCKED 🔒
   - LED: Red ON, Green OFF
   - LCD: "Door LOCKED / Scan to enter"
   - Buzzer: Single beep

Total time: ~7 seconds from unlock to locked

## 🚀 Next Steps

After PIN unlock works:
1. ✅ Test face recognition unlock
2. ✅ Wire all sensors (DHT11, LDR, HC-SR04)
3. ✅ Test sensor data display
4. ✅ Upload ESP32-CAM code
5. ✅ Integrate camera feed
6. ✅ End-to-end testing

---

**Quick Test Command:**
```bash
# Terminal 1: Backend
python app_face_recognition.py

# Terminal 2: Frontend
cd smart-home-recognition && npm start

# Terminal 3: Serial Monitor
# Connect to ESP8266 COM port at 115200 baud

# Browser: http://localhost:3000
# Click "Unlock with PIN" → Enter 0000 → Watch magic happen! ✨
```
