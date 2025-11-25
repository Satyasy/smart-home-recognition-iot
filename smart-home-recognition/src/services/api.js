// API Service untuk komunikasi dengan Flask Backend
const API_BASE_URL = 'http://localhost:5000/api';

class ApiService {
  // Health Check
  static async healthCheck() {
    try {
      const response = await fetch(`${API_BASE_URL}/health`);
      return await response.json();
    } catch (error) {
      console.error('Health check failed:', error);
      throw error;
    }
  }

  // Register Face
  static async registerFace(imageBase64, name, email = '', phone = '') {
    try {
      const response = await fetch(`${API_BASE_URL}/register`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          image: imageBase64,
          name,
          email,
          phone
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Register face failed:', error);
      throw error;
    }
  }

  // Recognize Face
  static async recognizeFace(imageBase64) {
    try {
      const response = await fetch(`${API_BASE_URL}/recognize`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          image: imageBase64
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Recognize face failed:', error);
      throw error;
    }
  }

  // Get All Users
  static async getUsers() {
    try {
      const response = await fetch(`${API_BASE_URL}/users`);
      return await response.json();
    } catch (error) {
      console.error('Get users failed:', error);
      throw error;
    }
  }

  // Get User by ID
  static async getUser(userId) {
    try {
      const response = await fetch(`${API_BASE_URL}/user/${userId}`);
      return await response.json();
    } catch (error) {
      console.error('Get user failed:', error);
      throw error;
    }
  }

  // Update User
  static async updateUser(userId, data) {
    try {
      const response = await fetch(`${API_BASE_URL}/user/${userId}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data)
      });
      return await response.json();
    } catch (error) {
      console.error('Update user failed:', error);
      throw error;
    }
  }

  // Delete User
  static async deleteUser(userId) {
    try {
      const response = await fetch(`${API_BASE_URL}/user/${userId}`, {
        method: 'DELETE'
      });
      return await response.json();
    } catch (error) {
      console.error('Delete user failed:', error);
      throw error;
    }
  }

  // Get Access Logs
  static async getLogs(limit = 50) {
    try {
      const response = await fetch(`${API_BASE_URL}/logs?limit=${limit}`);
      return await response.json();
    } catch (error) {
      console.error('Get logs failed:', error);
      throw error;
    }
  }

  // Clear Logs
  static async clearLogs() {
    try {
      const response = await fetch(`${API_BASE_URL}/logs/clear`, {
        method: 'DELETE'
      });
      return await response.json();
    } catch (error) {
      console.error('Clear logs failed:', error);
      throw error;
    }
  }

  // Get Config
  static async getConfig() {
    try {
      const response = await fetch(`${API_BASE_URL}/config`);
      return await response.json();
    } catch (error) {
      console.error('Get config failed:', error);
      throw error;
    }
  }

  // Verify Two Faces
  static async verifyFaces(image1Base64, image2Base64) {
    try {
      const response = await fetch(`${API_BASE_URL}/verify`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          image1: image1Base64,
          image2: image2Base64
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Verify faces failed:', error);
      throw error;
    }
  }
}

export default ApiService;
