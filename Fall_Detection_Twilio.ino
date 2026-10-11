
#include <Wire.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <base64.h>

// ==========================================
// 1. WIFI CONFIGURATION
// ==========================================

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ==========================================
// 2. TWILIO CONFIGURATION
// ==========================================
// Enter your own details locally.
// Use the sender number assigned to your
// verified recipient in Twilio Console.

const char* accountSID = "YOUR_ACCOUNT_SID";
const char* authToken = "YOUR_AUTH_TOKEN";

const char* twilioNumber = "+1XXXXXXXXXX";
const char* receiverNumber = "+91XXXXXXXXXX";

const char* host = "api.twilio.com";

// Twilio free-trial predefined SMS template
const char* smsTemplate = "sms_internal_alerts";

// ==========================================
// 3. MPU6050 CONFIGURATION
// ==========================================

const uint8_t MPU_ADDR = 0x68;

int16_t rawAx, rawAy, rawAz;
int16_t rawGx, rawGy, rawGz;

float ax, ay, az;
float gx, gy, gz;

float totalAccel = 0;
float totalGyro = 0;

// Fall detection thresholds
const float FREE_FALL_THRESHOLD = 0.55;
const float IMPACT_THRESHOLD = 2.0;
const float ROTATION_THRESHOLD = 150.0;

const float STILL_GYRO_THRESHOLD = 25.0;
const float STILL_ACCEL_MIN = 0.80;
const float STILL_ACCEL_MAX = 1.20;

// ==========================================
// 4. FALL DETECTION VARIABLES
// ==========================================

bool possibleFall = false;
bool impactDetected = false;
bool rotationDetected = false;

unsigned long freeFallTime = 0;
unsigned long impactTime = 0;
unsigned long rotationTime = 0;

// Prevent repeated SMS requests
const unsigned long SMS_COOLDOWN = 60000;

unsigned long lastSMSTime = 0;
bool smsAttempted = false;

// ==========================================
// 5. URL ENCODING
// ==========================================

String urlEncode(const String& text) {

  String encoded = "";
  const char* hex = "0123456789ABCDEF";

  for (unsigned int i = 0; i < text.length(); i++) {

    unsigned char c = text[i];

    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') ||
        c == '-' || c == '_' ||
        c == '.' || c == '~') {

      encoded += (char)c;

    } else {

      encoded += '%';
      encoded += hex[c >> 4];
      encoded += hex[c & 15];
    }
  }

  return encoded;
}

// ==========================================
// 6. CONNECT WIFI
// ==========================================

bool connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 15000) {

    Serial.print(".");
    delay(500);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WIFI CONNECTED!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    return true;
  }

  Serial.println("WIFI CONNECTION FAILED!");
  return false;
}

// ==========================================
// 7. SEND TWILIO SMS
// ==========================================

bool sendTwilioSMS() {

  Serial.println();
  Serial.println("============================");
  Serial.println("SENDING FALL TRIGGER SMS");
  Serial.println("============================");

  if (!connectWiFi()) {

    Serial.println("SMS FAILED: No WiFi.");
    return false;
  }

  WiFiClientSecure client;

  // For college prototype testing only.
  // Production should verify TLS certificates.
  client.setInsecure();
  client.setTimeout(15000);

  Serial.println("Connecting to Twilio...");

  if (!client.connect(host, 443)) {

    Serial.println("Twilio connection failed!");
    return false;
  }

  Serial.println("Connected to Twilio!");

  String postData =
    "To=" + urlEncode(String(receiverNumber)) +
    "&From=" + urlEncode(String(twilioNumber)) +
    "&Body=" + urlEncode(String(smsTemplate));

  String credentials =
    String(accountSID) + ":" + String(authToken);

  String authorization =
    base64::encode(credentials);

  String path =
    "/2010-04-01/Accounts/" +
    String(accountSID) +
    "/Messages.json";

  client.print("POST " + path + " HTTP/1.1\r\n");

  client.print("Host: " + String(host) + "\r\n");

  client.print("Authorization: Basic " +
               authorization + "\r\n");

  client.print(
    "Content-Type: application/x-www-form-urlencoded\r\n"
  );

  client.print(
    "Content-Length: " +
    String(postData.length()) + "\r\n"
  );

  client.print("Connection: close\r\n\r\n");
  client.print(postData);

  Serial.println("SMS request sent.");
  Serial.println("Waiting for response...");

  unsigned long start = millis();

  while (!client.available() &&
         millis() - start < 15000) {

    delay(50);
  }

  if (!client.available()) {

    Serial.println("No Twilio response.");
    client.stop();
    return false;
  }

  String statusLine =
    client.readStringUntil('\n');

  statusLine.trim();

  Serial.print("Twilio HTTP response: ");
  Serial.println(statusLine);

  bool accepted =
    statusLine.indexOf(" 201 ") >= 0;

  // Skip response headers
  while (client.connected() || client.available()) {

    String header = client.readStringUntil('\n');
    header.trim();

    if (header.length() == 0) {
      break;
    }
  }

  // Read response body for diagnostics
  String responseBody = "";
  unsigned long lastData = millis();

  while (client.connected() || client.available()) {

    while (client.available()) {

      responseBody += (char)client.read();
      lastData = millis();
    }

    if (millis() - lastData > 2000) {
      break;
    }

    delay(10);
  }

  client.stop();

  if (accepted) {

    Serial.println();
    Serial.println("****************************");
    Serial.println("SMS ACCEPTED BY TWILIO!");
    Serial.println("****************************");
    Serial.println("Check phone for delivery.");

    return true;

  } else {

    Serial.println();
    Serial.println("TWILIO SMS REQUEST FAILED!");
    Serial.println("Error details:");
    Serial.println(responseBody);

    return false;
  }
}

// ==========================================
// 8. READ MPU6050
// ==========================================

bool readMPU6050() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  Wire.requestFrom(
    (uint8_t)MPU_ADDR,
    (size_t)14,
    true
  );

  if (Wire.available() < 14) {
    return false;
  }

  rawAx = (Wire.read() << 8) | Wire.read();
  rawAy = (Wire.read() << 8) | Wire.read();
  rawAz = (Wire.read() << 8) | Wire.read();

  // Skip temperature readings
  Wire.read();
  Wire.read();

  rawGx = (Wire.read() << 8) | Wire.read();
  rawGy = (Wire.read() << 8) | Wire.read();
  rawGz = (Wire.read() << 8) | Wire.read();

  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  gx = rawGx / 131.0;
  gy = rawGy / 131.0;
  gz = rawGz / 131.0;

  totalAccel =
    sqrt(ax * ax + ay * ay + az * az);

  totalGyro =
    sqrt(gx * gx + gy * gy + gz * gz);

  return true;
}

// ==========================================
// 9. RESET FALL DETECTION
// ==========================================

void resetFallDetection() {

  possibleFall = false;
  impactDetected = false;
  rotationDetected = false;

  freeFallTime = 0;
  impactTime = 0;
  rotationTime = 0;
}

// ==========================================
// 10. SETUP
// ==========================================

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("============================");
  Serial.println(" FALL DETECTION + TWILIO");
  Serial.println("============================");

  // MPU6050 wiring:
  // SDA -> D2
  // SCL -> D1
  // VCC -> 3.3V
  // GND -> GND

  Wire.begin(D2, D1);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);

  byte status = Wire.endTransmission(true);

  if (status == 0) {

    Serial.println("MPU6050 CONNECTED!");

  } else {

    Serial.println("MPU6050 CONNECTION FAILED!");
  }

  delay(1000);

  connectWiFi();

  Serial.println();
  Serial.println("SYSTEM READY!");
  Serial.println("Monitoring for falls...");
  Serial.println();
}

// ==========================================
// 11. MAIN LOOP
// ==========================================

void loop() {

  if (!readMPU6050()) {

    Serial.println("MPU6050 READ FAILED!");
    delay(500);
    return;
  }

  Serial.print("Accel=");
  Serial.print(totalAccel, 2);

  Serial.print("g | Rotation=");
  Serial.print(totalGyro, 1);

  Serial.print(" deg/s | ");

  // STEP 1: POSSIBLE FREE FALL

  if (!possibleFall &&
      totalAccel < FREE_FALL_THRESHOLD) {

    possibleFall = true;
    freeFallTime = millis();

    Serial.println("POSSIBLE FALL");

    delay(50);
    return;
  }

  // STEP 2: IMPACT DETECTION

  if (possibleFall && !impactDetected) {

    if (totalAccel > IMPACT_THRESHOLD) {

      impactDetected = true;
      impactTime = millis();

      Serial.println("IMPACT DETECTED!");

      delay(50);
      return;
    }

    if (millis() - freeFallTime > 2000) {

      Serial.println("NO IMPACT - FALSE ALARM");

      resetFallDetection();
      delay(100);
      return;
    }

    Serial.println("WAITING FOR IMPACT...");
    delay(50);
    return;
  }

  // STEP 3: LARGE ROTATION

  if (impactDetected && !rotationDetected) {

    if (totalGyro > ROTATION_THRESHOLD) {

      rotationDetected = true;
      rotationTime = millis();

      Serial.println("LARGE ROTATION DETECTED!");
      Serial.println("Checking inactivity...");

      delay(50);
      return;
    }

    if (millis() - impactTime > 2000) {

      Serial.println("NO LARGE ROTATION - FALSE ALARM");

      resetFallDetection();
      delay(100);
      return;
    }

    Serial.println("CHECKING ROTATION...");
    delay(50);
    return;
  }

  // STEP 4: INACTIVITY CHECK

  if (rotationDetected) {

    if (millis() - rotationTime < 3000) {

      Serial.println("WAITING FOR INACTIVITY...");
      delay(100);
      return;
    }

    bool accelStable =
      totalAccel > STILL_ACCEL_MIN &&
      totalAccel < STILL_ACCEL_MAX;

    bool gyroStable =
      totalGyro < STILL_GYRO_THRESHOLD;

    if (accelStable && gyroStable) {

      Serial.println();
      Serial.println("****************************");
      Serial.println("      CONFIRMED FALL!");
      Serial.println("****************************");

      Serial.println("Inactivity detected.");

      // Send at most one request per
      // cooldown period.

      unsigned long now = millis();

      if (!smsAttempted ||
          now - lastSMSTime >= SMS_COOLDOWN) {

        // Mark attempt before networking
        // to avoid immediate duplicates.
        lastSMSTime = now;
        smsAttempted = true;

        bool sent = sendTwilioSMS();

        if (sent) {
          Serial.println("SMS REQUEST SUCCESSFUL!");
        } else {
          Serial.println("SMS REQUEST FAILED!");
        }

      } else {

        Serial.println(
          "SMS cooldown active. No duplicate SMS."
        );
      }

      delay(1000);

    } else {

      Serial.println();
      Serial.println("MOVEMENT DETECTED AFTER FALL");
      Serial.println("FALSE ALARM / PERSON MOVING");
    }

    resetFallDetection();
    delay(500);
    return;
  }

  Serial.println("NORMAL");
  delay(100);
}

