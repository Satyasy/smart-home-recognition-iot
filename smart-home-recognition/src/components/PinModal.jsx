import React, { useState } from 'react';
import { Lock, X, AlertCircle } from 'lucide-react';

const PinModal = ({ onClose, onSubmit }) => {
  const [pin, setPin] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  const handleNumberClick = (num) => {
    if (pin.length < 4) {
      setPin(pin + num);
      setError('');
    }
  };

  const handleDelete = () => {
    setPin(pin.slice(0, -1));
    setError('');
  };

  const handleSubmit = async () => {
    if (pin.length !== 4) {
      setError('PIN must be 4 digits');
      return;
    }

    setLoading(true);
    setError('');

    try {
      const result = await onSubmit(pin);
      if (!result.success) {
        setError(result.message || 'Invalid PIN');
        setPin('');
      }
    } catch (err) {
      setError('Connection error. Please try again.');
      setPin('');
    } finally {
      setLoading(false);
    }
  };

  const handleKeyPress = (e) => {
    if (e.key >= '0' && e.key <= '9' && pin.length < 4) {
      handleNumberClick(e.key);
    } else if (e.key === 'Backspace') {
      handleDelete();
    } else if (e.key === 'Enter' && pin.length === 4) {
      handleSubmit();
    }
  };

  return (
    <div 
      className="fixed inset-0 bg-black/70 backdrop-blur-sm flex items-center justify-center z-50 p-4"
      onKeyDown={handleKeyPress}
      tabIndex={0}
    >
      <div className="bg-gray-800 rounded-2xl border-2 border-gray-700 max-w-sm w-full p-6 shadow-2xl">
        {/* Header */}
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <div className="p-2 bg-blue-500/20 rounded-lg">
              <Lock className="w-6 h-6 text-blue-400" />
            </div>
            <div>
              <h2 className="text-xl font-bold text-white">Enter PIN</h2>
              <p className="text-sm text-gray-400">4-digit PIN required</p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-white transition-colors p-1"
          >
            <X className="w-6 h-6" />
          </button>
        </div>

        {/* PIN Display */}
        <div className="mb-6">
          <div className="flex justify-center gap-3 mb-2">
            {[0, 1, 2, 3].map((i) => (
              <div
                key={i}
                className={`w-14 h-14 rounded-xl border-2 flex items-center justify-center text-2xl font-bold transition-all ${
                  pin.length > i
                    ? 'bg-blue-500 border-blue-400 text-white scale-110'
                    : 'bg-gray-700/50 border-gray-600 text-gray-600'
                }`}
              >
                {pin.length > i ? '●' : '○'}
              </div>
            ))}
          </div>
          {error && (
            <div className="flex items-center gap-2 text-red-400 text-sm justify-center mt-3">
              <AlertCircle className="w-4 h-4" />
              <span>{error}</span>
            </div>
          )}
        </div>

        {/* Number Pad */}
        <div className="grid grid-cols-3 gap-3 mb-4">
          {[1, 2, 3, 4, 5, 6, 7, 8, 9].map((num) => (
            <button
              key={num}
              onClick={() => handleNumberClick(num.toString())}
              disabled={loading}
              className="h-16 bg-gradient-to-br from-gray-700 to-gray-800 hover:from-gray-600 hover:to-gray-700 text-white text-2xl font-bold rounded-xl border border-gray-600 transition-all transform active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed shadow-lg"
            >
              {num}
            </button>
          ))}
          
          {/* Delete Button */}
          <button
            onClick={handleDelete}
            disabled={loading || pin.length === 0}
            className="h-16 bg-gradient-to-br from-red-600 to-red-700 hover:from-red-500 hover:to-red-600 text-white font-bold rounded-xl border border-red-500 transition-all transform active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed shadow-lg"
          >
            ←
          </button>
          
          {/* Zero */}
          <button
            onClick={() => handleNumberClick('0')}
            disabled={loading}
            className="h-16 bg-gradient-to-br from-gray-700 to-gray-800 hover:from-gray-600 hover:to-gray-700 text-white text-2xl font-bold rounded-xl border border-gray-600 transition-all transform active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed shadow-lg"
          >
            0
          </button>
          
          {/* Submit Button */}
          <button
            onClick={handleSubmit}
            disabled={loading || pin.length !== 4}
            className="h-16 bg-gradient-to-br from-green-600 to-green-700 hover:from-green-500 hover:to-green-600 text-white font-bold rounded-xl border border-green-500 transition-all transform active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed shadow-lg"
          >
            {loading ? '...' : '✓'}
          </button>
        </div>

        {/* Info */}
        <div className="text-center text-xs text-gray-500">
          Default PIN: 0000
        </div>
      </div>
    </div>
  );
};

export default PinModal;
