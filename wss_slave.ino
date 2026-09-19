#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ============================================================
// DEVICE CONFIGURATION
// ============================================================

#define DEVICE_ID "PARKING-ESP32-001"

// Change this to a long random secret.
// Every ESP32 slave should have a DIFFERENT secret.
#define DEVICE_TOKEN "CHANGE_THIS_TO_A_LONG_RANDOM_SECRET"

// ============================================================
// WIFI CONFIGURATION
// ============================================================

const char* WIFI_SSID = "TITAPUTIN";
const char* WIFI_PASSWORD = "Renegade#2025";

// ============================================================
// RASPBERRY PI CONFIGURATION
// ============================================================

// Raspberry Pi IP address
const char* SERVER_HOST = "192.168.1.23";

// WSS server port
const uint16_t SERVER_PORT = 8766;

// WebSocket endpoint
const char* SERVER_PATH = "/parking";

// ============================================================
// TLS CERTIFICATE
// ============================================================
//
// Paste the CA certificate used to sign the Raspberry Pi
// server certificate here.
//
// Do NOT use setInsecure() for the final system.
//

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
// HARDWARE
// ============================================================

#define IR_SENSOR_PIN 13

// ============================================================
// TIMING
// ============================================================

#define SENSOR_SAMPLE_INTERVAL 100

// Send periodic status even when sensor state hasn't changed
#define STATUS_INTERVAL 5000

// ============================================================
// GLOBAL OBJECTS
// ============================================================

WebSocketsClient webSocket;

// ============================================================
// STATE
// ============================================================

int lastSensorState = -1;

unsigned long lastSensorSample = 0;
unsigned long lastStatusMessage = 0;

// ============================================================
// GET TIME
// ============================================================

String getTimestamp()
{
    time_t now;

    time(&now);

    if (now < 1700000000)
    {
        return "unsynchronized";
    }

    struct tm timeinfo;

    gmtime_r(&now, &timeinfo);

    char buffer[32];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%dT%H:%M:%SZ",
        &timeinfo
    );

    return String(buffer);
}

// ============================================================
// SEND AUTHENTICATION
// ============================================================

void sendAuthentication()
{
    Serial.println("Sending authentication...");

    StaticJsonDocument<256> doc;

    doc["type"] = "auth";
    doc["device_id"] = DEVICE_ID;
    doc["token"] = DEVICE_TOKEN;

    String message;

    serializeJson(doc, message);

    webSocket.sendTXT(message);

    Serial.print("TX AUTH: ");
    Serial.println(message);
}

// ============================================================
// SEND SENSOR EVENT
// ============================================================

void sendSensorEvent(int state)
{
    if (!webSocket.isConnected())
    {
        return;
    }

    StaticJsonDocument<384> doc;

    doc["type"] = "sensor";
    doc["device_id"] = DEVICE_ID;

    JsonObject sensor = doc.createNestedObject("sensor");

    sensor["type"] = "ir";
    sensor["gpio"] = IR_SENSOR_PIN;
    sensor["value"] = state;

    doc["timestamp"] = getTimestamp();
    doc["uptime_ms"] = millis();

    String message;

    serializeJson(doc, message);

    webSocket.sendTXT(message);

    Serial.print("TX SENSOR: ");
    Serial.println(message);
}

// ============================================================
// SEND DEVICE STATUS
// ============================================================

void sendStatus()
{
    if (!webSocket.isConnected())
    {
        return;
    }

    StaticJsonDocument<384> doc;

    doc["type"] = "status";
    doc["device_id"] = DEVICE_ID;

    doc["uptime_ms"] = millis();

    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["ip"] = WiFi.localIP().toString();

    doc["sensor"]["ir"] = digitalRead(IR_SENSOR_PIN);

    doc["timestamp"] = getTimestamp();

    String message;

    serializeJson(doc, message);

    webSocket.sendTXT(message);

    Serial.print("TX STATUS: ");
    Serial.println(message);
}

// ============================================================
// WEBSOCKET EVENT HANDLER
// ============================================================

void webSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
)
{
    switch (type)
    {
        // ----------------------------------------------------
        // CONNECTED
        // ----------------------------------------------------

        case WStype_CONNECTED:

            Serial.println();
            Serial.println("================================");
            Serial.println("WSS CONNECTED");
            Serial.println("================================");

            sendAuthentication();

            break;


        // ----------------------------------------------------
        // DISCONNECTED
        // ----------------------------------------------------

        case WStype_DISCONNECTED:

            Serial.println();
            Serial.println("WSS DISCONNECTED");

            break;


        // ----------------------------------------------------
        // TEXT MESSAGE
        // ----------------------------------------------------

        case WStype_TEXT:
        {
            Serial.print("RX: ");

            for (size_t i = 0; i < length; i++)
            {
                Serial.print((char)payload[i]);
            }

            Serial.println();

            StaticJsonDocument<512> doc;

            DeserializationError error =
                deserializeJson(
                    doc,
                    payload,
                    length
                );

            if (error)
            {
                Serial.print("JSON error: ");
                Serial.println(error.c_str());

                return;
            }

            const char* type =
                doc["type"];

            if (!type)
            {
                Serial.println("Message has no type");
                return;
            }

            // ------------------------------------------------
            // AUTHENTICATION RESULT
            // ------------------------------------------------

            if (strcmp(type, "auth_ok") == 0)
            {
                Serial.println(
                    "SERVER AUTHENTICATION ACCEPTED"
                );
            }

            else if (strcmp(type, "auth_failed") == 0)
            {
                Serial.println(
                    "SERVER REJECTED AUTHENTICATION"
                );

                // The server should close the connection.
                webSocket.disconnect();
            }

            // ------------------------------------------------
            // PING
            // ------------------------------------------------

            else if (strcmp(type, "ping") == 0)
            {
                StaticJsonDocument<128> response;

                response["type"] = "pong";
                response["device_id"] = DEVICE_ID;

                String message;

                serializeJson(
                    response,
                    message
                );

                webSocket.sendTXT(message);
            }

            // ------------------------------------------------
            // SERVER REQUESTS CURRENT SENSOR STATE
            // ------------------------------------------------

            else if (
                strcmp(type, "get_sensor") == 0
            )
            {
                sendSensorEvent(
                    digitalRead(IR_SENSOR_PIN)
                );
            }

            // ------------------------------------------------
            // UNKNOWN MESSAGE
            // ------------------------------------------------

            else
            {
                Serial.print(
                    "Unknown message type: "
                );

                Serial.println(type);
            }

            break;
        }


        // ----------------------------------------------------
        // BINARY
        // ----------------------------------------------------

        case WStype_BIN:

            Serial.println(
                "Received binary message"
            );

            break;


        default:
            break;
    }
}

// ============================================================
// CONNECT TO WIFI
// ============================================================

void connectWiFi()
{
    Serial.println();
    Serial.println(
        "Connecting to WiFi..."
    );

    WiFi.mode(WIFI_STA);

    WiFi.setSleep(false);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    while (
        WiFi.status() != WL_CONNECTED
    )
    {
        delay(500);

        Serial.print(".");
    }

    Serial.println();
    Serial.println(
        "WiFi connected"
    );

    Serial.print(
        "IP address: "
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.print(
        "RSSI: "
    );

    Serial.println(
        WiFi.RSSI()
    );
}

// ============================================================
// SYNCHRONIZE CLOCK
// ============================================================
//
// TLS certificates have validity periods, so the ESP32 needs
// a reasonably accurate clock before certificate validation.
//

void synchronizeClock()
{
    Serial.println();
    Serial.println(
        "Synchronizing clock..."
    );

    configTime(
        0,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );

    time_t now = time(nullptr);

    int attempts = 0;

    while (
        now < 1700000000 &&
        attempts < 30
    )
    {
        delay(500);

        Serial.print(".");

        now = time(nullptr);

        attempts++;
    }

    Serial.println();

    if (now >= 1700000000)
    {
        Serial.println(
            "Clock synchronized"
        );

        Serial.println(
            getTimestamp()
        );
    }
    else
    {
        Serial.println(
            "WARNING: Clock synchronization failed"
        );
    }
}

// ============================================================
// CONNECT TO WSS SERVER
// ============================================================

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
}

// ============================================================
// VIDEO CONFIGURATION
// ============================================================

// Set to false while the camera is not physically installed.
#define CAMERA_ENABLED false

// Available sources
enum VideoSource {
  VIDEO_PLACEHOLDER,
  VIDEO_CAMERA
};

// This is the source currently being used.
// With CAMERA_ENABLED=false this will remain PLACEHOLDER.
VideoSource videoSource = VIDEO_PLACEHOLDER;

// Target frame interval
const uint32_t VIDEO_FRAME_INTERVAL_MS = 500;   // 2 FPS

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println(
        "========================================"
    );

    Serial.println(
        "PARKING LOT IoT SLAVE"
    );

    Serial.println(
        "ESP32-CAM"
    );

    Serial.println(
        "========================================"
    );

    Serial.print(
        "Device ID: "
    );

    Serial.println(
        DEVICE_ID
    );

    // --------------------------------------------------------
    // IR SENSOR
    // --------------------------------------------------------

    pinMode(
        IR_SENSOR_PIN,
        INPUT
    );

    Serial.print(
        "IR sensor GPIO: "
    );

    Serial.println(
        IR_SENSOR_PIN
    );

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    connectWiFi();

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    synchronizeClock();

    // --------------------------------------------------------
    // WSS
    // --------------------------------------------------------

    connectWebSocket();

    Serial.println();
    Serial.println(
        "Slave initialized."
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // WebSocket processing
    // --------------------------------------------------------

    webSocket.loop();

    // --------------------------------------------------------
    // Sensor sampling
    // --------------------------------------------------------

    if (
        millis() - lastSensorSample >=
        SENSOR_SAMPLE_INTERVAL
    )
    {
        lastSensorSample = millis();

        int currentState =
            digitalRead(
                IR_SENSOR_PIN
            );

        // Send only when state changes
        if (
            currentState !=
            lastSensorState
        )
        {
            lastSensorState =
                currentState;

            Serial.print(
                "IR state changed: "
            );

            Serial.println(
                currentState
            );

            sendSensorEvent(
                currentState
            );
        }
    }

    // --------------------------------------------------------
    // Periodic status
    // --------------------------------------------------------

    if (
        millis() - lastStatusMessage >=
        STATUS_INTERVAL
    )
    {
        lastStatusMessage =
            millis();

        sendStatus();
    }
}