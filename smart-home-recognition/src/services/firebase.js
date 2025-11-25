// Firebase Realtime Database Service untuk ESP32 integration
// This connects to Firebase RTDB to get real-time sensor data from ESP32

const FIREBASE_DB_URL = 'https://iot-rc-ef82d-default-rtdb.asia-southeast1.firebasedatabase.app';

class FirebaseService {
  static subscribeSensorData(callback) {
    const eventSource = new EventSource(`${FIREBASE_DB_URL}/sensors.json`);
    
    eventSource.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        callback(data);
      } catch (error) {
        console.error('Error parsing sensor data:', error);
      }
    };

    eventSource.onerror = (error) => {
      console.error('EventSource error:', error);
      eventSource.close();
    };

    return () => eventSource.close();
  }

  static async getSensorData() {
    try {
      const response = await fetch(`${FIREBASE_DB_URL}/sensors.json`);
      return await response.json();
    } catch (error) {
      console.error('Get sensor data failed:', error);
      throw error;
    }
  }

  static async getAccessLogs(limit = 50) {
    try {
      const response = await fetch(
        `${FIREBASE_DB_URL}/access_logs.json?orderBy="$key"&limitToLast=${limit}`
      );
      const data = await response.json();
      
      if (!data) return [];
      
      return Object.keys(data).map(key => ({
        id: key,
        ...data[key]
      })).reverse();
    } catch (error) {
      console.error('Get access logs failed:', error);
      throw error;
    }
  }

  static async getUsers() {
    try {
      const response = await fetch(`${FIREBASE_DB_URL}/users.json`);
      const data = await response.json();
      
      if (!data) return [];
      
      return Object.keys(data).map(key => ({
        id: key,
        ...data[key]
      }));
    } catch (error) {
      console.error('Get users failed:', error);
      throw error;
    }
  }
}

export default FirebaseService;
