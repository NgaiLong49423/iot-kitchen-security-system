  #include "arduino_secrets.h"
  #include "thingProperties.h"

  #include <Wire.h>
  #include "RTClib.h"
  #include "esp_camera.h"
  #include <WiFi.h>
  #include <WiFiClientSecure.h>

  // ==================================================
  // GIAI ĐOẠN HOÀN THIỆN - CLOUD + CAMERA + TELEGRAM + GOOGLE SCRIPT
  // Bo mạch: Freenove ESP32-S3 WROOM + camera OV3660.
  // Phạm vi v3: hai Dashboard người dùng, xử lý an ninh cục bộ, camera,
  // Telegram, Google Apps Script và tự phục hồi kết nối. Bản dựng này chủ ý
  // không chứa BLE, quét Wi-Fi/MAC hoặc các kịch bản giả lập.
  // ==================================================


  // =======================
  // GIÁ TRỊ DỰ PHÒNG CHO API NGOÀI
  // Giá trị thật phải đặt trong arduino_secrets.h, không ghi cứng bí mật tại đây.
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

  #ifndef SECRET_GEMINI_API_KEY
  #define SECRET_GEMINI_API_KEY ""
  #endif

  // =======================
  // SƠ ĐỒ CHÂN NGOẠI VI TƯƠNG THÍCH VỚI CAMERA
  // =======================
  #define PIN_LDR_AO       1   // Chân ADC đọc điện áp từ cảm biến ánh sáng LDR.
  #define PIN_PIR_OUT      40  // Chân số đọc tín hiệu phát hiện chuyển động từ PIR.

  #define PIN_RTC_SDA      41  // Chân dữ liệu SDA của mô-đun thời gian thực DS1307.
  #define PIN_RTC_SCL      42  // Chân xung nhịp SCL của mô-đun DS1307.

  #define PIN_LED_RED      14  // Chân điều khiển LED đỏ báo nguy hiểm.
  #define PIN_LED_GREEN    21  // Chân điều khiển LED xanh báo trạng thái bình thường.
  #define PIN_BUZZER       47  // Chân điều khiển còi cảnh báo.

  #define PIN_US_TRIG      38  // Chân phát xung đo khoảng cách của HY-SRF05.
  #define PIN_US_ECHO      39  // Chân nhận độ rộng xung phản hồi của HY-SRF05.

  // =======================
  // SƠ ĐỒ CHÂN CAMERA FREENOVE ESP32-S3 WROOM + OV3660
  // Các GPIO bên dưới dành riêng cho camera, không dùng cho mô-đun ngoài.
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

  #define FORCE_SET_RTC_TIME_ONCE false // true: ghi thời gian biên dịch vào RTC một lần khi khởi động.

  // =======================
  // CẤU HÌNH CÒI
  // =======================
  // false: còi chủ động chỉ cần HIGH/LOW; true: còi thụ động cần phát tone().
  const bool BUZZER_USE_TONE = false; // Lưu loại còi để chọn cách điều khiển phù hợp.
  const int BUZZER_TONE_HZ = 2500;    // Lưu tần số âm thanh 2.500 Hz khi dùng còi thụ động.

  // =======================
  // CẤU HÌNH AN NINH
  // =======================
  const float OBJECT_NEAR_THRESHOLD_CM = 50.0;      // Ngưỡng khoảng cách để coi có vật/người ở gần.
  const float OBJECT_TOO_CLOSE_THRESHOLD_CM = 15.0; // Ngưỡng khoảng cách rất gần, làm tăng điểm xâm nhập.

  // Giữ true nếu che LDR làm giá trị analog tăng.
  // Đổi thành false nếu che LDR làm giá trị analog giảm.
  const bool LDR_COVER_WHEN_HIGH = true;          // Cho biết che LDR làm giá trị ADC tăng hay giảm.
  const int LDR_COVERED_THRESHOLD_HIGH = 2000;    // Ngưỡng xác định LDR bị che khi tín hiệu tăng.
  const int LDR_COVERED_THRESHOLD_LOW = 500;      // Ngưỡng xác định LDR bị che khi tín hiệu giảm.

  const int LDR_DELTA_ABNORMAL_THRESHOLD = 500;    // Độ biến thiên ánh sáng tối thiểu bị coi là bất thường.
  const unsigned long SABOTAGE_HOLD_MS = 3000;     // Thời gian điều kiện phá hoại phải kéo dài trước khi báo động.
  const unsigned long INTRUSION_HOLD_MS = 2000;    // Thời gian điểm xâm nhập phải duy trì trước khi báo động.

const unsigned long SENSOR_UPDATE_INTERVAL_MS = 500;       // Chu kỳ đọc cảm biến và cập nhật logic.
const unsigned long SERIAL_REPORT_INTERVAL_MS = 2000;      // Chu kỳ in báo cáo chẩn đoán ra Serial.
const unsigned long RED_BLINK_INTERVAL_MS = 250;           // Chu kỳ đảo trạng thái LED đỏ khi có cảnh báo.
const unsigned long GOOGLE_HEARTBEAT_INTERVAL_MS = 10000;  // Khoảng cách giữa hai heartbeat gửi Apps Script.
const unsigned long GOOGLE_HTTP_TIMEOUT_MS = 5000;         // Thời gian tối đa chờ một yêu cầu HTTP đến Apps Script.
const unsigned long SENSOR_BOOT_GRACE_MS = 3000;           // Khoảng bỏ qua cảm biến để phần cứng ổn định sau khởi động.
const unsigned long GEMINI_HTTP_TIMEOUT_MS = 8000;         // Thời gian tối đa chờ phản hồi Gemini.
const unsigned long TELEGRAM_TEXT_TIMEOUT_MS = 5000;       // Thời gian tối đa chờ gửi tin nhắn Telegram.
const unsigned long TELEGRAM_PHOTO_TIMEOUT_MS = 10000;     // Thời gian tối đa chờ gửi ảnh Telegram.
const unsigned long WIFI_RETRY_DELAYS_MS[] = {2000, 5000, 10000, 30000}; // Các mức chờ tăng dần giữa lần nối lại Wi-Fi.
const uint8_t AI_PERSON_FOLLOW_UP_PHOTO_COUNT = 3;         // Số ảnh kiểm tra bổ sung sau khi AI thấy người.
const unsigned long AI_PERSON_FOLLOW_UP_INTERVAL_MS = 1000; // Khoảng cách giữa các ảnh kiểm tra bổ sung.

  const char *GEMINI_API_HOST = "generativelanguage.googleapis.com"; // Tên máy chủ Gemini API.
  const char *GEMINI_PERSON_MODEL = "gemini-3.5-flash";              // Tên mô hình dùng để phân loại có/người.

  RTC_DS1307 rtc;                              // Đối tượng giao tiếp với đồng hồ thời gian thực DS1307.
  bool rtcOk = false;                          // Lưu kết quả khởi tạo RTC.
  bool cameraReady = false;                    // Cho biết driver và cảm biến camera đã sẵn sàng.
  unsigned long lastCameraCaptureMs = 0;       // Lưu mốc millis() của lần chụp gần nhất để chống chụp quá dày.
bool manualCapturePending = false;             // Chốt yêu cầu chụp thủ công đang chờ xử lý trong vòng lặp.
String lastAiPersonResult = "AI chưa chạy";    // Lưu kết quả phân loại người gần nhất từ Gemini.
uint8_t aiFollowUpPhotosRemaining = 0;         // Lưu số ảnh kiểm tra AI bổ sung còn phải chụp.
unsigned long nextAiFollowUpPhotoAtMs = 0;     // Lưu mốc millis() sớm nhất được chụp ảnh AI bổ sung tiếp theo.

  int lastLdrValue = -1;                       // Lưu mẫu LDR trước để tính mức thay đổi giữa hai lần đọc.
  unsigned long sabotageConditionStartedAt = 0; // Lưu lúc điều kiện phá hoại bắt đầu duy trì.

bool sosActive = false;                        // Chốt cho biết một cảnh báo SOS đang hoạt động.
String sosSource = "NONE";                     // Lưu nguồn kích hoạt SOS: trẻ em, người lớn hoặc lệnh Serial.

  int rawIntrusionScore = 0;                   // Lưu điểm xâm nhập thô trước khi áp dụng các điều kiện hệ thống.
  int lastIntrusionScoreAtTrigger = 0;         // Lưu điểm công bố tại thời điểm cảnh báo gần nhất kích hoạt.
  int lastRawIntrusionScoreAtTrigger = 0;      // Lưu điểm thô tại thời điểm cảnh báo gần nhất kích hoạt.
  String lastIntrusionReason = "NONE";         // Lưu mô tả các tín hiệu đã góp phần tạo cảnh báo.
  unsigned long intrusionConditionStartedAt = 0; // Lưu lúc điểm xâm nhập bắt đầu vượt ngưỡng.

  String lastScheduleTriggerKey = "";          // Lưu khóa ngày-phút đã chạy để lịch không kích hoạt lặp trong cùng phút.

  unsigned long lastGoogleScriptCallMs = 0;    // Lưu mốc lần gọi Apps Script gần nhất để giới hạn tần suất.
  String lastGoogleScriptEventKey = "";        // Lưu khóa sự kiện gần nhất nhằm tránh gửi trùng.
  String activeGoogleEventId = "";             // Lưu ID sự kiện SOS/phá hoại đang được Apps Script theo dõi.
unsigned long bootCompletedAtMs = 0;           // Lưu mốc khởi động để tính thời gian chờ ổn định cảm biến.

// Các giá trị chỉ dùng lúc chạy. Chúng được giữ cục bộ để dữ liệu cảm biến thô
// và dữ liệu debug không còn đồng bộ lên Arduino Cloud.
String last_event_type = "";                   // Mã máy đọc được của sự kiện cục bộ mới nhất.
String notification_channel = "";             // Kênh thông báo đang được chọn, ví dụ Telegram.
String notification_event_type = "NONE";      // Loại sự kiện mà yêu cầu thông báo hiện tại đại diện.
String sos_message = "";                       // Nội dung SOS gần nhất được hiển thị/gửi đi.
int event_counter = 0;                         // Tổng số sự kiện quan trọng đã ghi nhận từ lúc khởi động.
int ldr_value = 0;                             // Mẫu ADC hiện tại của cảm biến ánh sáng.
int ldr_delta = 0;                             // Độ chênh tuyệt đối giữa mẫu LDR hiện tại và mẫu trước.
float ultrasonic_distance = -1.0f;             // Khoảng cách siêu âm hiện tại; số âm nghĩa là đọc lỗi.
int current_hour = -1;                         // Giờ hiện tại từ RTC; -1 nghĩa là chưa có giờ hợp lệ.
int intrusion_score = 0;                       // Điểm xâm nhập đã xử lý để công bố và so ngưỡng.
int threat_level = 0;                          // Mức đe dọa tổng hợp từ 0 đến 4.
bool pir_detected = false;                     // Kết quả phát hiện chuyển động PIR gần nhất.
bool object_near = false;                      // Cho biết cảm biến siêu âm đang thấy vật ở gần.
bool pet_detected = false;                     // Cờ suy luận chuyển động có thể do vật nuôi.
bool light_abnormal = false;                   // Cho biết ánh sáng thay đổi đột ngột vượt ngưỡng.
bool ldr_covered = false;                      // Cho biết LDR có dấu hiệu bị che.
bool night_mode = false;                       // Cho biết thời gian RTC hiện thuộc khung giờ ban đêm.
bool device_tampered = false;                  // Cho biết thiết bị/cảm biến có dấu hiệu bị can thiệp.
bool auto_capture_photo_request = false;       // Yêu cầu nội bộ chụp ảnh tự động cho cảnh báo mới.
bool send_notification_request = false;        // Cờ tương thích biểu diễn có yêu cầu gửi thông báo.
bool buzzer_on = false;                        // Lưu trạng thái logic hiện tại của còi.
bool led_red_on = false;                       // Lưu trạng thái logic hiện tại của LED đỏ.
bool led_green_on = false;                     // Lưu trạng thái logic hiện tại của LED xanh.
bool cooldown_active = false;                  // Cho biết hệ thống đang ở giai đoạn hạn chế thao tác lặp.
// Mỗi cảnh báo đột nhập/phá hoại mới chỉ được gửi một email AI phát hiện người.
// Quy tắc này ngăn chuỗi ảnh bằng chứng bổ sung gây spam Gmail.
bool aiPersonEmailSentForCurrentAlert = false; // Chốt đã gửi email AI cho cảnh báo hiện tại để chống spam.

// Thay đổi giám sát heartbeat từ Dashboard bắt buộc phải được chuyển đến nơi.
// Request được giữ trong hàng đợi đến khi Apps Script xác nhận; heartbeat thường
// sẽ tạm dừng trong thời gian chờ xác nhận này.
bool heartbeatMonitorControlPending = false;           // Cho biết lệnh bật/tắt giám sát heartbeat còn chờ xác nhận.
bool desiredHeartbeatMonitorEnabled = true;            // Trạng thái giám sát heartbeat mà Dashboard mong muốn.
unsigned long heartbeatMonitorControlNextRetryAtMs = 0; // Mốc sớm nhất được thử gửi lại lệnh điều khiển.
uint8_t heartbeatMonitorControlRetryIndex = 0;          // Chỉ số mức trì hoãn Wi-Fi dùng cho lần thử lại hiện tại.

  // Gói ảnh chụp đồng nhất của tất cả cảm biến trong một chu kỳ xử lý.
  // Việc truyền cùng một snapshot giúp tính điểm và cập nhật Cloud không đọc
  // các cảm biến ở những thời điểm khác nhau.
  struct HardwareSnapshot {
    int ldrValue;          // Giá trị ADC của LDR trong chu kỳ hiện tại.
    int ldrDelta;          // Độ thay đổi LDR so với chu kỳ trước.
    bool pirDetected;      // Kết quả chuyển động PIR trong chu kỳ hiện tại.
    float distanceCm;      // Khoảng cách đo được, đơn vị centimet.
    bool ultrasonicOk;     // Cho biết phép đo siêu âm có hợp lệ.
    bool objectNear;       // Cho biết có vật nằm trong ngưỡng gần.
    bool objectTooClose;   // Cho biết có vật nằm trong ngưỡng rất gần.
    bool lightAbnormal;    // Cho biết thay đổi ánh sáng vượt ngưỡng bất thường.
    bool ldrCovered;       // Cho biết giá trị LDR phù hợp dấu hiệu bị che.
    bool rtcOk;            // Cho biết thời gian RTC trong snapshot hợp lệ.
    int hour;              // Giờ RTC tại thời điểm chụp snapshot.
    int minute;            // Phút RTC tại thời điểm chụp snapshot.
    bool nightMode;        // Cho biết snapshot được ghi trong khung giờ ban đêm.
    String timeText;       // Chuỗi ngày giờ hoàn chỉnh dùng cho log và thông báo.
  };


  // ==================================================
  // KHAI BÁO TRƯỚC CÁC HÀM
  // Giúp sketch biên dịch ổn định ngay cả khi bước tiền xử lý Arduino thay đổi.
  // ==================================================
  String twoDigits(int value);
  String getRtcTimeString();
  void setLastEvent(const String &type, const String &message);
  void incrementEventCounter();
  String urlEncode(const String &value);
  String responseValue(const String &response, const String &key);
  bool hasTelegramConfig();
  bool hasGoogleScriptConfig();
  bool hasGeminiConfig();
  String buildCommonCaption(const String &eventType, const String &reason);
  bool sendTelegramMessage(const String &message);
  bool sendTelegramPhoto(camera_fb_t *fb, const String &caption);
  String analyzePersonWithGemini(camera_fb_t *fb, uint8_t attempt = 0);
  void writeBase64ToClient(WiFiClientSecure &client, const uint8_t *data, size_t length);
  bool performGetRequest(String url, String &outResponse, int &outCode, int redirectDepth = 0);
  bool callGoogleAppsScript(const String &eventType, const String &source, const String &message);
  void requestGoogleScriptHeartbeatMonitorState(bool enabled);
  bool notifyGoogleScriptHeartbeatMonitorState(bool enabled);
  void processGoogleScriptHeartbeatMonitorControl();
  void processGoogleAppsScriptHeartbeat();
  void resolveGoogleAppsScriptCurrentEvent();
  bool isTelegramAllowed(const String &eventType);
  void notifySecurityTextEvent(const String &eventType, const String &source, const String &message);
void updateGoogleScriptStatusFromResponse(const String &response, bool ok, bool isHeartbeatEvent = false);
void maintainWiFiConnection();

  // ==================================================
  // CÁC HÀM CAMERA
  // ==================================================

  /**
   * Khởi tạo driver OV3660, ánh xạ chân, bộ đệm ảnh và các thông số cảm biến.
   * Trả về true khi camera dùng được; đồng thời cập nhật photo_status và Serial.
   */
  bool initCamera() {
    camera_config_t config; // Giữ ánh xạ chân và toàn bộ thông số khởi tạo camera.

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
      config.frame_size = FRAMESIZE_VGA;       // 640x480, clearer for AI person detection
      config.jpeg_quality = 10;
      // Đồ án chụp ảnh tĩnh rồi giữ frame trong lúc Telegram/AI tải ảnh lên.
      // Một bộ đệm cùng WHEN_EMPTY ngăn tràn hàng đợi camera (cam_hal: FB-OVF)
      // trong các request mạng chậm hơn này.
      config.fb_count = 1;
      config.fb_location = CAMERA_FB_IN_PSRAM;
      config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    } else {
      // Một framebuffer cho phép ESP32-S3 dùng QVGA ngay cả khi không thấy PSRAM.
      // Chất lượng này rõ hơn đáng kể so với dự phòng 160x120 cũ khi AI kiểm tra.
      Serial.println("[CAM] PSRAM NOT found. Using QVGA with one DRAM framebuffer.");
      config.frame_size = FRAMESIZE_QVGA;      // 320x240
      config.jpeg_quality = 12;
      config.fb_count = 1;
      config.fb_location = CAMERA_FB_IN_DRAM;
      config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }

    esp_err_t err = esp_camera_init(&config); // Mã lỗi ESP-IDF trả về khi khởi tạo camera.

    if (err != ESP_OK) {
      Serial.print("[CAM] Camera init failed. Error code: 0x");
      Serial.println((uint32_t)err, HEX);
      photo_status = "Camera khởi động thất bại";
      return false;
    }

    sensor_t *sensor = esp_camera_sensor_get(); // Con trỏ điều khiển cảm biến ảnh OV3660.

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

  /**
   * Loại bỏ frame cũ đang giữ trong bộ đệm rồi lấy một frame JPEG mới.
   * Trả về con trỏ framebuffer; bên gọi phải hoàn trả bằng esp_camera_fb_return().
   */
  camera_fb_t *captureFreshCameraFrame() {
    // WHEN_EMPTY giữ frame hoàn tất cuối trong framebuffer duy nhất. Cần bỏ frame
    // đó trước rồi chờ cảm biến cung cấp frame mới, để chụp thủ công không gửi
    // lại ảnh trước.
    camera_fb_t *staleFrame = esp_camera_fb_get();
    if (staleFrame != NULL) {
      esp_camera_fb_return(staleFrame);
      Serial.println("[CAM] Previous frame discarded; waiting for a fresh frame.");
      delay(250);
    }

    return esp_camera_fb_get();
  }

/**
 * Kiểm tra đủ ba điều kiện dùng AI: Gemini và camera được bật, API key hợp lệ.
 * Trả về true khi có thể gửi ảnh sang Gemini.
 */
bool hasGeminiConfig() {
  String apiKey = String(SECRET_GEMINI_API_KEY); // Bản String của khóa API để kiểm tra cấu hình.
  return gemini_enabled && camera_enabled && apiKey.length() > 0 && apiKey != "PASTE_YOUR_GEMINI_API_KEY_HERE";
  }

  /**
   * Mã hóa dữ liệu ảnh theo Base64 và ghi trực tiếp từng khối vào kết nối TLS.
   * @param client Kết nối đích đang mở; @param data vùng byte ảnh; @param length số byte.
   * Không trả về dữ liệu; cách ghi luồng giúp tránh tạo thêm chuỗi ảnh lớn trong RAM.
   */
  void writeBase64ToClient(WiFiClientSecure &client, const uint8_t *data, size_t length) {
    static const char BASE64_TABLE[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char encoded[1024];       // Bộ đệm tạm chứa một khối ký tự Base64 trước khi gửi.
    size_t encodedLength = 0; // Số ký tự hợp lệ đang có trong encoded.

    for (size_t i = 0; i < length; i += 3) {
      uint32_t triple = ((uint32_t)data[i]) << 16; // Nhóm tối đa ba byte nguồn dưới dạng 24 bit.
      bool hasSecondByte = i + 1 < length;          // Nhóm hiện tại có byte thứ hai hay không.
      bool hasThirdByte = i + 2 < length;           // Nhóm hiện tại có byte thứ ba hay không.
      if (hasSecondByte) triple |= ((uint32_t)data[i + 1]) << 8;
      if (hasThirdByte) triple |= data[i + 2];

      encoded[encodedLength++] = BASE64_TABLE[(triple >> 18) & 0x3F];
      encoded[encodedLength++] = BASE64_TABLE[(triple >> 12) & 0x3F];
      encoded[encodedLength++] = hasSecondByte ? BASE64_TABLE[(triple >> 6) & 0x3F] : '=';
      encoded[encodedLength++] = hasThirdByte ? BASE64_TABLE[triple & 0x3F] : '=';

      if (encodedLength == sizeof(encoded)) {
        client.write((const uint8_t *)encoded, encodedLength);
        encodedLength = 0;
      }
    }

    if (encodedLength > 0) {
      client.write((const uint8_t *)encoded, encodedLength);
    }
  }

  /**
   * Gửi một framebuffer JPEG đến Gemini và ép mô hình phân loại PERSON hoặc NONE.
   * @param fb Ảnh cần phân tích; @param attempt số lần thử, dùng để chỉ retry lỗi 503 một lần.
   * Trả về mã kết quả AI_* / PERSON_DETECTED / NO_PERSON để logic camera xử lý tiếp.
   */
  String analyzePersonWithGemini(camera_fb_t *fb, uint8_t attempt) {
    if (!hasGeminiConfig()) {
      Serial.println("[AI] Gemini API key is not configured.");
      return "AI_NOT_CONFIGURED";
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[AI] WiFi is not connected.");
      return "AI_NO_WIFI";
    }

    if (fb == NULL || fb->buf == NULL || fb->len == 0) {
      Serial.println("[AI] Camera buffer is empty.");
      return "AI_NO_IMAGE";
    }

    // Gemini nhận ảnh Base64 trong inline_data. Prompt cố ý giới hạn kết quả vào
    // một trong hai token để không cần xử lý mô tả tự do.
    const String jsonPrefix = // Phần JSON đứng trước dữ liệu ảnh Base64.
      "{\"contents\":[{\"parts\":[{\"text\":\"Inspect this low-resolution kitchen camera image. "
      "Answer PERSON if any part of a real human is visible, including a small, distant, "
      "partly occluded head, face, torso, arm, leg, or body at an image edge. "
      "Answer NONE only when there is no human at all. Reply with exactly one uppercase token: PERSON or NONE.\"},{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
    const String jsonSuffix = // Phần JSON đóng request và cấu hình cách Gemini trả lời.
      "\"}}]}],\"generationConfig\":{\"temperature\":0,\"maxOutputTokens\":64}}";
    const size_t base64Length = 4 * ((fb->len + 2) / 3); // Độ dài ảnh sau khi mã hóa Base64.
    const size_t contentLength = jsonPrefix.length() + base64Length + jsonSuffix.length(); // Tổng Content-Length.

    WiFiClientSecure client; // Kết nối TLS trực tiếp đến Gemini API.
    client.setInsecure();
    client.setTimeout(GEMINI_HTTP_TIMEOUT_MS);

    if (!client.connect(GEMINI_API_HOST, 443)) {
      Serial.println("[AI] Gemini connection failed.");
      return "AI_CONNECT_FAILED";
    }

    String path = "/v1/models/" + String(GEMINI_PERSON_MODEL) + ":generateContent"; // Đường dẫn REST của mô hình.
    client.print(String("POST ") + path + " HTTP/1.1\r\n");
    client.print(String("Host: ") + GEMINI_API_HOST + "\r\n");
    client.print("User-Agent: ESP32-S3-Kitchen-Security\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("x-goog-api-key: ");
    client.print(SECRET_GEMINI_API_KEY);
    client.print("\r\nContent-Length: ");
    client.print(contentLength);
    client.print("\r\n\r\n");
    client.print(jsonPrefix);
    writeBase64ToClient(client, fb->buf, fb->len);
    client.print(jsonSuffix);

    unsigned long startedAt = millis(); // Mốc bắt đầu chờ phản hồi để kiểm soát timeout.
    while (client.connected() && !client.available() && millis() - startedAt < GEMINI_HTTP_TIMEOUT_MS) {
      delay(10);
    }

    String statusLine = client.readStringUntil('\n'); // Dòng trạng thái HTTP đầu tiên.
    String response = client.readString();            // Phần phản hồi còn lại từ Gemini.
    bool requestOk = statusLine.indexOf("200") >= 0;  // true khi server trả HTTP 200.

    Serial.print("[AI] Gemini status: ");
    Serial.println(statusLine);

    if (!requestOk) {
      Serial.println("[AI] Gemini did not return HTTP 200.");
      Serial.print("[AI] Gemini error body: ");
      Serial.println(response);

      // Một lần thử lại ngắn xử lý lỗi quá tải tạm thời 503 của Gemini mà không
      // retry khóa sai, request sai hoặc lưu lượng cảnh báo bình thường.
      if (statusLine.indexOf("503") >= 0 && attempt == 0) {
        Serial.println("[AI] Gemini busy; retrying once in 1500 ms.");
        client.stop();
        delay(1500);
        return analyzePersonWithGemini(fb, 1);
      }
      return "AI_HTTP_ERROR";
    }

    Serial.print("[AI] Gemini response body: ");
    Serial.println(response);

    // Chỉ xét trường text do Gemini sinh. Response còn chứa HTTP header; từ ngữ
    // trong header tuyệt đối không được hiểu nhầm thành kết quả AI.
    String responseUpper = response; // Bản viết hoa để tìm token không phụ thuộc hoa/thường.
    responseUpper.toUpperCase();
    int textFieldAt = responseUpper.indexOf("\"TEXT\""); // Vị trí trường text do mô hình sinh.
    if (textFieldAt >= 0 && responseUpper.indexOf("PERSON", textFieldAt) >= 0) {
      Serial.println("[AI] PERSON_DETECTED");
      return "PERSON_DETECTED";
    }
    if (textFieldAt >= 0 && responseUpper.indexOf("NONE", textFieldAt) >= 0) {
      Serial.println("[AI] NO_PERSON");
      return "NO_PERSON";
    }

    Serial.println("[AI] Gemini response could not be classified.");
    return "AI_UNCLEAR";
  }

  /**
   * Chụp ảnh an ninh, gửi Telegram và tùy chọn phân tích ảnh bằng Gemini.
   * @param reason Lý do chụp dùng trong caption/log; @param analyzeWithGemini có chạy AI hay không.
   * Trả về true khi quy trình chụp/gửi chính thành công và cập nhật các trạng thái ảnh liên quan.
   */
  bool captureSecurityPhoto(const String &reason, bool analyzeWithGemini = true) {
  if (!camera_enabled) {
    photo_status = "Camera đang tắt từ Dashboard";
    return false;
  }
    if (!cameraReady) {
      photo_status = "Camera chưa sẵn sàng";
      setLastEvent("photo_capture_failed", "Camera not ready for " + reason + " at " + getRtcTimeString());
      Serial.println("[CAM] Capture skipped: camera not ready.");
      return false;
    }

    // Tránh vô tình chụp lặp quá nhanh trong thời gian nghỉ của phần cứng.
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

    camera_fb_t *fb = captureFreshCameraFrame(); // Frame JPEG mới; phải trả lại driver sau khi dùng.

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

    // AI chỉ là chức năng bổ sung: request thất bại không được dừng camera,
    // Telegram, cảm biến, còi hoặc luồng cảnh báo hiện có.
    if (analyzeWithGemini && gemini_enabled) {
      lastAiPersonResult = analyzePersonWithGemini(fb);
      if (lastAiPersonResult == "PERSON_DETECTED") {
        aiFollowUpPhotosRemaining = AI_PERSON_FOLLOW_UP_PHOTO_COUNT;
        nextAiFollowUpPhotoAtMs = millis() + AI_PERSON_FOLLOW_UP_INTERVAL_MS;
        setLastEvent(
          "ai_person_detected",
          "AI phát hiện có người. Hệ thống sẽ chụp thêm " + String(AI_PERSON_FOLLOW_UP_PHOTO_COUNT) +
          " ảnh trong vài giây tới."
        );

        // Chỉ ảnh an ninh tự động mới tạo email này. Ảnh thủ công không được coi
        // là đột nhập, và ảnh bằng chứng bổ sung không được tạo thông báo trùng.
        bool isAutomaticSecurityCapture = // Phân biệt ảnh cảnh báo tự động với ảnh chụp thủ công/bổ sung.
          reason == "AUTO_INTRUSION_ALERT" || reason == "AUTO_SABOTAGE_ALERT";
        if (isAutomaticSecurityCapture && !aiPersonEmailSentForCurrentAlert) {
          aiPersonEmailSentForCurrentAlert = true;
          String emailMessage = // Nội dung yêu cầu Apps Script gửi email khi AI thấy người.
            "Hệ thống đã phát hiện có người trong nhà. "
            "Vui lòng nhanh chóng kiểm tra ảnh trong Telegram để xác minh danh tính. "
            "Thời điểm phát hiện: " + getRtcTimeString() + ".";
          bool emailQueued = callGoogleAppsScript("ai_person_detected", "GEMINI", emailMessage); // Kết quả gửi yêu cầu email.
          notification_sent_status = emailQueued
            ? "AI phát hiện người: đã gửi email, kiểm tra Telegram"
            : "AI phát hiện người nhưng gửi email thất bại";
        }
      }
    } else {
      lastAiPersonResult = "AI_SKIPPED";
    }

  bool shouldSendToTelegram = telegram_enabled && camera_enabled && // Quyết định ảnh hiện tại có thuộc luồng Telegram.
      (reason == "MANUAL_DASHBOARD" ||
       reason == "AUTO_INTRUSION_ALERT" ||
       reason == "AUTO_SABOTAGE_ALERT" ||
       reason == "AI_PERSON_FOLLOW_UP");

    bool telegramPhotoSent = false; // Lưu kết quả gửi ảnh để cập nhật trạng thái cuối hàm.

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

      // Bằng chứng bổ sung chỉ gồm ảnh: không tạo tin nhắn Telegram riêng hay
      // caption sau khi Gemini đã phát hiện người.
      String caption = reason == "AI_PERSON_FOLLOW_UP" // Chú thích ảnh; ảnh bằng chứng bổ sung không có caption.
        ? ""
        : buildCommonCaption("photo_capture", reason);
      if (reason != "AI_PERSON_FOLLOW_UP") {
        if (lastAiPersonResult == "PERSON_DETECTED") {
          caption += "\nAI: Phát hiện có người trong khung hình.";
        } else if (lastAiPersonResult == "NO_PERSON") {
          caption += "\nAI: Không phát hiện người trong khung hình.";
        } else if (lastAiPersonResult != "AI_SKIPPED") {
          caption += "\nAI: Không thể xác nhận người (" + lastAiPersonResult + ").";
        }
      }
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

/**
 * Xử lý các yêu cầu chụp thủ công, chụp tự động và chuỗi ảnh AI bổ sung đang chờ.
 * Hàm được gọi trong loop(), không trả về và tự xóa/chuyển các cờ yêu cầu sau khi xử lý.
 */
void processCameraRequests() {
  if (manualCapturePending) {
    manualCapturePending = false;
    captureSecurityPhoto("MANUAL_DASHBOARD");
  }

  if (auto_capture_photo_request && auto_photo_on_alert) {
        String reason = notification_event_type == "sabotage_alert" // Lý do chụp suy ra từ cảnh báo đang chờ.
          ? "AUTO_SABOTAGE_ALERT"
          : "AUTO_INTRUSION_ALERT";
        captureSecurityPhoto(reason);
  }
  auto_capture_photo_request = false;

  if (aiFollowUpPhotosRemaining > 0 && millis() >= nextAiFollowUpPhotoAtMs) {
    aiFollowUpPhotosRemaining--;
    nextAiFollowUpPhotoAtMs = millis() + AI_PERSON_FOLLOW_UP_INTERVAL_MS;
    // Frame bổ sung là ảnh bằng chứng. Không gọi Gemini đệ quy lần nữa, nếu không
    // một người có thể tạo vòng lặp request tốn kém.
    captureSecurityPhoto("AI_PERSON_FOLLOW_UP", false);
  }
}

  // ==================================================
  // CÁC HÀM HỖ TRỢ CƠ BẢN
  // ==================================================

  /** Định dạng một số thành ít nhất hai chữ số; dùng cho giờ/phút/ngày trong chuỗi thời gian. */
  String twoDigits(int value) {
    if (value < 10) {
      return "0" + String(value);
    }
    return String(value);
  }

  /**
   * Đọc DS1307 và tạo chuỗi thời gian chuẩn dùng trong log/thông báo.
   * Trả về RTC_NOT_AVAILABLE khi RTC chưa khởi tạo được.
   */
  String getRtcTimeString() {
    if (!rtcOk) {
      return "RTC_NOT_FOUND";
    }

    DateTime now = rtc.now(); // Ảnh chụp ngày giờ hiện tại từ DS1307.
    String text = "";         // Chuỗi thời gian được ghép dần theo định dạng chuẩn.
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

  /** Chuyển mã loại sự kiện kỹ thuật thành tên tiếng Việt dễ đọc; giữ nguyên mã lạ. */
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

  /** Chuyển mã nguồn kích hoạt thành nhãn tiếng Việt để đưa vào thông báo. */
  String sourceTitleVi(const String &source) {
    if (source == "CHILD" || source == "CHILD_DEMO" || source == "CHILD_SERIAL") return "Trẻ em";
    if (source == "PARENT_ADMIN" || source == "PARENT_ADMIN_SERIAL") return "Phụ huynh/Admin";
    if (source == "DEVICE") return "Thiết bị";
    return source;
  }

  /** Chuyển mã trạng thái nâng cấp khẩn cấp thành mô tả tiếng Việt cho Dashboard. */
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

  /** Chuyển mã trạng thái gửi cho liên hệ cơ quan demo thành mô tả tiếng Việt. */
  String authorityStatusVi(const String &code) {
    if (code == "IDLE") return "Chưa gửi contact mô phỏng";
    if (code == "READY") return "Sẵn sàng gửi sau xác nhận";
    if (code == "SENT") return "Đã gửi contact mô phỏng";
    if (code == "FAILED") return "Gửi contact mô phỏng thất bại";
    if (code == "NOT_CONFIGURED") return "Chưa cấu hình contact mô phỏng";
    return code;
  }

  /** Ghi trạng thái nâng cấp SOS đã dịch vào biến Cloud emergency_escalation_status. */
  void setEscalationStatus(const String &code) {
    emergency_escalation_status = escalationStatusVi(code);
  }

  /** Ghi trạng thái gửi cơ quan demo đã dịch vào biến Cloud tương ứng. */
  void setAuthorityStatus(const String &code) {
    emergency_authority_message_status = authorityStatusVi(code);
  }

  /** Trả về true nếu notification_sent_status hiện chứa một trạng thái lỗi gửi đã biết. */
  bool isKnownNotificationFailureStatus() {
    return notification_sent_status == "NOT_CONFIGURED" ||
          notification_sent_status == "WIFI_NOT_CONNECTED" ||
          notification_sent_status == "CONNECT_FAILED" ||
          notification_sent_status == "Chưa cấu hình Telegram" ||
          notification_sent_status == "Không có WiFi để gửi Telegram" ||
          notification_sent_status == "Không kết nối được Telegram";
  }

  /** Lưu đồng thời mã sự kiện vào last_event_type và nội dung dễ đọc vào last_event. */
  void setLastEvent(const String &type, const String &message) {
    last_event_type = type;
    last_event = message;
  }

  /** Tăng bộ đếm sự kiện và đưa về 0 trước khi có nguy cơ vượt giới hạn số nguyên. */
  void incrementEventCounter() {
    event_counter++;
  }


  // ==================================================
  // CÁC HÀM API NGOÀI - TELEGRAM + GOOGLE APPS SCRIPT
  // ==================================================

  /** Mã hóa chuỗi thành dạng percent-encoding an toàn để ghép vào query URL. */
  String urlEncode(const String &value) {
    String encoded = "";                         // Kết quả URL-encoded được ghép dần.
    const char *hex = "0123456789ABCDEF";         // Bảng ký tự dùng biểu diễn một nibble hệ 16.

    for (size_t i = 0; i < value.length(); i++) {
      char c = value.charAt(i); // Ký tự nguồn tại vị trí đang mã hóa.

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

  /** Tách giá trị của một khóa dạng key=value trong phản hồi phân tách bằng dấu chấm phẩy. */
  String responseValue(const String &response, const String &key) {
    String marker = key + "=";           // Mẫu đánh dấu đầu trường cần tìm.
    int start = response.indexOf(marker); // Vị trí mẫu trong response.
    if (start < 0) {
      return "";
    }

    start += marker.length();
    int end = response.indexOf(';', start); // Vị trí kết thúc trường hoặc -1 nếu ở cuối response.
    String value = end < 0 ? response.substring(start) : response.substring(start, end); // Giá trị đã cắt.
    value.trim();
    return value;
  }

/** Trả về true khi Telegram được bật và token/chat ID không rỗng, không còn là placeholder. */
bool hasTelegramConfig() {
  return telegram_enabled && String(SECRET_TELEGRAM_BOT_TOKEN).length() > 10 &&
          String(SECRET_TELEGRAM_CHAT_ID).length() > 0;
  }

/** Trả về true khi Apps Script được bật và URL Web App đã được cấu hình. */
bool hasGoogleScriptConfig() {
  return google_script_enabled && String(SECRET_GOOGLE_SCRIPT_URL).startsWith("https://");
  }

  /** Tạo caption chung chứa loại sự kiện, lý do, thiết bị, vị trí và thời gian. */
  String buildCommonCaption(const String &eventType, const String &reason) {
    String text = ""; // Bộ tích lũy caption nhiều dòng.
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

  /**
   * Gửi tin nhắn văn bản đến chat Telegram đã cấu hình.
   * Trả về true khi Telegram API phản hồi HTTP thành công; đồng thời cập nhật trạng thái gửi.
   */
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

    WiFiClientSecure client; // Kết nối TLS dùng cho request gửi tin nhắn Telegram.
    client.setInsecure();
  client.setTimeout(TELEGRAM_TEXT_TIMEOUT_MS);

    if (!client.connect("api.telegram.org", 443)) {
      notification_sent_status = "Không kết nối được Telegram";
      Serial.println("[TG] Connect failed.");
      return false;
    }

    String body = "chat_id=" + urlEncode(String(SECRET_TELEGRAM_CHAT_ID)) + // Nội dung form URL-encoded gửi Telegram.
                  "&text=" + urlEncode(message);

    String path = "/bot" + String(SECRET_TELEGRAM_BOT_TOKEN) + "/sendMessage"; // Endpoint gửi tin nhắn của bot.

    client.print(String("POST ") + path + " HTTP/1.1\r\n");
    client.print("Host: api.telegram.org\r\n");
    client.print("User-Agent: ESP32-S3-Kitchen-Security\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\r\n\r\n");
    client.print(body);

    unsigned long start = millis(); // Mốc bắt đầu chờ phản hồi Telegram.
  while (client.connected() && !client.available() && millis() - start < TELEGRAM_TEXT_TIMEOUT_MS) {
      delay(10);
    }

    String statusLine = client.readStringUntil('\n'); // Dòng trạng thái HTTP Telegram.
    bool ok = statusLine.indexOf("200") >= 0;         // Kết quả HTTP 200 của thao tác gửi.

    Serial.print("[TG] sendMessage status: ");
    Serial.println(statusLine);

  client.stop();
  notification_sent_status = ok ? "Đã gửi thông báo" : "Gửi thông báo thất bại";
    return ok;
  }

  /**
   * Gửi framebuffer JPEG và caption bằng multipart/form-data đến Telegram.
   * Trả về true khi API chấp nhận ảnh; không sở hữu và không giải phóng framebuffer.
   */
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

    WiFiClientSecure client; // Kết nối TLS dùng cho request multipart gửi ảnh Telegram.
    client.setInsecure();
  client.setTimeout(TELEGRAM_PHOTO_TIMEOUT_MS);

    if (!client.connect("api.telegram.org", 443)) {
      notification_sent_status = "Không kết nối được Telegram";
      Serial.println("[TG] Photo connect failed.");
      return false;
    }

    String boundary = "----ESP32S3KitchenSecurityBoundary"; // Chuỗi phân cách các phần multipart.

    String head = ""; // Phần header multipart đứng trước dữ liệu JPEG.
    head += "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    head += String(SECRET_TELEGRAM_CHAT_ID) + "\r\n";
    head += "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
    head += caption + "\r\n";
    head += "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"photo\"; filename=\"security.jpg\"\r\n";
    head += "Content-Type: image/jpeg\r\n\r\n";

    String tail = "\r\n--" + boundary + "--\r\n"; // Phần đóng gói multipart sau dữ liệu JPEG.

    size_t contentLength = head.length() + fb->len + tail.length(); // Tổng số byte body multipart.
    String path = "/bot" + String(SECRET_TELEGRAM_BOT_TOKEN) + "/sendPhoto"; // Endpoint gửi ảnh của bot.

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

    unsigned long start = millis(); // Mốc bắt đầu chờ phản hồi gửi ảnh.
  while (client.connected() && !client.available() && millis() - start < TELEGRAM_PHOTO_TIMEOUT_MS) {
      delay(10);
    }

    String statusLine = client.readStringUntil('\n'); // Dòng trạng thái HTTP của sendPhoto.
    bool ok = statusLine.indexOf("200") >= 0;         // true khi Telegram nhận ảnh thành công.

    Serial.print("[TG] sendPhoto status: ");
    Serial.println(statusLine);

  client.stop();
  notification_sent_status = ok ? "Đã gửi ảnh qua Telegram" : "Gửi ảnh qua Telegram thất bại";
    return ok;
  }

  /**
   * Thực hiện HTTP GET, theo dõi redirect có giới hạn và xuất nội dung/mã HTTP qua tham chiếu.
   * Trả về true với phản hồi 2xx; url có thể được thay bằng URL redirect trong quá trình chạy.
   */
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

    int slashIndex = url.indexOf('/'); // Vị trí phân tách host và path trong URL đã bỏ scheme.
    String host = "";                  // Tên máy chủ (có thể kèm cổng) được tách từ URL.
    String path = "";                  // Đường dẫn và query của request.
    if (slashIndex >= 0) {
      host = url.substring(0, slashIndex);
      path = url.substring(slashIndex);
    } else {
      host = url;
      path = "/";
    }

    int port = 443;                    // Cổng TLS mặc định, có thể bị URL ghi đè.
    int colonIndex = host.indexOf(':'); // Vị trí dấu hai chấm phân tách host và cổng.
    if (colonIndex >= 0) {
      port = host.substring(colonIndex + 1).toInt();
      host = host.substring(0, colonIndex);
    }

    Serial.print("[HTTP] Connecting to: ");
    Serial.print(host);
    Serial.println(path);

    WiFiClientSecure client; // Kết nối TLS dùng chung cho Apps Script.
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

    unsigned long start = millis(); // Mốc bắt đầu chờ phản hồi HTTP.
    while (client.connected() && !client.available() && millis() - start < GOOGLE_HTTP_TIMEOUT_MS) {
      delay(10);
    }

    if (!client.available()) {
      Serial.println("[HTTP] No response received.");
      outCode = 0;
      client.stop();
      return false;
    }

    String statusLine = client.readStringUntil('\n'); // Dòng đầu chứa phiên bản HTTP và mã trạng thái.
    statusLine.trim();

    int firstSpace = statusLine.indexOf(' ');                  // Dấu cách trước mã HTTP.
    int secondSpace = statusLine.indexOf(' ', firstSpace + 1); // Dấu cách sau mã HTTP.
    int httpCode = 0;                                          // Mã HTTP đã parse; 0 nghĩa là chưa parse được.
    if (firstSpace >= 0 && secondSpace >= 0) {
      httpCode = statusLine.substring(firstSpace + 1, secondSpace).toInt();
    } else if (firstSpace >= 0) {
      httpCode = statusLine.substring(firstSpace + 1).toInt();
    }
    outCode = httpCode;

    String location = ""; // URL trong header Location khi server yêu cầu redirect.
    while (client.connected() || client.available()) {
      String line = client.readStringUntil('\n'); // Một dòng header đang được duyệt.
      line.trim();
      if (line.length() == 0) {
        break;
      }
      if (line.startsWith("Location:") || line.startsWith("location:")) {
        location = line.substring(9);
        location.trim();
      }
    }

    String body = ""; // Nội dung response sau phần header.
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

  /**
   * Phân tích phản hồi Apps Script và đồng bộ ID sự kiện, trạng thái xác nhận, địa chỉ và thông báo.
   * @param ok cho biết lớp HTTP thành công; @param isHeartbeatEvent phân biệt phản hồi heartbeat.
   */
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

    String returnedEventId = responseValue(response, "eventId"); // ID sự kiện máy chủ trả về cho lần polling sau.
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
    // Bộ phân tích có tính phòng vệ: chỉ cập nhật trường thật sự có trong response.
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
      // Nhánh dự phòng tương thích ngược với response Apps Script phiên bản cũ.
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

  /**
   * Tạo query chứa snapshot trạng thái rồi gọi Web App cho một sự kiện an ninh.
   * Trả về kết quả HTTP và cập nhật các biến trạng thái Apps Script trên Dashboard.
   */
  bool callGoogleAppsScript(const String &eventType, const String &source, const String &message) {
    bool isHeartbeatEvent = eventType == "heartbeat";           // Phân biệt heartbeat để áp giới hạn tần suất riêng.
    bool isAiPersonEvent = eventType == "ai_person_detected";    // Phân biệt email thông tin AI với luồng SOS.

    if (!hasGoogleScriptConfig()) {
      if (isHeartbeatEvent) {
        heartbeat_status = "Chưa cấu hình Apps Script";
      } else if (!isAiPersonEvent) {
        setEscalationStatus("NOT_CONFIGURED");
        setAuthorityStatus("NOT_CONFIGURED");
      }
      Serial.println("[GAS] Google Apps Script is not configured.");
      return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
      if (isHeartbeatEvent) {
        heartbeat_status = "Không có WiFi để gửi heartbeat";
      } else if (!isAiPersonEvent) {
        setEscalationStatus("WIFI_NOT_CONNECTED");
        setAuthorityStatus("FAILED");
      }
      Serial.println("[GAS] WiFi is not connected.");
      return false;
    }

    String eventKey = eventType + ":" + source; // Khóa chống gửi lặp cùng loại sự kiện và nguồn.
    if (lastGoogleScriptEventKey == eventKey && millis() - lastGoogleScriptCallMs < 15000) {
      Serial.println("[GAS] Skipped by cooldown.");
      return false;
    }

    lastGoogleScriptEventKey = eventKey;
    lastGoogleScriptCallMs = millis();

    String url = String(SECRET_GOOGLE_SCRIPT_URL); // URL request được ghép thêm toàn bộ query trạng thái.
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
    } else if (!isAiPersonEvent) {
      setEscalationStatus("SENDING");
      setAuthorityStatus("IDLE");
    }

    String response = "";                              // Body do Apps Script trả về.
    int code = 0;                                      // Mã trạng thái HTTP nhận được.
    bool ok = performGetRequest(url, response, code);  // Kết quả request tổng hợp.

    Serial.print("[GAS] HTTP code: ");
    Serial.println(code);
    if (response.length() > 0) {
      Serial.print("[GAS] Response: ");
      Serial.println(response.substring(0, 200));
    }

    if (!isAiPersonEvent) {
      updateGoogleScriptStatusFromResponse(response, ok, isHeartbeatEvent);
    }
    if (isHeartbeatEvent && !ok) {
      heartbeat_status = "Gửi heartbeat thất bại";
    }
    return ok;
  }

  /** Xếp hàng yêu cầu bật/tắt giám sát heartbeat để retry đến khi Apps Script xác nhận. */
  void requestGoogleScriptHeartbeatMonitorState(bool enabled) {
    desiredHeartbeatMonitorEnabled = enabled;
    heartbeatMonitorControlPending = true;
    heartbeatMonitorControlRetryIndex = 0;
    heartbeatMonitorControlNextRetryAtMs = millis();
  }

  // Tin điều khiển này chủ ý bỏ qua google_script_enabled để khi tắt công tắc
  // Dashboard, Apps Script có thể tạm dừng timeout trước khi bo mạch ngừng gửi
  // heartbeat bình thường.
  /** Gửi trạng thái giám sát heartbeat mong muốn đến Apps Script và trả về kết quả xác nhận. */
  bool notifyGoogleScriptHeartbeatMonitorState(bool enabled) {
    if (!String(SECRET_GOOGLE_SCRIPT_URL).startsWith("https://") || WiFi.status() != WL_CONNECTED) {
      return false;
    }

    String url = String(SECRET_GOOGLE_SCRIPT_URL); // URL lệnh bật/tắt heartbeat monitor.
    url += "?event=heartbeat_monitor_control";
    url += "&enabled=" + String(enabled ? "true" : "false");
    url += "&device=" + urlEncode(String(SECRET_DEVICE_NAME));
    url += "&location=" + urlEncode(String(SECRET_DEVICE_LOCATION));
    url += "&time=" + urlEncode(getRtcTimeString());

    String response = "";                             // Body xác nhận từ Apps Script.
    int code = 0;                                     // Mã HTTP của lệnh điều khiển.
    bool ok = performGetRequest(url, response, code); // Kết quả tầng HTTP.
    String expectedAck = enabled                      // Chuỗi ACK chính xác mong đợi theo trạng thái yêu cầu.
      ? "OK:HEARTBEAT_MONITOR_ENABLED"
      : "OK:HEARTBEAT_MONITOR_PAUSED";
    bool acknowledged = ok && response.indexOf(expectedAck) >= 0; // true khi cả HTTP và ACK đều hợp lệ.
    Serial.print("[GAS] Heartbeat monitor control: ");
    Serial.println(acknowledged ? "ACK" : "FAILED");
    return acknowledged;
  }

  /** Xử lý hàng đợi lệnh heartbeat-monitor, áp dụng backoff và giữ lệnh nếu gửi thất bại. */
  void processGoogleScriptHeartbeatMonitorControl() {
    if (!heartbeatMonitorControlPending || millis() < heartbeatMonitorControlNextRetryAtMs) {
      return;
    }

    if (notifyGoogleScriptHeartbeatMonitorState(desiredHeartbeatMonitorEnabled)) {
      heartbeatMonitorControlPending = false;
      heartbeatMonitorControlRetryIndex = 0;
      heartbeat_status = desiredHeartbeatMonitorEnabled
        ? "Đã xác nhận bật theo dõi heartbeat"
        : "Đã xác nhận tạm dừng theo dõi heartbeat";
      return;
    }

    uint8_t delayIndex = min( // Chỉ số backoff được chặn ở phần tử cuối mảng.
      heartbeatMonitorControlRetryIndex,
      (uint8_t)(sizeof(WIFI_RETRY_DELAYS_MS) / sizeof(WIFI_RETRY_DELAYS_MS[0]) - 1)
    );
    heartbeatMonitorControlNextRetryAtMs = millis() + WIFI_RETRY_DELAYS_MS[delayIndex];
    if (heartbeatMonitorControlRetryIndex < 255) {
      heartbeatMonitorControlRetryIndex++;
    }
    heartbeat_status = "Đang gửi lại lệnh theo dõi heartbeat";
  }

  /** Gửi heartbeat định kỳ khi Wi-Fi/dịch vụ được bật và không có lệnh điều khiển đang chờ. */
  void processGoogleAppsScriptHeartbeat() {
    static unsigned long lastHeartbeatMs = 0; // Giữ mốc heartbeat trước giữa các lần gọi hàm.

    // Thay đổi từ Dashboard phải đến Apps Script trước khi heartbeat thường
    // được gửi; nhờ đó request khác không che mất lệnh PAUSE đang chờ.
    if (heartbeatMonitorControlPending) {
      return;
    }

    if (!google_script_enabled || !heartbeat_enabled) {
      heartbeat_status = "Theo dõi heartbeat đang tạm dừng";
      return;
    }

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

    bool ok = callGoogleAppsScript("heartbeat", "DEVICE", "Heartbeat từ thiết bị lúc " + getRtcTimeString()); // Kết quả heartbeat.
    if (ok) {
      heartbeat_status = "Đang liên lạc bình thường";
      last_heartbeat_time = getRtcTimeString();
    }
  }

  /** Báo Apps Script đóng sự kiện đang hoạt động rồi xóa activeGoogleEventId cục bộ khi phù hợp. */
  void resolveGoogleAppsScriptCurrentEvent() {
    if (!hasGoogleScriptConfig() || WiFi.status() != WL_CONNECTED) {
      return;
    }

    String url = String(SECRET_GOOGLE_SCRIPT_URL); // URL resolve cho sự kiện activeGoogleEventId.
    url += "?action=resolve";
    url += "&device=" + urlEncode(String(SECRET_DEVICE_NAME));
    url += "&location=" + urlEncode(String(SECRET_DEVICE_LOCATION));
    url += "&time=" + urlEncode(getRtcTimeString());
    url += "&message=" + urlEncode("Device reset_alarm resolved local alerts.");
    if (activeGoogleEventId.length() > 0) {
      url += "&eventId=" + urlEncode(activeGoogleEventId);
    }

    String response = "";                             // Body phản hồi resolve.
    int code = 0;                                     // Mã HTTP phản hồi resolve.
    bool ok = performGetRequest(url, response, code); // Kết quả đóng sự kiện trên máy chủ.

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

  /** Kiểm tra loại sự kiện có thuộc danh sách được phép gửi Telegram hay không. */
  bool isTelegramAllowed(const String &eventType) {
    static unsigned long lastTelegramIntrusionAlertMs = 0; // Lần gửi đột nhập gần nhất để chống spam.
    static unsigned long lastTelegramSabotageAlertMs = 0;  // Lần gửi phá hoại gần nhất để chống spam.
    static unsigned long lastTelegramSosAlertMs = 0;       // Lần gửi SOS gần nhất để chống spam.
    static unsigned long lastTelegramManualCaptureMs = 0;  // Lần gửi ảnh thủ công gần nhất để chống spam.

    unsigned long nowMs = millis(); // Mốc hiện tại dùng so cooldown thông báo.
    String eventLower = eventType;  // Bản loại sự kiện sẽ được chuyển về chữ thường.
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

  /** Tạo nội dung cảnh báo chữ, gửi Telegram/Apps Script đúng luồng và cập nhật trạng thái thông báo. */
  void notifySecurityTextEvent(const String &eventType, const String &source, const String &message) {
    if (!isTelegramAllowed(eventType)) {
      notification_sent_status = "Đang chặn gửi lặp để tránh spam";
      Serial.println("[TG] Skipped text notification by cooldown.");
      return;
    }

    send_notification_request = true;
    notification_event_type = eventType;
    notification_channel = "telegram";

    String text = buildCommonCaption(eventType, source); // Nội dung Telegram hoàn chỉnh đang được xây dựng.
    text += "\nNội dung: ";
    text += message;

    notification_sent_status = "Đang gửi thông báo";
    bool sent = sendTelegramMessage(text); // Kết quả gửi dùng cập nhật trạng thái Dashboard.
    notification_sent_status = sent ? "Đã gửi thông báo" : "Gửi thông báo thất bại";
  }

  // ==================================================
  // ĐIỀU KHIỂN ĐẦU RA
  // ==================================================

  /** Bật/tắt LED đỏ vật lý và lưu trạng thái thực tế vào led_red_on. */
  void setRedLedPhysical(bool on) {
    digitalWrite(PIN_LED_RED, on ? HIGH : LOW);
  }

  /** Bật/tắt LED xanh vật lý và lưu trạng thái thực tế vào led_green_on. */
  void setGreenLedPhysical(bool on) {
    digitalWrite(PIN_LED_GREEN, on ? HIGH : LOW);
  }

  /** Điều khiển còi theo loại chủ động/thụ động và lưu trạng thái vào buzzer_on. */
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

  /** Suy ra trạng thái còi và LED từ các chốt SOS, phá hoại, đột nhập và trạng thái canh gác. */
  void applyAlarmOutputs() {
    bool anyAlarmActive = sosActive || sabotage_alert || intrusion_alert || critical_security_compromise; // Có cảnh báo ưu tiên hay không.

    buzzer_on = anyAlarmActive;
    led_red_on = anyAlarmActive;
    led_green_on = !anyAlarmActive;

    setGreenLedPhysical(led_green_on);
    setBuzzerPhysical(buzzer_on);

    if (led_red_on) {
      static unsigned long lastBlink = 0; // Mốc đảo LED đỏ trước đó, được giữ giữa các vòng lặp.
      static bool redState = false;       // Pha sáng/tắt hiện tại của LED đỏ nhấp nháy.

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
  // ĐỌC CẢM BIẾN
  // ==================================================

  /** Đọc nhiều mẫu ADC LDR, lấy trung bình để giảm nhiễu và trả về giá trị đã lọc. */
  int readLdrValue() {
    return analogRead(PIN_LDR_AO);
  }

  /** Tính độ chênh tuyệt đối giữa giá trị LDR hiện tại và lần trước, đồng thời cập nhật mẫu trước. */
  int calculateLdrDelta(int currentValue) {
    if (lastLdrValue < 0) {
      lastLdrValue = currentValue;
      return 0;
    }

    int delta = abs(currentValue - lastLdrValue); // Độ chênh tuyệt đối so với mẫu LDR trước.
    lastLdrValue = currentValue;
    return delta;
  }

  /** Đọc chân PIR và trả về true khi cảm biến đang báo có chuyển động. */
  bool readPirDetected() {
    return digitalRead(PIN_PIR_OUT) == HIGH;
  }

  /** Phát một xung trigger, đo echo và trả về khoảng cách cm; trả số âm nếu timeout/không hợp lệ. */
  float readUltrasonicDistanceCmOnce() {
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    unsigned long duration = pulseIn(PIN_US_ECHO, HIGH, 30000UL); // Độ rộng xung echo tính bằng micro giây.

    if (duration == 0) {
      return -1.0;
    }

    return duration * 0.0343 / 2.0;
  }

  /** Đọc siêu âm nhiều lần và tổng hợp các mẫu hợp lệ để giảm sai số tức thời. */
  float readUltrasonicDistanceCm() {
    float a = readUltrasonicDistanceCmOnce(); // Mẫu khoảng cách thứ nhất.
    delay(20);
    float b = readUltrasonicDistanceCmOnce(); // Mẫu khoảng cách thứ hai.
    delay(20);
    float c = readUltrasonicDistanceCmOnce(); // Mẫu khoảng cách thứ ba.

    float sum = 0.0; // Tổng các mẫu khoảng cách hợp lệ.
    int count = 0;   // Số mẫu hợp lệ được đưa vào tổng.

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

  /** So sánh giá trị LDR với ngưỡng đúng chiều phần cứng để xác định cảm biến bị che. */
  bool calculateLdrCovered(int value) {
    if (LDR_COVER_WHEN_HIGH) {
      return value >= LDR_COVERED_THRESHOLD_HIGH;
    }
    return value <= LDR_COVERED_THRESHOLD_LOW;
  }

  /** Trả về true nếu giờ hợp lệ nằm trong khung thời gian được quy ước là ban đêm. */
  bool calculateNightMode(int hour) {
    if (hour < 0) {
      return false;
    }
    return hour >= 22 || hour < 6;
  }

  /** Đọc tất cả cảm biến và RTC trong cùng chu kỳ rồi trả về một HardwareSnapshot nhất quán. */
  HardwareSnapshot readHardwareSnapshot() {
    HardwareSnapshot s; // Snapshot đang được điền trước khi trả về.

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
      DateTime now = rtc.now(); // Thời gian RTC tương ứng với snapshot cảm biến này.
      s.hour = now.hour();
      s.minute = now.minute();
    }

    s.nightMode = calculateNightMode(s.hour);

    return s;
  }

  /** Sao chép các trường snapshot sang biến runtime/Cloud để Dashboard và logic sau dùng chung dữ liệu. */
  void publishHardwareSnapshotToCloud(const HardwareSnapshot &s) {
    ldr_value = s.ldrValue;
    ldr_delta = s.ldrDelta;
    pir_detected = s.pirDetected;
    ultrasonic_distance = s.ultrasonicOk ? s.distanceCm : -1.0;
    object_near = s.objectNear;
    light_abnormal = s.lightAbnormal;
    ldr_covered = s.ldrCovered;
    // Dashboard cần đồng hồ RTC chính xác, nhưng các giá trị cảm biến thô bên trên
    // chỉ dùng lúc chạy và không phải thuộc tính Cloud trong Version3.
    static String lastPublishedTime = ""; // Thời gian RTC gần nhất đã công bố, tránh gán chuỗi lặp.
    if (s.timeText != lastPublishedTime) {
      current_time = s.timeText;
      lastPublishedTime = s.timeText;
    }
    current_hour = s.hour;
    night_mode = s.nightMode;
    device_tampered = s.ldrCovered || s.objectTooClose;
  }

  /** Theo dõi Wi-Fi và chủ động yêu cầu kết nối lại theo backoff khi đường truyền bị mất. */
  void maintainWiFiConnection() {
    static unsigned long lastAttemptAt = 0; // Mốc thử nối Wi-Fi gần nhất.
    static uint8_t retryIndex = 0;           // Chỉ số mức backoff hiện tại trong WIFI_RETRY_DELAYS_MS.

    if (WiFi.status() == WL_CONNECTED) {
      if (device_health_status == "Đang khôi phục kết nối WiFi") {
        device_health_status = "Thiết bị hoạt động bình thường";
      }
      retryIndex = 0;
      return;
    }

    unsigned long delayMs = WIFI_RETRY_DELAYS_MS[retryIndex]; // Thời gian chờ trước lần nối lại kế tiếp.
    if (millis() - lastAttemptAt < delayMs) {
      return;
    }

    lastAttemptAt = millis();
    WiFi.reconnect();
    if (retryIndex < 3) retryIndex++;
    device_health_status = "Đang khôi phục kết nối WiFi";
  }

  // ==================================================
  // LOGIC AN NINH
  // ==================================================

  /** Chuyển sensitivity_level thành ngưỡng điểm: mức nhạy càng cao thì ngưỡng càng thấp. */
  int getIntrusionThreshold() {
    if (sensitivity_level == 1) return 5;
    if (sensitivity_level == 2) return 4;
    if (sensitivity_level == 3) return 3;
    return 3;
  }

  /** Ép các cấu hình từ Cloud về miền hợp lệ để tránh giờ/phút hoặc mức nhạy sai. */
  void clampCloudConfigValues() {
    if (sensitivity_level < 1 || sensitivity_level > 3) {
      sensitivity_level = 3;
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

  /** Suy luận chuyển động có thể do vật nuôi từ tổ hợp PIR và khoảng cách trong snapshot. */
  bool calculatePetDetected(const HardwareSnapshot &s) {
    // Đây chỉ là bộ lọc mức demo, không phải kết quả phân loại vật nuôi đã xác nhận.
    return s.objectNear && !s.pirDetected && !s.lightAbnormal;
  }

  /** Cộng điểm các dấu hiệu PIR, khoảng cách, ánh sáng và thời gian để tạo điểm xâm nhập thô. */
  int calculateRawIntrusionScore(const HardwareSnapshot &s) {
    int score = 0; // Điểm thô được cộng dần từ từng dấu hiệu cảm biến.

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

  /** Tạo chuỗi giải thích những tín hiệu cảm biến nào làm phát sinh điểm/cảnh báo xâm nhập. */
  String buildIntrusionReason(const HardwareSnapshot &s) {
    String reason = ""; // Chuỗi giải thích được nối dần từ các điều kiện đang đúng.

    if (s.pirDetected) reason += "chuyển động PIR (+2); ";
    if (s.objectNear) reason += "vật thể ở gần (+2); ";
    if (s.lightAbnormal) reason += "ánh sáng thay đổi bất thường (+1); ";
    if (s.nightMode) reason += "đang trong khung giờ ban đêm (+1); ";

    if (reason.length() == 0) {
      reason = "chưa có tín hiệu đáng kể";
    }

    return reason;
  }

  /** Đồng bộ system_armed từ alarm_enabled sau khi xét các trạng thái vận hành liên quan. */
  void updateSystemArmed() {
    system_armed = alarm_enabled;
  }

  /** Chốt cảnh báo đột nhập mới, ghi bằng chứng điểm/lý do, yêu cầu ảnh và gửi thông báo. */
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
      aiPersonEmailSentForCurrentAlert = false;
      photo_status = "Đang chuẩn bị chụp ảnh tự động";
      send_notification_request = true;
      notification_event_type = "intrusion_alert";
      notification_sent_status = "Đang chụp ảnh để gửi cảnh báo";

      // Thông báo chữ là phương án dự phòng khi gửi ảnh bị tắt.
      // Nó chỉ gửi một lần vì hàm này chỉ chạy khi có cảnh báo mới.
      if (!(gemini_enabled && camera_enabled)) {
        notifySecurityTextEvent("intrusion_alert", "SENSOR", last_event);
      }
    }
  }

  /** Chốt cảnh báo phá hoại, yêu cầu ảnh, thông báo và mở quy trình theo dõi heartbeat. */
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

      // Sự kiện phá hoại có phản ứng cục bộ tức thời giống đột nhập: đầu ra báo
      // động được áp dụng ngay vòng lặp này và ảnh an ninh được gửi một lần.
      auto_capture_photo_request = true;
      aiPersonEmailSentForCurrentAlert = false;
      photo_status = "Đang chuẩn bị chụp ảnh phá hoại";
      send_notification_request = true;
      notification_event_type = "sabotage_alert";
      notification_sent_status = "Đang chụp ảnh để gửi cảnh báo phá hoại";

      // Khi Gemini và gửi ảnh được bật, luồng camera chính là kênh cảnh báo.
      // Không thêm tin nhắn Telegram riêng trước khi AI đánh giá frame.
      if (!(gemini_enabled && camera_enabled)) {
        notifySecurityTextEvent("sabotage_alert", "DEVICE", last_event);
      }
      callGoogleAppsScript("sabotage_alert", "DEVICE", last_event);
    }
  }

  /** Kích hoạt SOS từ nguồn chỉ định, ưu tiên đầu ra báo động và khởi tạo luồng xác nhận email. */
  void triggerSosAlert(const String &source) {
    if (!sosActive) {
      incrementEventCounter();
    }

    sosActive = true;
    sosSource = source;

    sos_message = "SOS từ " + sourceTitleVi(source) + " lúc " + getRtcTimeString() +
                  ". Còi/LED đỏ sẽ tiếp tục bật cho đến khi phụ huynh/Admin reset.";
    setEscalationStatus("WAITING_CONFIRMATION");
    setAuthorityStatus("IDLE");

    setLastEvent("sos_alert", sos_message);
    notifySecurityTextEvent("sos_alert", source, sos_message);
    callGoogleAppsScript("sos_alert", source, sos_message);
  }

  /** Xóa mọi chốt cảnh báo/yêu cầu phụ, tắt đầu ra và báo Apps Script giải quyết sự kiện hiện tại. */
  void resetAllAlerts() {
    intrusion_alert = false;
    sabotage_alert = false;
    sosActive = false;
    sosSource = "NONE";
    sos_child = false;
    sos_adult = false;

    critical_security_compromise = false;
    device_tampered = false;
    device_health_status = "Đã reset cảnh báo";
    setEscalationStatus("IDLE");
    setAuthorityStatus("IDLE");

    auto_capture_photo_request = false;
    aiPersonEmailSentForCurrentAlert = false;
    manualCapturePending = false;

    send_notification_request = false;
    notification_event_type = "NONE";
    notification_sent_status = "Chưa có cảnh báo cần gửi";

    sabotageConditionStartedAt = 0;
    intrusionConditionStartedAt = 0;

    incrementEventCounter();
    setLastEvent("alarm_reset", "Đã reset cảnh báo tại thiết bị lúc " + getRtcTimeString() + ". Vui lòng kiểm tra thực tế trước khi bật lại bảo vệ.");
    resolveGoogleAppsScriptCurrentEvent();

    setBuzzerPhysical(false);
    setRedLedPhysical(false);
    setGreenLedPhysical(true);
  }

  /** Theo dõi điều kiện che/phá cảm biến theo thời gian giữ và kích hoạt cảnh báo khi đủ lâu. */
  void updateSabotageLogic(const HardwareSnapshot &s) {
    // Chống phá hoại bản demo yêu cầu đồng thời cả hai dấu hiệu vật lý:
    // có vật ở rất gần và cảm biến ánh sáng bị che.
    bool sabotageCondition = s.ldrCovered && s.objectTooClose; // Tổ hợp dấu hiệu che LDR và áp sát thiết bị.

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

  /** Tính điểm xâm nhập, áp dụng ngưỡng/độ trễ và kích hoạt cảnh báo khi điều kiện duy trì. */
  void updateIntrusionLogic(const HardwareSnapshot &s) {
    pet_detected = calculatePetDetected(s);

    rawIntrusionScore = calculateRawIntrusionScore(s);

    // Simplified intrusion logic based strictly on requirements
    intrusion_score = rawIntrusionScore;

    // Bỏ qua giá trị PIR/siêu âm thoáng qua ngay sau khi cấp nguồn.
    if (bootCompletedAtMs != 0 && millis() - bootCompletedAtMs < SENSOR_BOOT_GRACE_MS) {
      return;
    }

    // Đột nhập cần đồng thời hai tín hiệu vật lý. Vật ở rất gần được dành riêng
    // cho luồng chống phá hoại và không tạo cảnh báo đột nhập.
    bool intrusionCondition = system_armed && // Điều kiện tổng hợp phải duy trì trước khi chốt đột nhập.
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

    // Quan trọng: intrusion_alert là chốt; điểm giảm không tự xóa cảnh báo.
  }

  /** Chọn câu trạng thái ưu tiên cao nhất (SOS, nghiêm trọng, phá hoại, đột nhập, canh gác). */
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

  alarm_status = "Đang tắt bảo vệ";
  }

  /** Tổng hợp RTC, camera, Wi-Fi và phá hoại thành mô tả sức khỏe thiết bị. */
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

  /** Tính mức đe dọa 0-4 dựa trên các chốt cảnh báo và điểm xâm nhập hiện tại. */
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

  /** Đồng bộ các cờ/trường tương thích mô tả yêu cầu thông báo và chụp ảnh hiện tại. */
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

  /** Điều phối toàn bộ logic an ninh của một snapshot rồi cập nhật đầu ra và trạng thái Cloud. */
  void updateSecurityLogic(const HardwareSnapshot &s) {
    clampCloudConfigValues();
    updateSystemArmed();
    updateSabotageLogic(s);
    updateIntrusionLogic(s);

    updateAlarmStatus();
    updateThreatLevel();
    updateDeviceHealthStatus();
    updateNotificationRequestPlaceholders();
    applyAlarmOutputs();
  }

  // ==================================================
  // LOGIC LỊCH TỰ ĐỘNG
  // ==================================================

  /** So khớp RTC với lịch tự bật/tắt, đảm bảo mỗi mốc chỉ chạy một lần trong cùng phút. */
  void updateScheduleLogic() {
    if (!schedule_enabled || !rtcOk) {
      return;
    }

    DateTime now = rtc.now(); // Thời gian hiện tại dùng so với lịch.
    String key = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " " + String(now.hour()) + ":" + String(now.minute()); // Khóa duy nhất của phút hiện tại.

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
  // LỆNH DEBUG QUA SERIAL
  // ==================================================

  /** In danh sách lệnh thử nghiệm hỗ trợ qua Serial Monitor. */
  void printHelp() {
    Serial.println();
    Serial.println("========== FINAL SERIAL COMMANDS ==========");
    Serial.println("h : show help");
    Serial.println("a : toggle alarm_enabled");
    Serial.println("c : trigger Child SOS");
    Serial.println("p : trigger Parent/Admin SOS");
    Serial.println("r : reset all alerts");
    Serial.println("1 : sensitivity_level = 1, threshold = 5");
    Serial.println("2 : sensitivity_level = 2, threshold = 4");
    Serial.println("3 : sensitivity_level = 3, threshold = 3");
    Serial.println("==============================================");
  }

  /** Thực thi một ký tự lệnh Serial để thử bật báo động, SOS, reset hoặc chỉnh độ nhạy. */
  void handleSerialCommand(char command) {
    if (command == '\n' || command == '\r' || command == ' ') {
      return;
    }

    if (command == 'h' || command == 'H') {
      printHelp();
    } else if (command == 'a' || command == 'A') {
      alarm_enabled = !alarm_enabled;
      setLastEvent("serial_alarm_enabled", String("Serial alarm_enabled = ") + (alarm_enabled ? "true" : "false"));
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

  /** Đọc hết dữ liệu đang chờ trong bộ đệm Serial và chuyển từng ký tự cho hàm xử lý lệnh. */
  void handleSerialInput() {
    while (Serial.available() > 0) {
      handleSerialCommand(Serial.read());
    }
  }

  /** In báo cáo chẩn đoán định kỳ gồm kết nối, cảm biến, cảnh báo và trạng thái gửi. */
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
    Serial.print("last_event                  : "); Serial.println(last_event);
    Serial.println("===========================================");
  }

  // ==================================================
  // KHỞI TẠO / VÒNG LẶP
  // ==================================================

  /**
   * Khởi tạo Serial, GPIO, RTC, giá trị runtime, camera, thuộc tính Cloud và kết nối mạng.
   * Arduino gọi đúng một lần sau khi bo mạch khởi động hoặc reset.
   */
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

    // Đây là các chốt cảnh báo runtime. Chúng phải bắt đầu ở trạng thái xóa sau
    // reset; cảm biến vật lý sẽ được đánh giá lại sau thời gian ổn định khởi động.
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

    // Giá trị Cloud mặc định. Sau khi kết nối, IoT Cloud có thể ghi đè thuộc tính
    // READWRITE bằng giá trị mới nhất trên Dashboard.
    alarm_enabled = true;
    system_armed = false;
    reset_alarm = false;
    sos_child = false;
    sos_adult = false;
    telegram_enabled = true;
    google_script_enabled = true;
    heartbeat_enabled = true;
    camera_enabled = false;
    gemini_enabled = false;
    auto_photo_on_alert = false;

    if (sensitivity_level < 1 || sensitivity_level > 3) {
      sensitivity_level = 3;
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
    heartbeat_status = "Chưa gửi heartbeat";
    last_heartbeat_time = "Chưa có heartbeat";
    device_health_status = "Thiết bị hoạt động bình thường";
    critical_security_compromise = false;

    // Trạng thái nâng cấp SOS đến cơ quan. Arduino Cloud có thể ghi đè giá trị
    // READWRITE sau khi kết nối.
    home_address_configured = false;
    sos_authority_note = "";
    setAuthorityStatus("IDLE");

    auto_capture_photo_request = false;
    cooldown_active = false;

    // =======================
    // KHỞI TẠO CAMERA
    // =======================
    // QUAN TRỌNG:
    // Phải gán cameraReady tại đây. Nếu setup() không gọi initCamera(),
    // manual_capture_photo luôn lỗi CAMERA_NOT_READY dù đấu dây camera đúng.
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

  /**
   * Vòng lặp chính: duy trì Cloud/Wi-Fi/heartbeat, đọc cảm biến theo chu kỳ và in báo cáo.
   * Các mốc millis() giúp không chặn luồng bằng delay dài.
   */
  void loop() {
    ArduinoCloud.update();
    handleSerialInput();
    maintainWiFiConnection();
    processGoogleScriptHeartbeatMonitorControl();
    processGoogleAppsScriptHeartbeat();

    static unsigned long lastSensorUpdate = 0; // Mốc chu kỳ đọc cảm biến gần nhất.
    if (millis() - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL_MS) {
      lastSensorUpdate = millis();

      HardwareSnapshot snapshot = readHardwareSnapshot(); // Bộ dữ liệu cảm biến nhất quán của chu kỳ hiện tại.
      publishHardwareSnapshotToCloud(snapshot);
      updateScheduleLogic();
      updateSecurityLogic(snapshot);
      processCameraRequests();
    }

    static unsigned long lastReport = 0; // Mốc lần in báo cáo Serial gần nhất.
    if (millis() - lastReport >= SERIAL_REPORT_INTERVAL_MS) {
      lastReport = millis();
      printSecurityReport();
    }
  }

  // ==================================================
  // CALLBACK TỪ CLOUD
  // ==================================================

  /** Callback ghi nhận khi người dùng thay đổi alarm_enabled trên Dashboard. */
  void onAlarmEnabledChange() {
    setLastEvent(
      "cloud_alarm_enabled",
      alarm_enabled
        ? "Phụ huynh/Admin đã bật chức năng phát hiện đột nhập."
        : "Phụ huynh/Admin đã tắt chức năng phát hiện đột nhập. SOS và chống phá hoại vẫn hoạt động."
    );
  }

  /** Callback xử lý nút reset_alarm; reset xong tự trả nút về false. */
  void onResetAlarmChange() {
    if (reset_alarm) {
      resetAllAlerts();
      reset_alarm = false;
    }
  }

/** Callback kích hoạt SOS trẻ em khi nút sos_child được bật rồi tự nhả nút. */
void onSosChildChange() {
  if (sos_child) {
    triggerSosAlert("CHILD");
    sos_child = false;
  }
}

/** Callback kích hoạt SOS người lớn khi nút sos_adult được bật rồi tự nhả nút. */
void onSosAdultChange() {
  if (sos_adult) {
    triggerSosAlert("PARENT_ADULT");
    sos_adult = false;
  }
}

  /** Callback chuẩn hóa mức nhạy mới và ghi sự kiện thay đổi cấu hình. */
  void onSensitivityLevelChange() {
    clampCloudConfigValues();
    setLastEvent("cloud_sensitivity", "Đã cập nhật mức nhạy phát hiện đột nhập: " + String(sensitivity_level) + ".");
  }

/** Callback chuyển nút chụp Cloud thành yêu cầu nội bộ để loop() xử lý an toàn. */
void onManualCapturePhotoChange() {
  if (manual_capture_photo) {
    manualCapturePending = true;
    manual_capture_photo = false;
    photo_status = "Đã nhận yêu cầu chụp ảnh thủ công";
    setLastEvent("manual_capture_requested", "Người lớn yêu cầu chụp ảnh thủ công lúc " + getRtcTimeString() + ".");
  }
}

  /** Callback bật/tắt lịch, chuẩn hóa thời gian và ghi thông báo lịch tương ứng. */
  void onScheduleEnabledChange() {
    clampCloudConfigValues();
    if (schedule_enabled) {
      setLastEvent(
        "cloud_schedule_enabled",
        "Đã bật chế độ lịch tự động thành công. Hệ thống sẽ tự bật bảo vệ lúc " +
        twoDigits(auto_arm_hour) + ":" + twoDigits(auto_arm_minute) +
        " và tự tắt bảo vệ lúc " + twoDigits(auto_disarm_hour) + ":" +
        twoDigits(auto_disarm_minute) + "."
      );
    } else {
      setLastEvent(
        "cloud_schedule_disabled",
        "Đã tắt chế độ lịch tự động. Hệ thống sẽ không tự bật/tắt bảo vệ theo lịch."
      );
    }
  }

  /** Callback chuẩn hóa giờ tự bật bảo vệ sau khi Dashboard thay đổi. */
  void onAutoArmHourChange() {
    clampCloudConfigValues();
  }

  /** Callback chuẩn hóa phút tự bật bảo vệ sau khi Dashboard thay đổi. */
  void onAutoArmMinuteChange() {
    clampCloudConfigValues();
  }

  /** Callback chuẩn hóa giờ tự tắt bảo vệ sau khi Dashboard thay đổi. */
  void onAutoDisarmHourChange() {
    clampCloudConfigValues();
  }

  /** Callback chuẩn hóa phút tự tắt bảo vệ sau khi Dashboard thay đổi. */
  void onAutoDisarmMinuteChange() {
    clampCloudConfigValues();
  }

/** Callback ghi nhận việc bật/tắt tự động chụp ảnh khi có cảnh báo. */
void onAutoPhotoOnAlertChange() {
  setLastEvent("cloud_auto_photo", auto_photo_on_alert
    ? "Đã bật tự chụp ảnh khi có sự kiện cảnh báo."
    : "Đã tắt tự chụp ảnh khi có sự kiện cảnh báo.");
}

/** Callback ghi nhận quyền bật/tắt camera từ Dashboard. */
void onCameraEnabledChange() {
  setLastEvent("cloud_camera", camera_enabled
    ? "Đã bật camera thành công. Hệ thống có thể chụp ảnh theo yêu cầu hoặc khi xảy ra cảnh báo."
    : "Đã tắt camera. Hệ thống sẽ không chụp hoặc gửi ảnh mới.");
}

/** Callback ghi nhận quyền bật/tắt phân tích ảnh bằng Gemini. */
void onGeminiEnabledChange() {
  setLastEvent("cloud_gemini", gemini_enabled
    ? "Đã bật nhận diện người qua ảnh bằng AI. AI chỉ phân tích khi camera đang bật."
    : "Đã tắt nhận diện người qua ảnh bằng AI.");
}

/** Callback xếp lệnh đồng bộ heartbeat-monitor và ghi trạng thái Apps Script mới. */
void onGoogleScriptEnabledChange() {
  requestGoogleScriptHeartbeatMonitorState(google_script_enabled && heartbeat_enabled);
  setLastEvent("cloud_apps_script", google_script_enabled
    ? "Đã bật gửi Gmail qua Google Apps Script."
    : "Đã tắt gửi Gmail qua Google Apps Script.");
}

/** Callback xếp lệnh đồng bộ và ghi nhận việc bật/tắt heartbeat. */
void onHeartbeatEnabledChange() {
  requestGoogleScriptHeartbeatMonitorState(google_script_enabled && heartbeat_enabled);
  setLastEvent("cloud_heartbeat", heartbeat_enabled
    ? "Đã bật tín hiệu theo dõi thiết bị (heartbeat) liên tục."
    : "Đã tắt tín hiệu theo dõi thiết bị (heartbeat).");
}

/** Callback ghi nhận việc bật/tắt kênh Telegram. */
void onTelegramEnabledChange() {
  setLastEvent("cloud_telegram", telegram_enabled
    ? "Đã bật gửi thông báo Telegram."
    : "Đã tắt gửi thông báo Telegram.");
}

  /** Callback ghi nhận độ dài ghi chú SOS nhưng không in nội dung nhạy cảm ra log. */
  void onSosAuthorityNoteChange() {
    // Không in toàn bộ ghi chú ra Serial/log vì có thể chứa nội dung khẩn cấp nhạy cảm.
    setLastEvent(
      "cloud_sos_note",
      "sos_authority_note updated, length=" + String(sos_authority_note.length()) +
      " at " + getRtcTimeString()
    );
  }
