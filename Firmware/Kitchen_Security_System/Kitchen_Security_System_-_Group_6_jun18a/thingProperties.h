// Tệp được Arduino IoT Cloud sinh tự động.
// Chú ý: chỉ bổ sung comment để giải thích; không thay đổi tên/kiểu/quyền truy cập
// của các thuộc tính vì Dashboard và firmware cùng phụ thuộc vào các khai báo này.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// Mã đăng nhập duy nhất của bo mạch trên Arduino IoT Cloud.
const char DEVICE_LOGIN_NAME[]  = "20c65f7e-3a23-4472-8634-3df4df3e8a76";

const char SSID[]               = SECRET_SSID;             // Lưu tên mạng Wi-Fi mà ESP32-S3 sẽ kết nối.
const char PASS[]               = SECRET_OPTIONAL_PASS;    // Lưu mật khẩu Wi-Fi (WPA) hoặc khóa mạng (WEP).
const char DEVICE_KEY[]         = SECRET_DEVICE_KEY;       // Lưu khóa bí mật để bo mạch xác thực với Arduino IoT Cloud.

// Các nguyên mẫu callback bên dưới được Arduino IoT Cloud gọi khi một thuộc tính
// READWRITE tương ứng thay đổi từ Dashboard.
void onSosAuthorityNoteChange();
void onAutoArmHourChange();
void onAutoArmMinuteChange();
void onAutoDisarmHourChange();
void onAutoDisarmMinuteChange();
void onSensitivityLevelChange();
void onAlarmEnabledChange();
void onAutoPhotoOnAlertChange();
void onCameraEnabledChange();
void onGeminiEnabledChange();
void onGoogleScriptEnabledChange();
void onHeartbeatEnabledChange();
void onManualCapturePhotoChange();
void onResetAlarmChange();
void onScheduleEnabledChange();
void onSosAdultChange();
void onSosChildChange();
void onTelegramEnabledChange();

// ===== Thuộc tính dạng chuỗi hiển thị trạng thái trên Dashboard =====
String alarm_status;                         // Lưu mô tả trạng thái tổng quát hiện tại của hệ thống báo động.
String current_time;                         // Lưu thời gian hiện tại đọc từ RTC để đồng bộ lên Cloud.
String device_health_status;                 // Lưu mô tả tình trạng hoạt động/sức khỏe của thiết bị.
String emergency_authority_message_status;   // Lưu trạng thái gửi thư khẩn cấp đến liên hệ cơ quan demo.
String emergency_escalation_status;          // Lưu mã trạng thái của quy trình xác nhận và nâng cấp SOS.
String heartbeat_status;                     // Lưu kết quả lần gửi heartbeat gần nhất đến Apps Script.
String last_event;                           // Lưu nội dung dễ đọc của sự kiện hệ thống mới nhất.
String last_heartbeat_time;                  // Lưu thời điểm heartbeat gần nhất đã được gửi thành công.
String notification_sent_status;             // Lưu trạng thái gửi thông báo Telegram/Gmail gần nhất.
String photo_status;                         // Lưu trạng thái khởi tạo, chụp, phân tích hoặc gửi ảnh camera.
String sos_authority_note;                    // Lưu ghi chú khẩn cấp do người lớn nhập để gửi kèm khi xác nhận SOS.

// ===== Thuộc tính số dùng để cấu hình lịch và độ nhạy =====
int auto_arm_hour;                            // Lưu giờ (0-23) tự động bật bảo vệ.
int auto_arm_minute;                          // Lưu phút (0-59) tự động bật bảo vệ.
int auto_disarm_hour;                         // Lưu giờ (0-23) tự động tắt bảo vệ.
int auto_disarm_minute;                       // Lưu phút (0-59) tự động tắt bảo vệ.
int sensitivity_level;                       // Lưu mức nhạy phát hiện đột nhập từ 1 đến 3.

// ===== Cờ bật/tắt và cờ trạng thái đồng bộ với Dashboard =====
bool alarm_enabled;                           // Cho biết người dùng có bật chức năng phát hiện đột nhập hay không.
bool auto_photo_on_alert;                     // Cho biết có tự chụp ảnh khi phát sinh cảnh báo hay không.
bool camera_enabled;                          // Cho biết camera có được phép chụp và gửi ảnh hay không.
bool critical_security_compromise;            // Báo thiết bị đang ở trạng thái an ninh bị xâm phạm nghiêm trọng.
bool gemini_enabled;                          // Cho biết có cho phép Gemini phân tích người trong ảnh hay không.
bool google_script_enabled;                   // Cho biết có cho phép gửi dữ liệu đến Google Apps Script hay không.
bool heartbeat_enabled;                       // Cho biết có cho phép gửi tín hiệu heartbeat định kỳ hay không.
bool home_address_configured;                 // Cho biết Apps Script đã có địa chỉ nhà hợp lệ để nâng cấp SOS hay chưa.
bool intrusion_alert;                         // Báo chốt cảnh báo đột nhập hiện đang hoạt động.
bool manual_capture_photo;                    // Nút yêu cầu chụp ảnh thủ công từ Dashboard.
bool reset_alarm;                             // Nút yêu cầu xóa toàn bộ cảnh báo đang chốt.
bool sabotage_alert;                          // Báo chốt cảnh báo phá hoại/che cảm biến đang hoạt động.
bool schedule_enabled;                        // Cho biết lịch tự bật/tắt bảo vệ có đang được sử dụng hay không.
bool sos_adult;                               // Nút SOS dành cho phụ huynh/người lớn trên Dashboard.
bool sos_child;                               // Nút SOS dành cho trẻ em trên Dashboard.
bool system_armed;                            // Trạng thái tổng hợp cho biết hệ thống hiện có đang canh gác hay không.
bool telegram_enabled;                        // Cho biết có cho phép gửi thông báo qua Telegram hay không.

/**
 * Đăng ký toàn bộ biến Cloud, quyền READ/READWRITE, chu kỳ đồng bộ và callback.
 * Hàm không trả về dữ liệu; tác động phụ là cấu hình đối tượng ArduinoCloud.
 */
void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(alarm_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(current_time, READ, 1 * SECONDS, NULL);
  ArduinoCloud.addProperty(device_health_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(emergency_authority_message_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(emergency_escalation_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(heartbeat_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(last_event, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(last_heartbeat_time, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(notification_sent_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(photo_status, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(sos_authority_note, READWRITE, ON_CHANGE, onSosAuthorityNoteChange);
  ArduinoCloud.addProperty(auto_arm_hour, READWRITE, ON_CHANGE, onAutoArmHourChange);
  ArduinoCloud.addProperty(auto_arm_minute, READWRITE, ON_CHANGE, onAutoArmMinuteChange);
  ArduinoCloud.addProperty(auto_disarm_hour, READWRITE, ON_CHANGE, onAutoDisarmHourChange);
  ArduinoCloud.addProperty(auto_disarm_minute, READWRITE, ON_CHANGE, onAutoDisarmMinuteChange);
  ArduinoCloud.addProperty(sensitivity_level, READWRITE, ON_CHANGE, onSensitivityLevelChange);
  ArduinoCloud.addProperty(alarm_enabled, READWRITE, ON_CHANGE, onAlarmEnabledChange);
  ArduinoCloud.addProperty(auto_photo_on_alert, READWRITE, ON_CHANGE, onAutoPhotoOnAlertChange);
  ArduinoCloud.addProperty(camera_enabled, READWRITE, ON_CHANGE, onCameraEnabledChange);
  ArduinoCloud.addProperty(critical_security_compromise, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(gemini_enabled, READWRITE, ON_CHANGE, onGeminiEnabledChange);
  ArduinoCloud.addProperty(google_script_enabled, READWRITE, ON_CHANGE, onGoogleScriptEnabledChange);
  ArduinoCloud.addProperty(heartbeat_enabled, READWRITE, ON_CHANGE, onHeartbeatEnabledChange);
  ArduinoCloud.addProperty(home_address_configured, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(intrusion_alert, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(manual_capture_photo, READWRITE, ON_CHANGE, onManualCapturePhotoChange);
  ArduinoCloud.addProperty(reset_alarm, READWRITE, ON_CHANGE, onResetAlarmChange);
  ArduinoCloud.addProperty(sabotage_alert, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(schedule_enabled, READWRITE, ON_CHANGE, onScheduleEnabledChange);
  ArduinoCloud.addProperty(sos_adult, READWRITE, ON_CHANGE, onSosAdultChange);
  ArduinoCloud.addProperty(sos_child, READWRITE, ON_CHANGE, onSosChildChange);
  ArduinoCloud.addProperty(system_armed, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(telegram_enabled, READWRITE, ON_CHANGE, onTelegramEnabledChange);

}

// Đối tượng quản lý kết nối Wi-Fi mà ArduinoCloud.begin() sử dụng.
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
