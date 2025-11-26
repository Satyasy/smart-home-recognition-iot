# Smart Home Face Recognition System

Sistem keamanan pintu pintar menggunakan ESP32-CAM untuk face recognition, terintegrasi dengan React frontend dan Flask backend.

## 🏗️ Arsitektur Sistem

```
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│   ESP32-CAM     │─────▶│  Flask Backend  │◀────▶│ Firebase RTDB   │
│  (Face Capture) │      │ (Face Recognition)│     │  (Database)     │
└─────────────────┘      └─────────────────┘      └─────────────────┘
                                  ▲
                                  │
                                  ▼
                         ┌─────────────────┐
                         │ React Frontend  │
                         │   (Dashboard)   │
                         └─────────────────┘
```

## 📋 Fitur

### Backend (Flask + face_recognition)
- ✅ Face recognition menggunakan library `face_recognition` (dlib)
- ✅ Register user baru dengan foto wajah
- ✅ Recognize wajah dari ESP32-CAM
- ✅ Verify dua wajah apakah sama
- ✅ CRUD operations untuk user management
- ✅ Access logs dengan timestamp
- ✅ Firebase Realtime Database integration
- ✅ RESTful API endpoints

### Frontend (React)
- ✅ Real-time camera feed dari ESP32-CAM
- ✅ Live face recognition display
- ✅ User registration modal
- ✅ Access logs history
- ✅ Activity charts
- ✅ Sensor status monitoring (PIR, Servo, LED, Buzzer)
- ✅ Responsive dashboard dengan TailwindCSS

### Hardware
- ESP32-CAM untuk camera feed
- PIR Motion Sensor
- Servo motor untuk door lock
- LED indicators (Red/Green)
- Buzzer untuk alert

## 🚀 Setup & Installation

### 1. Backend Setup

```powershell
# Buat virtual environment
python -m venv .venv

# Aktivasi virtual environment
.venv\Scripts\Activate.ps1

# Install dependencies
pip install Flask face_recognition opencv-python numpy firebase-admin Pillow cmake dlib

# Atau install dari requirements.txt
pip install -r requirements.txt

# Jalankan backend server
python app_face_recognition.py
```

Backend akan berjalan di: `http://localhost:5000`

### 2. Frontend Setup

```powershell
# Masuk ke folder frontend
cd smart-home-recognition

# Install dependencies
npm install

# Konfigurasi environment variables
# Edit file .env dan sesuaikan IP address

# Jalankan development server
npm start
```

Frontend akan berjalan di: `http://localhost:3000`

## ⚙️ Konfigurasi

### Backend Configuration

Edit `app_face_recognition.py`:

```python
# Firebase configuration
'databaseURL': 'https://your-database.firebaseio.com/'

# Face recognition threshold (default: 0.6)
TOLERANCE = 0.6  # Lower = more strict
```

### Frontend Configuration

Edit `.env`:

```env
# ESP32-CAM IP Address
REACT_APP_ESP32CAM_IP=192.168.5.96

# Flask Backend URL
REACT_APP_API_URL=http://localhost:5000/api
```

### Firebase Setup

1. Buat project di [Firebase Console](https://console.firebase.google.com/)
2. Enable Realtime Database
3. Download `serviceAccountKey.json`
4. Letakkan di root folder project
5. Update database URL di `app_face_recognition.py`

## 📡 API Endpoints

### User Management

```
POST   /api/register          - Register user baru
POST   /api/recognize          - Recognize wajah
POST   /api/verify             - Verify dua wajah
GET    /api/users              - Get semua users
GET    /api/user/:id           - Get user by ID
PUT    /api/user/:id           - Update user
DELETE /api/user/:id           - Delete user
```

### Logs & System

```
GET    /api/logs               - Get access logs
DELETE /api/logs/clear         - Clear semua logs
GET    /api/config             - Get system configuration
GET    /api/health             - Health check
```

### Example Request - Register User

```javascript
POST /api/register
Content-Type: application/json

{
  "image": "base64_encoded_image_string",
  "name": "John Doe",
  "email": "john@example.com",
  "phone": "08123456789"
}
```

### Example Response

```json
{
  "success": true,
  "message": "User John Doe registered successfully",
  "user_id": "user_20250126123456"
}
```

## 🔧 ESP32-CAM Setup

### Upload Code

1. Buka `esp_code/esp32_face_recognition.ino`
2. Update WiFi credentials
3. Update server IP addresses
4. Upload ke ESP32-CAM menggunakan Arduino IDE

### ESP32-CAM Stream URL

```
http://<ESP32_IP>/stream
```

## 📊 Database Structure (Firebase)

### Users Collection

```json
{
  "users": {
    "user_20250126123456": {
      "name": "John Doe",
      "email": "john@example.com",
      "phone": "08123456789",
      "face_encoding": [...],
      "registered_at": "2025-01-26T12:34:56",
      "status": "active",
      "model": "face_recognition"
    }
  }
}
```

### Access Logs Collection

```json
{
  "access_logs": {
    "log_id_1": {
      "timestamp": "2025-01-26T12:34:56",
      "authorized": true,
      "user_id": "user_20250126123456",
      "user_name": "John Doe",
      "confidence": 95.5
    }
  }
}
```

## 🐛 Troubleshooting

### Backend Issues

**Problem:** DLL load failed while importing tensorflow

**Solution:** Gunakan `app_face_recognition.py` yang menggunakan library `face_recognition` instead of DeepFace/TensorFlow

**Problem:** Firebase connection error

**Solution:** 
- Pastikan `serviceAccountKey.json` ada
- Check database URL di config
- Verify Firebase rules

### Frontend Issues

**Problem:** Cannot connect to backend

**Solution:**
- Pastikan backend running di port 5000
- Check CORS settings
- Verify API_URL di `.env`

**Problem:** Camera feed not showing

**Solution:**
- Verify ESP32-CAM IP address
- Check ESP32-CAM is powered and connected to WiFi
- Test stream URL directly in browser

### ESP32-CAM Issues

**Problem:** Camera initialization failed

**Solution:**
- Reset ESP32-CAM
- Check power supply (5V 2A minimum)
- Verify camera module connection

## 📝 Tech Stack

- **Backend:** Python 3.13, Flask, face_recognition (dlib), OpenCV
- **Frontend:** React 18, TailwindCSS, Lucide Icons, Recharts
- **Database:** Firebase Realtime Database
- **Hardware:** ESP32-CAM, PIR Sensor, Servo Motor, LED, Buzzer

## 🎯 Future Improvements

- [ ] Real-time WebSocket connection untuk instant updates
- [ ] Multiple camera support
- [ ] Mobile app (React Native)
- [ ] Face recognition dengan mask detection
- [ ] Email/SMS notifications
- [ ] Advanced analytics dashboard
- [ ] MQTT protocol integration
- [ ] Cloud deployment (Heroku/AWS)

## 👨‍💻 Development

```powershell
# Backend (Terminal 1)
.venv\Scripts\python.exe app_face_recognition.py

# Frontend (Terminal 2)
cd smart-home-recognition
npm start
```

## 📄 License

MIT License - Feel free to use for your projects!

## 🤝 Contributing

Contributions are welcome! Please open an issue or submit a pull request.

---

**Made with ❤️ for Smart Home Security**
