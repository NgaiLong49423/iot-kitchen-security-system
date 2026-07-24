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
const float OBJECT_TOO_CLOSE_THRESHOLD_CM = 15.0;

// If covering the LDR makes the analog value go high, keep true.
// If covering the LDR makes the analog value go low, change this to false.
const bool LDR_COVER_WHEN_HIGH = true;
const int LDR_COVERED_THRESHOLD_HIGH = 2000;
const int LDR_COVERED_THRESHOLD_LOW = 500;

const int LDR_DELTA_ABNORMAL_THRESHOLD = 500;
const unsigned long SABOTAGE_HOLD_MS = 3000;
const unsigned long INTRUSION_HOLD_MS = 2000;

const unsigned long SENSOR_UPDATE_INTERVAL_MS = 500;
const unsigned long SERIAL_REPORT_INTERVAL_MS = 2000;
const unsigned long RED_BLINK_INTERVAL_MS = 250;
const unsigned long GOOGLE_HEARTBEAT_INTERVAL_MS = 30000;
const unsigned long GOOGLE_HTTP_TIMEOUT_MS = 5000;
const unsigned long SENSOR_BOOT_GRACE_MS = 3000;

RTC_DS1307 rtc;
bool rtcOk = false;
bool cameraReady = false;
unsigned long lastCameraCaptureMs = 0;
bool manualCapturePending = false;

int lastLdrValue = -1;
unsigned long sabotageConditionStartedAt = 0;

bool sosActive = false;
String sosSource = "NONE";
int activeDemoScenario = 0;

int rawIntrusionScore = 0;
int lastIntrusionScoreAtTrigger = 0;
int lastRawIntrusionScoreAtTrigger = 0;
String lastIntrusionReason = "NONE";
unsigned long intrusionConditionStartedAt = 0;

String lastScheduleTriggerKey = "";

unsigned long lastGoogleScriptCallMs = 0;
String lastGoogleScriptEventKey = "";
String activeGoogleEventId = "";
unsigned long bootCompletedAtMs = 0;

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
String responseValue(const String &response, const String &key);
bool hasTelegramConfig();
bool hasGoogleScriptConfig();
String buildCommonCaption(const String &eventType, const String &reason);
bool sendTelegramMessage(const String &message);
bool sendTelegramPhoto(camera_fb_t *fb, const String &caption);
bool performGetRequest(String url, String &outResponse, int &outCode, int redirectDepth = 0);
bool callGoogleAppsScript(const String &eventType, const String &source, const String &message);
void processGoogleAppsScriptHeartbeat();
void resolveGoogleAppsScriptCurrentEvent();
bool isTelegramAllowed(const String &eventType);
void notifySecurityTextEvent(const String &eventType, const String &source, const String &message);
void updateTrustedDeviceStatusPlaceholders();
void resetTrustedBleRuntimeState();
void updateGoogleScriptStatusFromResponse(const String &response, bool ok, bool isHeartbeatEvent = false);

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
    photo_status = "Camera khởi động thất bại";
    return false;
  }

  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor == NULL) {
    Serial.println("[CAM] Sensor pointer is NULL.");
    photo_status = "Không đọc được cảm biến camera";
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

  photo_status = "Camera sẵn sàng";
  return true;
}

camera_fb_t *captureFreshCameraFrame() {
  // Release the currently queued frame, then allow the sensor to produce a new one.
  camera_fb_t *queuedFrame = esp_camera_fb_get();
  if (queuedFrame != NULL) {
    esp_camera_fb_return(queuedFrame);
    Serial.println("[CAM] Queued frame discarded.");
  } else {
    Serial.println("[CAM] No queued frame available to discard.");
  }

  delay(120);
  return esp_camera_fb_get();
}

bool captureSecurityPhoto(const String &reason) {
  if (!cameraReady) {
    photo_status = "Camera chưa sẵn sàng";
    setLastEvent("photo_capture_failed", "Camera not ready for " + reason + " at " + getRtcTimeString());
    Serial.println("[CAM] Capture skipped: camera not ready.");
    return false;
  }

  // Avoid accidental repeated captures too fast (hardware cooldown)
  if (millis() - lastCameraCaptureMs < 1500) {
    photo_status = "Camera đang chờ ổn định";
    Serial.println("[CAM] Capture skipped: short camera cooldown.");
    return false;
  }

  lastCameraCaptureMs = millis();
  photo_status = "Đang chụp ảnh";

  Serial.println();
  Serial.println("========== SECURITY PHOTO CAPTURE ==========");
  Serial.print("[CAM] Reason: ");
  Serial.println(reason);

  camera_fb_t *fb = captureFreshCameraFrame();

  if (fb == NULL) {
    photo_status = "Chụp ảnh thất bại";
    notification_sent_status = "Không có ảnh để gửi";
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
    reason == "AUTO_INTRUSION_ALERT" ||
    reason == "AUTO_SABOTAGE_ALERT";

  bool telegramPhotoSent = false;

  if (shouldSendToTelegram) {
    if (!isTelegramAllowed(reason)) {
      photo_status = "Tạm dừng gửi ảnh để tránh lặp cảnh báo";
      notification_sent_status = "Đang chặn gửi lặp để tránh spam";
      Serial.println("[TG] Skipped photo notification by cooldown.");
      esp_camera_fb_return(fb);
      return false;
    }

    notification_channel = "telegram";
    notification_event_type = reason;
    notification_sent_status = "Đang gửi ảnh qua Telegram";

    String caption = buildCommonCaption("photo_capture", reason);
    telegramPhotoSent = sendTelegramPhoto(fb, caption);
  }

  esp_camera_fb_return(fb);

  if (shouldSendToTelegram) {
    if (telegramPhotoSent) {
      photo_status = "Đã chụp và gửi ảnh";
      notification_sent_status = "Đã gửi thông báo";
    } else {
      photo_status = "Đã chụp ảnh nhưng gửi thất bại";
      if (!isKnownNotificationFailureStatus()) {
        notification_sent_status = "Gửi thông báo thất bại";
      }
    }
  } else {
    photo_status = "Đã chụp ảnh";
  }

  setLastEvent("photo_captured", "Photo captured for " + reason + " at " + getRtcTimeString());

  Serial.println("[CAM] Frame returned to driver.");
  Serial.println("============================================");

  return true;
}

void processCameraRequests() {
  bool demoLockActive = demo_mode && activeDemoScenario != 0;

  if (manualCapturePending) {
    manualCapturePending = false;

    if (!demoLockActive || activeDemoScenario == 5) {
      captureSecurityPhoto("MANUAL_DASHBOARD");
    } else {
      photo_status = "Yêu cầu chụp ảnh bị chặn bởi chế độ demo";
    }
  }

  if (auto_capture_photo_request) {
    if (!demoLockActive || activeDemoScenario == 2 || activeDemoScenario == 3 || activeDemoScenario == 4) {
      String reason = notification_event_type == "sabotage_alert"
        ? "AUTO_SABOTAGE_ALERT"
        : "AUTO_INTRUSION_ALERT";
      captureSecurityPhoto(reason);
    } else {
      photo_status = "Chụp ảnh tự động bị chặn bởi chế độ demo";
    }
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

String eventTitleVi(const String &type) {
  if (type == "intrusion_alert") return "Cảnh báo đột nhập";
  if (type == "sabotage_alert") return "Cảnh báo phá hoại thiết bị";
  if (type == "sos_alert") return "Yêu cầu SOS khẩn cấp";
  if (type == "photo_capture") return "Chụp ảnh an ninh";
  if (type == "photo_captured") return "Đã chụp ảnh";
  if (type == "photo_capture_failed") return "Không chụp được ảnh";
  if (type == "alarm_reset") return "Đã reset cảnh báo";
  if (type == "schedule_auto_arm") return "Tự bật bảo vệ theo lịch";
  if (type == "schedule_auto_disarm") return "Tự tắt bảo vệ theo lịch";
  if (type == "camera_ready") return "Camera sẵn sàng";
  if (type == "camera_init_failed") return "Camera chưa sẵn sàng";
  if (type == "system_boot") return "Thiết bị vừa khởi động";
  return type;
}

String sourceTitleVi(const String &source) {
  if (source == "CHILD" || source == "CHILD_DEMO" || source == "CHILD_SERIAL") return "Trẻ em";
  if (source == "PARENT_ADMIN" || source == "PARENT_ADMIN_SERIAL") return "Phụ huynh/Admin";
  if (source == "DEVICE") return "Thiết bị";
  return source;
}

String escalationStatusVi(const String &code) {
  if (code == "IDLE") return "Chưa có yêu cầu khẩn cấp";
  if (code == "SENDING") return "Đang gửi yêu cầu xác nhận";
  if (code == "WAITING_CONFIRMATION") return "Đang chờ phụ huynh/Admin xác nhận";
  if (code == "CONFIRMED") return "Đã được phụ huynh/Admin xác nhận";
  if (code == "SENT") return "Đã gửi email đến contact mô phỏng";
  if (code == "FAILED") return "Gửi thất bại, cần kiểm tra cấu hình hoặc mạng";
  if (code == "NOT_CONFIGURED") return "Chưa cấu hình đủ địa chỉ hoặc contact mô phỏng";
  if (code == "WIFI_NOT_CONNECTED") return "Thiết bị chưa có WiFi để gửi yêu cầu";
  if (code == "MONITORING") return "Đang theo dõi sau cảnh báo phá hoại";
  return code;
}

String authorityStatusVi(const String &code) {
  if (code == "IDLE") return "Chưa gửi contact mô phỏng";
  if (code == "READY") return "Sẵn sàng gửi sau xác nhận";
  if (code == "SENT") return "Đã gửi contact mô phỏng";
  if (code == "FAILED") return "Gửi contact mô phỏng thất bại";
  if (code == "NOT_CONFIGURED") return "Chưa cấu hình contact mô phỏng";
  return code;
}

void setEscalationStatus(const String &code) {
  emergency_escalation_status = escalationStatusVi(code);
}

void setAuthorityStatus(const String &code) {
  emergency_authority_message_status = authorityStatusVi(code);
}

bool isKnownNotificationFailureStatus() {
  return notification_sent_status == "NOT_CONFIGURED" ||
         notification_sent_status == "WIFI_NOT_CONNECTED" ||
         notification_sent_status == "CONNECT_FAILED" ||
         notification_sent_status == "Chưa cấu hình Telegram" ||
         notification_sent_status == "Không có WiFi để gửi Telegram" ||
         notification_sent_status == "Không kết nối được Telegram";
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

String responseValue(const String &response, const String &key) {
  String marker = key + "=";
  int start = response.indexOf(marker);
  if (start < 0) {
    return "";
  }

  start += marker.length();
  int end = response.indexOf(';', start);
  String value = end < 0 ? response.substring(start) : response.substring(start, end);
  value.trim();
  return value;
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
  text += "Sự kiện: ";
  text += eventTitleVi(eventType);
  text += "\nThiết bị: ";
  text += SECRET_DEVICE_NAME;
  text += "\nKhu vực: ";
  text += SECRET_DEVICE_LOCATION;
  text += "\nNguồn/kịch bản: ";
  text += sourceTitleVi(reason);
  text += "\nTrạng thái: ";
  text += alarm_status;
  text += "\nMức nguy hiểm: ";
  text += String(threat_level);
  text += "\nĐiểm nghi ngờ: ";
  text += String(intrusion_score);
  text += "\nThời gian: ";
  text += getRtcTimeString();
  text += "\nHành động tiếp theo: Vui lòng kiểm tra khu vực và chỉ bấm reset_alarm sau khi an toàn.";
  return text;
}

bool sendTelegramMessage(const String &message) {
  notification_channel = "telegram";

  if (!hasTelegramConfig()) {
    notification_sent_status = "Chưa cấu hình Telegram";
    Serial.println("[TG] Telegram is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    notification_sent_status = "Không có WiFi để gửi Telegram";
    Serial.println("[TG] WiFi is not connected.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  if (!client.connect("api.telegram.org", 443)) {
    notification_sent_status = "Không kết nối được Telegram";
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

  notification_sent_status = ok ? "Đã gửi thông báo" : "Gửi thông báo thất bại";
  return ok;
}

bool sendTelegramPhoto(camera_fb_t *fb, const String &caption) {
  notification_channel = "telegram";

  if (!hasTelegramConfig()) {
    notification_sent_status = "Chưa cấu hình Telegram";
    Serial.println("[TG] Telegram is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    notification_sent_status = "Không có WiFi để gửi Telegram";
    Serial.println("[TG] WiFi is not connected.");
    return false;
  }

  if (fb == NULL || fb->buf == NULL || fb->len == 0) {
    notification_sent_status = "Không có ảnh để gửi";
    Serial.println("[TG] Photo buffer is empty.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(20000);

  if (!client.connect("api.telegram.org", 443)) {
    notification_sent_status = "Không kết nối được Telegram";
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

  notification_sent_status = ok ? "Đã gửi ảnh qua Telegram" : "Gửi ảnh qua Telegram thất bại";
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
  client.setTimeout(GOOGLE_HTTP_TIMEOUT_MS);

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
  while (client.connected() && !client.available() && millis() - start < GOOGLE_HTTP_TIMEOUT_MS) {
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

void updateGoogleScriptStatusFromResponse(const String &response, bool ok, bool isHeartbeatEvent) {
  if (!ok) {
    if (isHeartbeatEvent) {
      heartbeat_status = "Gửi heartbeat thất bại";
    } else {
      setEscalationStatus("FAILED");
      setAuthorityStatus("FAILED");
    }
    return;
  }

  if (response.indexOf("OK:HEARTBEAT") >= 0) {
    heartbeat_status = "Đang liên lạc bình thường";
    last_heartbeat_time = getRtcTimeString();
    return;
  }

  String returnedEventId = responseValue(response, "eventId");
  if (returnedEventId.length() > 0 && returnedEventId != "NONE") {
    activeGoogleEventId = returnedEventId;
  }

  if (response.indexOf("OK:SABOTAGE_MONITORING") >= 0 ||
      response.indexOf("OK:MONITORING") >= 0) {
    setEscalationStatus("MONITORING");
    setAuthorityStatus("IDLE");
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
      response.indexOf("OK:WAITING_CONFIRMATION") >= 0 ||
      response.indexOf("OK:CRITICAL_WAITING_CONFIRMATION") >= 0) {
    setEscalationStatus("WAITING_CONFIRMATION");
  } else if (response.indexOf("emergency_escalation_status=CONFIRMED") >= 0) {
    setEscalationStatus("CONFIRMED");
  } else if (response.indexOf("emergency_escalation_status=SENT") >= 0) {
    setEscalationStatus("SENT");
  } else if (response.indexOf("emergency_escalation_status=FAILED") >= 0) {
    setEscalationStatus("FAILED");
  } else if (response.indexOf("emergency_escalation_status=NOT_CONFIGURED") >= 0) {
    setEscalationStatus("NOT_CONFIGURED");
  } else if (response.indexOf("OK:SABOTAGE_MONITORING") < 0 &&
             response.indexOf("OK:MONITORING") < 0) {
    // Backward-compatible fallback for older Apps Script responses.
    setEscalationStatus("SENT");
  }

  if (response.indexOf("emergency_authority_message_status=IDLE") >= 0) {
    setAuthorityStatus("IDLE");
  } else if (response.indexOf("emergency_authority_message_status=READY") >= 0) {
    setAuthorityStatus("READY");
  } else if (response.indexOf("emergency_authority_message_status=SENT") >= 0) {
    setAuthorityStatus("SENT");
  } else if (response.indexOf("emergency_authority_message_status=FAILED") >= 0) {
    setAuthorityStatus("FAILED");
  } else if (response.indexOf("emergency_authority_message_status=NOT_CONFIGURED") >= 0) {
    setAuthorityStatus("NOT_CONFIGURED");
  }

  if (response.indexOf("OK:CRITICAL_WAITING_CONFIRMATION") >= 0) {
    critical_security_compromise = true;
    device_health_status = "Sự cố nghiêm trọng sau phá hoại";
  }
}

bool callGoogleAppsScript(const String &eventType, const String &source, const String &message) {
  bool isHeartbeatEvent = eventType == "heartbeat";

  if (!hasGoogleScriptConfig()) {
    if (isHeartbeatEvent) {
      heartbeat_status = "Chưa cấu hình Apps Script";
    } else {
      setEscalationStatus("NOT_CONFIGURED");
      setAuthorityStatus("NOT_CONFIGURED");
    }
    Serial.println("[GAS] Google Apps Script is not configured.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (isHeartbeatEvent) {
      heartbeat_status = "Không có WiFi để gửi heartbeat";
    } else {
      setEscalationStatus("WIFI_NOT_CONNECTED");
      setAuthorityStatus("FAILED");
    }
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

  if (isHeartbeatEvent) {
    heartbeat_status = "Đang gửi heartbeat";
  } else {
    setEscalationStatus("SENDING");
    setAuthorityStatus("IDLE");
  }

  String response = "";
  int code = 0;
  bool ok = performGetRequest(url, response, code);

  Serial.print("[GAS] HTTP code: ");
  Serial.println(code);
  if (response.length() > 0) {
    Serial.print("[GAS] Response: ");
    Serial.println(response.substring(0, 200));
  }

  updateGoogleScriptStatusFromResponse(response, ok, isHeartbeatEvent);
  if (isHeartbeatEvent && !ok) {
    heartbeat_status = "Gửi heartbeat thất bại";
  }
  return ok;
}

void processGoogleAppsScriptHeartbeat() {
  static unsigned long lastHeartbeatMs = 0;

  if (lastHeartbeatMs != 0 && millis() - lastHeartbeatMs < GOOGLE_HEARTBEAT_INTERVAL_MS) {
    return;
  }

  lastHeartbeatMs = millis();

  if (!hasGoogleScriptConfig()) {
    heartbeat_status = "Chưa cấu hình Apps Script";
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    heartbeat_status = "Không có WiFi để gửi heartbeat";
    return;
  }

  bool ok = callGoogleAppsScript("heartbeat", "DEVICE", "Heartbeat từ thiết bị lúc " + getRtcTimeString());
  if (ok) {
    heartbeat_status = "Đang liên lạc bình thường";
    last_heartbeat_time = getRtcTimeString();
  }
}

void resolveGoogleAppsScriptCurrentEvent() {
  if (!hasGoogleScriptConfig() || WiFi.status() != WL_CONNECTED) {
    return;
  }

  String url = String(SECRET_GOOGLE_SCRIPT_URL);
  url += "?action=resolve";
  url += "&device=" + urlEncode(String(SECRET_DEVICE_NAME));
  url += "&location=" + urlEncode(String(SECRET_DEVICE_LOCATION));
  url += "&time=" + urlEncode(getRtcTimeString());
  url += "&message=" + urlEncode("Device reset_alarm resolved local alerts.");
  if (activeGoogleEventId.length() > 0) {
    url += "&eventId=" + urlEncode(activeGoogleEventId);
  }

  String response = "";
  int code = 0;
  bool ok = performGetRequest(url, response, code);

  Serial.print("[GAS] resolve HTTP code: ");
  Serial.println(code);
  if (response.length() > 0) {
    Serial.print("[GAS] resolve response: ");
    Serial.println(response.substring(0, 160));
  }
  if (!ok) {
    Serial.println("[GAS] resolve failed, local reset still completed.");
  } else if (response.indexOf("OK:RESOLVED") >= 0) {
    activeGoogleEventId = "";
  }
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
    notification_sent_status = "Đang chặn gửi lặp để tránh spam";
    Serial.println("[TG] Skipped text notification by cooldown.");
    return;
  }

  send_notification_request = true;
  notification_event_type = eventType;
  notification_channel = "telegram";

  String text = buildCommonCaption(eventType, source);
  text += "\nNội dung: ";
  text += message;

  notification_sent_status = "Đang gửi thông báo";
  bool sent = sendTelegramMessage(text);
  notification_sent_status = sent ? "Đã gửi thông báo" : "Gửi thông báo thất bại";
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
  bool anyAlarmActive = sosActive || sabotage_alert || intrusion_alert || critical_security_compromise;

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

  return score;
}

String buildIntrusionReason(const HardwareSnapshot &s) {
  String reason = "";

  if (s.pirDetected) reason += "chuyển động PIR (+2); ";
  if (s.objectNear) reason += "vật thể ở gần (+2); ";
  if (s.lightAbnormal) reason += "ánh sáng thay đổi bất thường (+1); ";
  if (s.nightMode) reason += "đang trong khung giờ ban đêm (+1); ";

  if (reason.length() == 0) {
    reason = "chưa có tín hiệu đáng kể";
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
  system_armed = alarm_enabled;
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
      "Cảnh báo đột nhập: điểm nghi ngờ " + String(lastIntrusionScoreAtTrigger) +
      ", tín hiệu liên quan: " + lastIntrusionReason +
      ". Hệ thống đã bật còi/LED đỏ và yêu cầu chụp ảnh lúc " + getRtcTimeString() + "."
    );

    Serial.println();
    Serial.println("[ALERT] INTRUSION TRIGGERED");
    Serial.print("[ALERT] score = ");
    Serial.println(lastIntrusionScoreAtTrigger);
    Serial.print("[ALERT] reason = ");
    Serial.println(lastIntrusionReason);

    auto_capture_photo_request = true;
    photo_status = "Đang chuẩn bị chụp ảnh tự động";
    send_notification_request = true;
    notification_event_type = "intrusion_alert";
    notification_sent_status = "Đang chụp ảnh để gửi cảnh báo";
  }
}

void triggerSabotageAlert() {
  if (!sabotage_alert) {
    sabotage_alert = true;
    incrementEventCounter();
    device_tampered = true;
    device_health_status = "Đang theo dõi sau cảnh báo phá hoại";
    setLastEvent(
      "sabotage_alert",
      "Cảnh báo phá hoại: thiết bị phát hiện vật thể áp sát đồng thời che cảm biến ánh sáng lúc " +
      getRtcTimeString() + ". Hệ thống đang theo dõi heartbeat để phát hiện mất liên lạc."
    );
    threat_level = 4;

    // A sabotage event has the same immediate local response as an intrusion:
    // alarm outputs are applied on this loop and a security photo is sent once.
    auto_capture_photo_request = true;
    photo_status = "Đang chuẩn bị chụp ảnh phá hoại";
    send_notification_request = true;
    notification_event_type = "sabotage_alert";
    notification_sent_status = "Đang chụp ảnh để gửi cảnh báo phá hoại";

    notifySecurityTextEvent("sabotage_alert", "DEVICE", last_event);
    callGoogleAppsScript("sabotage_alert", "DEVICE", last_event);
  }
}

void triggerSosAlert(const String &source) {
  if (!sosActive) {
    incrementEventCounter();
  }

  sosActive = true;
  sosSource = source;

  sos_message = "SOS từ " + sourceTitleVi(source) + " lúc " + getRtcTimeString() +
                ". Còi/LED đỏ sẽ tiếp tục bật cho đến khi phụ huynh/Admin reset.";
  emergency_confirmation_requested = true;
  setEscalationStatus("WAITING_CONFIRMATION");
  setAuthorityStatus("IDLE");

  setLastEvent("sos_alert", sos_message);
  notifySecurityTextEvent("sos_alert", source, sos_message);
  callGoogleAppsScript("sos_alert", source, sos_message);
}

void triggerDemoCriticalCompromise() {
  if (!sabotage_alert) {
    sabotage_alert = true;
    device_tampered = true;
    incrementEventCounter();
  }

  critical_security_compromise = true;
  threat_level = 4;
  device_health_status = "Sự cố nghiêm trọng sau phá hoại";
  heartbeat_status = "Demo: mô phỏng mất heartbeat sau phá hoại";
  alarm_status = "Sự cố nghiêm trọng sau phá hoại";

  setLastEvent(
    "critical_security_compromise",
    "Demo DS-03: mô phỏng thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau cảnh báo phá hoại lúc " +
    getRtcTimeString() + ". Đây là đầu vào demo có kiểm soát, không cần rút nguồn thật."
  );

  notifySecurityTextEvent("critical_security_compromise", "DEVICE", last_event);
  callGoogleAppsScript("critical_security_compromise", "DEVICE", last_event);
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
  critical_security_compromise = false;
  device_tampered = false;
  device_health_status = "Đã reset cảnh báo";
  setEscalationStatus("IDLE");
  setAuthorityStatus("IDLE");

  auto_capture_photo_request = false;
  manualCapturePending = false;

  send_notification_request = false;
  notification_event_type = "NONE";
  notification_sent_status = "Chưa có cảnh báo cần gửi";

  sabotageConditionStartedAt = 0;
  intrusionConditionStartedAt = 0;
  activeDemoScenario = 0;

  incrementEventCounter();
  setLastEvent("alarm_reset", "Đã reset cảnh báo tại thiết bị lúc " + getRtcTimeString() + ". Vui lòng kiểm tra thực tế trước khi bật lại bảo vệ.");
  resolveGoogleAppsScriptCurrentEvent();

  setBuzzerPhysical(false);
  setRedLedPhysical(false);
  setGreenLedPhysical(true);
}

void updateSabotageLogic(const HardwareSnapshot &s) {
  // Demo anti-sabotage requires both physical signs at the same time:
  // an object is very close and the light sensor is covered.
  bool sabotageCondition = s.ldrCovered && s.objectTooClose;

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

  // Ignore transient PIR/ultrasonic readings immediately after power-up.
  if (bootCompletedAtMs != 0 && millis() - bootCompletedAtMs < SENSOR_BOOT_GRACE_MS) {
    return;
  }

  // Intrusion requires two physical signals together. Very close objects
  // are reserved for the anti-sabotage flow and do not create intrusion.
  bool intrusionCondition = system_armed &&
                            s.pirDetected &&
                            s.objectNear &&
                            !s.objectTooClose;

  if (intrusionCondition) {
    if (intrusionConditionStartedAt == 0) {
      intrusionConditionStartedAt = millis();
    }

    if (!intrusion_alert && millis() - intrusionConditionStartedAt >= INTRUSION_HOLD_MS) {
      triggerIntrusionAlert(s);
    }
  } else if (!intrusion_alert) {
    intrusionConditionStartedAt = 0;
  }

  // Important: intrusion_alert is latched. It does not auto-clear when score drops.
}

void updateAlarmStatus() {
  if (sosActive) {
    alarm_status = "SOS khẩn cấp đang hoạt động";
    return;
  }

  if (critical_security_compromise) {
    alarm_status = "Sự cố nghiêm trọng sau phá hoại";
    return;
  }

  if (sabotage_alert) {
    alarm_status = "Cảnh báo phá hoại thiết bị";
    return;
  }

  if (intrusion_alert) {
    alarm_status = "Cảnh báo đột nhập";
    return;
  }

  if (system_armed) {
    alarm_status = "Đang bảo vệ khu vực bếp";
    return;
  }

  if (known_device_present) {
    alarm_status = "Đang tắt bảo vệ";
    return;
  }

  alarm_status = "Đang tắt bảo vệ";
}

void updateDeviceHealthStatus() {
  if (critical_security_compromise) {
    device_health_status = "Sự cố nghiêm trọng sau phá hoại";
    return;
  }

  if (sabotage_alert) {
    device_health_status = "Đang theo dõi sau cảnh báo phá hoại";
    return;
  }

  if (heartbeat_status.length() == 0) {
    heartbeat_status = "Chưa gửi heartbeat";
  }

  if (device_health_status.length() == 0) {
    device_health_status = "Thiết bị hoạt động bình thường";
  }
}

void updateThreatLevel() {
  if (sosActive) {
    threat_level = 4;
    return;
  }

  if (sabotage_alert) {
    threat_level = 4;
    return;
  }

  if (intrusion_alert) {
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

  if (notification_sent_status.length() == 0 || notification_sent_status == "PENDING_PHASE5" || notification_sent_status == "IDLE") {
    notification_sent_status = "Chưa có cảnh báo cần gửi";
  }
}

void updateSecurityLogic(const HardwareSnapshot &s) {
  clampCloudConfigValues();
  updateTrustedDeviceStatusPlaceholders();
  updateSystemArmed();

  // Demo lock: only the selected real scenario is allowed to react.
  if (demo_mode && activeDemoScenario != 0) {
    if (activeDemoScenario == 2) {
      updateIntrusionLogic(s);
    } else if (activeDemoScenario == 3 || activeDemoScenario == 4) {
      updateSabotageLogic(s);
    }
  } else {
    updateSabotageLogic(s);
    updateIntrusionLogic(s);
  }

  updateAlarmStatus();
  updateThreatLevel();
  updateDeviceHealthStatus();
  updateNotificationRequestPlaceholders();
  applyAlarmOutputs();
}

// ==================================================
// SCHEDULE LOGIC
// ==================================================

void updateScheduleLogic() {
  if (demo_mode && activeDemoScenario != 0 && activeDemoScenario != 1) {
    return;
  }

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
    setLastEvent("schedule_auto_arm", "Hệ thống đã tự bật bảo vệ theo lịch lúc " + getRtcTimeString() + ".");
    return;
  }

  if (now.hour() == auto_disarm_hour && now.minute() == auto_disarm_minute) {
    alarm_enabled = false;
    lastScheduleTriggerKey = key;
    incrementEventCounter();
    setLastEvent("schedule_auto_disarm", "Hệ thống đã tự tắt chế độ phát hiện đột nhập theo lịch lúc " + getRtcTimeString() + ". SOS và chống phá hoại vẫn hoạt động.");
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

  if (demo_scenario == 0) {
    return;
  }

  if (activeDemoScenario == demo_scenario) {
    // Arduino Cloud may resend the same READWRITE value after reconnect.
    // Do not replay a demo event or send another notification.
    demo_scenario = 0;
    return;
  }

  activeDemoScenario = demo_scenario;

  if (demo_scenario == 1) {
    setLastEvent("demo_ds01_ready", "Demo DS-01 đã chọn: chờ giờ tự bật/tắt theo lịch đã cấu hình.");
  } else if (demo_scenario == 2) {
    alarm_enabled = true;
    updateSystemArmed();
    intrusionConditionStartedAt = 0;
    setLastEvent("demo_ds02_ready", "Demo DS-02 đã chọn: chờ PIR và vật thể trong vùng 15-50 cm liên tục 2 giây.");
  } else if (demo_scenario == 3) {
    alarm_enabled = false;
    updateSystemArmed();
    sabotageConditionStartedAt = 0;
    setLastEvent("demo_ds04_ready", "Demo DS-04 đã chọn: chờ vật thể áp sát và che cảm biến ánh sáng liên tục 3 giây.");
  } else if (demo_scenario == 4) {
    alarm_enabled = false;
    updateSystemArmed();
    sabotageConditionStartedAt = 0;
    setLastEvent("demo_ds03_ready", "Demo DS-03 đã chọn: cần phát hiện phá hoại trước, sau đó rút nguồn để kiểm tra mất heartbeat.");
  } else if (demo_scenario == 5) {
    setLastEvent("demo_ds05_ready", "Demo DS-05 đã chọn: chờ yêu cầu chụp ảnh thủ công từ Dashboard.");
  } else if (demo_scenario == 6) {
    setLastEvent("demo_ds06_ready", "Demo DS-06 đã chọn: chờ nút SOS từ trẻ em hoặc phụ huynh/Admin.");
  } else {
    activeDemoScenario = 0;
    setLastEvent("demo_invalid", "Số demo không hợp lệ. Chỉ dùng các số từ 1 đến 6.");
  }

  demo_scenario = 0;
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
  Serial.println("x : demo critical compromise after sabotage");
  Serial.println("r : reset all alerts");
  Serial.println("Dashboard demo lock: 1=DS01, 2=DS02, 3=DS04, 4=DS03, 5=DS05, 6=DS06");
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
  } else if (command == 'x' || command == 'X') {
    triggerDemoCriticalCompromise();
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
  Serial.print("object_near                 : "); Serial.println(object_near ? "true" : "false");
  Serial.print("ldr_value                   : "); Serial.println(ldr_value);
  Serial.print("ldr_covered                 : "); Serial.println(ldr_covered ? "true" : "false");
  Serial.print("intrusion_score             : "); Serial.println(intrusion_score);
  Serial.print("threat_level                : "); Serial.println(threat_level);
  Serial.print("sabotage_alert              : "); Serial.println(sabotage_alert ? "true" : "false");
  Serial.print("notification_sent_status    : "); Serial.println(notification_sent_status);
  Serial.print("emergency_escalation_status : "); Serial.println(emergency_escalation_status);
  Serial.print("emergency_authority_status  : "); Serial.println(emergency_authority_message_status);
  Serial.print("home_address_configured     : "); Serial.println(home_address_configured ? "true" : "false");
  Serial.print("active_demo_scenario        : "); Serial.println(activeDemoScenario);
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

  // These are runtime alarm latches. They must start clear after a reboot;
  // the physical sensors are evaluated again after the boot grace period.
  intrusion_alert = false;
  sabotage_alert = false;
  critical_security_compromise = false;
  device_tampered = false;
  sosActive = false;
  bootCompletedAtMs = millis();

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

  alarm_status = "Đang khởi động thiết bị";
  last_event_type = "system_boot";
  last_event = "Thiết bị vừa khởi động lúc " + getRtcTimeString() + ". Hệ thống đang kiểm tra cảm biến, camera và kết nối cloud.";
  current_time = getRtcTimeString();

  setEscalationStatus("IDLE");
  notification_channel = "telegram";
  notification_event_type = "NONE";
  notification_sent_status = "Chưa có cảnh báo cần gửi";
  photo_status = "Camera chưa có yêu cầu chụp";
  sos_message = "Chưa có SOS";
  last_unknown_mac = "NONE";
  last_alert_time = "Chưa có cảnh báo";
  heartbeat_status = "Chưa gửi heartbeat";
  last_heartbeat_time = "Chưa có heartbeat";
  device_health_status = "Thiết bị hoạt động bình thường";
  critical_security_compromise = false;

  // New SRS v0.6.7 Cloud variables: SOS authority escalation + BLE trusted device status.
  // READWRITE values may be overwritten by Arduino Cloud after connection.
  home_address_configured = false;
  sos_authority_note = "";
  setAuthorityStatus("IDLE");

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
    setLastEvent("camera_ready", "Camera đã sẵn sàng lúc " + getRtcTimeString() + ".");
  } else {
    Serial.println("[CAM] Camera NOT ready.");
    setLastEvent("camera_init_failed", "Camera chưa sẵn sàng lúc " + getRtcTimeString() + ". Hệ thống vẫn giữ còi, LED và cảnh báo chữ.");
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
  processGoogleAppsScriptHeartbeat();

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
  setLastEvent(
    "cloud_alarm_enabled",
    alarm_enabled
      ? "Phụ huynh/Admin đã bật chức năng phát hiện đột nhập."
      : "Phụ huynh/Admin đã tắt chức năng phát hiện đột nhập. SOS và chống phá hoại vẫn hoạt động."
  );
}

void onResetAlarmChange() {
  if (reset_alarm) {
    resetAllAlerts();
    reset_alarm = false;
  }
}

void onSosChildChange() {
  if (sos_child) {
    if (!demo_mode || activeDemoScenario == 0 || activeDemoScenario == 6) {
      triggerSosAlert("CHILD");
    } else {
      setLastEvent("demo_blocked_sos", "Nút SOS bị chặn vì chế độ demo đang chọn kịch bản khác.");
    }
    sos_child = false;
  }
}

void onSosAdultChange() {
  if (sos_adult) {
    if (!demo_mode || activeDemoScenario == 0 || activeDemoScenario == 6) {
      triggerSosAlert("PARENT_ADMIN");
    } else {
      setLastEvent("demo_blocked_sos", "Nút SOS bị chặn vì chế độ demo đang chọn kịch bản khác.");
    }
    sos_adult = false;
  }
}

void onSensitivityLevelChange() {
  clampCloudConfigValues();
  setLastEvent("cloud_sensitivity", "Đã cập nhật mức nhạy phát hiện đột nhập: " + String(sensitivity_level) + ".");
}

void onDemoModeChange() {
  if (!demo_mode) {
    activeDemoScenario = 0;
  }
  setLastEvent("cloud_demo_mode", demo_mode ? "Đã bật chế độ demo có kiểm soát." : "Đã tắt chế độ demo.");
}

void onDemoScenarioChange() {
  applyDemoScenario();
}

void onManualCapturePhotoChange() {
  if (manual_capture_photo) {
    if (!demo_mode || activeDemoScenario == 0 || activeDemoScenario == 5) {
      // Dashboard buttons can return to false before the next camera-processing
      // cycle, so retain the command independently of the Cloud property.
      manualCapturePending = true;
      manual_capture_photo = false;
      photo_status = "Đã nhận yêu cầu chụp ảnh thủ công";
      setLastEvent("manual_capture_requested", "Phụ huynh/Admin yêu cầu chụp ảnh thủ công lúc " + getRtcTimeString() + ".");
    } else {
      manualCapturePending = false;
      manual_capture_photo = false;
      photo_status = "Yêu cầu chụp ảnh bị chặn bởi chế độ demo";
      setLastEvent("demo_blocked_manual_photo", "Yêu cầu chụp ảnh bị chặn vì chế độ demo đang chọn kịch bản khác.");
    }
  }
}

void onRearmDelaySecondsChange() {
  clampCloudConfigValues();
}

void onScheduleEnabledChange() {
  setLastEvent("cloud_schedule", schedule_enabled ? "Đã bật lịch tự động chống trộm." : "Đã tắt lịch tự động chống trộm.");
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
  setLastEvent("cloud_unknown_wifi", "Biến WiFi/MAC lạ là legacy trong phạm vi demo mới và không dùng cho cảnh báo chính.");
}

void onKnownDevicePresentChange() {
  updateTrustedDeviceStatusPlaceholders();
  setLastEvent("cloud_known_device", "Biến thiết bị đáng tin là legacy trong phạm vi demo mới và không dùng để tự tắt bảo vệ.");
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
    "BLE trusted phone là legacy trong phạm vi demo mới và không dùng cho kịch bản chính."
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
