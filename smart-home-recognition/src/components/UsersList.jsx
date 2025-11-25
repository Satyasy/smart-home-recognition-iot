import React, { useState, useEffect } from 'react';
import { Users, Trash2, UserPlus } from 'lucide-react';
import ApiService from '../services/api';
import RegisterUser from './RegisterUser';

const UsersList = () => {
  const [users, setUsers] = useState([]);
  const [loading, setLoading] = useState(true);
  const [showRegister, setShowRegister] = useState(false);

  const loadUsers = async () => {
    try {
      const result = await ApiService.getUsers();
      if (result.success) {
        setUsers(result.users);
      }
    } catch (error) {
      console.error('Failed to load users:', error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadUsers();
  }, []);

  const handleDelete = async (userId, userName) => {
    if (window.confirm(`Hapus ${userName}?`)) {
      try {
        const result = await ApiService.deleteUser(userId);
        if (result.success) {
          loadUsers();
        } else {
          alert(result.message);
        }
      } catch (error) {
        alert('Gagal menghapus pengguna');
      }
    }
  };

  return (
    <div className="bg-gray-800/50 backdrop-blur-lg rounded-2xl p-6 border border-gray-700">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Users className="w-5 h-5 text-blue-400" />
          <h2 className="text-lg font-bold text-white">Pengguna Terdaftar</h2>
        </div>
        <button
          onClick={() => setShowRegister(true)}
          className="flex items-center gap-2 px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition"
        >
          <UserPlus className="w-4 h-4" />
          Tambah
        </button>
      </div>

      {loading ? (
        <div className="text-center text-gray-400 py-8">Loading...</div>
      ) : users.length === 0 ? (
        <div className="text-center text-gray-400 py-8">
          Belum ada pengguna terdaftar
        </div>
      ) : (
        <div className="space-y-2">
          {users.map((user) => (
            <div
              key={user.user_id}
              className="flex items-center justify-between p-3 bg-gray-700/50 rounded-lg hover:bg-gray-700 transition"
            >
              <div>
                <div className="text-white font-medium">{user.name}</div>
                <div className="text-sm text-gray-400">
                  {user.email || 'No email'}
                </div>
                <div className="text-xs text-gray-500">
                  Registered: {new Date(user.registered_at).toLocaleDateString('id-ID')}
                </div>
              </div>
              <button
                onClick={() => handleDelete(user.user_id, user.name)}
                className="p-2 text-red-400 hover:bg-red-500/20 rounded-lg transition"
              >
                <Trash2 className="w-5 h-5" />
              </button>
            </div>
          ))}
        </div>
      )}

      {showRegister && (
        <RegisterUser
          onSuccess={loadUsers}
          onClose={() => setShowRegister(false)}
        />
      )}
    </div>
  );
};

export default UsersList;
