import React, { useState } from 'react';
import { Camera, Activity } from 'lucide-react';

const CameraFeed = ({ sensorData, doorStatus }) => {
  const [cameraStream, setCameraStream] = useState('active');

  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center gap-2 mb-4">
        <Camera className="w-5 h-5 text-blue-400" />
        <h3 className="text-xl font-semibold text-white">ESP32 CAM - Face Recognition</h3>
        <span className={`ml-auto px-3 py-1 rounded-full text-xs font-semibold ${
          cameraStream === 'active' ? 'bg-green-500/20 text-green-400' : 'bg-red-500/20 text-red-400'
        }`}>
          {cameraStream === 'active' ? '● LIVE' : '● OFFLINE'}
        </span>
      </div>
      
      {/* Camera Preview */}
      <div className="relative bg-black rounded-lg overflow-hidden aspect-video mb-4">
        <div className="absolute inset-0 flex items-center justify-center">
          <Camera className="w-24 h-24 text-gray-600" />
        </div>
        <div className="absolute inset-0 bg-gradient-to-t from-black/80 to-transparent" />
        
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

      {/* PIR Sensor Status */}
      <div className={`p-4 rounded-lg ${
        sensorData.pir.motion ? 'bg-yellow-500/20 border border-yellow-500' : 'bg-gray-700/30 border border-gray-600'
      }`}>
        <div className="flex items-center gap-3">
          <Activity className={`w-5 h-5 ${sensorData.pir.motion ? 'text-yellow-400 animate-pulse' : 'text-gray-400'}`} />
          <div>
            <div className="font-semibold text-white">PIR Motion Sensor</div>
            <div className={`text-sm ${sensorData.pir.motion ? 'text-yellow-400' : 'text-gray-400'}`}>
              {sensorData.pir.motion ? 'Motion Detected' : 'No Motion'}
            </div>
          </div>
        </div>
      </div>

      {/* Alert Banner */}
      {sensorData.pir.motion && doorStatus === 'locked' && (
        <div className="mt-4 bg-yellow-500/20 border border-yellow-500 rounded-lg p-3">
          <div className="flex items-center gap-2">
            <Activity className="w-4 h-4 text-yellow-400 animate-pulse" />
            <span className="text-yellow-400 text-sm font-semibold">⚠️ Motion detected while door is locked!</span>
          </div>
        </div>
      )}
    </div>
  );
};

export default CameraFeed;