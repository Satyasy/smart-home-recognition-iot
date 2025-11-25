import React from 'react';
import { Activity } from 'lucide-react';
import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

const SecurityChart = ({ activityHistory }) => {
  return (
    <div className="bg-gray-800/50 backdrop-blur-sm rounded-xl p-6 border border-gray-700">
      <div className="flex items-center gap-2 mb-4">
        <Activity className="w-5 h-5 text-blue-400" />
        <h3 className="text-xl font-semibold text-white">Activity Monitor</h3>
        <span className="ml-auto text-gray-400 text-sm">Real-time</span>
      </div>

      {activityHistory.length === 0 ? (
        <div className="text-center py-12 text-gray-500">
          <Activity className="w-12 h-12 mx-auto mb-2 opacity-50" />
          <p>Waiting for activity data...</p>
        </div>
      ) : (
        <ResponsiveContainer width="100%" height={250}>
          <AreaChart data={activityHistory}>
            <defs>
              <linearGradient id="colorMotion" x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.8}/>
                <stop offset="95%" stopColor="#3b82f6" stopOpacity={0}/>
              </linearGradient>
            </defs>
            <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
            <XAxis 
              dataKey="time" 
              stroke="#9ca3af"
              tick={{ fill: '#9ca3af', fontSize: 12 }}
            />
            <YAxis 
              stroke="#9ca3af"
              tick={{ fill: '#9ca3af', fontSize: 12 }}
            />
            <Tooltip 
              contentStyle={{ 
                backgroundColor: '#1f2937', 
                border: '1px solid #374151',
                borderRadius: '8px',
                color: '#fff'
              }}
            />
            <Area 
              type="monotone" 
              dataKey="motion" 
              stroke="#3b82f6" 
              fillOpacity={1} 
              fill="url(#colorMotion)" 
              name="Motion Detection"
            />
          </AreaChart>
        </ResponsiveContainer>
      )}

      {/* Activity Stats */}
      <div className="mt-4 grid grid-cols-2 gap-3">
        <div className="bg-blue-500/10 border border-blue-500/30 rounded-lg p-3">
          <div className="text-blue-400 text-xs mb-1">Total Motion Events</div>
          <div className="text-white text-2xl font-bold">
            {activityHistory.filter(h => h.motion === 1).length}
          </div>
        </div>
        <div className="bg-purple-500/10 border border-purple-500/30 rounded-lg p-3">
          <div className="text-purple-400 text-xs mb-1">Monitoring Time</div>
          <div className="text-white text-2xl font-bold">
            {Math.floor(activityHistory.length * 3 / 60)}m
          </div>
        </div>
      </div>
    </div>
  );
};

export default SecurityChart;