// Test utility untuk ESP8266
const ESP8266_BASE_URL = process.env.REACT_APP_ESP8266_IP 
  ? `http://${process.env.REACT_APP_ESP8266_IP}` 
  : 'http://192.168.5.250';

// Test koneksi ke ESP8266
export async function testESP8266Connection() {
  console.log('=== ESP8266 Connection Test ===');
  console.log('Base URL:', ESP8266_BASE_URL);
  
  const tests = [
    {
      name: 'Ping test (GET /)',
      url: ESP8266_BASE_URL,
      method: 'GET'
    },
    {
      name: 'Sensor data test',
      url: `${ESP8266_BASE_URL}/sensor`,
      method: 'GET'
    },
    {
      name: 'Status test',
      url: `${ESP8266_BASE_URL}/status`,
      method: 'GET'
    }
  ];

  for (const test of tests) {
    console.log(`\nTesting: ${test.name}`);
    console.log(`URL: ${test.url}`);
    
    try {
      const controller = new AbortController();
      const timeout = setTimeout(() => controller.abort(), 3000);
      
      const response = await fetch(test.url, {
        method: test.method,
        signal: controller.signal,
        mode: 'cors'
      });
      
      clearTimeout(timeout);
      
      console.log(`✅ Status: ${response.status}`);
      
      try {
        const data = await response.json();
        console.log('Response data:', data);
      } catch (e) {
        const text = await response.text();
        console.log('Response text:', text);
      }
      
    } catch (error) {
      if (error.name === 'AbortError') {
        console.log('❌ Timeout - ESP8266 tidak merespon dalam 3 detik');
      } else if (error.message.includes('Failed to fetch')) {
        console.log('❌ Failed to fetch - Kemungkinan:');
        console.log('   1. ESP8266 tidak running');
        console.log('   2. CORS tidak enabled di ESP8266');
        console.log('   3. IP address salah');
        console.log('   4. Firewall blocking');
      } else {
        console.log('❌ Error:', error.message);
      }
    }
  }
  
  console.log('\n=== Test Complete ===');
}

// Simulasi data ESP8266 untuk development
export const mockESP8266Data = {
  sensor: {
    temperature: 25.5,
    humidity: 65.0,
    light: 450,
    distance: 15.5
  },
  status: {
    servo: {
      angle: 0,
      locked: true
    },
    led: {
      red: true,
      green: false
    },
    buzzer: {
      active: false
    },
    lcd: {
      line1: "Door LOCKED",
      line2: "Waiting..."
    }
  }
};

// Check if ESP8266 is reachable
export async function isESP8266Reachable() {
  try {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 2000);
    
    const response = await fetch(ESP8266_BASE_URL, {
      method: 'GET',
      signal: controller.signal,
      mode: 'no-cors' // Try no-cors first
    });
    
    clearTimeout(timeout);
    return true;
  } catch (error) {
    return false;
  }
}

export default {
  testESP8266Connection,
  mockESP8266Data,
  isESP8266Reachable
};
