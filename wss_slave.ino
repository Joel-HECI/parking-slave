#include <Arduino.h>

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "esp_camera.h"
#include "placeholder.h"
// ============================================================
// CONFIGURATION
// ============================================================

const char* WIFI_SSID     = "David";
const char* WIFI_PASSWORD = "holaxd123";

const char* SERVER_HOST = "10.193.75.46";
const uint16_t SERVER_PORT = 8766;
const char* SERVER_PATH = "/parking";

#define DEVICE_ID "PARKING-ESP32-001"
#define DEVICE_TOKEN "EQRTaGx_kFcalO3eCjd_5qaBZjJV2MpMrqxuRhRTCSw"

// ============================================================
// CAMERA CONFIGURATION
// ============================================================

// IMPORTANT:
// Keep this false until the camera is physically installed.
#define CAMERA_ENABLED true

// Frames per second when streaming.
const uint32_t VIDEO_FRAME_INTERVAL_MS = 100;

// ============================================================
// IR SENSOR
// ============================================================

#define IR_SENSOR_PIN 13

int lastIRState = -1;

// ============================================================
// AI-THINKER ESP32-CAM CAMERA PINS
// ============================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ============================================================
// VIDEO SOURCE
// ============================================================

enum VideoSource {
  VIDEO_PLACEHOLDER,
  VIDEO_CAMERA
};

VideoSource videoSource = VIDEO_CAMERA;

bool cameraInitialized = false;

uint32_t lastVideoFrame = 0;
uint32_t videoFrameCounter = 0;

// ============================================================
// TLS CA CERTIFICATE
// ============================================================

const char* ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFWTCCA0GgAwIBAgIUJndPVYQNXyr1oNkENM83XXl3FXswDQYJKoZIhvcNAQEL
BQAwPDELMAkGA1UEBhMCQ08xFDASBgNVBAoMC1BhcmtpbmctSW9UMRcwFQYDVQQD
DA5QYXJraW5nIElvVCBDQTAeFw0yNjA5MTkxNzEwNDhaFw0zNjA5MTYxNzEwNDha
MDwxCzAJBgNVBAYTAkNPMRQwEgYDVQQKDAtQYXJraW5nLUlvVDEXMBUGA1UEAwwO
UGFya2luZyBJb1QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC4
f5z4zLpyC02l7Q9UGEuWOz7a3wG9H/OJFINUEeIhvaEsab3+QBDGHufTHwffkAlU
mEv88UaWHQ7bS3wmZ5GLz4TtFEgKPXt27DurS4bljmMvTh6xUXx9mektRrDNVuQ9
suLUE3ceqku8bpLj8S8PuqZlsqgW4R6s3eJhQcfdInYmgZll5aG6EdE+oAGH2aq7
VYDuDgOWLjgNifQ90AK2x8oN7lX06U06gzDHWlN0pIp7MweHJiKewDpAMv1LkMdF
CtvdCHZNxqmyzom/rEJFDXx/AArXfqBH+uCICztl+jZozgnvztFqdvzJ6e12NaA8
kJkFbMDbiKsJ+CC1SSBBguGL8XRSq7dIbu85YCfgqCbajWZL7UMsGcrdtftMvEUz
+ZzmVZiRdsvqOqpJkqmwytR1b1iCAC7r0lLkLqKQwiqEvA7lI9Nk42HaMgN2LTn8
biIiI1JBKUQub2fUb2UX5YwBodFZ8TQDhimmrrv7aevuna/k31FnCxE44zw71Yb/
cUhDh1wEkrN3LgItobewyhtK3GhmNk0pv5zAJtft3XQKmwOwJKu3/vSp0qhG7rh7
6GIr3R5QLDVtXO9SFZ8p/ZjbX9CTxMsasfzDvYCoX81g1V9DYK03VlPKws21LDAb
EOzpe7Uuv1TgBLGA87PKGF6E5FxaMzgYsZrSxLcPZQIDAQABo1MwUTAdBgNVHQ4E
FgQUu2v74IIGxH4v/ZUGywAKxiGcDRkwHwYDVR0jBBgwFoAUu2v74IIGxH4v/ZUG
ywAKxiGcDRkwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAgEAhdh1
jP5O/XAl53f0+fjDHbxe33fAJrQdovI7vvTYYGr12YxFmg90N2eZEiXtEFxLlm9T
B0asdmFfN1zB2N6BWC1zr7FOOKNnr5oniqeBgBFzfFsnge7QKZa8Ql1uteQ3a0Md
d2ybLCIR/vvqEwWXME6wS2XP4nX60zGwNY2mC4p55jxsFdXo2I296h4gxGOeKadD
t4eLXemYU9IqB2bO7nw0IdWarcshrGk4JwvCkoXzGxQ/aiO/3GKZVu9iJ/WObVyS
I0EVWQzZBYQvlbxxbdKoxWn4TPy0vC69wlbDWI0oQZ42+e8GuUIzQvZZyD00od5u
Sy2wPpEGVxBMiIRD909fuqys0m2iYFaF0lWB7++IJJW8SoFnYSpJHbJlXkxVg+0u
gqGtgQ79OUp/fgoSSM+eTT01lnrFvB8K1M93xrmDUORrzBpqv+gYrs05byrAIB2a
6T1m9ZaXKCgtQwtFoPiIqyvqS5EyVSMECPt4frVa4I7dKWVFHqdCgpZ3uCBIj+Lo
YfPm2N10NIONxZ8D84CB9vjrL+gkL+1DVvs3jA69yCgLbFxslFJ6dDZDRIXvtmgr
f8l4djoujyCHHiIDXV/kog3kVh2lvQrsjwfncIAgc9jMz4mHo1juSkHCgEJYNagK
lVfjQukCRNqjjDVGmFO3IM8Dt67JYzyLIaTb714=
-----END CERTIFICATE-----
)EOF";

// ============================================================
// WEBSOCKET
// ============================================================

WebSocketsClient webSocket;

bool websocketConnected = false;
bool authenticated = false;

// ============================================================
// PLACEHOLDER JPEG
// ============================================================
//
// This is a very small JPEG placeholder.
// The host receives it as a normal binary JPEG frame.
//
// The ESP32 doesn't need to know anything about how the host
// displays it.
//
// ============================================================


// ============================================================
// TIME
// ============================================================

String getTimestamp() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char buffer[32];

  strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%dT%H:%M:%S%z",
    &timeinfo
  );

  return String(buffer);
}

// ============================================================
// WIFI
// ============================================================

void connectWiFi() {

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}

// ============================================================
// NTP
// ============================================================

void synchronizeTime() {

  Serial.println("Synchronizing time...");

  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );

  struct tm timeinfo;

  for (int i = 0; i < 20; i++) {

    if (getLocalTime(&timeinfo)) {

      Serial.println("Time synchronized");
      return;
    }

    delay(500);
  }

  Serial.println("WARNING: NTP synchronization failed");
}

// ============================================================
// CAMERA INITIALIZATION
// ============================================================

bool initializeCamera() {

#if !CAMERA_ENABLED

  Serial.println();
  Serial.println("Camera support compiled out.");
  Serial.println("Using PLACEHOLDER video source.");

  return false;

#else

  Serial.println();
  Serial.println("Initializing AI-Thinker camera...");

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

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

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  // Start conservatively.
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  if (psramFound()) {

    Serial.println("PSRAM detected");

    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

  } else {

    Serial.println("PSRAM not detected");

    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.printf(
      "Camera initialization failed: 0x%x\n",
      err
    );

    Serial.println(
      "Falling back to PLACEHOLDER video."
    );

    return false;
  }

  sensor_t* sensor = esp_camera_sensor_get();

  if (sensor != nullptr) {

    sensor->set_framesize(
      sensor,
      psramFound() ? FRAMESIZE_VGA : FRAMESIZE_QVGA
    );
  }

  Serial.println("Camera initialized successfully");

  return true;

#endif
}

// ============================================================
// VIDEO SOURCE STATUS
// ============================================================

const char* videoSourceName() {

  switch (videoSource) {

    case VIDEO_CAMERA:
      return "camera";

    case VIDEO_PLACEHOLDER:
    default:
      return "placeholder";
  }
}

// ============================================================
// SEND VIDEO STATUS
// ============================================================

void sendVideoStatus() {

  if (!websocketConnected || !authenticated) {
    return;
  }

  JsonDocument doc;

  doc["type"] = "video_status";
  doc["device_id"] = DEVICE_ID;
  doc["source"] = videoSourceName();
  doc["camera_enabled"] = CAMERA_ENABLED;
  doc["camera_initialized"] = cameraInitialized;
  doc["timestamp"] = getTimestamp();

  String output;

  serializeJson(doc, output);

  webSocket.sendTXT(output);
}

// ============================================================
// SEND PLACEHOLDER FRAME
// ============================================================

void sendPlaceholderFrame() {

  if (!websocketConnected || !authenticated) {
    return;
  }

  // Send metadata first.
  JsonDocument doc;

  doc["type"] = "video_frame";
  doc["device_id"] = DEVICE_ID;
  doc["source"] = "placeholder";
  doc["format"] = "jpeg";
  doc["frame_id"] = videoFrameCounter++;
  doc["timestamp"] = getTimestamp();

  String metadata;

  serializeJson(doc, metadata);

  webSocket.sendTXT(metadata);

  // Then send JPEG as binary WebSocket frame.
  webSocket.sendBIN(
    (uint8_t*)placeholder_jpg,
    placeholder_jpg_len
  );
}

// ============================================================
// SEND CAMERA FRAME
// ============================================================

void sendCameraFrame() {

#if !CAMERA_ENABLED

  sendPlaceholderFrame();
  return;

#else

  if (!cameraInitialized) {

    sendPlaceholderFrame();
    return;
  }

  camera_fb_t* fb = esp_camera_fb_get();

  if (fb == nullptr) {

    Serial.println(
      "Camera capture failed - sending placeholder"
    );

    // Automatically recover to placeholder.
    videoSource = VIDEO_PLACEHOLDER;

    sendVideoStatus();
    sendPlaceholderFrame();

    return;
  }

  JsonDocument doc;

  doc["type"] = "video_frame";
  doc["device_id"] = DEVICE_ID;
  doc["source"] = "camera";
  doc["format"] = "jpeg";
  doc["width"] = fb->width;
  doc["height"] = fb->height;
  doc["frame_id"] = videoFrameCounter++;
  doc["timestamp"] = getTimestamp();

  String metadata;

  serializeJson(doc, metadata);

  webSocket.sendTXT(metadata);

  webSocket.sendBIN(
    fb->buf,
    fb->len
  );

  esp_camera_fb_return(fb);

#endif
}

// ============================================================
// VIDEO LOOP
// ============================================================

void videoLoop() {

  if (!websocketConnected || !authenticated) {
    return;
  }

  uint32_t now = millis();

  if (now - lastVideoFrame < VIDEO_FRAME_INTERVAL_MS) {
    return;
  }

  lastVideoFrame = now;

  switch (videoSource) {

    case VIDEO_CAMERA:
      sendCameraFrame();
      break;

    case VIDEO_PLACEHOLDER:
    default:
      sendPlaceholderFrame();
      break;
  }
}

// ============================================================
// SEND SENSOR
// ============================================================

void sendSensorEvent() {

  if (!websocketConnected || !authenticated) {
    return;
  }

  int state = digitalRead(IR_SENSOR_PIN);

  JsonDocument doc;

  doc["type"] = "sensor";
  doc["device_id"] = DEVICE_ID;

  JsonObject sensor = doc["sensor"].to<JsonObject>();

  sensor["type"] = "ir";
  sensor["gpio"] = IR_SENSOR_PIN;
  sensor["value"] = state;

  doc["timestamp"] = getTimestamp();
  doc["uptime_ms"] = millis();

  String output;

  serializeJson(doc, output);

  webSocket.sendTXT(output);

  Serial.print("TX SENSOR: ");
  Serial.println(output);
}

// ============================================================
// SEND STATUS
// ============================================================

void sendStatus() {

  if (!websocketConnected || !authenticated) {
    return;
  }

  JsonDocument doc;

  doc["type"] = "status";
  doc["device_id"] = DEVICE_ID;

  doc["uptime_ms"] = millis();

  JsonObject wifi = doc["wifi"].to<JsonObject>();

  wifi["rssi"] = WiFi.RSSI();
  wifi["ip"] = WiFi.localIP().toString();

  JsonObject sensor = doc["sensor"].to<JsonObject>();

  sensor["ir"] = digitalRead(IR_SENSOR_PIN);

  JsonObject video = doc["video"].to<JsonObject>();

  video["source"] = videoSourceName();
  video["camera_enabled"] = CAMERA_ENABLED;
  video["camera_initialized"] = cameraInitialized;

  doc["timestamp"] = getTimestamp();

  String output;

  serializeJson(doc, output);

  webSocket.sendTXT(output);
}

// ============================================================
// HANDLE SERVER MESSAGE
// ============================================================

void handleServerMessage(uint8_t* payload, size_t length) {

  String message;

  for (size_t i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("RX: ");
  Serial.println(message);

  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, message);

  if (error) {

    Serial.println(
      "Invalid JSON from server"
    );

    return;
  }

  const char* type =
    doc["type"] | "";

  // ----------------------------------------------------------
  // AUTH OK
  // ----------------------------------------------------------

  if (strcmp(type, "auth_ok") == 0) {

    authenticated = true;

    Serial.println(
      "SERVER AUTHENTICATION ACCEPTED"
    );

    sendVideoStatus();

    return;
  }

  // ----------------------------------------------------------
  // AUTH FAILED
  // ----------------------------------------------------------

  if (strcmp(type, "auth_failed") == 0) {

    authenticated = false;

    Serial.println(
      "SERVER AUTHENTICATION FAILED"
    );

    return;
  }

  // ----------------------------------------------------------
  // PING
  // ----------------------------------------------------------

  if (strcmp(type, "ping") == 0) {

    JsonDocument response;

    response["type"] = "pong";
    response["device_id"] = DEVICE_ID;

    String output;

    serializeJson(response, output);

    webSocket.sendTXT(output);

    return;
  }

  // ----------------------------------------------------------
  // GET SENSOR
  // ----------------------------------------------------------

  if (strcmp(type, "get_sensor") == 0) {

    sendSensorEvent();

    return;
  }

  // ----------------------------------------------------------
  // VIDEO SOURCE CONTROL
  // ----------------------------------------------------------
  //
  // This gives the server the ability to select:
  //
  //   "placeholder"
  //   "camera"
  //
  // Later this can be useful for diagnostics.
  //

  if (strcmp(type, "set_video_source") == 0) {

    const char* source =
      doc["source"] | "placeholder";

    if (strcmp(source, "camera") == 0) {

#if CAMERA_ENABLED

      if (cameraInitialized) {

        videoSource = VIDEO_CAMERA;

        Serial.println(
          "Video source changed to CAMERA"
        );

      } else {

        videoSource = VIDEO_PLACEHOLDER;

        Serial.println(
          "Camera unavailable - keeping PLACEHOLDER"
        );
      }

#else

      videoSource = VIDEO_PLACEHOLDER;

      Serial.println(
        "Camera disabled in firmware - keeping PLACEHOLDER"
      );

#endif

    } else {

      videoSource = VIDEO_PLACEHOLDER;

      Serial.println(
        "Video source changed to PLACEHOLDER"
      );
    }

    sendVideoStatus();

    return;
  }
}

// ============================================================
// WEBSOCKET EVENT
// ============================================================

void webSocketEvent(
  WStype_t type,
  uint8_t* payload,
  size_t length
) {

  switch (type) {

    case WStype_DISCONNECTED:

      Serial.println("WSS DISCONNECTED");

      websocketConnected = false;
      authenticated = false;

      break;

    case WStype_CONNECTED:

      Serial.println("WSS CONNECTED");

      websocketConnected = true;
      authenticated = false;

      // ------------------------------------------------------
      // AUTHENTICATION
      // ------------------------------------------------------

      {
        JsonDocument doc;

        doc["type"] = "auth";
        doc["device_id"] = DEVICE_ID;
        doc["token"] = DEVICE_TOKEN;

        String output;

        serializeJson(doc, output);

        Serial.print("TX AUTH: ");
        Serial.println(output);

        webSocket.sendTXT(output);
      }

      break;

    case WStype_TEXT:

      handleServerMessage(
        payload,
        length
      );

      break;

    case WStype_BIN:

      // Server should not normally send binary data
      // to the ESP32.
      Serial.printf(
        "RX BINARY: %u bytes\n",
        (unsigned int)length
      );

      break;

    case WStype_ERROR:

      Serial.println("WSS ERROR");

      break;

    default:
      break;
  }
}

void connectWebSocket()
{
    Serial.println();
    Serial.println(
        "Starting secure WebSocket..."
    );

    // --------------------------------------------------------
    // WSS / TLS WITH CA CERTIFICATE VALIDATION
    // --------------------------------------------------------

    webSocket.beginSslWithCA(
        SERVER_HOST,
        SERVER_PORT,
        SERVER_PATH,
        ROOT_CA
    );

    // --------------------------------------------------------
    // Reconnect automatically
    // --------------------------------------------------------

    webSocket.setReconnectInterval(
        5000
    );

    // --------------------------------------------------------
    // Event callback
    // --------------------------------------------------------

    webSocket.onEvent(
        webSocketEvent
    );

    // --------------------------------------------------------
    // Heartbeat
    // --------------------------------------------------------

    webSocket.enableHeartbeat(
        15000,
        3000,
        2
    );

    Serial.println(
    "WSS client initialized"
  );
}
// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" PARKING ESP32-CAM SLAVE");
  Serial.println("================================");

  // ----------------------------------------------------------
  // IR SENSOR
  // ----------------------------------------------------------

  pinMode(
    IR_SENSOR_PIN,
    INPUT
  );

  lastIRState =
    digitalRead(IR_SENSOR_PIN);

  // ----------------------------------------------------------
  // WIFI
  // ----------------------------------------------------------

  connectWiFi();

  // ----------------------------------------------------------
  // TIME
  // ----------------------------------------------------------

  synchronizeTime();

  // ----------------------------------------------------------
  // CAMERA
  // ----------------------------------------------------------

  cameraInitialized =
    initializeCamera();

  if (cameraInitialized) {

    videoSource = VIDEO_CAMERA;

  } else {

    videoSource = VIDEO_PLACEHOLDER;
  }

  Serial.print("Video source: ");
  Serial.println(videoSourceName());

  // ----------------------------------------------------------
  // WSS
  // ----------------------------------------------------------

  connectWebSocket();

}

// ============================================================
// LOOP
// ============================================================

uint32_t lastSensorSample = 0;
uint32_t lastStatus = 0;

void loop() {

  webSocket.loop();

  // ----------------------------------------------------------
  // IR SENSOR
  // ----------------------------------------------------------

  uint32_t now = millis();

  if (now - lastSensorSample >= 100) {

    lastSensorSample = now;

    int currentState =
      digitalRead(IR_SENSOR_PIN);

    if (currentState != lastIRState) {

      lastIRState = currentState;

      Serial.print(
        "IR changed: "
      );

      Serial.println(currentState);

      sendSensorEvent();
    }
  }

  // ----------------------------------------------------------
  // STATUS
  // ----------------------------------------------------------

  if (now - lastStatus >= 5000) {

    lastStatus = now;

    sendStatus();
  }

  // ----------------------------------------------------------
  // VIDEO
  // ----------------------------------------------------------

  videoLoop();
}