# ESP32-CAM Dashboard Integration - Troubleshooting Guide

## 🔧 Problem: ESP32-CAM tidak bisa di-fetch dari Dashboard

### Symptoms:
- ✅ Bisa akses `http://192.168.5.86` via browser
- ✅ Bisa akses `http://192.168.5.86/stream` via browser
- ✅ Bisa akses `http://192.168.5.86/status` via browser
- ❌ Dashboard tidak bisa fetch (Network error / CORS error)

### Root Causes:

#### 1. **CORS Headers** ✅ SUDAH FIXED
ESP32-CAM code sudah menambahkan CORS headers:
```cpp
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}
```

#### 2. **Port Mismatch** ✅ SUDAH FIXED
- **Before**: CameraFeed menggunakan port **81** (`http://192.168.5.86:81/stream`)
- **After**: CameraFeed menggunakan port **80** (`http://192.168.5.86/stream`)
- ESP32-CAM hanya buka 1 server di port **80** untuk semua endpoint

#### 3. **Stream vs API Endpoints**
ESP32-CAM memiliki 2 jenis endpoint:

**A. MJPEG Stream (untuk <img> tag)**
```
GET http://192.168.5.86/stream
- Returns: multipart/x-mixed-replace (continuous JPEG frames)
- CORS: Enabled in HTTP headers
- Usage: <img src="http://192.168.5.86/stream" />
```

**B. JSON API (untuk fetch())**
```
GET http://192.168.5.86/status
POST http://192.168.5.86/recognize
GET http://192.168.5.86/capture
```

---

## ✅ Solutions Implemented

### 1. **Update CameraFeed.jsx** (Port Fix)
```jsx
// BEFORE (WRONG)
const streamUrl = `http://${camIp}:81/stream`;

// AFTER (CORRECT)
const streamUrl = `http://${camIp}/stream`;
```

### 2. **Add ESP32-CAM Status Polling** (Dashboard.jsx)
```jsx
import esp32CamService from './services/esp32cam';

const [esp32CamStatus, setEsp32CamStatus] = useState('connecting');

// Check ESP32-CAM status setiap 3 detik
const esp32CamInterval = setInterval(() => {
  checkESP32CamStatus();
}, 3000);

const checkESP32CamStatus = async () => {
  try {
    const status = await esp32CamService.getStatus();
    if (status && status.status === 'online') {
      setEsp32CamStatus('online');
    }
  } catch (error) {
    setEsp32CamStatus('offline');
  }
};
```

### 3. **Add Status Indicator** (Dashboard Header)
```jsx
<div className="flex items-center gap-2">
  <div className={`w-2 h-2 rounded-full ${
    esp32CamStatus === 'online' ? 'bg-green-400 animate-pulse' : 
    esp32CamStatus === 'offline' ? 'bg-red-400' : 'bg-yellow-400'
  }`} />
  <span className="text-xs text-gray-500">
    ESP32-CAM: {esp32CamStatus === 'online' ? 'Connected' : 'Disconnected'}
  </span>
</div>
```

---

## 🧪 Testing Steps

### 1. **Test ESP32-CAM Directly (Browser)**
Buka browser dan akses:
```
http://192.168.5.86          → Harus tampil web interface
http://192.168.5.86/stream   → Harus tampil live stream
http://192.168.5.86/status   → Harus return JSON
```

### 2. **Test dari Dashboard (Console)**
Buka Dashboard → F12 (Developer Tools) → Console:
```javascript
// Test status endpoint
fetch('http://192.168.5.86/status')
  .then(r => r.json())
  .then(data => console.log('Status:', data))
  .catch(e => console.error('Error:', e));

// Test stream URL (should load image)
const img = new Image();
img.src = 'http://192.168.5.86/stream';
img.onload = () => console.log('✅ Stream loaded');
img.onerror = () => console.error('❌ Stream error');
```

### 3. **Check Network Tab**
F12 → Network tab → Filter by "86":
- Harus ada request ke `http://192.168.5.86/status`
- Status code harus **200 OK**
- Response headers harus ada `Access-Control-Allow-Origin: *`

---

## 🐛 Common Issues & Fixes

### Issue 1: "Failed to fetch" di Console
**Cause**: ESP32-CAM tidak responding atau IP salah
**Fix**:
1. Cek ESP32-CAM Serial Monitor: `IP Address: 192.168.5.86`
2. Ping dari terminal: `ping 192.168.5.86`
3. Akses via browser: `http://192.168.5.86`

### Issue 2: CORS Error
**Cause**: CORS headers tidak ada atau salah
**Fix**: Pastikan ESP32-CAM code punya `enableCORS()` dan dipanggil di semua handlers:
```cpp
void handleStatus() {
  enableCORS();  // ← PENTING!
  // ... rest of code
}
```

### Issue 3: Stream tidak muncul tapi status OK
**Cause**: Port mismatch atau URL salah
**Fix**: 
```jsx
// CameraFeed.jsx - Pastikan tidak ada :81
const streamUrl = `http://192.168.5.86/stream`;  // CORRECT
// const streamUrl = `http://192.168.5.86:81/stream`;  // WRONG
```

### Issue 4: Timeout saat fetch
**Cause**: Timeout terlalu pendek atau ESP32-CAM lambat
**Fix**: Increase timeout di `esp32cam.js`:
```javascript
constructor() {
  this.timeout = 20000; // 20 seconds (sudah diupdate)
}
```

---

## 📊 Expected Console Output

### Dashboard Console (saat berhasil):
```
📷 ESP32-CAM Stream URL: http://192.168.5.86/stream
[ESP32-CAM] Status: {status: "online", device: "ESP32-CAM", ip: "192.168.5.86", ...}
📷 ESP32-CAM online: 192.168.5.86
```

### ESP32-CAM Serial Monitor (saat dashboard fetch):
```
[HTTP] GET /status - Dashboard polling
[STREAM] Client connected
```

---

## 🎯 Checklist Before Testing

- [ ] ESP32-CAM WiFi credentials sudah diganti
- [ ] ESP32-CAM code sudah di-upload
- [ ] Serial Monitor menampilkan `IP Address: 192.168.5.86`
- [ ] Bisa akses `http://192.168.5.86` via browser
- [ ] Dashboard sudah restart (`npm start`)
- [ ] `.env` sudah set `REACT_APP_ESP32_CAM_IP=192.168.5.86`
- [ ] Browser console tidak ada CORS error

---

## 🚀 Final Test

Jika semua sudah OK, di Dashboard header harus muncul:
```
● Backend: Connected
● ESP8266: Connected
● ESP32-CAM: Connected  ← Harus hijau!
```

Dan di CameraFeed harus muncul:
```
🎥 Live stream dari ESP32-CAM
Status: ● LIVE (hijau)
```

---

## 📞 Still Not Working?

Check Serial Monitor ESP32-CAM untuk error messages:
```
[WIFI] IP Address: 192.168.5.86
[OK] HTTP server started
[READY] ESP32-CAM is ready!
```

Jika tidak ada output, ESP32-CAM mungkin crash atau WiFi tidak connect.
