#include "arduino_secrets.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "thingProperties.h"

// ==================================================
// 1. PIN MAPPING - Theo mạch bạn đã test pass
// ==================================================
#define PIR_PIN       13
#define LDR_DO_PIN    14
#define FLAME_DO_PIN  15
#define LED_RED_PIN   2
#define BUZZER_PIN    12

// ==================================================
// 2. LOGIC MODULE THẬT CỦA BẠN
// ==================================================
// PIR: không chuyển động = LOW, có chuyển động = HIGH
// LDR: ngoài sáng = LOW, che tối = HIGH
// Flame: không có lửa = HIGH, có lửa = LOW
#define PIR_ACTIVE_STATE       HIGH
#define LDR_COVERED_STATE      HIGH
#define FLAME_ACTIVE_STATE     LOW

// ==================================================
// 3. DEMO CONFIG
// ==================================================
// Buzzer của bạn là passive buzzer, phải dùng tone()
#define BUZZER_FREQUENCY 2000

// Nếu chưa có cảm biến nhiệt độ thật, để true để mô phỏng nhiệt độ khi demo cháy.
// Khi thuyết trình phải nói rõ: "nhiệt độ đang mô phỏng vì chưa gắn cảm biến nhiệt riêng".
const bool DEMO_SIMULATE_TEMPERATURE = true;

// Bật/tắt gửi Gmail và Telegram
const bool ENABLE_GMAIL = true;
const bool ENABLE_TELEGRAM = true;

// Chống spam gửi cảnh báo quá nhanh
const unsigned long ALERT_COOLDOWN_MS = 15000;

// Cửa sổ đếm số lần bấm SOS trẻ em
const unsigned long SOS_PRESS_WINDOW_MS = 3000;

// Chu kỳ đọc cảm biến
const unsigned long SENSOR_READ_INTERVAL_MS = 300;

// Chu kỳ nháy LED + buzzer
const unsigned long ALARM_BLINK_INTERVAL_MS = 300;

// ==================================================
// 4. BIẾN NỘI BỘ
// ==================================================
unsigned long lastSensorReadMs = 0;
unsigned long lastBlinkMs = 0;
unsigned long lastAlertSentMs = 0;

bool alarmLatched = false;       // Latched = giữ báo động cho tới khi bấm reset
bool physicalAlarmState = false; // trạng thái nháy thực tế LED/buzzer

bool fireEmailSent = false;
bool sabotageEmailSent = false;
bool intrusionEmailSent = false;
bool sosEmailSent = false;

int sosChildPressCount = 0;
int sosAdultPressCount = 0;
unsigned long lastSosChildPressMs = 0;
unsigned long lastSosAdultPressMs = 0;
bool pendingSosChild = false;
bool pendingSosAdult = false;

// ==================================================
// 5. HÀM ĐỌC CẢM BIẾN
// ==================================================
bool readPirDetected() {
  return digitalRead(PIR_PIN) == PIR_ACTIVE_STATE;
}

bool readLdrCovered() {
  return digitalRead(LDR_DO_PIN) == LDR_COVERED_STATE;
}

bool readFlameDetected() {
  return digitalRead(FLAME_DO_PIN) == FLAME_ACTIVE_STATE;
}

// ==================================================
// 6. OUTPUT LED + BUZZER
// ==================================================
void stopPhysicalAlarm() {
  digitalWrite(LED_RED_PIN, LOW);
  noTone(BUZZER_PIN);
  physicalAlarmState = false;
}

void updatePhysicalAlarm() {
  if (!alarmLatched) {
    stopPhysicalAlarm();
    led_red_on = false;
    buzzer_on = false;
    return;
  }

  // Dashboard chỉ cần biết báo động đang bật, không cần nhấp nháy theo từng nhịp
  led_red_on = true;
  buzzer_on = true;

  if (millis() - lastBlinkMs >= ALARM_BLINK_INTERVAL_MS) {
    lastBlinkMs = millis();
    physicalAlarmState = !physicalAlarmState;

    digitalWrite(LED_RED_PIN, physicalAlarmState ? HIGH : LOW);

    if (physicalAlarmState) {
      tone(BUZZER_PIN, BUZZER_FREQUENCY);
    } else {
      noTone(BUZZER_PIN);
    }
  }
}

// ==================================================
// 7. URL ENCODE - mã hóa tham số URL
// ==================================================
// URL encode = biến khoảng trắng/ký tự đặc biệt thành dạng an toàn khi gửi HTTP
String urlEncode(String value) {
  String encoded = "";

  for (int i = 0; i < value.length(); i++) {
    char c = value.charAt(i);

    if (
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '-' || c == '_' || c == '.' || c == '~'
    ) {
      encoded += c;
    } else if (c == ' ') {
      encoded += "%20";
    } else {
      char hex[4];
      sprintf(hex, "%%%02X", (unsigned char)c);
      encoded += hex;
    }
  }

  return encoded;
}

// ==================================================
// 8. GỬI GMAIL QUA GOOGLE APPS SCRIPT
// ==================================================
bool sendGoogleAlert(String type, int level, String eventId) {
  if (!ENABLE_GMAIL) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[GMAIL] WiFi not connected");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  String url = String(SECRET_GOOGLE_SCRIPT_URL);
  url += "?type=" + urlEncode(type);
  url += "&target=family";
  url += "&level=" + String(level);
  url += "&eventId=" + urlEncode(eventId);

  // Tham số bổ sung để email nhìn rõ hơn
  url += "&flame=" + String(flame_detected ? "1" : "0");
  url += "&temp=" + String(kitchen_temperature, 1);
  url += "&ldr=" + String(ldr_value);
  url += "&pir=" + String(pir_detected ? "1" : "0");

  Serial.println("[GMAIL] Request URL:");
  Serial.println(url);

  if (!http.begin(client, url)) {
    Serial.println("[GMAIL] http.begin failed");
    return false;
  }

  int httpCode = http.GET();
  String payload = http.getString();

  Serial.print("[GMAIL] HTTP code = ");
  Serial.println(httpCode);
  Serial.print("[GMAIL] Response = ");
  Serial.println(payload);

  http.end();

  return httpCode >= 200 && httpCode < 400;
}

// ==================================================
// 9. GỬI TELEGRAM
// ==================================================
bool sendTelegramAlert(String message) {
  if (!ENABLE_TELEGRAM) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[TELEGRAM] WiFi not connected");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  String url = "https://api.telegram.org/bot";
  url += String(SECRET_TELEGRAM_BOT_TOKEN);
  url += "/sendMessage?chat_id=";
  url += urlEncode(String(SECRET_TELEGRAM_CHAT_ID));
  url += "&text=";
  url += urlEncode(message);

  Serial.println("[TELEGRAM] Sending message...");

  if (!http.begin(client, url)) {
    Serial.println("[TELEGRAM] http.begin failed");
    return false;
  }

  int httpCode = http.GET();
  String payload = http.getString();

  Serial.print("[TELEGRAM] HTTP code = ");
  Serial.println(httpCode);
  Serial.print("[TELEGRAM] Response = ");
  Serial.println(payload);

  http.end();

  return httpCode >= 200 && httpCode < 400;
}

// ==================================================
// 10. GỬI CẢNH BÁO CHUNG
// ==================================================
void sendAlertNotification(String type, int level, String message) {
  if (millis() - lastAlertSentMs < ALERT_COOLDOWN_MS) {
    Serial.println("[ALERT] Cooldown active, skip sending");
    return;
  }

  lastAlertSentMs = millis();

  String eventId = type + "_" + String(event_counter) + "_" + String(millis());

  send_email_request = true;
  email_event_type = type;
  email_sent_status = "EMAIL_SENDING";

  ArduinoCloud.update();

  bool gmailOk = sendGoogleAlert(type, level, eventId);
  bool telegramOk = sendTelegramAlert(message);

  if (gmailOk && telegramOk) {
    email_sent_status = "EMAIL_SENT";
  } else if (gmailOk && !telegramOk) {
    email_sent_status = "EMAIL_SENT_TELEGRAM_FAILED";
  } else if (!gmailOk && telegramOk) {
    email_sent_status = "EMAIL_FAILED_TELEGRAM_SENT";
  } else {
    email_sent_status = "EMAIL_FAILED";
  }

  send_email_request = false;

  ArduinoCloud.update();
}

// ==================================================
// 11. LOG EVENT
// ==================================================
void addEvent(String type, String message) {
  event_counter++;
  last_event_type = type;
  last_event = message;

  Serial.print("[EVENT] ");
  Serial.print(type);
  Serial.print(" | ");
  Serial.println(message);
}

// ==================================================
// 12. TRIGGER CÁC KỊCH BẢN
// ==================================================
void triggerSos(bool isChild, int level) {
  alarmLatched = true;

  sos_level = level;

  if (isChild) {
    alarm_status = "SOS_CHILD_ALERT";
    sos_message = "SOS child level " + String(level);
    threat_level = level >= 3 ? 4 : 3;

    addEvent("SOS_CHILD", "SOS child level " + String(level) + " triggered from Arduino Cloud");

    if (level >= 2 && !sosEmailSent) {
      sosEmailSent = true;

      sendAlertNotification(
        "sos_child",
        level,
        "[Kitchen Security] SOS child level " + String(level) + " tai khu vuc bep"
      );
    }

  } else {
    alarm_status = "SOS_ADULT_ALERT";
    sos_message = "SOS adult level " + String(level);
    threat_level = level >= 3 ? 4 : 3;

    addEvent("SOS_ADULT", "SOS adult level " + String(level) + " triggered from Arduino Cloud");

    if (level >= 2 && !sosEmailSent) {
      sosEmailSent = true;

      sendAlertNotification(
        "sos_adult",
        level,
        "[Kitchen Security] SOS adult level " + String(level) + " tai khu vuc bep"
      );
    }
  }
}

void triggerFireAlert() {
  alarmLatched = true;

  fire_alert = true;
  flame_detected = true;
  flame_value = 200;
  threat_level = 4;
  alarm_status = "FIRE_ALERT";

  addEvent("FIRE", "Fire alert detected by Flame Sensor");

  if (!fireEmailSent) {
    fireEmailSent = true;

    sendAlertNotification(
      "fire",
      4,
      "[Kitchen Security] FIRE ALERT tai khu vuc bep"
    );
  }
}

void triggerSabotageAlert() {
  alarmLatched = true;

  sabotage_alert = true;
  threat_level = 3;
  alarm_status = "SABOTAGE_ALERT";

  addEvent("SABOTAGE", "LDR covered and PIR detected movement");

  if (!sabotageEmailSent) {
    sabotageEmailSent = true;

    sendAlertNotification(
      "sabotage",
      3,
      "[Kitchen Security] SABOTAGE ALERT: LDR bi che va PIR phat hien chuyen dong"
    );
  }
}

void triggerIntrusionDemo() {
  alarmLatched = true;

  intrusion_alert = true;
  threat_level = 3;
  alarm_status = "INTRUSION_ALERT";

  addEvent("INTRUSION", "Demo intrusion alert triggered");

  if (!intrusionEmailSent) {
    intrusionEmailSent = true;

    sendAlertNotification(
      "intrusion",
      3,
      "[Kitchen Security] INTRUSION ALERT demo"
    );
  }
}

// ==================================================
// 13. RESET HỆ THỐNG
// ==================================================
void resetSystem() {
  Serial.println("[RESET] Reset alarm");

  alarmLatched = false;
  stopPhysicalAlarm();

  fire_alert = false;
  sabotage_alert = false;
  intrusion_alert = false;

  threat_level = 0;
  sos_level = 0;
  sos_message = "";

  led_red_on = false;
  buzzer_on = false;

  send_email_request = false;
  email_event_type = "";
  email_sent_status = "EMAIL_IDLE";

  photo_status = "PHOTO_IDLE";

  fireEmailSent = false;
  sabotageEmailSent = false;
  intrusionEmailSent = false;
  sosEmailSent = false;

  pendingSosChild = false;
  pendingSosAdult = false;
  sosChildPressCount = 0;
  sosAdultPressCount = 0;

  reset_alarm = false;
  sos_child = false;
  sos_adult = false;

  alarm_status = system_armed ? "ARMED" : "SAFE";

  addEvent("RESET", "Alarm reset by dashboard");
}

// ==================================================
// 14. ĐỌC CẢM BIẾN VÀ XỬ LÝ LOGIC
// ==================================================
void readSensorsAndProcess() {
  if (millis() - lastSensorReadMs < SENSOR_READ_INTERVAL_MS) {
    return;
  }

  lastSensorReadMs = millis();

  bool pirNow = readPirDetected();
  bool ldrCoveredNow = readLdrCovered();
  bool flameNow = readFlameDetected();

  pir_detected = pirNow;
  flame_detected = flameNow;

  // Vì LDR và Flame hiện dùng DO, không có AO thật.
  // Nên giá trị này là quy ước để dashboard Gauge dễ nhìn.
  ldr_value = ldrCoveredNow ? 100 : 900;
  flame_value = flameNow ? 200 : 900;

  if (DEMO_SIMULATE_TEMPERATURE) {
    kitchen_temperature = flameNow ? 65.0 : 31.0;
  } else {
    kitchen_temperature = 0.0;
  }

  // Trong bản demo này, system_armed đi theo alarm_enabled
  system_armed = alarm_enabled;

  bool sabotageNow = system_armed && ldrCoveredNow && pirNow;

  if (flameNow && !fire_alert) {
    triggerFireAlert();
  }

  if (sabotageNow && !sabotage_alert) {
    triggerSabotageAlert();
  }

  if (!alarmLatched) {
    if (system_armed) {
      alarm_status = "ARMED";
    } else {
      alarm_status = "SAFE";
    }

    threat_level = 0;
    fire_alert = false;
    sabotage_alert = false;
    intrusion_alert = false;
  }

  Serial.print("[SENSOR] PIR=");
  Serial.print(pirNow ? "TRUE" : "FALSE");
  Serial.print(" | LDR_COVERED=");
  Serial.print(ldrCoveredNow ? "TRUE" : "FALSE");
  Serial.print(" | FLAME=");
  Serial.print(flameNow ? "TRUE" : "FALSE");
  Serial.print(" | alarm_status=");
  Serial.println(alarm_status);
}

// ==================================================
// 15. XỬ LÝ SOS BẤM NHIỀU LẦN
// ==================================================
void processPendingSos() {
  if (pendingSosChild && millis() - lastSosChildPressMs >= SOS_PRESS_WINDOW_MS) {
    int level = sosChildPressCount;

    if (level < 1) level = 1;
    if (level > 3) level = 3;

    pendingSosChild = false;
    sosChildPressCount = 0;

    triggerSos(true, level);
  }

  if (pendingSosAdult && millis() - lastSosAdultPressMs >= SOS_PRESS_WINDOW_MS) {
    int level = sosAdultPressCount;

    if (level < 1) level = 1;
    if (level > 3) level = 3;

    pendingSosAdult = false;
    sosAdultPressCount = 0;

    triggerSos(false, level);
  }
}

// ==================================================
// 16. KHỞI TẠO GIÁ TRỊ CLOUD
// ==================================================
void initCloudValues() {
  alarm_enabled = true;
  system_armed = true;

  alarm_status = "ARMED";
  email_event_type = "";
  email_sent_status = "EMAIL_IDLE";
  last_event = "System booted";
  last_event_type = "BOOT";
  photo_status = "PHOTO_IDLE";
  sos_message = "";

  kitchen_temperature = DEMO_SIMULATE_TEMPERATURE ? 31.0 : 0.0;
  ultrasonic_distance = 0.0;

  demo_scenario = 0;
  event_counter = 0;
  flame_value = 900;
  ldr_value = 900;
  sos_level = 0;
  threat_level = 0;
  unknown_wifi_count = 0;

  access_status = false;
  buzzer_on = false;
  capture_photo = false;
  demo_mode = false;
  fire_alert = false;
  flame_detected = false;
  intrusion_alert = false;
  known_device_present = false;
  led_red_on = false;
  pet_detected = false;
  pir_detected = false;
  reset_alarm = false;
  rfid_authorized = false;
  sabotage_alert = false;
  send_email_request = false;
  sos_adult = false;
  sos_child = false;
  unknown_wifi_alert = false;
}

// ==================================================
// 17. SETUP + LOOP
// ==================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("====================================");
  Serial.println(" Kitchen Security System - Group 6");
  Serial.println(" ESP32-CAM Arduino Cloud Demo");
  Serial.println("====================================");

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_DO_PIN, INPUT);
  pinMode(FLAME_DO_PIN, INPUT);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  stopPhysicalAlarm();

  initProperties();
  initCloudValues();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("[SETUP] Done. Waiting for Arduino Cloud connection...");
}

void loop() {
  ArduinoCloud.update();

  readSensorsAndProcess();
  processPendingSos();
  updatePhysicalAlarm();
}

// ==================================================
// 18. CALLBACK TỪ ARDUINO CLOUD
// ==================================================
void onAlarmEnabledChange() {
  system_armed = alarm_enabled;

  if (!alarmLatched) {
    alarm_status = system_armed ? "ARMED" : "SAFE";
  }

  addEvent("ALARM_ENABLED", alarm_enabled ? "Alarm enabled from dashboard" : "Alarm disabled from dashboard");
}

void onResetAlarmChange() {
  if (reset_alarm) {
    resetSystem();
  }
}

void onSosChildChange() {
  if (sos_child) {
    sosChildPressCount++;
    lastSosChildPressMs = millis();
    pendingSosChild = true;

    sos_message = "SOS child press received. Count = " + String(sosChildPressCount);
    alarm_status = "SOS_INPUT_WAITING";

    Serial.print("[SOS CHILD] Press count = ");
    Serial.println(sosChildPressCount);

    // Tự nhả nút Cloud để lần sau bấm tiếp được
    sos_child = false;
  }
}

void onSosAdultChange() {
  if (sos_adult) {
    sosAdultPressCount++;
    lastSosAdultPressMs = millis();
    pendingSosAdult = true;

    sos_message = "SOS adult press received. Count = " + String(sosAdultPressCount);
    alarm_status = "SOS_INPUT_WAITING";

    Serial.print("[SOS ADULT] Press count = ");
    Serial.println(sosAdultPressCount);

    // Tự nhả nút Cloud để lần sau bấm tiếp được
    sos_adult = false;
  }
}

void onDemoModeChange() {
  addEvent("DEMO_MODE", demo_mode ? "Demo mode enabled" : "Demo mode disabled");
}

void onDemoScenarioChange() {
  if (!demo_mode) {
    Serial.println("[DEMO] demo_mode is false, ignore demo_scenario");
    return;
  }

  Serial.print("[DEMO] Scenario = ");
  Serial.println(demo_scenario);

  if (demo_scenario == 0) {
    resetSystem();
  } else if (demo_scenario == 1) {
    triggerIntrusionDemo();
  } else if (demo_scenario == 6) {
    triggerSabotageAlert();
  } else if (demo_scenario == 7) {
    triggerFireAlert();
  } else if (demo_scenario == 8) {
    triggerSos(true, 3);
  } else {
    addEvent("DEMO", "Unknown demo scenario: " + String(demo_scenario));
  }
}

void onCapturePhotoChange() {
  if (capture_photo) {
    photo_status = "PHOTO_NOT_IMPLEMENTED_IN_THIS_DEMO";
    addEvent("PHOTO", "Capture photo requested but camera capture is not implemented");
    capture_photo = false;
  }
}