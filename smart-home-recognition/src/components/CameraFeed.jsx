import React, { useState, useEffect, useRef } from 'react';
import { Camera, Activity, UserPlus, RefreshCw } from 'lucide-react';
import RegisterUser from './RegisterUser';

const CameraFeed = ({ sensorData, doorStatus, onFaceRecognition, backendStatus }) => {
  const [cameraStream, setCameraStream] = useState('connecting');
  const [showRegister, setShowRegister] = useState(false);
  const [esp32CamUrl, setEsp32CamUrl] = useState('');
  const imgRef = useRef(null);

  useEffect(() => {
    // Get ESP32-CAM URL from environment or use default
    const camIp = process.env.REACT_APP_ESP32_CAM_IP || '192.168.5.86';
    const streamUrl = `http://${camIp}:81/stream`;  // Port 81 untuk stream
    setEsp32CamUrl(streamUrl);
    console.log('📷 ESP32-CAM Stream URL:', streamUrl);
  }, []);

  const handleImageLoad = () => {
    setCameraStream('active');
  };

  const handleImageError = () => {
    setCameraStream('offline');
  };

  const refreshCamera = () => {
    setCameraStream('connecting');
    const camIp = process.env.REACT_APP_ESP32_CAM_IP || '192.168.5.86';
    const streamUrl = `http://${camIp}:81/stream?t=${Date.now()}`;
    setEsp32CamUrl(streamUrl);
    
    console.log('🔄 Refreshing camera stream:', streamUrl);
    
    if (imgRef.current) {
      imgRef.current.src = streamUrl;
    }
  };

  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Camera className="w-5 h-5 text-blue-400" />
          <h3 className="text-xl font-semibold text-white">ESP32 CAM - Face Recognition</h3>
          <span className={`px-3 py-1 rounded-full text-xs font-semibold ${
            cameraStream === 'active' ? 'bg-green-500/20 text-green-400' : 
            cameraStream === 'connecting' ? 'bg-yellow-500/20 text-yellow-400' : 
            'bg-red-500/20 text-red-400'
          }`}>
            {cameraStream === 'active' ? '● LIVE' : 
             cameraStream === 'connecting' ? '● CONNECTING' : '● OFFLINE'}
          </span>
        </div>
        <div className="flex gap-2">
          <button
            onClick={refreshCamera}
            className="p-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors"
            title="Refresh Camera"
          >
            <RefreshCw className="w-4 h-4 text-white" />
          </button>
          <button
            onClick={() => setShowRegister(true)}
            className="flex items-center gap-2 px-3 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg transition-colors text-white text-sm font-semibold"
          >
            <UserPlus className="w-4 h-4" />
            Register User
          </button>
        </div>
      </div>
      
      {/* Camera Preview */}
      <div className="relative bg-black rounded-lg overflow-hidden aspect-video mb-4">
        {cameraStream === 'offline' ? (
          <div className="absolute inset-0 flex items-center justify-center flex-col gap-3">
            <Camera className="w-24 h-24 text-gray-600" />
            <p className="text-gray-400 text-sm">Camera Offline</p>
            <button
              onClick={refreshCamera}
              className="px-4 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg text-white text-sm"
            >
              Reconnect
            </button>
          </div>
        ) : (
          <>
            {esp32CamUrl && (
              <img
                ref={imgRef}
                src={esp32CamUrl}
                alt="ESP32-CAM Stream"
                className="w-full h-full object-cover"
                onLoad={handleImageLoad}
                onError={handleImageError}
              />
            )}
            {cameraStream === 'connecting' && (
              <div className="absolute inset-0 flex items-center justify-center bg-black/50">
                <div className="text-white text-center">
                  <RefreshCw className="w-12 h-12 animate-spin mx-auto mb-2" />
                  <p className="text-sm">Connecting to camera...</p>
                </div>
              </div>
            )}
          </>
        )}
        <div className="absolute inset-0 bg-gradient-to-t from-black/80 to-transparent pointer-events-none" />
        
        {/* Face Detection Overlay */}
        {sensorData.faceRecognition.detected && (
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="border-4 border-green-400 rounded-lg w-48 h-64 relative animate-pulse">
              <div className="absolute -top-8 left-0 right-0 text-center">
                <div className="bg-green-500 text-white px-4 py-2 rounded-lg inline-block">
                  <div className="font-semibold">{sensorData.faceRecognition.name}</div>
                  <div className="text-xs">Confidence: {sensorData.faceRecognition.confidence.toFixed(1)}%</div>
                </div>
              </div>
            </div>
          </div>
        )}
        
        <div className="absolute bottom-4 left-4 right-4 flex justify-between items-end">
          <div className="text-white">
            <div className="text-sm opacity-75">Camera Status</div>
            <div className="font-semibold">640x480 @ 30fps</div>
          </div>
          <div className="text-right">
            {sensorData.faceRecognition.detected && (
              <div className={`px-3 py-1 rounded-full text-sm font-semibold ${
                sensorData.faceRecognition.confidence > 80 
                  ? 'bg-green-500 text-white' 
                  : 'bg-yellow-500 text-white'
              }`}>
                Face Detected
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Ultrasonic Sensor Status */}
      <div className={`p-4 rounded-lg ${
        sensorData.ultrasonic.objectDetected ? 'bg-yellow-500/20 border border-yellow-500' : 'bg-gray-700/30 border border-gray-600'
      }`}>
        <div className="flex items-center gap-3">
          <Activity className={`w-5 h-5 ${sensorData.ultrasonic.objectDetected ? 'text-yellow-400 animate-pulse' : 'text-gray-400'}`} />
          <div>
            <div className="font-semibold text-white">HC-SR04 Ultrasonic Sensor</div>
            <div className={`text-sm ${sensorData.ultrasonic.objectDetected ? 'text-yellow-400' : 'text-gray-400'}`}>
              Distance: {sensorData.ultrasonic.distance.toFixed(1)} cm
              {sensorData.ultrasonic.objectDetected && ' - Object Detected!'}
            </div>
          </div>
        </div>
      </div>

      {/* Alert Banner */}
      {sensorData.ultrasonic.objectDetected && doorStatus === 'locked' && (
        <div className="mt-4 bg-yellow-500/20 border border-yellow-500 rounded-lg p-3">
          <div className="flex items-center gap-2">
            <Activity className="w-4 h-4 text-yellow-400 animate-pulse" />
            <span className="text-yellow-400 text-sm font-semibold">⚠️ Object detected near door while locked!</span>
          </div>
        </div>
      )}

      {/* Register User Modal */}
      {showRegister && (
        <RegisterUser
          onClose={() => setShowRegister(false)}
          onSuccess={(response) => {
            console.log('User registered:', response);
            // You can add notification here
          }}
        />
      )}
    </div>
  );
};

export default CameraFeed;