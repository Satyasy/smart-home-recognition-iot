import React from 'react';
import { Lightbulb, Power } from 'lucide-react';

const LampControl = ({ lampOn, onToggle, loading }) => {
  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Lightbulb className={`w-5 h-5 ${lampOn ? 'text-yellow-400' : 'text-gray-400'}`} />
          <h3 className="text-xl font-semibold text-white">Lamp Control</h3>
        </div>
        <span className={`px-3 py-1 rounded-full text-xs font-semibold ${
          lampOn ? 'bg-yellow-500/20 text-yellow-400' : 'bg-gray-500/20 text-gray-400'
        }`}>
          {lampOn ? '● ON' : '● OFF'}
        </span>
      </div>

      {/* Lamp Visual Indicator */}
      <div className="flex justify-center mb-6">
        <div className={`relative w-32 h-32 rounded-full flex items-center justify-center transition-all duration-300 ${
          lampOn 
            ? 'bg-gradient-to-br from-yellow-300 to-yellow-500 shadow-lg shadow-yellow-500/50' 
            : 'bg-gray-700 shadow-inner'
        }`}>
          <Lightbulb className={`w-16 h-16 transition-all duration-300 ${
            lampOn ? 'text-white animate-pulse' : 'text-gray-500'
          }`} />
          {lampOn && (
            <div className="absolute inset-0 rounded-full bg-yellow-300 opacity-30 animate-ping"></div>
          )}
        </div>
      </div>

      {/* Control Button */}
      <button
        onClick={onToggle}
        disabled={loading}
        className={`w-full flex items-center justify-center gap-2 px-6 py-4 rounded-lg font-semibold text-lg transition-all duration-200 ${
          lampOn
            ? 'bg-yellow-500 hover:bg-yellow-600 text-gray-900 shadow-lg shadow-yellow-500/30'
            : 'bg-gray-700 hover:bg-gray-600 text-white shadow-lg'
        } ${loading ? 'opacity-50 cursor-not-allowed' : ''}`}
      >
        <Power className="w-5 h-5" />
        {loading ? 'Processing...' : lampOn ? 'Turn OFF' : 'Turn ON'}
      </button>

      {/* Status Info */}
      <div className="mt-4 p-3 bg-gray-700/30 rounded-lg">
        <div className="flex items-center justify-between text-sm">
          <span className="text-gray-400">Relay Status:</span>
          <span className={`font-semibold ${lampOn ? 'text-yellow-400' : 'text-gray-400'}`}>
            {lampOn ? 'Active (HIGH)' : 'Inactive (LOW)'}
          </span>
        </div>
        <div className="flex items-center justify-between text-sm mt-2">
          <span className="text-gray-400">GPIO Pin:</span>
          <span className="text-gray-300 font-mono">GPIO1 (TX)</span>
        </div>
      </div>

      {/* Quick Actions */}
      <div className="mt-4 grid grid-cols-2 gap-2">
        <button
          onClick={() => onToggle('on')}
          disabled={loading || lampOn}
          className="px-4 py-2 bg-green-600 hover:bg-green-700 disabled:bg-gray-700 disabled:text-gray-500 disabled:cursor-not-allowed text-white rounded-lg text-sm font-medium transition-colors"
        >
          Force ON
        </button>
        <button
          onClick={() => onToggle('off')}
          disabled={loading || !lampOn}
          className="px-4 py-2 bg-red-600 hover:bg-red-700 disabled:bg-gray-700 disabled:text-gray-500 disabled:cursor-not-allowed text-white rounded-lg text-sm font-medium transition-colors"
        >
          Force OFF
        </button>
      </div>
    </div>
  );
};

export default LampControl;
