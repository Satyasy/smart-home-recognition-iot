// ESP32-CAM Service untuk komunikasi dengan ESP32-CAM
const ESP32_CAM_IP = process.env.REACT_APP_ESP32_CAM_IP || '192.168.5.86';
const ESP32_CAM_BASE_URL = `http://${ESP32_CAM_IP}`;

class ESP32CamService {
  constructor() {
    this.streamUrl = `${ESP32_CAM_BASE_URL}/stream`;
    this.timeout = 20000; // 10 seconds
  }

  // Helper untuk fetch dengan timeout
  async fetchWithTimeout(url, options = {}, timeout = this.timeout) {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), timeout);

    try {
      const response = await fetch(url, {
        ...options,
        signal: controller.signal,
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
      });

      clearTimeout(timeoutId);

      if (!response.ok) {
        const errorData = await response.json().catch(() => ({ message: 'Unknown error' }));
        throw new Error(errorData.message || `HTTP ${response.status}`);
      }

      return response;
    } catch (error) {
      clearTimeout(timeoutId);
      if (error.name === 'AbortError') {
        throw new Error('Request timeout');
      }
      throw error;
    }
  }

  // Get camera status
  async getStatus() {
    try {
      const response = await this.fetchWithTimeout(`${ESP32_CAM_BASE_URL}/status`, {
        method: 'GET',
      });
      
      const data = await response.json();
      console.log('[ESP32-CAM] Status:', data);
      return data;
    } catch (error) {
      console.error('[ESP32-CAM] Status error:', error);
      throw error;
    }
  }

  // Get stream URL
  getStreamUrl() {
    return this.streamUrl;
  }

  // Capture image (returns base64)
  async captureImage() {
    try {
      console.log('[ESP32-CAM] Capturing image...');
      const response = await this.fetchWithTimeout(`${ESP32_CAM_BASE_URL}/capture`, {
        method: 'GET',
      });
      
      const data = await response.json();
      
      if (data.success && data.image) {
        console.log('[ESP32-CAM] Image captured:', data.size, 'bytes');
        return {
          success: true,
          image: data.image,
          size: data.size,
          width: data.width,
          height: data.height,
        };
      } else {
        throw new Error(data.message || 'Capture failed');
      }
    } catch (error) {
      console.error('[ESP32-CAM] Capture error:', error);
      throw error;
    }
  }

  // Trigger face recognition
  async recognizeFace() {
    try {
      console.log('[ESP32-CAM] Triggering face recognition...');
      const response = await this.fetchWithTimeout(
        `${ESP32_CAM_BASE_URL}/recognize`,
        {
          method: 'POST',
        },
        20000 // 15 seconds timeout for recognition
      );
      
      const data = await response.json();
      console.log('[ESP32-CAM] Recognition result:', data);
      
      return {
        success: data.success,
        recognized: data.recognized,
        name: data.name,
        confidence: data.confidence,
        message: data.message,
      };
    } catch (error) {
      console.error('[ESP32-CAM] Recognition error:', error);
      throw error;
    }
  }

  // Set flash brightness (0-255)
  async setFlashBrightness(brightness) {
    try {
      if (brightness < 0 || brightness > 255) {
        throw new Error('Brightness must be between 0-255');
      }

      console.log('[ESP32-CAM] Setting flash brightness:', brightness);
      const response = await this.fetchWithTimeout(
        `${ESP32_CAM_BASE_URL}/flash?brightness=${brightness}`,
        {
          method: 'GET',
        }
      );
      
      const data = await response.json();
      console.log('[ESP32-CAM] Flash brightness set:', data);
      return data;
    } catch (error) {
      console.error('[ESP32-CAM] Flash control error:', error);
      throw error;
    }
  }

  // Test connection
  async testConnection() {
    try {
      const status = await this.getStatus();
      return {
        connected: true,
        ip: status.ip,
        device: status.device,
        rssi: status.rssi,
        uptime: status.uptime,
      };
    } catch (error) {
      return {
        connected: false,
        error: error.message,
      };
    }
  }
}

const esp32CamService = new ESP32CamService();
export default esp32CamService;
