import React from 'react';
import { Thermometer, Droplets, Sun, Ruler, Activity } from 'lucide-react';

const SensorStatus = ({ sensorData }) => {
  // Helper untuk status cahaya
  const getLightStatus = (value) => {
    if (value < 300) return { text: 'SANGAT GELAP', color: 'text-gray-400' };
    if (value < 500) return { text: 'GELAP', color: 'text-gray-300' };
    if (value < 700) return { text: 'REDUP', color: 'text-yellow-300' };
    return { text: 'TERANG', color: 'text-yellow-400' };
  };

  const lightStatus = getLightStatus(sensorData.light);

  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center gap-2 mb-4">
        <Activity className="w-5 h-5 text-blue-400" />
        <h3 className="text-lg font-semibold text-white">Sensor Status</h3>
      </div>

      {/* DHT11 - Temperature & Humidity */}
      <div className="space-y-3">
        <div className="p-4 bg-gradient-to-r from-red-900/30 to-orange-900/30 rounded-lg border border-red-700/50">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Thermometer className="w-5 h-5 text-red-400" />
              <span className="text-gray-300 text-sm">Temperature</span>
            </div>
            <div className="text-right">
              <div className="text-2xl font-bold text-white">
                {sensorData.temperature.toFixed(1)}°C
              </div>
              <div className="text-xs text-gray-400">DHT11</div>
            </div>
          </div>
        </div>

        <div className="p-4 bg-gradient-to-r from-blue-900/30 to-cyan-900/30 rounded-lg border border-blue-700/50">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Droplets className="w-5 h-5 text-blue-400" />
              <span className="text-gray-300 text-sm">Humidity</span>
            </div>
            <div className="text-right">
              <div className="text-2xl font-bold text-white">
                {sensorData.humidity.toFixed(1)}%
              </div>
              <div className="text-xs text-gray-400">DHT11</div>
            </div>
          </div>
        </div>

        {/* LDR - Light Sensor */}
        <div className="p-4 bg-gradient-to-r from-yellow-900/30 to-amber-900/30 rounded-lg border border-yellow-700/50">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Sun className="w-5 h-5 text-yellow-400" />
              <span className="text-gray-300 text-sm">Light Level</span>
            </div>
            <div className="text-right">
              <div className="text-2xl font-bold text-white">{sensorData.light}</div>
              <div className={`text-xs font-semibold ${lightStatus.color}`}>
                {lightStatus.text}
              </div>
            </div>
          </div>
        </div>

        {/* HC-SR04 - Ultrasonic */}
        <div className="p-4 bg-gradient-to-r from-purple-900/30 to-pink-900/30 rounded-lg border border-purple-700/50">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Ruler className="w-5 h-5 text-purple-400" />
              <span className="text-gray-300 text-sm">Distance</span>
            </div>
            <div className="text-right">
              <div className="text-2xl font-bold text-white">
                {sensorData.distance.toFixed(1)} cm
              </div>
              <div className="text-xs text-gray-400">
                {sensorData.distance < 30 ? (
                  <span className="text-green-400">✓ Object Detected</span>
                ) : (
                  <span className="text-gray-400">No Object</span>
                )}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SensorStatus;