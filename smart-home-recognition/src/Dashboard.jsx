import React, { useState, useEffect } from 'react';
import { Shield, Lock, Unlock, TestTube, Wifi } from 'lucide-react';
import CameraFeed from './components/CameraFeed';
import SensorStatus from './components/SensorStatus';
import ActuatorControl from './components/ActuatorControl';
import AccessLog from './components/AccessLog';
import SecurityChart from './components/SecurityChart';
import PinModal from './components/PinModal';
import LampControl from './components/LampControl';
import UserManagement from './components/UserManagement';
import RegisterUser from './components/RegisterUser';
import ApiService from './services/api';
import esp8266Service from './services/esp8266';
import esp32CamService from './services/esp32cam';
import { testESP8266Connection, mockESP8266Data } from './utils/esp8266Test';

const Dashboard = () => {
  const [doorStatus, setDoorStatus] = useState('locked');
  const [sensorData, setSensorData] = useState({
    faceRecognition: { detected: false, name: '', confidence: 0 },
    fingerprint: { matched: false, userId: '' },
    ultrasonic: { distance: 0, objectDetected: false },
    servo: { angle: 0, locked: true },
    led: { red: true, green: false },
    buzzer: { active: false },
    temperature: 0,
    humidity: 0,
    light: 0,
    distance: 0
  });
  
  const [accessLog, setAccessLog] = useState([]);
  const [activityHistory, setActivityHistory] = useState([]);
  const [backendStatus, setBackendStatus] = useState('connecting');
  const [esp8266Status, setEsp8266Status] = useState('connecting');
  const [esp32CamStatus, setEsp32CamStatus] = useState('connecting');
  const [showPinModal, setShowPinModal] = useState(false);
  const [showRegisterModal, setShowRegisterModal] = useState(false);
  const [testMode, setTestMode] = useState(false);
  const [lampOn, setLampOn] = useState(false);
  const [lampLoading, setLampLoading] = useState(false);

  // Load data saat component mount
  useEffect(() => {
    loadAccessLogs();
    checkBackendHealth();
    loadESP8266Data();
    
    // Refresh logs setiap 10 detik
    const logsInterval = setInterval(() => {
      loadAccessLogs();
    }, 10000);

    // Health check setiap 30 detik
    const healthInterval = setInterval(() => {
      checkBackendHealth();
    }, 30000);

    // Load ESP8266 data setiap 2 detik
    const esp8266Interval = setInterval(() => {
      loadESP8266Data();
    }, 2000);

    // Check ESP32-CAM status setiap 3 detik
    const esp32CamInterval = setInterval(() => {
      checkESP32CamStatus();
    }, 3000);

    return () => {
      clearInterval(logsInterval);
      clearInterval(healthInterval);
      clearInterval(esp8266Interval);
      clearInterval(esp32CamInterval);
    };
  }, []);

  // Check backend health
  const checkBackendHealth = async () => {
    try {
      const response = await ApiService.healthCheck();
      if (response.status === 'healthy') {
        setBackendStatus('online');
      }
    } catch (error) {
      console.error('Backend health check failed:', error);
      setBackendStatus('offline');
    }
  };

  // Check ESP32-CAM status
  const checkESP32CamStatus = async () => {
    try {
      const status = await esp32CamService.getStatus();
      if (status && status.status === 'online') {
        setEsp32CamStatus('online');
        console.log('📷 ESP32-CAM online:', status.ip);
      } else {
        setEsp32CamStatus('offline');
      }
    } catch (error) {
      console.error('ESP32-CAM status check failed:', error);
      setEsp32CamStatus('offline');
    }
  };

  // Load data dari ESP8266
  const loadESP8266Data = async () => {
    try {
      // Jika test mode aktif, gunakan mock data
      if (testMode) {
        console.log('🧪 TEST MODE: Using mock ESP8266 data');
        setSensorData(prev => ({
          ...prev,
          temperature: mockESP8266Data.sensor.temperature,
          humidity: mockESP8266Data.sensor.humidity,
          light: mockESP8266Data.sensor.light,
          distance: mockESP8266Data.sensor.distance,
          ultrasonic: {
            distance: mockESP8266Data.sensor.distance,
            objectDetected: mockESP8266Data.sensor.distance < 30
          },
          servo: mockESP8266Data.status.servo,
          led: mockESP8266Data.status.led,
          buzzer: mockESP8266Data.status.buzzer
        }));
        setEsp8266Status('online');
        setDoorStatus(mockESP8266Data.status.servo.locked ? 'locked' : 'unlocked');
        return;
      }

      console.log('Attempting to connect to ESP8266...');
      
      const sensorResponse = await esp8266Service.getSensorData();
      console.log('ESP8266 Sensor Response:', sensorResponse);
      
      const statusResponse = await esp8266Service.getActuatorStatus();
      console.log('ESP8266 Status Response:', statusResponse);
      
      if (sensorResponse && statusResponse) {
        setSensorData(prev => ({
          ...prev,
          temperature: sensorResponse.temperature || 0,
          humidity: sensorResponse.humidity || 0,
          light: sensorResponse.light || 0,
          distance: sensorResponse.distance || 0,
          ultrasonic: {
            distance: sensorResponse.distance || 0,
            objectDetected: (sensorResponse.distance || 0) < 30
          },
          servo: {
            angle: statusResponse.servo?.angle || 0,
            locked: statusResponse.servo?.locked !== false
          },
          relay: {
            lamp: statusResponse.relay?.lamp || false
          },
          led: {
            red: statusResponse.led?.red !== false,
            green: statusResponse.led?.green === true
          },
          buzzer: {
            active: statusResponse.buzzer?.active === true
          }
        }));
        
        setEsp8266Status('online');
        console.log('ESP8266 connected successfully!');
        
        // Update door status
        setDoorStatus(statusResponse.servo?.locked !== false ? 'locked' : 'unlocked');
        
        // Update lamp status
        setLampOn(statusResponse.relay?.lamp === true);
      }
    } catch (error) {
      console.error('ESP8266 Connection Error:', error);
      console.error('Error details:', error.message);
      setEsp8266Status('offline');
      
      // Show detailed error in console for debugging
      if (error.message === 'Request timeout') {
        console.error('ESP8266 timeout - Check if ESP8266 is running and responding');
      } else if (error.message.includes('Failed to fetch')) {
        console.error('ESP8266 fetch failed - Possible CORS issue or ESP8266 not responding');
      }
    }
  };

  // Load access logs dari Firebase via backend
  const loadAccessLogs = async () => {
    try {
      const response = await ApiService.getAccessLogs(20);
      if (response.success && response.logs) {
        // Transform logs ke format yang sesuai dengan UI
        const transformedLogs = response.logs.map(log => ({
          time: new Date(log.timestamp).toLocaleTimeString('id-ID'),
          date: new Date(log.timestamp).toLocaleDateString('id-ID'),
          method: 'Face Recognition',
          user: log.user_name || 'Unknown',
          status: log.authorized ? 'success' : 'failed',
          type: log.authorized ? 'entry' : 'alert',
          confidence: log.confidence || 0
        }));
        setAccessLog(transformedLogs);
      }
    } catch (error) {
      console.error('Failed to load access logs:', error);
    }
  };

  // Update activity history untuk grafik
  useEffect(() => {
    const interval = setInterval(() => {
      const now = new Date();
      const time = now.toLocaleTimeString('id-ID');
      
      // Update activity history berdasarkan data ultrasonic
      const objectDetected = sensorData.ultrasonic.objectDetected;
      
      setActivityHistory(prev => {
        const newData = [...prev, {
          time: time,
          motion: objectDetected ? 1 : 0,
          access: 0
        }].slice(-20);
        return newData;
      });
    }, 3000);

    return () => clearInterval(interval);
  }, [sensorData.ultrasonic.objectDetected]);

  // Fungsi untuk handle face recognition dari ESP32-CAM
  const handleFaceRecognition = async (imageBase64) => {
    try {
      const response = await ApiService.recognizeFace(imageBase64);
      
      if (response.success) {
        if (response.authorized && response.user) {
          // Face recognized - unlock door
          const user = response.user;
          setSensorData(prev => ({
            ...prev,
            faceRecognition: {
              detected: true,
              name: user.name,
              confidence: user.confidence
            }
          }));
          unlockDoor('Face Recognition', user.name);
        } else {
          // Face not recognized
          setSensorData(prev => ({
            ...prev,
            faceRecognition: {
              detected: true,
              name: 'Unknown',
              confidence: 0
            }
          }));
          triggerAlert('Wajah tidak dikenali');
        }
      }
      
      // Reload logs setelah recognition
      loadAccessLogs();
    } catch (error) {
      console.error('Face recognition error:', error);
      triggerAlert('Face recognition error');
    }
  };

  // Fungsi untuk scan fingerprint
  const handleFingerprint = () => {
    const matched = Math.random() > 0.3;
    const userId = matched ? `FP-${Math.floor(Math.random() * 1000)}` : '';
    
    setSensorData(prev => ({
      ...prev,
      fingerprint: { matched, userId }
    }));

    if (matched) {
      unlockDoor('Fingerprint', userId);
    } else {
      triggerAlert('Fingerprint tidak terdaftar');
    }
  };

  // Fungsi untuk unlock door dengan PIN
  const unlockDoorWithPin = async (pin) => {
    try {
      // Verify PIN di backend
      const response = await ApiService.verifyPin(pin);
      
      if (response.authorized) {
        // Kirim perintah unlock ke ESP8266
        await esp8266Service.unlockDoor(pin);
        
        // Update UI
        setDoorStatus('unlocked');
        
        // Update LCD
        await esp8266Service.updateLCD('Door UNLOCKED', `User: ${response.user?.name || 'PIN User'}`);
        
        // Reload logs
        loadAccessLogs();
        
        // Auto lock setelah 5 detik
        setTimeout(async () => {
          await lockDoor();
        }, 5000);
        
        return { success: true, message: response.message };
      } else {
        // PIN salah - trigger buzzer
        await esp8266Service.controlBuzzer(true);
        await esp8266Service.updateLCD('Access DENIED', 'Invalid PIN');
        
        setTimeout(async () => {
          await esp8266Service.controlBuzzer(false);
          await esp8266Service.updateLCD('Door LOCKED', 'Waiting...');
        }, 3000);
        
        return { success: false, message: 'Invalid PIN' };
      }
    } catch (error) {
      console.error('Unlock error:', error);
      return { success: false, message: 'Connection error' };
    }
  };
  
  // Fungsi untuk unlock door dari face recognition
  const unlockDoor = async (method, identifier) => {
    try {
      // Kirim perintah unlock ke ESP8266
      await esp8266Service.unlockDoor('0000'); // Default PIN
      
      setDoorStatus('unlocked');
      
      // Update LCD
      await esp8266Service.updateLCD('Door UNLOCKED', `${method}: ${identifier}`);
      
      // Tambah log lokal
      const now = new Date();
      const newLog = {
        time: now.toLocaleTimeString('id-ID'),
        date: now.toLocaleDateString('id-ID'),
        method: method,
        user: identifier,
        status: 'success',
        type: 'entry'
      };
      
      setAccessLog(prev => [newLog, ...prev].slice(0, 10));

      // Auto lock setelah 5 detik
      setTimeout(async () => {
        await lockDoor();
      }, 5000);
    } catch (error) {
      console.error('Unlock error:', error);
    }
  };

  // Fungsi untuk lock door
  const lockDoor = async () => {
    try {
      // Kirim perintah lock ke ESP8266
      await esp8266Service.lockDoor();
      
      setDoorStatus('locked');
      
      // Update LCD
      await esp8266Service.updateLCD('Door LOCKED', 'Waiting...');
    } catch (error) {
      console.error('Lock error:', error);
    }
  };

  // Fungsi untuk control lamp
  const handleLampToggle = async (action) => {
    setLampLoading(true);
    try {
      const state = action || 'toggle'; // 'on', 'off', or 'toggle'
      const response = await esp8266Service.controlLamp(state);
      
      if (response && response.success) {
        setLampOn(response.lamp === 'on');
        console.log('💡 Lamp:', response.lamp);
        
        // Log activity
        const now = new Date();
        const newLog = {
          time: now.toLocaleTimeString('id-ID'),
          date: now.toLocaleDateString('id-ID'),
          method: 'Lamp Control',
          user: 'Manual',
          status: 'success',
          type: 'lamp',
          detail: response.lamp === 'on' ? 'Lamp turned ON' : 'Lamp turned OFF'
        };
        setAccessLog(prev => [newLog, ...prev].slice(0, 10));
      }
    } catch (error) {
      console.error('Lamp control error:', error);
    } finally {
      setLampLoading(false);
    }
  };

  // Fungsi untuk trigger alert (buzzer)
  const triggerAlert = (reason) => {
    setSensorData(prev => ({
      ...prev,
      buzzer: { active: true },
      led: { red: true, green: false }
    }));

    const now = new Date();
    const newLog = {
      time: now.toLocaleTimeString('id-ID'),
      date: now.toLocaleDateString('id-ID'),
      method: 'Alert',
      user: reason,
      status: 'failed',
      type: 'alert'
    };
    
    setAccessLog(prev => [newLog, ...prev].slice(0, 10));

    setTimeout(() => {
      setSensorData(prev => ({
        ...prev,
        buzzer: { active: false }
      }));
    }, 3000);
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-blue-900 to-gray-900 p-4">
      <div className="max-w-7xl mx-auto">
        {/* Header */}
        <div className="mb-6">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <Shield className="w-10 h-10 text-blue-400" />
              <div>
                <h1 className="text-3xl font-bold text-white">Smart Door Lock System</h1>
                <p className="text-gray-400">ESP32 CAM + Face Recognition & Fingerprint</p>
                <div className="flex items-center gap-3 mt-1">
                  <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${
                      backendStatus === 'online' ? 'bg-green-400 animate-pulse' : 
                      backendStatus === 'offline' ? 'bg-red-400' : 'bg-yellow-400'
                    }`} />
                    <span className="text-xs text-gray-500">
                      Backend: {backendStatus === 'online' ? 'Connected' : 
                                backendStatus === 'offline' ? 'Disconnected' : 'Connecting...'}
                    </span>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${
                      esp8266Status === 'online' ? 'bg-green-400 animate-pulse' : 
                      esp8266Status === 'offline' ? 'bg-red-400' : 'bg-yellow-400'
                    }`} />
                    <span className="text-xs text-gray-500">
                      ESP8266: {esp8266Status === 'online' ? 'Connected' : 
                                esp8266Status === 'offline' ? 'Disconnected' : 'Connecting...'}
                    </span>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${
                      esp32CamStatus === 'online' ? 'bg-green-400 animate-pulse' : 
                      esp32CamStatus === 'offline' ? 'bg-red-400' : 'bg-yellow-400'
                    }`} />
                    <span className="text-xs text-gray-500">
                      ESP32-CAM: {esp32CamStatus === 'online' ? 'Connected' : 
                                  esp32CamStatus === 'offline' ? 'Disconnected' : 'Connecting...'}
                    </span>
                  </div>
                  <button
                    onClick={() => setTestMode(!testMode)}
                    className={`flex items-center gap-2 px-3 py-1 rounded-lg text-xs font-medium transition-colors ${
                      testMode 
                        ? 'bg-yellow-500/20 text-yellow-400 border border-yellow-500/50' 
                        : 'bg-gray-700/50 text-gray-400 border border-gray-600/50 hover:bg-gray-700'
                    }`}
                    title="Toggle test mode with mock data"
                  >
                    <TestTube className="w-3 h-3" />
                    {testMode ? 'Test Mode ON' : 'Test Mode'}
                  </button>
                  <button
                    onClick={() => testESP8266Connection()}
                    className="flex items-center gap-2 px-3 py-1 rounded-lg text-xs font-medium bg-blue-500/20 text-blue-400 border border-blue-500/50 hover:bg-blue-500/30 transition-colors"
                    title="Run ESP8266 connection test in console"
                  >
                    <Wifi className="w-3 h-3" />
                    Test ESP8266
                  </button>
                </div>
              </div>
            </div>
            <div className="flex items-center gap-3 bg-gray-800/50 px-6 py-3 rounded-xl border border-gray-700">
              {doorStatus === 'locked' ? (
                <>
                  <Lock className="w-6 h-6 text-red-400" />
                  <span className="text-red-400 font-semibold">LOCKED</span>
                </>
              ) : (
                <>
                  <Unlock className="w-6 h-6 text-green-400" />
                  <span className="text-green-400 font-semibold">UNLOCKED</span>
                </>
              )}
            </div>
          </div>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* ESP32 CAM Feed */}
          <div className="lg:col-span-2">
            <CameraFeed 
              sensorData={sensorData} 
              doorStatus={doorStatus}
              onFaceRecognition={handleFaceRecognition}
              backendStatus={backendStatus}
            />
          </div>

          {/* Sensor & Actuator Status */}
          <div className="space-y-6">
            <SensorStatus 
              sensorData={sensorData}
              onFingerprintScan={handleFingerprint}
            />
            <ActuatorControl 
              sensorData={sensorData}
              onUnlock={() => setShowPinModal(true)}
              onLock={lockDoor}
            />
            <LampControl 
              lampOn={lampOn}
              onToggle={handleLampToggle}
              loading={lampLoading}
            />
          </div>
        </div>

        {/* User Management */}
        <div className="mt-6">
          <UserManagement 
            onRegisterClick={() => setShowRegisterModal(true)}
          />
        </div>

        {/* Access Log & Chart */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mt-6">
          <AccessLog accessLog={accessLog} />
          <SecurityChart activityHistory={activityHistory} />
        </div>
      </div>
      
      {/* PIN Modal */}
      {showPinModal && (
        <PinModal
          onClose={() => setShowPinModal(false)}
          onSubmit={async (pin) => {
            const result = await unlockDoorWithPin(pin);
            if (result.success) {
              setShowPinModal(false);
            }
            return result;
          }}
        />
      )}

      {/* Register User Modal */}
      {showRegisterModal && (
        <RegisterUser
          onClose={() => setShowRegisterModal(false)}
          onSuccess={(result) => {
            console.log('[DASHBOARD] User registered:', result);
            // Reload akan terjadi otomatis di UserManagement
          }}
        />
      )}
    </div>
  );
};

export default Dashboard;