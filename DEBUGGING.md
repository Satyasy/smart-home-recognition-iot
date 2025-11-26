# Debugging Guide - Dashboard Blank Issue

## ✅ Masalah yang Sudah Diperbaiki

### 1. SensorStatus Component - Missing Data
**Problem:** Dashboard blank karena `SensorStatus` component membutuhkan data yang tidak ada

**Fixed:**
- Menambahkan `temperature`, `humidity`, `light`, `distance` ke initial state
- Menambahkan simulasi data untuk sensor-sensor ini

### 2. ActuatorControl Component - Missing Props
**Problem:** Component membutuhkan `onUnlock` dan `onLock` props

**Fixed:**
- Menambahkan props `onUnlock` dan `onLock` saat memanggil component
- Connect ke fungsi `unlockDoor` dan `lockDoor`

## 🔍 Cara Debug Dashboard

### Check Browser Console
1. Buka http://localhost:3000
2. Press F12 untuk buka Developer Tools
3. Lihat tab "Console" untuk error messages
4. Lihat tab "Network" untuk API call errors

### Common Issues

#### Dashboard Hitam/Blank
**Possible Causes:**
- Missing component data
- CSS tidak load (Tailwind not compiled)
- JavaScript error di console
- Component crash karena undefined props

**Solutions:**
1. Check browser console untuk error
2. Verify Tailwind CSS di index.css
3. Pastikan semua dependencies installed: `npm install`
4. Clear cache dan reload: Ctrl + Shift + R

#### API Connection Failed
**Symptoms:**
- Backend status shows "Disconnected"
- Access logs tidak muncul
- CORS errors di console

**Solutions:**
1. Pastikan backend running: `python app_face_recognition.py`
2. Check backend URL di `.env`: `REACT_APP_API_URL=http://localhost:5000/api`
3. Verify CORS settings di Flask backend
4. Test API directly: `curl http://localhost:5000/api/health`

#### Camera Feed Tidak Muncul
**Symptoms:**
- Camera box shows "Offline"
- No video stream

**Solutions:**
1. Check ESP32-CAM IP di `.env`
2. Test stream URL di browser: `http://ESP32_IP/stream`
3. Verify ESP32-CAM connected to WiFi
4. Check firewall settings

## 🛠️ Quick Fixes

### Restart Everything
```powershell
# Stop all terminals (Ctrl+C)

# Terminal 1 - Backend
cd C:\Users\Netra Ops\Documents\www\smart-home-face-recognition
.venv\Scripts\Activate.ps1
python app_face_recognition.py

# Terminal 2 - Frontend
cd C:\Users\Netra Ops\Documents\www\smart-home-face-recognition\smart-home-recognition
npm start
```

### Clear Cache & Reinstall
```powershell
# Frontend
cd smart-home-recognition
rm -r node_modules
rm package-lock.json
npm install
npm start
```

### Force Reload Browser
- Chrome/Edge: Ctrl + Shift + R
- Firefox: Ctrl + F5

## 📝 Component Dependencies Checklist

### Dashboard.jsx
- ✅ All sensor data initialized
- ✅ API service imported
- ✅ Backend health check
- ✅ Access logs loading
- ✅ Props passed to child components

### CameraFeed.jsx
- ✅ ESP32-CAM URL configured
- ✅ RegisterUser modal
- ✅ Image load/error handlers
- ✅ Props: sensorData, doorStatus, onFaceRecognition, backendStatus

### SensorStatus.jsx
- ✅ Props: sensorData (with temp, humidity, light, distance)
- ✅ onFingerprintScan callback

### ActuatorControl.jsx
- ✅ Props: sensorData, onUnlock, onLock
- ✅ Manual control buttons

### AccessLog.jsx
- ✅ Props: accessLog array
- ✅ Empty state handling

### SecurityChart.jsx
- ✅ Props: activityHistory array
- ✅ Recharts library
- ✅ Empty state handling

## 🎯 Testing Checklist

After fixing:
- [ ] Dashboard loads (not blank)
- [ ] Header shows with title
- [ ] Camera feed box appears
- [ ] Sensor status cards visible
- [ ] Actuator controls work
- [ ] Access logs section visible
- [ ] Security chart renders
- [ ] Backend status indicator shows
- [ ] No errors in console
- [ ] Register button clickable

## 💡 Development Tips

### Hot Reload
- Frontend akan auto-reload saat save file
- Backend perlu restart manual jika edit Python file

### Console Logging
Add debug logs:
```javascript
console.log('Dashboard mounted');
console.log('Sensor data:', sensorData);
console.log('Backend status:', backendStatus);
```

### Component Error Boundaries
Wrap components dengan error boundary untuk catch errors

### API Response Logging
Check API responses:
```javascript
const response = await ApiService.healthCheck();
console.log('Health check:', response);
```

---

**Last Updated:** November 26, 2025
