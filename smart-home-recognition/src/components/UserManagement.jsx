import React, { useState, useEffect } from 'react';
import { Users, UserPlus, Trash2, Search, AlertCircle, CheckCircle, RefreshCw } from 'lucide-react';
import ApiService from '../services/api';

const UserManagement = ({ onRegisterClick }) => {
  const [users, setUsers] = useState([]);
  const [loading, setLoading] = useState(true);
  const [searchQuery, setSearchQuery] = useState('');
  const [deleteConfirm, setDeleteConfirm] = useState(null);
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');

  useEffect(() => {
    loadUsers();
  }, []);

  const loadUsers = async () => {
    setLoading(true);
    setError('');
    try {
      const response = await ApiService.getUsers();
      if (response.success) {
        setUsers(response.users || []);
      } else {
        setError('Failed to load users');
      }
    } catch (err) {
      setError('Error connecting to server');
      console.error('Load users error:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleDelete = async (userId, userName) => {
    try {
      setError('');
      setSuccess('');
      const response = await ApiService.deleteUser(userId);
      
      if (response.success) {
        setSuccess(`User ${userName} deleted successfully`);
        setDeleteConfirm(null);
        loadUsers(); // Reload list
        
        // Clear success message after 3 seconds
        setTimeout(() => setSuccess(''), 3000);
      } else {
        setError(response.message || 'Failed to delete user');
      }
    } catch (err) {
      setError('Error deleting user');
      console.error('Delete error:', err);
    }
  };

  const filteredUsers = users.filter(user => 
    user.name.toLowerCase().includes(searchQuery.toLowerCase())
  );

  const formatDate = (dateString) => {
    if (!dateString) return 'N/A';
    try {
      return new Date(dateString).toLocaleDateString('en-US', {
        year: 'numeric',
        month: 'short',
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit'
      });
    } catch {
      return dateString;
    }
  };

  return (
    <div className="bg-gray-800 rounded-xl border border-gray-700 overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-blue-500/20 to-purple-500/20 p-4 border-b border-gray-700">
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2">
            <Users className="w-6 h-6 text-blue-400" />
            <h2 className="text-xl font-bold text-white">Registered Users</h2>
            <span className="bg-blue-500/20 text-blue-400 px-2 py-1 rounded-full text-sm font-semibold">
              {users.length}
            </span>
          </div>
          <div className="flex gap-2">
            <button
              onClick={loadUsers}
              disabled={loading}
              className="flex items-center gap-2 px-3 py-2 bg-gray-700 text-white rounded-lg hover:bg-gray-600 transition-colors disabled:opacity-50"
            >
              <RefreshCw className={`w-4 h-4 ${loading ? 'animate-spin' : ''}`} />
              Refresh
            </button>
            <button
              onClick={onRegisterClick}
              className="flex items-center gap-2 px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition-colors"
            >
              <UserPlus className="w-4 h-4" />
              Add User
            </button>
          </div>
        </div>

        {/* Search */}
        <div className="relative">
          <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-4 h-4 text-gray-400" />
          <input
            type="text"
            placeholder="Search by name..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="w-full bg-gray-700 text-white pl-10 pr-4 py-2 rounded-lg border border-gray-600 focus:border-blue-400 focus:outline-none"
          />
        </div>
      </div>

      {/* Messages */}
      {error && (
        <div className="m-4 bg-red-500/20 border border-red-500 text-red-400 px-4 py-2 rounded-lg flex items-center gap-2">
          <AlertCircle className="w-4 h-4" />
          {error}
        </div>
      )}
      
      {success && (
        <div className="m-4 bg-green-500/20 border border-green-500 text-green-400 px-4 py-2 rounded-lg flex items-center gap-2">
          <CheckCircle className="w-4 h-4" />
          {success}
        </div>
      )}

      {/* User List */}
      <div className="overflow-x-auto">
        {loading ? (
          <div className="flex items-center justify-center p-8">
            <RefreshCw className="w-8 h-8 text-blue-400 animate-spin" />
          </div>
        ) : filteredUsers.length === 0 ? (
          <div className="text-center p-8 text-gray-400">
            {searchQuery ? 'No users found matching your search' : 'No registered users yet'}
          </div>
        ) : (
          <table className="w-full">
            <thead className="bg-gray-700/50">
              <tr>
                <th className="px-4 py-3 text-left text-xs font-medium text-gray-300 uppercase tracking-wider">
                  Name
                </th>
                <th className="px-4 py-3 text-left text-xs font-medium text-gray-300 uppercase tracking-wider">
                  Status
                </th>
                <th className="px-4 py-3 text-left text-xs font-medium text-gray-300 uppercase tracking-wider">
                  Registered
                </th>
                <th className="px-4 py-3 text-left text-xs font-medium text-gray-300 uppercase tracking-wider">
                  Model
                </th>
                <th className="px-4 py-3 text-right text-xs font-medium text-gray-300 uppercase tracking-wider">
                  Actions
                </th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-700">
              {filteredUsers.map((user) => (
                <tr key={user.user_id} className="hover:bg-gray-700/30 transition-colors">
                  <td className="px-4 py-3">
                    <div className="text-white font-medium">{user.name}</div>
                    <div className="text-xs text-gray-400">{user.user_id}</div>
                  </td>
                  <td className="px-4 py-3">
                    <span className={`inline-flex items-center px-2 py-1 rounded-full text-xs font-medium ${
                      user.status === 'active' 
                        ? 'bg-green-500/20 text-green-400' 
                        : 'bg-gray-500/20 text-gray-400'
                    }`}>
                      {user.status || 'active'}
                    </span>
                  </td>
                  <td className="px-4 py-3 text-sm text-gray-300">
                    {formatDate(user.registered_at)}
                  </td>
                  <td className="px-4 py-3 text-sm text-gray-300">
                    {user.model || 'unknown'}
                  </td>
                  <td className="px-4 py-3 text-right">
                    {deleteConfirm === user.user_id ? (
                      <div className="flex items-center justify-end gap-2">
                        <button
                          onClick={() => handleDelete(user.user_id, user.name)}
                          className="px-3 py-1 bg-red-500 text-white text-sm rounded hover:bg-red-600 transition-colors"
                        >
                          Confirm
                        </button>
                        <button
                          onClick={() => setDeleteConfirm(null)}
                          className="px-3 py-1 bg-gray-600 text-white text-sm rounded hover:bg-gray-500 transition-colors"
                        >
                          Cancel
                        </button>
                      </div>
                    ) : (
                      <button
                        onClick={() => setDeleteConfirm(user.user_id)}
                        className="inline-flex items-center gap-1 px-3 py-1 bg-red-500/20 text-red-400 rounded hover:bg-red-500/30 transition-colors"
                      >
                        <Trash2 className="w-4 h-4" />
                        Delete
                      </button>
                    )}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
};

export default UserManagement;
