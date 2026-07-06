#include "arduino_secrets.h"
#include "thingProperties.h"

#include <Wire.h>
#include "RTClib.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// ==================================================
// FINAL PHASE - CLOUD + CAMERA + TELEGRAM + GOOGLE SCRIPT
// Board: Freenove ESP32-S3 WROOM + Camera OV3660
// Scope: hardware + security logic + Arduino Cloud variables + camera capture + Telegram + Google Apps Script
// BLE scanning remains demo/controlled-input based in this build
// ==================================================


// =======================
// EXTERNAL API FALLBACKS
// Put real values in arduino_secrets.h. Do not hardcode secrets in this file.
// =======================
#ifndef SECRET_GOOGLE_SCRIPT_URL
#define SECRET_GOOGLE_SCRIPT_URL ""
#endif

#ifndef SECRET_TELEGRAM_BOT_TOKEN
#define SECRET_TELEGRAM_BOT_TOKEN ""
#endif

#ifndef SECRET_TELEGRAM_CHAT_ID
#define SECRET_TELEGRAM_CHAT_ID ""
#endif

#ifndef SECRET_DEVICE_NAME
#define SECRET_DEVICE_NAME "ESP32-S3 Kitchen Security"
#endif

#ifndef SECRET_DEVICE_LOCATION
#define SECRET_DEVICE_LOCATION "Kitchen"
#endif

// =======================
// CAMERA-COMPATIBLE PIN MAP
// =======================
#define PIN_LDR_AO       1
#define PIN_PIR_OUT      40

#define PIN_RTC_SDA      41
#define PIN_RTC_SCL      42

#define PIN_LED_RED      14
#define PIN_LED_GREEN    21
#define PIN_BUZZER       47

#define PIN_US_TRIG      38
#define PIN_US_ECHO      39

// =======================
// FRENNOVE ESP32-S3 WROOM + OV3660 CAMERA PIN MAP
// Do not use these GPIOs for external modules.
// =======================
#define CAM_PWDN_GPIO_NUM    -1
#define CAM_RESET_GPIO_NUM   -1

#define CAM_XCLK_GPIO_NUM    15
#define CAM_SIOD_GPIO_NUM    4
#define CAM_SIOC_GPIO_NUM    5

#define CAM_Y2_GPIO_NUM      11
#define CAM_Y3_GPIO_NUM      9
#define CAM_Y4_GPIO_NUM      8
#define CAM_Y5_GPIO_NUM      10
#define CAM_Y6_GPIO_NUM      12
#define CAM_Y7_GPIO_NUM      18
#define CAM_Y8_GPIO_NUM      17
#define CAM_Y9_GPIO_NUM      16

#define CAM_VSYNC_GPIO_NUM   6
#define CAM_HREF_GPIO_NUM    7
#define CAM_PCLK_GPIO_NUM    13

#define FORCE_SET_RTC_TIME_ONCE false

// =======================
// BUZZER CONFIG
// =======================
// false = active buzzer, HIGH/LOW is enough.
// true  = passive buzzer, tone() is required.
const bool BUZZER_USE_TONE = false;
const int BUZZER_TONE_HZ = 2500;

// =======================
// SECURITY CONFIG
// =======================
const float OBJECT_NEAR_THRESHOLD_CM = 50.0;
const float OBJECT_TOO_CLOSE_THRESHOLD_CM = 8.0;

// If covering the LDR makes the analog value go high, keep true.
// If covering the LDR makes the analog value go low, change this to false.
const bool LDR_COVER_WHEN_HIGH = true;
const int LDR_COVERED_THRESHOLD_HIGH = 3800;
const int LDR_COVERED_THRESHOLD_LOW = 500;

const int LDR_DELTA_ABNORMAL_THRESHOLD = 500;
const unsigned long SABOTAGE_HOLD_MS = 5000;

const unsigned long SENSOR_UPDATE_INTERVAL_MS = 500;
const unsigned long SERIAL_REPORT_INTERVAL_MS = 2000;
const unsigned long RED_BLINK_INTERVAL_MS = 250;

RTC_DS1307 rtc;
bool rtcOk = false;
bool cameraReady = false;
unsigned long lastCameraCaptureMs = 0;

int lastLdrValue = -1;
unsigned long sabotageConditionStartedAt = 0;

bool sosActive = false;
String sosSource = "NONE";

int rawIntrusionScore = 0;
int lastIntrusionScoreAtTrigger = 0;
int lastRawIntrusionScoreAtTrigger = 0;
String lastIntrusionReason = "NONE";

String lastScheduleTriggerKey = "";

unsigned long lastGoogleScriptCallMs = 0;
String lastGoogleScriptEventKey = "";

struct HardwareSnapshot {
  int ldrValue;
  int ldrDelta;
  bool pirDetected;
  float distanceCm;
  bool ultrasonicOk;
  bool objectNear;
  bool objectTooClose;
  bool lightAbnormal;
  bool ldrCovered;
  bool rtcOk;
  int hour;
  int minute;
  bool nightMode;
  String timeText;
};


// ==================================================
// FORWARD DECLARATIONS
// These make the sketch safe even if Arduino preprocessing changes.
// ==================================================
String twoDigits(int value);
String getRtcTimeString();
void setLastEvent(const String &type, const String &message);
void incrementEventCounter();
String urlEncode(const String &value);
bool hasTelegramConfig();
bool hasGoogleScriptConfig();
String buildCommonCaption(const String &eventType, const String &reason);
bool sendTelegramMessage(const String &message);
bool sendTelegramPhoto(camera_fb_t *fb, const String &caption);
bool performGetRequest(String url, String &outResponse, int &outCode, int redirectDepth = 0);
bool callGoogleAppsScript(const String &eventType, const String &source, const String &message);
bool isTelegramAllowed(const String &eventType);
void notifySecurityTextEvent(const String &eventType, const String &source, const String &message);
void updateTrustedDeviceStatusPlaceholders();
void resetTrustedBleRuntimeState();
void updateGoogleScriptStatusFromResponse(const String &response, bool ok);

// ==================================================
// CAMERA FUNCTIONS
// ==================================================

bool initCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = CAM_Y2_GPIO_NUM;
  config.pin_d1 = CAM_Y3_GPIO_NUM;
  config.pin_d2 = CAM_Y4_GPIO_NUM;
  config.pin_d3 = CAM_Y5_GPIO_NUM;
  config.pin_d4 = CAM_Y6_GPIO_NUM;
  config.pin_d5 = CAM_Y7_GPIO_NUM;
  config.pin_d6 = CAM_Y8_GPIO_NUM;
  config.pin_d7 = CAM_Y9_GPIO_NUM;

  config.pin_xclk = CAM_XCLK_GPIO_NUM;
  config.pin_pclk = CAM_PCLK_GPIO_NUM;
  config.pin_vsync = CAM_VSYNC_GPIO_NUM;
  config.pin_href = CAM_HREF_GPIO_NUM;

  config.pin_sccb_sda = CAM_SIOD_GPIO_NUM;
  config.pin_sccb_scl = CAM_SIOC_GPIO_NUM;

  config.pin_pwdn = CAM_PWDN_GPIO_NUM;
  config.pin_reset = CAM_RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    Serial.println("[CAM] PSRAM found.");
    config.frame_size = FRAMESIZE_QVGA;      // 320x240
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("[CAM] PSRAM NOT found. Using QQVGA.");
    config.frame_size = FRAMESIZE_QQVGA;     // 160x120
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.print("[CAM] Camera init failed. Error code: 0x");
    Serial.println((uint32_t)err, HEX);
    photo_status = "INIT_FAILED";
    return false;
  }

  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor == NULL) {
    Serial.println("[CAM] Sensor pointer is NULL.");
    photo_status = "SENSOR_NULL";
    return false;
  }

  Serial.print("[CAM] Sensor PID: 0x");
  Serial.println(sensor->id.PID, HEX);

  if (sensor->id.PID == OV3660_PID) {
    Serial.println("[CAM] OV3660 detected.");
    sensor->set_hmirror(sensor, 1);
    sensor->set_vflip(sensor, 0);
    sensor->set_brightness(sensor, 1);
    sensor->set_saturation(sensor, 0);
    sensor->set_ae_level(sensor, -3);
  } else {
    Serial.println("[CAM] Camera detected, but PID is not OV3660.");
  }

  photo_status = "IDLE";
  return true;
}

bool captureSecurityPhoto(const String &reason) {
  if (!cameraReady) {
    photo_status = "CAMERA_NOT_READY";
    setLastEvent("photo_capture_failed", "Camera not ready for " + reason + " at " + getRtcTimeString());
    Serial.println("[CAM] Capture skipped: camera not ready.");
    return false;
  }

  // Avoid accidental repeated captures too fast (hardware cooldown)
  if (millis() - lastCameraCaptureMs < 1500) {
    photo_status = "COOLDOWN";
    Serial.println("[CAM] Capture skipped: short camera cooldown.");
    return false;
  }

  lastCameraCaptureMs = millis();
  photo_status = "CAPTURING";

  Serial.println();
  Serial.println("========== SECURITY PHOTO CAPTURE ==========");
  Serial.print("[CAM] Reason: ");
  Serial.println(reason);

  camera_fb_t *fb = esp_camera_fb_get();

  if (fb == NULL) {
    photo_status = "FAILED";
    notification_sent_status = "PHOTO_FAILED";
    setLastEvent("photo_capture_failed", "Photo capture failed for " + reason + " at " + getRtcTimeString());
    Serial.println("[CAM] Capture failed: framebuffer is NULL.");
    Serial.println("============================================");
    return false;
  }

  Serial.println("[CAM] Capture OK.");
  Serial.print("[CAM] Width      : ");
  Serial.println(fb->width);
  Serial.print("[CAM] Height     : ");
  Serial.println(fb->height);
  Serial.print("[CAM] Format     : ");
  Serial.println(fb->format);
  Serial.print("[CAM] Image bytes: ");
  Serial.println(fb->len);

  bool shouldSendToTelegram =
    reason == "MANUAL_DASHBOARD" ||
    reason == "AUTO_INTRUSION_ALERT";

  bool telegramPhotoSent = false;

  if (shouldSendToTelegram) {
    if (!isTelegramAllowed(reason)) {
      photo_status = "TG_COOLDOWN";
      notification_sent_status = "COOLDOWN";
      Serial.println("[TG] Skipped photo notification by cooldown.");
      esp_camera_fb_return(fb);
      return false;
    }

    notification_channel = "telegram";
    notification_event_type = reason;
    notification_sent_status = "SENDING_PHOTO";

    String caption = buildCommonCaption("photo_capture", reason);
    telegramPhotoSent = sendTelegramPhoto(fb, caption);
  }

  esp_camera_fb_return(fb);

  if (shouldSendToTelegram) {
    if (telegramPhotoSent) {
      photo_status = "CAPTURED_SENT";
      notification_sent_status = "SENT";
    } else {
      photo_status = "CAPTURED_SEND_FAILED";
      if (notification_sent_status != "NOT_CONFIGURED" &&
          notification_sent_status != "WIFI_NOT_CONNECTED" &&
          notification_sent_status != "CONNECT_FAILED") {
        notification_sent_status = "FAILED";
      }
    }
  } else {
    photo_status = "CAPTURED";
  }

  setLastEvent("photo_captured", "Photo captured for " + reason + " at " + getRtcTimeString());

  Serial.println("[CAM] Frame returned to driver.");
  Serial.println("============================================");

  return true;
}

void processCameraRequests() {
  if (manual_capture_photo) {
    captureSecurityPhoto("MANUAL_DASHBOARD");
    manual_capture_photo = false;
  }

  if (auto_capture_photo_request) {
    captureSecurityPhoto("AUTO_INTRUSION_ALERT");
    auto_capture_photo_request = false;
  }
}

// ==================================================
// BASIC HELPERS
// ==================================================

String twoDigits(int value) {
  if (value < 10) {
    return "0" + String(value);
  }
  return String(value);
}

String getRtcTimeString() {
  if (!rtcOk) {
    return "RTC_NOT_FOUND";
  }

  DateTime now = rtc.now();
  String text = "";
  text += String(now.year());
  text += "-";
  text += twoDigits(now.month());
  text += "-";
  text += twoDigits(now.day());
  text += " ";
  text += twoDigits(now.hour());
  text += ":";
  text += twoDigits(now.minute());
  text += ":";
  text += twoDigits(now.second());
  return text;
}

void setLastEvent(const String &type, const String &message) {
  last_event_type = type;
  last_event = message;
}

void incrementEventCounter() {
  event_counter++;
}


// ==================================================
// EXTERNAL API FUNCTIONS - TELEGRAM + GOOGLE APPS SCRIPT
// ==================================================

String urlEncode(const String &value) {
  String encoded = "";
  const char *hex = "0123456789ABCDEF";

  for (size_t i = 0; i < value.length(); i++) {
    char c = value.charAt(i);

    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += "%20";
    } else {
      encoded += '%';
      encoded += hex[(c >> 4) & 0x0F];
      encoded += hex[c & 0x0F];
    }
  }

  return encoded;
}

bool hasTelegramConfig() {
  return String(SECRET_TELEGRAM_BOT_TOKEN).length() > 10 &&
         String(SECRET_TELEGRAM_CHAT_ID).length() > 0;
}

bool hasGoogleScriptConfig() {
  return String(SECRET_GOOGLE_SCRIPT_URL).startsWith("https://");
}

String buildCommonCaption(const String &eventType, const String &reason) {
  String text = "";
  text += "Device: ";
  text += SECRET_DEVICE_NAME;
  text += "\nLocation: ";
  text += SECRET_DEVICE_LOCATION;
  text += "\nEvent: ";
  text += eventType;
  text += "\nReason: ";
  text += reason;
  text += "\nStatus: ";
  text += alarm_status;
  text += "\nThreat level: ";
  text += String(threat_level);
  text += "\nIntrusion score: ";
  text += String(intrusion_score);
  text += "\nTime: ";
  text += getRtcTimeString();
  text += "\nLast event: ";
  text += last_event_type;
  return text;
}

bool sendTelegramMessage(const String &message) {
  notification_channel = "telegram";

  if (!hasTelegramConfig()) {
    notification_sent_status = "NOT_CONFIGURED";
    Serial.println("[TG] Telegram is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    notification_sent_status = "WIFI_NOT_CONNECTED";
    Serial.println("[TG] WiFi is not connected.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  if (!client.connect("api.telegram.org", 443)) {
    notification_sent_status = "CONNECT_FAILED";
    Serial.println("[TG] Connect failed.");
    return false;
  }

  String body = "chat_id=" + urlEncode(String(SECRET_TELEGRAM_CHAT_ID)) +
                "&text=" + urlEncode(message);

  String path = "/bot" + String(SECRET_TELEGRAM_BOT_TOKEN) + "/sendMessage";

  client.print(String("POST ") + path + " HTTP/1.1\r\n");
  client.print("Host: api.telegram.org\r\n");
  client.print("User-Agent: ESP32-S3-Kitchen-Security\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: ");
  client.print(body.length());
  client.print("\r\n\r\n");
  client.print(body);

  unsigned long start = millis();
  while (client.connected() && !client.available() && millis() - start < 15000) {
    delay(10);
  }

  String statusLine = client.readStringUntil('\n');
  bool ok = statusLine.indexOf("200") >= 0;

  Serial.print("[TG] sendMessage status: ");
  Serial.println(statusLine);

  notification_sent_status = ok ? "SENT" : "FAILED";
  return ok;
}

bool sendTelegramPhoto(camera_fb_t *fb, const String &caption) {
  notification_channel = "telegram";

  if (!hasTelegramConfig()) {
    notification_sent_status = "NOT_CONFIGURED";
    Serial.println("[TG] Telegram is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    notification_sent_status = "WIFI_NOT_CONNECTED";
    Serial.println("[TG] WiFi is not connected.");
    return false;
  }

  if (fb == NULL || fb->buf == NULL || fb->len == 0) {
    notification_sent_status = "PHOTO_EMPTY";
    Serial.println("[TG] Photo buffer is empty.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(20000);

  if (!client.connect("api.telegram.org", 443)) {
    notification_sent_status = "CONNECT_FAILED";
    Serial.println("[TG] Photo connect failed.");
    return false;
  }

  String boundary = "----ESP32S3KitchenSecurityBoundary";

  String head = "";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  head += String(SECRET_TELEGRAM_CHAT_ID) + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  head += caption + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"security.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  size_t contentLength = head.length() + fb->len + tail.length();
  String path = "/bot" + String(SECRET_TELEGRAM_BOT_TOKEN) + "/sendPhoto";

  client.print(String("POST ") + path + " HTTP/1.1\r\n");
  client.print("Host: api.telegram.org\r\n");
  client.print("User-Agent: ESP32-S3-Kitchen-Security\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
  client.print("Content-Length: ");
  client.print(contentLength);
  client.print("\r\n\r\n");

  client.print(head);
  client.write(fb->buf, fb->len);
  client.print(tail);

  unsigned long start = millis();
  while (client.connected() && !client.available() && millis() - start < 20000) {
    delay(10);
  }

  String statusLine = client.readStringUntil('\n');
  bool ok = statusLine.indexOf("200") >= 0;

  Serial.print("[TG] sendPhoto status: ");
  Serial.println(statusLine);

  notification_sent_status = ok ? "SENT" : "FAILED";
  return ok;
}

bool performGetRequest(String url, String &outResponse, int &outCode, int redirectDepth) {
  if (redirectDepth > 4) {
    Serial.println("[HTTP] Max redirects reached.");
    outCode = 508;
    return false;
  }

  if (url.startsWith("https://")) {
    url = url.substring(8);
  } else if (url.startsWith("http://")) {
    url = url.substring(7);
  }

  int slashIndex = url.indexOf('/');
  String host = "";
  String path = "";
  if (slashIndex >= 0) {
    host = url.substring(0, slashIndex);
    path = url.substring(slashIndex);
  } else {
    host = url;
    path = "/";
  }

  int port = 443;
  int colonIndex = host.indexOf(':');
  if (colonIndex >= 0) {
    port = host.substring(colonIndex + 1).toInt();
    host = host.substring(0, colonIndex);
  }

  Serial.print("[HTTP] Connecting to: ");
  Serial.print(host);
  Serial.println(path);

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  if (!client.connect(host.c_str(), port)) {
    Serial.println("[HTTP] Connection failed.");
    outCode = 0;
    return false;
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n");
  client.print("Host: " + host + "\r\n");
  client.print("User-Agent: ESP32-S3-Kitchen-Security\r\n");
  client.print("Connection: close\r\n\r\n");

  unsigned long start = millis();
  while (client.connected() && !client.available() && millis() - start < 15000) {
    delay(10);
  }

  if (!client.available()) {
    Serial.println("[HTTP] No response received.");
    outCode = 0;
    client.stop();
    return false;
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  int firstSpace = statusLine.indexOf(' ');
  int secondSpace = statusLine.indexOf(' ', firstSpace + 1);
  int httpCode = 0;
  if (firstSpace >= 0 && secondSpace >= 0) {
    httpCode = statusLine.substring(firstSpace + 1, secondSpace).toInt();
  } else if (firstSpace >= 0) {
    httpCode = statusLine.substring(firstSpace + 1).toInt();
  }
  outCode = httpCode;

  String location = "";
  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      break;
    }
    if (line.startsWith("Location:") || line.startsWith("location:")) {
      location = line.substring(9);
      location.trim();
    }
  }

  String body = "";
  while (client.available()) {
    body += client.readString();
  }
  client.stop();
  outResponse = body;

  if (httpCode == 301 || httpCode == 302 || httpCode == 303 || httpCode == 307 || httpCode == 308) {
    if (location.length() > 0) {
      Serial.println("[HTTP] Redirecting to: " + location);
      return performGetRequest(location, outResponse, outCode, redirectDepth + 1);
    }
  }

  return (httpCode >= 200 && httpCode < 300);
}

void updateGoogleScriptStatusFromResponse(const String &response, bool ok) {
  if (!ok) {
    emergency_escalation_status = "FAILED";
    emergency_authority_message_status = "FAILED";
    return;
  }

  // Apps Script returns compact text such as:
  // OK:WAITING_CONFIRMATION;eventId=...;emergency_escalation_status=WAITING_CONFIRMATION;...
  // This parser is defensive: it only updates values when the response contains them.
  if (response.indexOf("home_address_configured=true") >= 0) {
    home_address_configured = true;
  } else if (response.indexOf("home_address_configured=false") >= 0) {
    home_address_configured = false;
  }

  if (response.indexOf("emergency_escalation_status=WAITING_CONFIRMATION") >= 0 ||
      response.indexOf("OK:WAITING_CONFIRMATION") >= 0) {
    emergency_escalation_status = "WAITING_CONFIRMATION";
  } else if (response.indexOf("emergency_escalation_status=CONFIRMED") >= 0) {
    emergency_escalation_status = "CONFIRMED";
  } else if (response.indexOf("emergency_escalation_status=SENT") >= 0) {
    emergency_escalation_status = "SENT";
  } else if (response.indexOf("emergency_escalation_status=FAILED") >= 0) {
    emergency_escalation_status = "FAILED";
  } else if (response.indexOf("emergency_escalation_status=NOT_CONFIGURED") >= 0) {
    emergency_escalation_status = "NOT_CONFIGURED";
  } else {
    // Backward-compatible fallback for older Apps Script responses.
    emergency_escalation_status = "SENT";
  }

  if (response.indexOf("emergency_authority_message_status=IDLE") >= 0) {
    emergency_authority_message_status = "IDLE";
  } else if (response.indexOf("emergency_authority_message_status=READY") >= 0) {
    emergency_authority_message_status = "READY";
  } else if (response.indexOf("emergency_authority_message_status=SENT") >= 0) {
    emergency_authority_message_status = "SENT";
  } else if (response.indexOf("emergency_authority_message_status=FAILED") >= 0) {
    emergency_authority_message_status = "FAILED";
  } else if (response.indexOf("emergency_authority_message_status=NOT_CONFIGURED") >= 0) {
    emergency_authority_message_status = "NOT_CONFIGURED";
  }
}

bool callGoogleAppsScript(const String &eventType, const String &source, const String &message) {
  if (!hasGoogleScriptConfig()) {
    emergency_escalation_status = "NOT_CONFIGURED";
    emergency_authority_message_status = "NOT_CONFIGURED";
    Serial.println("[GAS] Google Apps Script is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    emergency_escalation_status = "WIFI_NOT_CONNECTED";
    emergency_authority_message_status = "FAILED";
    Serial.println("[GAS] WiFi is not connected.");
    return false;
  }

  String eventKey = eventType + ":" + source;
  if (lastGoogleScriptEventKey == eventKey && millis() - lastGoogleScriptCallMs < 15000) {
    Serial.println("[GAS] Skipped by cooldown.");
    return false;
  }

  lastGoogleScriptEventKey = eventKey;
  lastGoogleScriptCallMs = millis();

  String url = String(SECRET_GOOGLE_SCRIPT_URL);
  url += "?event=" + urlEncode(eventType);
  url += "&source=" + urlEncode(source);
  url += "&device=" + urlEncode(String(SECRET_DEVICE_NAME));
  url += "&location=" + urlEncode(String(SECRET_DEVICE_LOCATION));
  url += "&status=" + urlEncode(alarm_status);
  url += "&threat=" + urlEncode(String(threat_level));
  url += "&score=" + urlEncode(String(intrusion_score));
  url += "&time=" + urlEncode(getRtcTimeString());
  url += "&message=" + urlEncode(message);
  if (sos_authority_note.length() > 0) {
    url += "&sos_authority_note=" + urlEncode(sos_authority_note);
  }

  emergency_escalation_status = "SENDING";
  emergency_authority_message_status = "IDLE";

  String response = "";
  int code = 0;
  bool ok = performGetRequest(url, response, code);

  Serial.print("[GAS] HTTP code: ");
  Serial.println(code);
  if (response.length() > 0) {
    Serial.print("[GAS] Response: ");
    Serial.println(response.substring(0, 200));
  }

  updateGoogleScriptStatusFromResponse(response, ok);
  return ok;
}

bool isTelegramAllowed(const String &eventType) {
  static unsigned long lastTelegramIntrusionAlertMs = 0;
  static unsigned long lastTelegramSabotageAlertMs = 0;
  static unsigned long lastTelegramSosAlertMs = 0;
  static unsigned long lastTelegramManualCaptureMs = 0;

  unsigned long nowMs = millis();
  String eventLower = eventType;
  eventLower.toLowerCase();

  if (eventLower.indexOf("intrusion") >= 0) {
    if (lastTelegramIntrusionAlertMs != 0 && nowMs - lastTelegramIntrusionAlertMs < 15000) return false;
    lastTelegramIntrusionAlertMs = nowMs;
  } else if (eventLower.indexOf("sabotage") >= 0) {
    if (lastTelegramSabotageAlertMs != 0 && nowMs - lastTelegramSabotageAlertMs < 15000) return false;
    lastTelegramSabotageAlertMs = nowMs;
  } else if (eventLower.indexOf("sos") >= 0) {
    if (lastTelegramSosAlertMs != 0 && nowMs - lastTelegramSosAlertMs < 15000) return false;
    lastTelegramSosAlertMs = nowMs;
  } else if (eventLower.indexOf("manual") >= 0) {
    if (lastTelegramManualCaptureMs != 0 && nowMs - lastTelegramManualCaptureMs < 15000) return false;
    lastTelegramManualCaptureMs = nowMs;
  }
  return true;
}

void notifySecurityTextEvent(const String &eventType, const String &source, const String &message) {
  if (!isTelegramAllowed(eventType)) {
    notification_sent_status = "COOLDOWN";
    Serial.println("[TG] Skipped text notification by cooldown.");
    return;
  }

  send_notification_request = true;
  notification_event_type = eventType;
  notification_channel = "telegram";

  String text = buildCommonCaption(eventType, source);
  text += "\nMessage: ";
  text += message;

  notification_sent_status = "SENDING";
  bool sent = sendTelegramMessage(text);
  notification_sent_status = sent ? "SENT" : "FAILED";
}

// ==================================================
// OUTPUT CONTROL
// ==================================================

void setRedLedPhysical(bool on) {
  digitalWrite(PIN_LED_RED, on ? HIGH : LOW);
}

void setGreenLedPhysical(bool on) {
  digitalWrite(PIN_LED_GREEN, on ? HIGH : LOW);
}

void setBuzzerPhysical(bool on) {
  if (BUZZER_USE_TONE) {
    if (on) {
      tone(PIN_BUZZER, BUZZER_TONE_HZ);
    } else {
      noTone(PIN_BUZZER);
    }
  } else {
    digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
  }
}

void applyAlarmOutputs() {
  bool anyAlarmActive = sosActive || sabotage_alert || intrusion_alert;

  buzzer_on = anyAlarmActive;
  led_red_on = anyAlarmActive;
  led_green_on = !anyAlarmActive;

  setGreenLedPhysical(led_green_on);
  setBuzzerPhysical(buzzer_on);

  if (led_red_on) {
    static unsigned long lastBlink = 0;
    static bool redState = false;

    if (millis() - lastBlink >= RED_BLINK_INTERVAL_MS) {
      lastBlink = millis();
      redState = !redState;
      setRedLedPhysical(redState);
    }
  } else {
    setRedLedPhysical(false);
  }
}

// ==================================================
// SENSOR READERS
// ==================================================

int readLdrValue() {
  return analogRead(PIN_LDR_AO);
}

int calculateLdrDelta(int currentValue) {
  if (lastLdrValue < 0) {
    lastLdrValue = currentValue;
    return 0;
  }

  int delta = abs(currentValue - lastLdrValue);
  lastLdrValue = currentValue;
  return delta;
}

bool readPirDetected() {
  return digitalRead(PIN_PIR_OUT) == HIGH;
}

float readUltrasonicDistanceCmOnce() {
  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_US_ECHO, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

float readUltrasonicDistanceCm() {
  float a = readUltrasonicDistanceCmOnce();
  delay(20);
  float b = readUltrasonicDistanceCmOnce();
  delay(20);
  float c = readUltrasonicDistanceCmOnce();

  float sum = 0.0;
  int count = 0;

  if (a > 0) {
    sum += a;
    count++;
  }
  if (b > 0) {
    sum += b;
    count++;
  }
  if (c > 0) {
    sum += c;
    count++;
  }

  if (count == 0) {
    return -1.0;
  }

  return sum / count;
}

bool calculateLdrCovered(int value) {
  if (LDR_COVER_WHEN_HIGH) {
    return value >= LDR_COVERED_THRESHOLD_HIGH;
  }
  return value <= LDR_COVERED_THRESHOLD_LOW;
}

bool calculateNightMode(int hour) {
  if (hour < 0) {
    return false;
  }
  return hour >= 22 || hour < 6;
}

HardwareSnapshot readHardwareSnapshot() {
  HardwareSnapshot s;

  s.ldrValue = readLdrValue();
  s.ldrDelta = calculateLdrDelta(s.ldrValue);

  s.pirDetected = readPirDetected();

  s.distanceCm = readUltrasonicDistanceCm();
  s.ultrasonicOk = s.distanceCm > 0;
  s.objectNear = s.ultrasonicOk && s.distanceCm <= OBJECT_NEAR_THRESHOLD_CM;
  s.objectTooClose = s.ultrasonicOk && s.distanceCm <= OBJECT_TOO_CLOSE_THRESHOLD_CM;

  s.lightAbnormal = s.ldrDelta >= LDR_DELTA_ABNORMAL_THRESHOLD;
  s.ldrCovered = calculateLdrCovered(s.ldrValue);

  s.rtcOk = rtcOk;
  s.hour = -1;
  s.minute = -1;
  s.timeText = getRtcTimeString();

  if (rtcOk) {
    DateTime now = rtc.now();
    s.hour = now.hour();
    s.minute = now.minute();
  }

  s.nightMode = calculateNightMode(s.hour);

  return s;
}

void publishHardwareSnapshotToCloud(const HardwareSnapshot &s) {
  ldr_value = s.ldrValue;
  ldr_delta = s.ldrDelta;
  pir_detected = s.pirDetected;
  ultrasonic_distance = s.ultrasonicOk ? s.distanceCm : -1.0;
  object_near = s.objectNear;
  light_abnormal = s.lightAbnormal;
  ldr_covered = s.ldrCovered;
  current_time = s.timeText;
  current_hour = s.hour;
  night_mode = s.nightMode;
  device_tampered = s.ldrCovered || s.objectTooClose;
}

// ==================================================
// SECURITY LOGIC
// ==================================================

int getIntrusionThreshold() {
  if (sensitivity_level == 1) return 5;
  if (sensitivity_level == 2) return 4;
  if (sensitivity_level == 3) return 3;
  return 3;
}

void clampCloudConfigValues() {
  if (sensitivity_level < 1 || sensitivity_level > 3) {
    sensitivity_level = 3;
  }

  if (rearm_delay_seconds < 0) {
    rearm_delay_seconds = 0;
  }

  if (auto_arm_hour < 0 || auto_arm_hour > 23) {
    auto_arm_hour = 22;
  }
  if (auto_arm_minute < 0 || auto_arm_minute > 59) {
    auto_arm_minute = 0;
  }
  if (auto_disarm_hour < 0 || auto_disarm_hour > 23) {
    auto_disarm_hour = 6;
  }
  if (auto_disarm_minute < 0 || auto_disarm_minute > 59) {
    auto_disarm_minute = 0;
  }
}

bool calculatePetDetected(const HardwareSnapshot &s) {
  // Demo-level filter only. This is not a confirmed pet classification.
  return s.objectNear && !s.pirDetected && !s.lightAbnormal;
}

int calculateRawIntrusionScore(const HardwareSnapshot &s) {
  int score = 0;

  if (s.pirDetected) {
    score += 2;
  }
  if (s.objectNear) {
    score += 2;
  }
  if (s.lightAbnormal) {
    score += 1;
  }
  if (s.nightMode) {
    score += 1;
  }

  // Tuân thủ luật 3.5 SRS: WiFi/MAC lạ được cộng 1 điểm nếu có cảm biến vật lý nghi ngờ đi kèm
  if (unknown_wifi_alert && (s.pirDetected || s.objectNear || s.lightAbnormal)) {
    score += 1;
  }

  return score;
}

String buildIntrusionReason(const HardwareSnapshot &s) {
  String reason = "";

  if (s.pirDetected) reason += "PIR(+2) ";
  if (s.objectNear) reason += "OBJECT_NEAR(+2) ";
  if (s.lightAbnormal) reason += "LIGHT_ABNORMAL(+1) ";
  if (s.nightMode) reason += "NIGHT_MODE(+1) ";
  if (unknown_wifi_alert) reason += "UNKNOWN_WIFI(+1) ";

  if (reason.length() == 0) {
    reason = "NONE";
  }

  return reason;
}

void resetTrustedBleRuntimeState() {
  trusted_ble_present = false;
  trusted_ble_rssi = 0;
  trusted_ble_last_seen_seconds = -1;
}

void updateTrustedDeviceStatusPlaceholders() {
  // BLE scan is not implemented in this build yet.
  // This keeps the new SRS/Thing variables consistent without changing the old manual demo flow.
  if (!trusted_ble_detection_enabled) {
    resetTrustedBleRuntimeState();
    trusted_device_source = known_device_present ? "DASHBOARD_INPUT" : "NONE";
    return;
  }

  if (trusted_ble_present) {
    trusted_device_source = "BLE";
    known_device_present = true;
    return;
  }

  // When BLE scanning is enabled but no BLE device is currently found, keep the existing
  // Dashboard/Serial controlled input working so old demos are not broken.
  trusted_device_source = known_device_present ? "DASHBOARD_INPUT" : "NONE";
}

void updateSystemArmed() {
  system_armed = alarm_enabled && !known_device_present;
}

void triggerIntrusionAlert(const HardwareSnapshot &s) {
  if (!intrusion_alert) {
    intrusion_alert = true;
    incrementEventCounter();

    lastRawIntrusionScoreAtTrigger = rawIntrusionScore;
    lastIntrusionScoreAtTrigger = intrusion_score;
    lastIntrusionReason = buildIntrusionReason(s);

    setLastEvent(
      "intrusion_alert",
      "Intrusion score=" + String(lastIntrusionScoreAtTrigger) +
      " reason=" + lastIntrusionReason +
      " at " + getRtcTimeString()
    );

    Serial.println();
    Serial.println("[ALERT] INTRUSION TRIGGERED");
    Serial.print("[ALERT] score = ");
    Serial.println(lastIntrusionScoreAtTrigger);
    Serial.print("[ALERT] reason = ");
    Serial.println(lastIntrusionReason);

    auto_capture_photo_request = true;
    photo_status = "AUTO_CAPTURE_REQUESTED";
    send_notification_request = true;
    notification_event_type = "intrusion_alert";
    notification_sent_status = "CAPTURING_PHOTO";
  }
}

void triggerSabotageAlert() {
  if (!sabotage_alert) {
    sabotage_alert = true;
    incrementEventCounter();
    setLastEvent("sabotage_alert", "Sabotage alert at " + getRtcTimeString());
    notifySecurityTextEvent("sabotage_alert", "DEVICE", last_event);
  }
}

void triggerSosAlert(const String &source) {
  if (!sosActive) {
    incrementEventCounter();
  }

  sosActive = true;
  sosSource = source;

  sos_message = "SOS from " + source + " at " + getRtcTimeString();
  emergency_confirmation_requested = true;
  emergency_escalation_status = "WAITING_CONFIRMATION";
  emergency_authority_message_status = "IDLE";

  setLastEvent("sos_alert", sos_message);
  notifySecurityTextEvent("sos_alert", source, sos_message);
  callGoogleAppsScript("sos_alert", source, sos_message);
}

void resetAllAlerts() {
  intrusion_alert = false;
  sabotage_alert = false;
  sosActive = false;
  sosSource = "NONE";
  sos_child = false;
  sos_adult = false;

  emergency_confirmation_requested = false;
  emergency_confirmed = false;
  emergency_escalation_status = "IDLE";
  emergency_authority_message_status = "IDLE";

  auto_capture_photo_request = false;

  send_notification_request = false;
  notification_event_type = "NONE";
  notification_sent_status = "IDLE";

  sabotageConditionStartedAt = 0;

  incrementEventCounter();
  setLastEvent("alarm_reset", "Alarm reset at " + getRtcTimeString());

  setBuzzerPhysical(false);
  setRedLedPhysical(false);
  setGreenLedPhysical(true);
}

void updateSabotageLogic(const HardwareSnapshot &s) {
  bool sabotageCondition = s.ldrCovered || s.objectTooClose;

  if (sabotageCondition) {
    if (sabotageConditionStartedAt == 0) {
      sabotageConditionStartedAt = millis();
    }

    if (!sabotage_alert && millis() - sabotageConditionStartedAt >= SABOTAGE_HOLD_MS) {
      triggerSabotageAlert();
    }
  } else {
    if (!sabotage_alert) {
      sabotageConditionStartedAt = 0;
    }
  }
}

void updateIntrusionLogic(const HardwareSnapshot &s) {
  pet_detected = calculatePetDetected(s);

  rawIntrusionScore = calculateRawIntrusionScore(s);

  // Simplified intrusion logic based strictly on requirements
  intrusion_score = rawIntrusionScore;

  if (system_armed && intrusion_score >= getIntrusionThreshold()) {
    triggerIntrusionAlert(s);
  }

  // Important: intrusion_alert is latched. It does not auto-clear when score drops.
}

void updateAlarmStatus() {
  if (sosActive) {
    alarm_status = "SOS_ALERT";
    return;
  }

  if (sabotage_alert) {
    alarm_status = "SABOTAGE_ALERT";
    return;
  }

  if (intrusion_alert) {
    alarm_status = "INTRUSION_ALERT";
    return;
  }

  if (system_armed) {
    alarm_status = "ARMED";
    return;
  }

  if (known_device_present) {
    alarm_status = "DISARMED_BY_TRUSTED_DEVICE";
    return;
  }

  alarm_status = "DISARMED";
}

void updateThreatLevel() {
  if (sosActive) {
    threat_level = 4;
    return;
  }

  if (sabotage_alert || intrusion_alert) {
    threat_level = 3;
    return;
  }

  if (intrusion_score >= getIntrusionThreshold()) {
    threat_level = 2;
    return;
  }

  if (intrusion_score > 0) {
    threat_level = 1;
    return;
  }

  threat_level = 0;
}

void updateNotificationRequestPlaceholders() {
  notification_channel = "telegram";

  if (sosActive) {
    send_notification_request = true;
    notification_event_type = "sos_alert";
    return;
  }

  if (sabotage_alert) {
    send_notification_request = true;
    notification_event_type = "sabotage_alert";
    return;
  }

  if (intrusion_alert) {
    send_notification_request = true;
    notification_event_type = "intrusion_alert";
    return;
  }

  auto_capture_photo_request = false;
  send_notification_request = false;
  notification_event_type = "NONE";

  if (notification_sent_status.length() == 0 || notification_sent_status == "PENDING_PHASE5") {
    notification_sent_status = "IDLE";
  }
}

void updateSecurityLogic(const HardwareSnapshot &s) {
  clampCloudConfigValues();
  updateTrustedDeviceStatusPlaceholders();
  updateSystemArmed();
  updateSabotageLogic(s);
  updateIntrusionLogic(s);
  updateAlarmStatus();
  updateThreatLevel();
  updateNotificationRequestPlaceholders();
  applyAlarmOutputs();
}

// ==================================================
// SCHEDULE LOGIC
// ==================================================

void updateScheduleLogic() {
  if (!schedule_enabled || !rtcOk) {
    return;
  }

  DateTime now = rtc.now();
  String key = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " " + String(now.hour()) + ":" + String(now.minute());

  if (key == lastScheduleTriggerKey) {
    return;
  }

  if (now.hour() == auto_arm_hour && now.minute() == auto_arm_minute) {
    alarm_enabled = true;
    lastScheduleTriggerKey = key;
    incrementEventCounter();
    setLastEvent("schedule_auto_arm", "Auto arm at " + getRtcTimeString());
    return;
  }

  if (now.hour() == auto_disarm_hour && now.minute() == auto_disarm_minute) {
    alarm_enabled = false;
    lastScheduleTriggerKey = key;
    incrementEventCounter();
    setLastEvent("schedule_auto_disarm", "Auto disarm at " + getRtcTimeString());
    return;
  }
}

// ==================================================
// DEMO SUPPORT
// ==================================================

void applyDemoScenario() {
  if (!demo_mode) {
    return;
  }

  if (demo_scenario == 10) {
    known_device_present = true;
    setLastEvent("demo_known_device", "Demo: known_device_present = true");
    demo_scenario = 0;
  } else if (demo_scenario == 11) {
    known_device_present = false;
    setLastEvent("demo_known_device", "Demo: known_device_present = false");
    demo_scenario = 0;
  } else if (demo_scenario == 20) {
    triggerSosAlert("CHILD_DEMO");
    demo_scenario = 0;
  } else if (demo_scenario == 21) {
    resetAllAlerts();
    demo_scenario = 0;
  }
}

// ==================================================
// SERIAL DEBUG COMMANDS
// ==================================================

void printHelp() {
  Serial.println();
  Serial.println("========== FINAL SERIAL COMMANDS ==========");
  Serial.println("h : show help");
  Serial.println("a : toggle alarm_enabled");
  Serial.println("k : toggle known_device_present locally");
  Serial.println("c : trigger Child SOS");
  Serial.println("p : trigger Parent/Admin SOS");
  Serial.println("r : reset all alerts");
  Serial.println("1 : sensitivity_level = 1, threshold = 5");
  Serial.println("2 : sensitivity_level = 2, threshold = 4");
  Serial.println("3 : sensitivity_level = 3, threshold = 3");
  Serial.println("==============================================");
}

void handleSerialCommand(char command) {
  if (command == '\n' || command == '\r' || command == ' ') {
    return;
  }

  if (command == 'h' || command == 'H') {
    printHelp();
  } else if (command == 'a' || command == 'A') {
    alarm_enabled = !alarm_enabled;
    setLastEvent("serial_alarm_enabled", String("Serial alarm_enabled = ") + (alarm_enabled ? "true" : "false"));
  } else if (command == 'k' || command == 'K') {
    known_device_present = !known_device_present;
    setLastEvent("serial_known_device", String("Serial known_device_present = ") + (known_device_present ? "true" : "false"));
  } else if (command == 'c' || command == 'C') {
    triggerSosAlert("CHILD_SERIAL");
  } else if (command == 'p' || command == 'P') {
    triggerSosAlert("PARENT_ADMIN_SERIAL");
  } else if (command == 'r' || command == 'R') {
    resetAllAlerts();
  } else if (command == '1') {
    sensitivity_level = 1;
  } else if (command == '2') {
    sensitivity_level = 2;
  } else if (command == '3') {
    sensitivity_level = 3;
  } else {
    Serial.print("[WARN] Unknown command: ");
    Serial.println(command);
  }
}

void handleSerialInput() {
  while (Serial.available() > 0) {
    handleSerialCommand(Serial.read());
  }
}

void printSecurityReport() {
  Serial.println();
  Serial.println("========== SERIAL MONITOR REPORT ==========");
  Serial.print("Cloud status                : "); Serial.println(ArduinoCloud.connected() ? "CONNECTED" : "NOT_CONNECTED");
  Serial.print("camera_ready                : "); Serial.println(cameraReady ? "true" : "false");
  Serial.print("photo_status                : "); Serial.println(photo_status);
  Serial.print("alarm_status                : "); Serial.println(alarm_status);
  Serial.print("system_armed                : "); Serial.println(system_armed ? "true" : "false");
  Serial.print("pir_detected                : "); Serial.println(pir_detected ? "true" : "false");
  Serial.print("ultrasonic_distance         : "); Serial.print(ultrasonic_distance); Serial.println(" cm");
  Serial.print("ldr_value                   : "); Serial.println(ldr_value);
  Serial.print("intrusion_score             : "); Serial.println(intrusion_score);
  Serial.print("threat_level                : "); Serial.println(threat_level);
  Serial.print("notification_sent_status    : "); Serial.println(notification_sent_status);
  Serial.print("emergency_escalation_status : "); Serial.println(emergency_escalation_status);
  Serial.print("emergency_authority_status  : "); Serial.println(emergency_authority_message_status);
  Serial.print("home_address_configured     : "); Serial.println(home_address_configured ? "true" : "false");
  Serial.print("trusted_ble_detection       : "); Serial.println(trusted_ble_detection_enabled ? "true" : "false");
  Serial.print("trusted_ble_present         : "); Serial.println(trusted_ble_present ? "true" : "false");
  Serial.print("trusted_device_source       : "); Serial.println(trusted_device_source);
  Serial.print("trusted_ble_rssi            : "); Serial.println(trusted_ble_rssi);
  Serial.print("trusted_ble_last_seen_sec   : "); Serial.println(trusted_ble_last_seen_seconds);
  Serial.print("last_event                  : "); Serial.println(last_event);
  Serial.println("===========================================");
}

// ==================================================
// SETUP / LOOP
// ==================================================

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("================================================");
  Serial.println("ESP32-S3 IOT ANTI-THEFT - FINAL");
  Serial.println("Cloud + Camera + Telegram + Google Script");
  Serial.println("================================================");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_PIR_OUT, INPUT);
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);

  digitalWrite(PIN_US_TRIG, LOW);
  setRedLedPhysical(false);
  setGreenLedPhysical(true);
  setBuzzerPhysical(false);

  analogReadResolution(12);

  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);
  rtcOk = rtc.begin();

  if (rtcOk) {
  Serial.println("[RTC] DS1307 found.");

  if (FORCE_SET_RTC_TIME_ONCE) {
    Serial.println("[RTC] Force setting RTC time from compile time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else if (!rtc.isrunning()) {
    Serial.println("[RTC] RTC is not running. Setting from compile time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

} else {
  Serial.println("[RTC] DS1307 NOT FOUND.");
}

  // Cloud default values. The IoT Cloud may overwrite READWRITE values
  // with the latest dashboard values after connection.
  alarm_enabled = true;
  system_armed = false;
  reset_alarm = false;
  sos_child = false;
  sos_adult = false;

  if (sensitivity_level < 1 || sensitivity_level > 3) {
    sensitivity_level = 3;
  }

  if (rearm_delay_seconds <= 0) {
    rearm_delay_seconds = 30;
  }

  if (auto_arm_hour < 0 || auto_arm_hour > 23) auto_arm_hour = 22;
  if (auto_arm_minute < 0 || auto_arm_minute > 59) auto_arm_minute = 0;
  if (auto_disarm_hour < 0 || auto_disarm_hour > 23) auto_disarm_hour = 6;
  if (auto_disarm_minute < 0 || auto_disarm_minute > 59) auto_disarm_minute = 0;

  alarm_status = "BOOTING";
  last_event_type = "system_boot";
  last_event = "System boot at " + getRtcTimeString();
  current_time = getRtcTimeString();

  emergency_escalation_status = "IDLE";
  notification_channel = "telegram";
  notification_event_type = "NONE";
  notification_sent_status = "IDLE";
  photo_status = "IDLE";
  sos_message = "NONE";
  last_unknown_mac = "NONE";
  last_alert_time = "NONE";

  // New SRS v0.6.7 Cloud variables: SOS authority escalation + BLE trusted device status.
  // READWRITE values may be overwritten by Arduino Cloud after connection.
  home_address_configured = false;
  sos_authority_note = "";
  emergency_authority_message_status = "IDLE";

  trusted_ble_detection_enabled = false;
  resetTrustedBleRuntimeState();
  trusted_device_source = "NONE";

  rearm_countdown_remaining = 0;
  unknown_wifi_count = 0;
  unknown_wifi_alert = false;
  auto_capture_photo_request = false;
  cooldown_active = false;
  emergency_confirmation_requested = false;
  emergency_confirmed = false;

  // =======================
  // CAMERA INIT
  // =======================
  // IMPORTANT:
  // cameraReady must be assigned here. If setup() does not call initCamera(),
  // manual_capture_photo will always fail with CAMERA_NOT_READY even if the camera wiring is OK.
  cameraReady = initCamera();

  if (cameraReady) {
    Serial.println("[CAM] Camera ready.");
    setLastEvent("camera_ready", "Camera init OK at " + getRtcTimeString());
  } else {
    Serial.println("[CAM] Camera NOT ready.");
    setLastEvent("camera_init_failed", "Camera init failed at " + getRtcTimeString());
  }

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  printHelp();
  Serial.println("[INFO] Final build started. Watch Arduino Cloud dashboard, camera status, Telegram, and Apps Script status.");
}

void loop() {
  ArduinoCloud.update();
  handleSerialInput();
  applyDemoScenario();

  static unsigned long lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL_MS) {
    lastSensorUpdate = millis();

    HardwareSnapshot snapshot = readHardwareSnapshot();
    publishHardwareSnapshotToCloud(snapshot);
    updateScheduleLogic();
    updateSecurityLogic(snapshot);
    processCameraRequests();
  }

  static unsigned long lastReport = 0;
  if (millis() - lastReport >= SERIAL_REPORT_INTERVAL_MS) {
    lastReport = millis();
    printSecurityReport();
  }
}

// ==================================================
// CLOUD CALLBACKS
// ==================================================

void onAlarmEnabledChange() {
  setLastEvent("cloud_alarm_enabled", String("Cloud alarm_enabled = ") + (alarm_enabled ? "true" : "false"));
}

void onResetAlarmChange() {
  if (reset_alarm) {
    resetAllAlerts();
    reset_alarm = false;
  }
}

void onSosChildChange() {
  if (sos_child) {
    triggerSosAlert("CHILD");
    sos_child = false;
  }
}

void onSosAdultChange() {
  if (sos_adult) {
    triggerSosAlert("PARENT_ADMIN");
    sos_adult = false;
  }
}

void onSensitivityLevelChange() {
  clampCloudConfigValues();
  setLastEvent("cloud_sensitivity", "sensitivity_level = " + String(sensitivity_level));
}

void onDemoModeChange() {
  setLastEvent("cloud_demo_mode", String("demo_mode = ") + (demo_mode ? "true" : "false"));
}

void onDemoScenarioChange() {
  applyDemoScenario();
}

void onManualCapturePhotoChange() {
  if (manual_capture_photo) {
    photo_status = "MANUAL_CAPTURE_REQUESTED";
    setLastEvent("manual_capture_requested", "Manual capture requested at " + getRtcTimeString());
  }
}

void onRearmDelaySecondsChange() {
  clampCloudConfigValues();
}

void onScheduleEnabledChange() {
  setLastEvent("cloud_schedule", String("schedule_enabled = ") + (schedule_enabled ? "true" : "false"));
}

void onAutoArmHourChange() {
  clampCloudConfigValues();
}

void onAutoArmMinuteChange() {
  clampCloudConfigValues();
}

void onAutoDisarmHourChange() {
  clampCloudConfigValues();
}

void onAutoDisarmMinuteChange() {
  clampCloudConfigValues();
}

void onUnknownWifiDetectionEnabledChange() {
  if (!unknown_wifi_detection_enabled) {
    unknown_wifi_alert = false;
    unknown_wifi_count = 0;
    last_unknown_mac = "NONE";
  }
  setLastEvent("cloud_unknown_wifi", String("unknown_wifi_detection_enabled = ") + (unknown_wifi_detection_enabled ? "true" : "false"));
}

void onKnownDevicePresentChange() {
  updateTrustedDeviceStatusPlaceholders();
  setLastEvent("cloud_known_device", String("known_device_present = ") + (known_device_present ? "true" : "false") +
               ", source=" + trusted_device_source);
}

void onTrustedBleDetectionEnabledChange() {
  if (!trusted_ble_detection_enabled) {
    resetTrustedBleRuntimeState();
    trusted_device_source = known_device_present ? "DASHBOARD_INPUT" : "NONE";
  } else {
    // Real BLE scanning will be added in the next implementation step.
    // For now, enabling this switch only exposes the SRS variables and keeps the old manual demo flow stable.
    trusted_device_source = known_device_present ? "DASHBOARD_INPUT" : "NONE";
  }

  setLastEvent(
    "cloud_ble_detection",
    String("trusted_ble_detection_enabled = ") +
    (trusted_ble_detection_enabled ? "true" : "false") +
    ", source=" + trusted_device_source
  );
}

void onSosAuthorityNoteChange() {
  // Do not print the full note to Serial/log because it may contain sensitive emergency context.
  setLastEvent(
    "cloud_sos_note",
    "sos_authority_note updated, length=" + String(sos_authority_note.length()) +
    " at " + getRtcTimeString()
  );
}
