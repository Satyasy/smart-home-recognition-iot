import React, { useState, useEffect } from 'react';
import { Shield, Lock, Unlock } from 'lucide-react';
import CameraFeed from './components/CameraFeed';
import SensorStatus from './components/SensorStatus';
import ActuatorControl from './components/ActuatorControl';
import AccessLog from './components/AccessLog';
import SecurityChart from './components/SecurityChart';
import UsersList from './components/UsersList';
import ApiService from './services/api';
import FirebaseService from './services/firebase';

const Dashboard = () => {
  const [doorStatus, setDoorStatus] = useState('locked');
  const [sensorData, setSensorData] = useState({
    faceRecognition: { detected: false, name: '', confidence: 0 },
    temperature: 0,
    humidity: 0,
    light: 0,
    distance: 0,
    servo: { angle: 0, locked: true },
    led: { red: true, green: false },
    buzzer: { active: false },
    lcd: { line1: 'Door: LOCKED', line2: 'Waiting...' }
  });
  
  const [accessLog, setAccessLog] = useState([]);
  const [activityHistory, setActivityHistory] = useState([]);
  const [serverStatus, setServerStatus] = useState({ online: false, model: '' });

  // Check server health on mount
  useEffect(() => {
    const checkHealth = async () => {
      try {
        const health = await ApiService.healthCheck();
        setServerStatus({ 
          online: health.status === 'healthy', 
          model: health.model || '' 
        });
      } catch (error) {
        console.error('Server offline:', error);
        setServerStatus({ online: false, model: '' });
      }
    };
    
    checkHealth();
    const healthInterval = setInterval(checkHealth, 30000); // Check every 30s
    
    return () => clearInterval(healthInterval);
  }, []);

  // Load access logs from Firebase
  useEffect(() => {
    const loadLogs = async () => {
      try {
        const logs = await FirebaseService.getAccessLogs(20);
        const formattedLogs = logs.map(log => ({
          time: new Date(log.timestamp).toLocaleTimeString('id-ID'),
          date: new Date(log.timestamp).toLocaleDateString('id-ID'),
          method: 'Face Recognition',
          user: log.user_name || 'Unknown',
          status: log.authorized ? 'success' : 'failed',
          type: log.authorized ? 'entry' : 'alert',
          confidence: log.confidence || 0
        }));
        setAccessLog(formattedLogs);
      } catch (error) {
        console.error('Failed to load logs:', error);
      }
    };
    
    loadLogs();
    const logInterval = setInterval(loadLogs, 5000); // Refresh every 5s
    
    return () => clearInterval(logInterval);
  }, []);

  // Monitor sensor data from ESP8266
  useEffect(() => {
    const loadSensorData = async () => {
      try {
        // Fetch dari ESP8266 /status endpoint
        const ESP8266_IP = 'http://192.168.50.250'; // Sesuaikan dengan IP ESP8266
        const response = await fetch(`${ESP8266_IP}/status`);
        
        if (response.ok) {
          const data = await response.json();
          setSensorData(prev => ({
            ...prev,
            temperature: data.temperature || 0,
            humidity: data.humidity || 0,
            light: data.light || 0,
            distance: data.distance || 0,
            servo: { 
              angle: data.door_locked ? 0 : 180, 
              locked: data.door_locked 
            },
            faceRecognition: {
              detected: data.last_user !== '',
              name: data.last_user || '',
              confidence: data.last_confidence || 0
            }
          }));
          
          setDoorStatus(data.door_locked ? 'locked' : 'unlocked');
        }
      } catch (error) {
        // Silently fail jika ESP8266 tidak dapat dijangkau
        console.error('Failed to load sensor data from ESP8266:', error);
      }
    };
    
    loadSensorData();
    const sensorInterval = setInterval(loadSensorData, 2000); // Refresh every 2s
    
    return () => clearInterval(sensorInterval);
  }, []);

  // Update activity history untuk grafik
  useEffect(() => {
    const interval = setInterval(() => {
      const now = new Date();
      const time = now.toLocaleTimeString('id-ID');
      
      setActivityHistory(prev => {
        const newData = [...prev, {
          time: time,
          motion: sensorData.distance < 30 ? 1 : 0, // Deteksi objek <30cm
          access: doorStatus === 'unlocked' ? 1 : 0
        }].slice(-20);
        return newData;
      });
    }, 3000);

    return () => clearInterval(interval);
  }, [sensorData.distance, doorStatus]);

  // Fungsi untuk unlock door manual (via React UI)
  const handleManualUnlock = async () => {
    try {
      const ESP8266_IP = 'http://192.168.50.250';
      await fetch(`${ESP8266_IP}/unlock`);
      unlockDoor('Manual', 'Dashboard User');
    } catch (error) {
      console.error('Failed to unlock door:', error);
    }
  };
  
  // Fungsi untuk lock door manual
  const handleManualLock = async () => {
    try {
      const ESP8266_IP = 'http://192.168.50.250';
      await fetch(`${ESP8266_IP}/lock`);
      lockDoor();
    } catch (error) {
      console.error('Failed to lock door:', error);
    }
  };

  // Fungsi untuk unlock door
  const unlockDoor = (method, identifier) => {
    setDoorStatus('unlocked');
    
    // Kontrol aktuator
    setSensorData(prev => ({
      ...prev,
      servo: { angle: 180, locked: false }, // 180 derajat untuk unlock
      led: { red: false, green: true },
      buzzer: { active: false }
    }));

    // Tambah log akses
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
    setTimeout(() => {
      lockDoor();
    }, 5000);
  };

  // Fungsi untuk lock door
  const lockDoor = () => {
    setDoorStatus('locked');
    setSensorData(prev => ({
      ...prev,
      servo: { angle: 0, locked: true },
      led: { red: true, green: false }
    }));
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
          <div className="flex items-center justify-between flex-wrap gap-4">
            <div className="flex items-center gap-3">
              <Shield className="w-10 h-10 text-blue-400" />
              <div>
                <h1 className="text-3xl font-bold text-white">Smart Door Lock System</h1>
                <p className="text-gray-400">ESP32 CAM + Face Recognition</p>
                <div className="flex items-center gap-2 mt-1">
                  <div className={`w-2 h-2 rounded-full ${serverStatus.online ? 'bg-green-400' : 'bg-red-400'}`}></div>
                  <span className="text-xs text-gray-400">
                    {serverStatus.online ? `Server Online (${serverStatus.model})` : 'Server Offline'}
                  </span>
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
            />
          </div>

          {/* Sensor & Actuator Status */}
          <div className="space-y-6">
            <SensorStatus 
              sensorData={sensorData}
            />
            <ActuatorControl 
              sensorData={sensorData}
              onUnlock={handleManualUnlock}
              onLock={handleManualLock}
            />
          </div>
        </div>

        {/* Access Log & Chart */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mt-6">
          <AccessLog accessLog={accessLog} />
          <SecurityChart activityHistory={activityHistory} />
        </div>

        {/* Users List */}
        <div className="mt-6">
          <UsersList />
        </div>
      </div>
    </div>
  );
};

export default Dashboard;