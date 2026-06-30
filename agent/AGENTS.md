# Quy tắc làm việc cho Agent trong Repository IoT Kitchen Security System

## 1. Bối cảnh dự án
* **Project:** IoT Based Anti-Theft System / Kitchen Security System - Group 6.
* **Board chính:** ESP32-CAM.
* **Cloud:** Arduino IoT Cloud.
* **Gửi cảnh báo:** Google Apps Script Gmail + Telegram text.
* **Camera:** Tạm thời bỏ qua vì module hiện tại không hỗ trợ JPEG ổn định.

## 2. Nguyên tắc biến Arduino Cloud
Chỉ được sử dụng các biến đã định nghĩa sẵn trong file [thingProperties.h](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/Kitchen_Security_System_-_Group_6_jun18a/thingProperties.h). Tuyệt đối không tự ý thêm biến mới vào Cloud mà không cập nhật cloud properties tương ứng.

### Các biến được phép sử dụng:
* `alarm_status` (String)
* `email_event_type` (String)
* `email_sent_status` (String)
* `last_event` (String)
* `last_event_type` (String)
* `sos_message` (String)
* `kitchen_temperature` (float)
* `flame_value` (int)
* `ldr_value` (int)
* `demo_scenario` (int)
* `event_counter` (int)
* `sos_level` (int)
* `threat_level` (int)
* `alarm_enabled` (bool)
* `buzzer_on` (bool)
* `demo_mode` (bool)
* `fire_alert` (bool)
* `flame_detected` (bool)
* `intrusion_alert` (bool)
* `led_red_on` (bool)
* `pet_detected` (bool)
* `pir_detected` (bool)
* `reset_alarm` (bool)
* `sabotage_alert` (bool)
* `send_email_request` (bool)
* `sos_adult` (bool)
* `sos_child` (bool)
* `system_armed` (bool)
* `unknown_wifi_alert` (bool)
* `unknown_wifi_count` (int)
* `last_unknown_mac` (String)

### Các biến tuyệt đối KHÔNG được dùng (không có trong properties):
* `cooldown_active`
* `last_alert_time`
* `ldr_delta`
* `light_abnormal`
* `ldr_covered`
* `object_near`
* `intrusion_score`
* `device_tampered`
* `current_time`
* `current_hour`
* `night_mode`

## 3. Quy tắc văn phong cảnh báo
Tuyệt đối không viết thông báo kiểu debug kỹ thuật. Tin nhắn gửi đi phải mang tính đời thường, trực quan và thể hiện rõ sự khẩn cấp.

* **Không dùng:** "flame_detected changed to true", "fire_alert = 1", "sabotage_alert activated", "email_event_type changed".
* **Phải dùng:**
  * "NGUY HIỂM! Hệ thống phát hiện dấu hiệu cháy tại khu vực bếp. Vui lòng kiểm tra ngay."
  * "CẢNH BÁO KHẨN CẤP! Trẻ em vừa kích hoạt SOS. Người lớn cần kiểm tra tình hình ngay."
  * "CẢNH BÁO PHÁ HOẠI! Thiết bị phát hiện có người che cảm biến trong lúc có chuyển động gần khu vực bếp."
  * "Hệ thống đã bật còi và đèn đỏ để cảnh báo tại chỗ."
  * "Thông báo đã được gửi đến người nhà qua Gmail và Telegram."

## 4. Quy tắc trạng thái
Chuẩn hóa các giá trị trạng thái cho hệ thống log và dashboard:

### Giá trị cho `last_event_type`:
* `SOS_CHILD`
* `SOS_ADULT`
* `FIRE`
* `SABOTAGE`
* `INTRUSION`
* `PET_IGNORED`
* `SYSTEM_RESET`
* `DEMO`
* `EMAIL_SENT`
* `EMAIL_FAILED`
* `TELEGRAM_SENT`
* `TELEGRAM_FAILED`

### Giá trị cho `alarm_status`:
* `SAFE`
* `ARMED`
* `SOS_CHILD_ALERT`
* `SOS_ADULT_ALERT`
* `FIRE_ALERT`
* `SABOTAGE_ALERT`
* `INTRUSION_ALERT`
* `PET_IGNORED`
* `DEMO_RUNNING`
* `EMAIL_SENDING`
* `ALERT_SENT`
* `ALERT_FAILED`

### Mức độ đe dọa `threat_level`:
* `0` = An toàn
* `1` = Nghi ngờ nhẹ
* `2` = Cảnh báo
* `3` = Nguy hiểm
* `4` = Khẩn cấp

## 5. Quy tắc Demo
Khi `demo_mode == true`, cho phép sử dụng biến `demo_scenario` để ép chạy các kịch bản demo:
* `0` = Chạy bình thường
* `1` = Demo đột nhập
* `2` = Demo vật nuôi bị bỏ qua
* `6` = Demo phá hoại thiết bị
* `7` = Demo báo cháy
* `8` = Demo SOS trẻ em

*Lưu ý:* Sau khi xử lý một demo scenario, hệ thống phải tự đưa `demo_scenario = 0` để tránh chạy lặp liên tục. Không được tự ý tắt `demo_mode`.
