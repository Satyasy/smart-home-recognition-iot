# 🚀 Quick Start Guide - Smart Home Face Recognition

## Langkah-langkah Setup Lengkap

### 1️⃣ Clone & Setup Project

```powershell
# Clone repository (jika belum)
cd C:\Users\Netra Ops\Documents\www\smart-home-face-recognition
```

### 2️⃣ Setup Backend (Flask)

```powershell
# Buat virtual environment
python -m venv .venv

# Aktivasi virtual environment
.venv\Scripts\Activate.ps1

# Install dependencies
pip install Flask face_recognition opencv-python numpy firebase-admin Pillow cmake dlib

# Jalankan backend
python app_face_recognition.py
```

✅ Backend running di: **http://localhost:5000**

### 3️⃣ Setup Frontend (React)

Buka terminal baru:

```powershell
# Masuk ke folder frontend
cd C:\Users\Netra Ops\Documents\www\smart-home-face-recognition\smart-home-recognition

# Install dependencies
npm install

# Jalankan frontend
npm start
```

✅ Frontend running di: **http://localhost:3000**

Browser akan otomatis terbuka ke dashboard.

### 4️⃣ Setup ESP32-CAM (Hardware)

1. **Upload Code ke ESP32-CAM:**
   - Buka file: `esp_code/esp32_face_recognition.ino`
   - Update WiFi credentials:
     ```cpp
     const char* ssid = "YOUR_WIFI_SSID";
     const char* password = "YOUR_WIFI_PASSWORD";
     ```
   - Upload ke ESP32-CAM menggunakan Arduino IDE

2. **Catat IP Address ESP32-CAM:**
   - Setelah upload, buka Serial Monitor
   - Catat IP address yang ditampilkan (misal: 192.168.1.100)

3. **Update IP di Frontend:**
   - Edit file: `smart-home-recognition/.env`
   - Update: `REACT_APP_ESP32CAM_IP=192.168.1.100`
   - Restart frontend (Ctrl+C dan `npm start` lagi)

### 5️⃣ Setup Firebase

1. **Buat Project di Firebase:**
   - Buka: https://console.firebase.google.com/
   - Klik "Add project"
   - Beri nama project

2. **Enable Realtime Database:**
   - Di sidebar, pilih "Realtime Database"
   - Klik "Create Database"
   - Pilih lokasi: asia-southeast1
   - Start in test mode

3. **Download Service Account Key:**
   - Settings > Project settings > Service accounts
   - Klik "Generate new private key"
   - Save as `serviceAccountKey.json` di root folder project

4. **Update Database URL:**
   - Copy database URL dari Firebase Console
   - Edit `app_face_recognition.py`, line 17:
     ```python
     'databaseURL': 'https://your-project.firebaseio.com/'
     ```

5. **Restart Backend:**
   ```powershell
   # Stop backend (Ctrl+C)
   # Start lagi
   python app_face_recognition.py
   ```

## ✅ Checklist Sistem Berjalan

Pastikan semua komponen ini running:

- [ ] ✅ Backend Flask: http://localhost:5000
- [ ] ✅ Frontend React: http://localhost:3000
- [ ] ✅ ESP32-CAM Stream: http://ESP32_IP/stream
- [ ] ✅ Firebase Database: Connected

## 🎯 Cara Menggunakan

### Register User Baru

1. Buka dashboard: http://localhost:3000
2. Klik tombol **"Register User"**
3. Upload foto wajah (pastikan hanya 1 wajah)
4. Isi nama, email, dan phone
5. Klik "Register"

### Test Face Recognition

**Cara 1: Via Postman/API**

```bash
POST http://localhost:5000/api/recognize
Content-Type: application/json

{
  "image": "base64_encoded_image_dari_esp32cam"
}
```

**Cara 2: Via ESP32-CAM (Real Implementation)**

ESP32-CAM akan otomatis:
1. Capture gambar saat PIR sensor detect motion
2. Kirim ke backend untuk recognition
3. Backend response dengan user info
4. ESP32 unlock door jika authorized

### Lihat Access Logs

- Dashboard akan menampilkan semua access logs
- Refresh otomatis setiap 10 detik
- Menampilkan: timestamp, user name, status (success/failed), confidence

## 🔧 Testing Endpoints

### Test Backend Health

```powershell
curl http://localhost:5000/api/health
```

Response:
```json
{
  "status": "healthy",
  "timestamp": "2025-01-26T12:34:56",
  "model": "face_recognition",
  "version": "2.0.1"
}
```

### Test Get Users

```powershell
curl http://localhost:5000/api/users
```

### Test Get Logs

```powershell
curl http://localhost:5000/api/logs
```

## 🐛 Troubleshooting Common Issues

### Backend tidak bisa start

**Error:** `DLL load failed while importing tensorflow`

**Solution:** Gunakan `app_face_recognition.py` instead of `app.py`

### Frontend tidak connect ke backend

**Error:** `Proxy error: Could not proxy request`

**Solution:** 
1. Pastikan backend running di port 5000
2. Check di terminal backend ada output: `Running on http://127.0.0.1:5000`

### Camera feed tidak muncul

**Error:** Camera shows "Offline"

**Solution:**
1. Pastikan ESP32-CAM connected ke WiFi
2. Test stream URL di browser: `http://ESP32_IP/stream`
3. Update IP di `.env` file
4. Restart frontend

### Firebase error

**Error:** `Permission denied` atau `Connection refused`

**Solution:**
1. Check `serviceAccountKey.json` ada di root folder
2. Verify database URL benar
3. Check Firebase rules di console
4. Make sure database is in same region

## 📱 Demo Flow

1. **PIR Sensor** detect motion → LED Red menyala
2. **ESP32-CAM** capture image → Kirim ke backend
3. **Backend** recognize face → Check di database
4. **If Authorized:**
   - Response: user info + confidence
   - LED Green menyala
   - Servo unlock door (90°)
   - Log access (success)
5. **If Not Authorized:**
   - Response: unauthorized
   - LED Red tetap
   - Buzzer berbunyi
   - Log access (failed)

## 🎨 UI Features

- Real-time camera feed dari ESP32-CAM
- Live face detection overlay
- Sensor status indicators
- Access logs dengan color-coded status
- Activity chart (motion & access)
- Responsive design (mobile-friendly)
- Register user modal
- Backend connection status

## 📞 Need Help?

Jika ada masalah:
1. Check semua terminal untuk error messages
2. Verify IP addresses di `.env`
3. Make sure Firebase configured correctly
4. Test each component individually
5. Check Arduino Serial Monitor for ESP32 logs

---

**Happy Coding! 🎉**
