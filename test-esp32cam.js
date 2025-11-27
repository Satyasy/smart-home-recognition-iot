// Test ESP32-CAM Connection dari Browser Console
// Copy paste code ini ke Browser Console (F12) saat Dashboard terbuka

console.log('🧪 Testing ESP32-CAM Connection...\n');

// Test 1: Status Endpoint
console.log('Test 1: Fetching /status...');
fetch('http://192.168.5.86/status')
  .then(response => {
    console.log('✅ Status Response:', response.status, response.statusText);
    console.log('   Headers:', [...response.headers.entries()]);
    return response.json();
  })
  .then(data => {
    console.log('✅ Status Data:', data);
  })
  .catch(error => {
    console.error('❌ Status Error:', error);
  });

// Test 2: Stream Endpoint (Image Load)
console.log('\nTest 2: Testing /stream...');
const testImg = new Image();
testImg.onload = () => {
  console.log('✅ Stream image loaded successfully!');
  console.log('   Image size:', testImg.width, 'x', testImg.height);
};
testImg.onerror = (e) => {
  console.error('❌ Stream image error:', e);
};
testImg.src = 'http://192.168.5.86/stream?test=' + Date.now();

// Test 3: CORS Preflight
console.log('\nTest 3: Testing CORS preflight...');
fetch('http://192.168.5.86/status', {
  method: 'OPTIONS',
})
  .then(response => {
    console.log('✅ CORS Preflight Response:', response.status);
    console.log('   CORS Headers:');
    console.log('   - Allow-Origin:', response.headers.get('Access-Control-Allow-Origin'));
    console.log('   - Allow-Methods:', response.headers.get('Access-Control-Allow-Methods'));
    console.log('   - Allow-Headers:', response.headers.get('Access-Control-Allow-Headers'));
  })
  .catch(error => {
    console.error('❌ CORS Preflight Error:', error);
  });

// Test 4: Network Connectivity
console.log('\nTest 4: Testing network connectivity...');
fetch('http://192.168.5.86/status', {
  mode: 'cors',
  credentials: 'omit',
})
  .then(response => {
    console.log('✅ Network connectivity OK');
    console.log('   Response Type:', response.type);
    console.log('   Response OK:', response.ok);
  })
  .catch(error => {
    console.error('❌ Network Error:', error);
    console.error('   Error Type:', error.name);
    console.error('   Error Message:', error.message);
  });

// Test 5: Using ESP32CamService
console.log('\nTest 5: Using ESP32CamService class...');
setTimeout(() => {
  import('./services/esp32cam.js')
    .then(module => {
      const esp32CamService = module.default;
      return esp32CamService.getStatus();
    })
    .then(status => {
      console.log('✅ ESP32CamService.getStatus():', status);
    })
    .catch(error => {
      console.error('❌ ESP32CamService Error:', error);
    });
}, 1000);

console.log('\n⏳ Running all tests... check results above\n');
