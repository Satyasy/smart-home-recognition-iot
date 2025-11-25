import React from 'react';
import { Clock, CheckCircle, XCircle, AlertTriangle } from 'lucide-react';

const AccessLog = ({ accessLog }) => {
  const getStatusIcon = (status) => {
    if (status === 'success') return <CheckCircle className="w-4 h-4 text-green-400" />;
    if (status === 'failed') return <XCircle className="w-4 h-4 text-red-400" />;
    return <AlertTriangle className="w-4 h-4 text-yellow-400" />;
  };

  const getStatusColor = (status) => {
    if (status === 'success') return 'bg-green-500/20 border-green-500 text-green-400';
    if (status === 'failed') return 'bg-red-500/20 border-red-500 text-red-400';
    return 'bg-yellow-500/20 border-yellow-500 text-yellow-400';
  };

  const getTypeLabel = (type) => {
    if (type === 'entry') return '🚪 Entry';
    if (type === 'exit') return '🚶 Exit';
    return '⚠️ Alert';
  };

  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center gap-2 mb-4">
        <Clock className="w-5 h-5 text-blue-400" />
        <h3 className="text-xl font-semibold text-white">Access Log</h3>
        <span className="ml-auto text-gray-400 text-sm">Last 10 activities</span>
      </div>

      {accessLog.length === 0 ? (
        <div className="text-center py-8 text-gray-500">
          <Clock className="w-12 h-12 mx-auto mb-2 opacity-50" />
          <p>No access log yet</p>
        </div>
      ) : (
        <div className="space-y-3 max-h-96 overflow-y-auto">
          {accessLog.map((log, index) => (
            <div 
              key={index}
              className={`p-4 rounded-lg border ${getStatusColor(log.status)} transition-all hover:scale-102`}
            >
              <div className="flex items-start justify-between mb-2">
                <div className="flex items-center gap-2">
                  {getStatusIcon(log.status)}
                  <span className="font-semibold">{getTypeLabel(log.type)}</span>
                </div>
                <span className="text-xs opacity-75">{log.time}</span>
              </div>
              <div className="text-sm opacity-90">
                <div className="font-mono">{log.method}</div>
                <div className="text-xs opacity-75 mt-1">{log.user}</div>
              </div>
            </div>
          ))}
        </div>
      )}

      {/* Summary Stats */}
      <div className="mt-4 pt-4 border-t border-gray-700 grid grid-cols-3 gap-3">
        <div className="text-center">
          <div className="text-2xl font-bold text-green-400">
            {accessLog.filter(log => log.status === 'success').length}
          </div>
          <div className="text-xs text-gray-400">Success</div>
        </div>
        <div className="text-center">
          <div className="text-2xl font-bold text-red-400">
            {accessLog.filter(log => log.status === 'failed').length}
          </div>
          <div className="text-xs text-gray-400">Failed</div>
        </div>
        <div className="text-center">
          <div className="text-2xl font-bold text-blue-400">
            {accessLog.length}
          </div>
          <div className="text-xs text-gray-400">Total</div>
        </div>
      </div>
    </div>
  );
};

export default AccessLog;