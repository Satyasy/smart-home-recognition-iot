#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ==== CONFIGURE ====
// IP server Flask (PC / Raspberry Pi)
String FACE_SERVER_BASE = "http://192.168.5.221:5000";

// IP ESP8266 utama
String ESP8266_BASE = "http://192.168.5.250";

// WiFi
const char* ssid = "Zfox";
const char* password = "asdfg1234";

// Camera pins (AI Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
  } else {
      config.frame_size = FRAMESIZE_QVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
      Serial.printf("Camera init failed: 0x%x", err);
      return;
  }
}

void loop() {
  // Check sensor (ESP8266) apakah ada orang mendekat
  HTTPClient http;
  http.begin(ESP8266_BASE + "/check-distance");
  int code = http.GET();
  if (code == 200) {
    String resp = http.getString();
    if (resp.indexOf("should_capture\":true") != -1) {
      Serial.println("Object detected, capturing...");
      captureAndSend();
    }
  }
  http.end();

  delay(3000);
}

void captureAndSend() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  HTTPClient http;
  http.begin(FACE_SERVER_BASE + "/recognize");
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server response:");
    Serial.println(response);

    // Kirim ke ESP8266
    HTTPClient sendResult;
    sendResult.begin(ESP8266_BASE + "/face-result");
    sendResult.addHeader("Content-Type", "application/json");
    sendResult.POST(response);
    sendResult.end();
  }

  http.end();
  esp_camera_fb_return(fb);
}
