// API Service untuk komunikasi dengan Flask Backend
const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

class ApiService {
  // Helper untuk fetch dengan error handling
  async fetchWithError(url, options = {}) {
    try {
      const response = await fetch(url, {
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
        ...options,
      });

      const data = await response.json();
      
      if (!response.ok) {
        throw new Error(data.message || 'API request failed');
      }

      return data;
    } catch (error) {
      console.error('API Error:', error);
      throw error;
    }
  }

  // Register new user dengan face image
  async registerUser(imageBase64, name, email, phone = '') {
    return this.fetchWithError(`${API_BASE_URL}/register`, {
      method: 'POST',
      body: JSON.stringify({
        image: imageBase64,
        name,
        email,
        phone
      }),
    });
  }

  // Recognize face dari ESP32-CAM
  async recognizeFace(imageBase64) {
    return this.fetchWithError(`${API_BASE_URL}/recognize`, {
      method: 'POST',
      body: JSON.stringify({
        image: imageBase64
      }),
    });
  }

  // Verify two faces
  async verifyFaces(image1Base64, image2Base64) {
    return this.fetchWithError(`${API_BASE_URL}/verify`, {
      method: 'POST',
      body: JSON.stringify({
        image1: image1Base64,
        image2: image2Base64
      }),
    });
  }

  // Get all registered users
  async getUsers() {
    return this.fetchWithError(`${API_BASE_URL}/users`, {
      method: 'GET',
    });
  }

  // Get specific user
  async getUser(userId) {
    return this.fetchWithError(`${API_BASE_URL}/user/${userId}`, {
      method: 'GET',
    });
  }

  // Update user
  async updateUser(userId, data) {
    return this.fetchWithError(`${API_BASE_URL}/user/${userId}`, {
      method: 'PUT',
      body: JSON.stringify(data),
    });
  }

  // Delete user
  async deleteUser(userId) {
    return this.fetchWithError(`${API_BASE_URL}/user/${userId}`, {
      method: 'DELETE',
    });
  }

  // Get access logs
  async getAccessLogs(limit = 50) {
    return this.fetchWithError(`${API_BASE_URL}/logs?limit=${limit}`, {
      method: 'GET',
    });
  }

  // Clear all logs
  async clearLogs() {
    return this.fetchWithError(`${API_BASE_URL}/logs/clear`, {
      method: 'DELETE',
    });
  }

  // Get system configuration
  async getConfig() {
    return this.fetchWithError(`${API_BASE_URL}/config`, {
      method: 'GET',
    });
  }

  // Health check
  async healthCheck() {
    return this.fetchWithError(`${API_BASE_URL}/health`, {
      method: 'GET',
    });
  }

  // Verify PIN
  async verifyPin(pin) {
    return this.fetchWithError(`${API_BASE_URL}/verify-pin`, {
      method: 'POST',
      body: JSON.stringify({
        pin: pin
      }),
    });
  }

  // Get all PINs
  async getPins() {
    return this.fetchWithError(`${API_BASE_URL}/pins`, {
      method: 'GET',
    });
  }

  // Create new PIN
  async createPin(pin, userName) {
    return this.fetchWithError(`${API_BASE_URL}/pin`, {
      method: 'POST',
      body: JSON.stringify({
        pin: pin,
        user_name: userName
      }),
    });
  }
}

const apiService = new ApiService();
export default apiService;
