import React from 'react';
import { CircleDot, Lightbulb, Bell, Lock, Unlock, MonitorSmartphone } from 'lucide-react';

const ActuatorControl = ({ sensorData, onUnlock, onLock }) => {
  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <h3 className="text-lg font-semibold text-white mb-4">Actuator Control</h3>
      
      {/* Manual Control Buttons */}
      <div className="mb-4 flex gap-2">
        <button
          onClick={onUnlock}
          disabled={!sensorData.servo.locked}
          className="flex-1 bg-gradient-to-r from-green-600 to-emerald-600 hover:from-green-700 hover:to-emerald-700 disabled:from-gray-600 disabled:to-gray-700 disabled:opacity-50 text-white font-semibold py-3 px-4 rounded-lg transition-all transform hover:scale-105 disabled:transform-none flex items-center justify-center gap-2 shadow-lg"
        >
          <Unlock className="w-5 h-5" />
          Unlock
        </button>
        <button
          onClick={onLock}
          disabled={sensorData.servo.locked}
          className="flex-1 bg-gradient-to-r from-red-600 to-rose-600 hover:from-red-700 hover:to-rose-700 disabled:from-gray-600 disabled:to-gray-700 disabled:opacity-50 text-white font-semibold py-3 px-4 rounded-lg transition-all transform hover:scale-105 disabled:transform-none flex items-center justify-center gap-2 shadow-lg"
        >
          <Lock className="w-5 h-5" />
          Lock
        </button>
      </div>
      
      {/* Servo Motor */}
      <div className="mb-4 p-4 bg-gray-700/30 rounded-lg border border-gray-600">
        <div className="flex items-center gap-3 mb-2">
          <CircleDot className="w-5 h-5 text-blue-400" />
          <span className="text-white font-semibold">Pintu Rumah (SG90)</span>
        </div>
        <div className="flex items-center justify-between">
          <div>
            <div className="text-gray-400 text-sm">Position</div>
            <div className="text-white font-mono text-lg">{sensorData.servo.angle}°</div>
          </div>
          <div className={`px-3 py-1 rounded-full text-xs font-semibold ${
            sensorData.servo.locked ? 'bg-red-500/20 text-red-400' : 'bg-green-500/20 text-green-400'
          }`}>
            {sensorData.servo.locked ? '🔒 LOCKED' : '🔓 UNLOCKED'}
          </div>
        </div>
        <div className="mt-2 w-full bg-gray-600 rounded-full h-2">
          <div 
            className={`h-2 rounded-full transition-all duration-500 ${
              sensorData.servo.locked ? 'bg-red-500' : 'bg-green-500'
            }`}
            style={{ width: `${(sensorData.servo.angle / 180) * 100}%` }}
          />
        </div>
      </div>

      {/* LED Indicator */}
      <div className="mb-4 p-4 bg-gray-700/30 rounded-lg border border-gray-600">
        <div className="flex items-center gap-3 mb-3">
          <Lightbulb className="w-5 h-5 text-yellow-400" />
          <span className="text-white font-semibold">LED Indicator</span>
        </div>
        <div className="flex gap-4">
          <div className="flex-1">
            <div className={`flex items-center gap-2 p-3 rounded-lg ${
              sensorData.led.red ? 'bg-red-500/20 border border-red-500' : 'bg-gray-800/50'
            }`}>
              <div className={`w-4 h-4 rounded-full ${
                sensorData.led.red ? 'bg-red-500 animate-pulse shadow-lg shadow-red-500/50' : 'bg-red-900'
              }`} />
              <div>
                <div className="text-white text-sm font-semibold">Red LED</div>
                <div className={`text-xs ${sensorData.led.red ? 'text-red-400' : 'text-gray-500'}`}>
                  {sensorData.led.red ? 'ON' : 'OFF'}
                </div>
              </div>
            </div>
          </div>
          <div className="flex-1">
            <div className={`flex items-center gap-2 p-3 rounded-lg ${
              sensorData.led.green ? 'bg-green-500/20 border border-green-500' : 'bg-gray-800/50'
            }`}>
              <div className={`w-4 h-4 rounded-full ${
                sensorData.led.green ? 'bg-green-500 animate-pulse shadow-lg shadow-green-500/50' : 'bg-green-900'
              }`} />
              <div>
                <div className="text-white text-sm font-semibold">Green LED</div>
                <div className={`text-xs ${sensorData.led.green ? 'text-green-400' : 'text-gray-500'}`}>
                  {sensorData.led.green ? 'ON' : 'OFF'}
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Buzzer */}
      <div className="mb-4 p-4 bg-gray-700/30 rounded-lg border border-gray-600">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <Bell className={`w-5 h-5 ${sensorData.buzzer.active ? 'text-red-400 animate-bounce' : 'text-gray-400'}`} />
            <div>
              <div className="text-white font-semibold">Buzzer Alarm</div>
              <div className="text-gray-400 text-sm">Active Buzzer 5V</div>
            </div>
          </div>
          <div className={`px-3 py-1 rounded-full text-xs font-semibold ${
            sensorData.buzzer.active ? 'bg-red-500 text-white animate-pulse' : 'bg-gray-600 text-gray-400'
          }`}>
            {sensorData.buzzer.active ? '🔊 ACTIVE' : '🔇 SILENT'}
          </div>
        </div>
      </div>

      {/* LCD Display */}
      <div className="p-4 bg-gradient-to-r from-cyan-900/30 to-blue-900/30 rounded-lg border border-cyan-700/50">
        <div className="flex items-center gap-3 mb-3">
          <MonitorSmartphone className="w-5 h-5 text-cyan-400" />
          <div>
            <div className="text-white font-semibold">LCD I2C 16x2</div>
            <div className="text-gray-400 text-xs">Address: 0x27</div>
          </div>
        </div>
        <div className="bg-green-900/40 border-2 border-green-700/50 rounded-lg p-3 font-mono">
          <div className="text-green-300 text-sm mb-1">
            {sensorData.lcd?.line1 || 'Door: LOCKED'}
          </div>
          <div className="text-green-300 text-sm">
            {sensorData.lcd?.line2 || 'Waiting...'}
          </div>
        </div>
      </div>
    </div>
  );
};

export default ActuatorControl;