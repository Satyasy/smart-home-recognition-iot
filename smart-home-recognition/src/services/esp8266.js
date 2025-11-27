// ESP8266 Service untuk komunikasi dengan hardware
const ESP8266_BASE_URL = process.env.REACT_APP_ESP8266_IP 
  ? `http://${process.env.REACT_APP_ESP8266_IP}` 
  : 'http://192.168.5.250';

class ESP8266Service {
  // Helper untuk fetch dengan timeout
  async fetchWithTimeout(url, options = {}, timeout = 5000) {
    const controller = new AbortController();
    const id = setTimeout(() => controller.abort(), timeout);

    try {
      const response = await fetch(url, {
        ...options,
        signal: controller.signal,
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
      });
      clearTimeout(id);

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      return data;
    } catch (error) {
      clearTimeout(id);
      if (error.name === 'AbortError') {
        throw new Error('Request timeout');
      }
      throw error;
    }
  }

  // Get sensor data dari ESP8266
  async getSensorData() {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/sensor`);
      return data;
    } catch (error) {
      console.error('Failed to get sensor data:', error);
      throw error;
    }
  }

  // Get status aktuator dari ESP8266
  async getActuatorStatus() {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/status`);
      return data;
    } catch (error) {
      console.error('Failed to get actuator status:', error);
      throw error;
    }
  }

  // Unlock door dengan PIN
  async unlockDoor(pin) {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/unlock`, {
        method: 'POST',
        body: JSON.stringify({ pin: pin })
      });
      return data;
    } catch (error) {
      console.error('Failed to unlock door:', error);
      throw error;
    }
  }

  // Lock door
  async lockDoor() {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/lock`, {
        method: 'POST',
        body: JSON.stringify({})
      });
      return data;
    } catch (error) {
      console.error('Failed to lock door:', error);
      throw error;
    }
  }

  // Control LED
  async controlLED(color, state) {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/led`, {
        method: 'POST',
        body: JSON.stringify({
          color: color, // 'red' or 'green'
          state: state  // true or false
        })
      });
      return data;
    } catch (error) {
      console.error('Failed to control LED:', error);
      throw error;
    }
  }

  // Control Buzzer
  async controlBuzzer(state) {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/buzzer`, {
        method: 'POST',
        body: JSON.stringify({ state: state })
      });
      return data;
    } catch (error) {
      console.error('Failed to control buzzer:', error);
      throw error;
    }
  }

  // Update LCD Display
  async updateLCD(line1, line2) {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/lcd`, {
        method: 'POST',
        body: JSON.stringify({
          line1: line1,
          line2: line2
        })
      });
      return data;
    } catch (error) {
      console.error('Failed to update LCD:', error);
      throw error;
    }
  }

  // Control Lamp (Relay)
  async controlLamp(state) {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/lamp`, {
        method: 'POST',
        body: JSON.stringify({ state: state }) // 'on', 'off', or 'toggle'
      });
      return data;
    } catch (error) {
      console.error('Failed to control lamp:', error);
      throw error;
    }
  }

  // Trigger Alert (buzzer + LED flash)
  async triggerAlert() {
    try {
      const data = await this.fetchWithTimeout(`${ESP8266_BASE_URL}/alert`, {
        method: 'POST',
        body: JSON.stringify({})
      });
      return data;
    } catch (error) {
      console.error('Failed to trigger alert:', error);
      throw error;
    }
  }
}

const esp8266Service = new ESP8266Service();
export default esp8266Service;
