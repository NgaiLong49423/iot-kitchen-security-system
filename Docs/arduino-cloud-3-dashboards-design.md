# Thiết Kế 3 Dashboard Cho Arduino Cloud

| Item | Nội dung |
|---|---|
| Project | IoT Based Anti-Theft System |
| Nhóm | Group 6 - SE2034 |
| Mục đích tài liệu | Mô tả cách tạo và bố trí 3 Dashboard trên Arduino Cloud |
| Phạm vi | Chỉ nói về Dashboard, widget và biến cần hiển thị/điều khiển |
| Version | 1.0 |
| Updated | June 19, 2026 |

---

# 1. Tổng Quan Dashboard

Dự án nên có **3 Dashboard** theo vai trò người dùng:

| STT | Dashboard | Người dùng | Mục đích chính |
|---|---|---|---|
| 1 | `Child SOS Dashboard` | Trẻ em | Chỉ bấm SOS và xem trạng thái cơ bản |
| 2 | `Parent Control Dashboard` | Người lớn | Điều khiển hệ thống, xem cảnh báo, xử lý báo động |
| 3 | `Admin Demo Dashboard` | Leader/người demo | Xem toàn bộ hệ thống và chọn kịch bản demo |

Nguyên tắc thiết kế:

```text
Trẻ em → ít nút nhất, tránh bấm nhầm.
Người lớn → đủ quyền xử lý hệ thống.
Admin → có toàn bộ biến để kiểm thử và demo.
```

---

# 2. Dashboard 1 - Child SOS Dashboard

## 2.1. Mục đích

Dashboard này dành cho **trẻ em**.

Trẻ em chỉ cần một chức năng chính:

```text
Bấm SOS khi gặp nguy hiểm.
```

Không nên cho trẻ em thấy các nút như reset, bật/tắt bảo vệ, demo mode hoặc chụp ảnh, vì dễ bấm nhầm và làm rối hệ thống.

---

## 2.2. Danh sách widget

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `SOS KHẨN CẤP` | Push Button hoặc Switch | `sos_child` | Trẻ em bấm SOS để gửi tín hiệu khẩn cấp |
| 2 | `Trạng thái hệ thống` | Value / Messenger | `alarm_status` | Cho biết hệ thống đang ở trạng thái nào |
| 3 | `Sự kiện gần nhất` | Value / Messenger | `last_event` | Hiển thị sự kiện vừa xảy ra |

---

## 2.3. Biến dùng trong dashboard

```cpp
bool sos_child;
String alarm_status;
String last_event;
```

---

## 2.4. Bố cục gợi ý

```text
┌──────────────────────────────┐
│        SOS KHẨN CẤP          │
└──────────────────────────────┘

┌──────────────────────────────┐
│ Trạng thái hệ thống          │
│ alarm_status                 │
└──────────────────────────────┘

┌──────────────────────────────┐
│ Sự kiện gần nhất             │
│ last_event                   │
└──────────────────────────────┘
```

---

## 2.5. Ghi chú khi tạo

Nút `SOS KHẨN CẤP` nên làm to, dễ nhìn, dễ bấm.

Nếu dùng Switch, sau khi board nhận được SOS thì code nên tự đưa `sos_child` về `false` sau khi xử lý xong để tránh giữ trạng thái SOS mãi.

Gợi ý xử lý:

```text
Trẻ em bấm sos_child = true
→ board xử lý SOS
→ ghi log
→ gửi email người lớn
→ board set sos_child = false
```

---

# 3. Dashboard 2 - Parent Control Dashboard

## 3.1. Mục đích

Dashboard này dành cho **người lớn trong nhà**.

Người lớn cần:

```text
Bật/tắt bảo vệ.
Bấm SOS người lớn.
Reset báo động.
Xem trạng thái hệ thống.
Xem báo cháy.
Xem nhiệt độ bếp.
Xem email có gửi thành công không.
Chủ động yêu cầu chụp ảnh.
```

Dashboard này không cần hiển thị toàn bộ biến kỹ thuật như RFID UID, MAC lạ, demo scenario hoặc các biến debug chi tiết.

---

## 3.2. Danh sách widget

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Bật/Tắt bảo vệ` | Switch | `alarm_enabled` | Người lớn bật/tắt chế độ bảo vệ |
| 2 | `SOS Người Lớn` | Push Button hoặc Switch | `sos_adult` | Kích hoạt cảnh báo tại chỗ |
| 3 | `Reset Báo Động` | Push Button | `reset_alarm` | Dừng còi/LED và reset cảnh báo |
| 4 | `Trạng thái hệ thống` | Value / Messenger | `alarm_status` | Xem trạng thái hiện tại |
| 5 | `Sự kiện gần nhất` | Value / Messenger | `last_event` | Biết chuyện gì vừa xảy ra |
| 6 | `Mức nguy hiểm` | Value hoặc Gauge | `threat_level` | Xem mức độ nguy hiểm |
| 7 | `Báo cháy` | Status / LED | `fire_alert` | Biết hệ thống đã xác nhận cháy chưa |
| 8 | `Tín hiệu cảm biến lửa` | Gauge / Value | `flame_value` | Xem giá trị Flame Sensor |
| 9 | `Nhiệt độ bếp` | Gauge / Chart | `kitchen_temperature` | Xem nhiệt độ bếp theo °C |
| 10 | `Kết quả gửi email` | Value / Messenger | `email_sent_status` | Kiểm tra email đã gửi thành công hay chưa |
| 11 | `Chụp ảnh` | Push Button hoặc Switch | `capture_photo` | Yêu cầu ESP32-CAM chụp ảnh |
| 12 | `Trạng thái ảnh` | Value / Messenger | `photo_status` | Xem camera chụp thành công hay thất bại |

---

## 3.3. Biến dùng trong dashboard

```cpp
bool alarm_enabled;
bool sos_adult;
bool reset_alarm;
String alarm_status;
String last_event;
int threat_level;
bool fire_alert;
int flame_value;
float kitchen_temperature;
String email_sent_status;
bool capture_photo;
String photo_status;
```

---

## 3.4. Bố cục gợi ý

```text
┌──────────────────┐ ┌──────────────────┐
│ Bật/Tắt bảo vệ   │ │ Reset Báo Động    │
│ alarm_enabled    │ │ reset_alarm       │
└──────────────────┘ └──────────────────┘

┌──────────────────┐ ┌──────────────────┐
│ SOS Người Lớn    │ │ Chụp ảnh          │
│ sos_adult        │ │ capture_photo     │
└──────────────────┘ └──────────────────┘

┌────────────────────────────────────────┐
│ Trạng thái hệ thống                    │
│ alarm_status                           │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│ Sự kiện gần nhất                       │
│ last_event                             │
└────────────────────────────────────────┘

┌──────────────────┐ ┌──────────────────┐
│ Mức nguy hiểm    │ │ Báo cháy          │
│ threat_level     │ │ fire_alert        │
└──────────────────┘ └──────────────────┘

┌──────────────────┐ ┌──────────────────┐
│ Tín hiệu lửa     │ │ Nhiệt độ bếp      │
│ flame_value      │ │ kitchen_temp      │
└──────────────────┘ └──────────────────┘

┌────────────────────────────────────────┐
│ Kết quả gửi email                      │
│ email_sent_status                      │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│ Trạng thái ảnh                         │
│ photo_status                           │
└────────────────────────────────────────┘
```

---

## 3.5. Ghi chú khi tạo

`Reset Báo Động` nên dùng Push Button để tránh giữ trạng thái reset mãi.

`SOS Người Lớn` có thể dùng Push Button hoặc Switch. Nếu dùng Switch, code nên tự set `sos_adult = false` sau khi xử lý xong.

`flame_value` và `kitchen_temperature` nên dùng Gauge hoặc Chart để người lớn dễ quan sát biến động.

Dashboard người lớn không nên có:

```text
demo_mode
demo_scenario
rfid_uid
last_unknown_mac
unknown_wifi_count
intrusion_score
cooldown_active
```

Các biến này chủ yếu phục vụ debug hoặc demo, không cần cho người lớn xem.

---

# 4. Dashboard 3 - Admin Demo Dashboard

## 4.1. Mục đích

Dashboard này dành cho **leader, người demo hoặc người kiểm thử**.

Admin cần:

```text
Xem toàn bộ trạng thái hệ thống.
Điều khiển các nút quan trọng.
Theo dõi dữ liệu cảm biến.
Theo dõi email, log, camera.
Bật demo mode.
Chọn kịch bản demo.
```

Dashboard này có nhiều widget nhất, dùng khi kiểm thử hoặc bảo vệ đồ án.

---

## 4.2. Nhóm 1 - Điều khiển hệ thống

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Bật/Tắt bảo vệ` | Switch | `alarm_enabled` | Bật/tắt chế độ bảo vệ |
| 2 | `Hệ thống đang Armed` | Status / LED | `system_armed` | Xem hệ thống có đang gài bảo vệ thật không |
| 3 | `Reset báo động` | Push Button | `reset_alarm` | Dừng báo động |
| 4 | `Trạng thái hệ thống` | Value / Messenger | `alarm_status` | Xem trạng thái tổng quát |

---

## 4.3. Nhóm 2 - Demo

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Bật chế độ demo` | Switch | `demo_mode` | Bật/tắt chế độ demo |
| 2 | `Chọn kịch bản demo` | Slider / Value / Dropdown nếu có | `demo_scenario` | Chọn nhanh kịch bản cần trình diễn |

Ý nghĩa `demo_scenario`:

| Giá trị | Kịch bản |
|---|---|
| 0 | Chạy bình thường |
| 1 | Demo đột nhập |
| 2 | Demo vật nuôi |
| 3 | Demo mất điện / khôi phục |
| 4 | Demo WiFi lạ |
| 5 | Demo tự động bật/tắt bảo vệ |
| 6 | Demo phá hoại thiết bị |
| 7 | Demo cháy |
| 8 | Demo SOS trẻ em |

Ghi chú:

```text
demo_mode và demo_scenario chỉ dùng cho kiểm thử/trình diễn.
Khi chạy thực tế, demo_mode = false thì hệ thống vẫn xử lý theo cảm biến thật.
```

---

## 4.4. Nhóm 3 - SOS

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `SOS Trẻ Em` | Push Button / Switch | `sos_child` | Test SOS trẻ em |
| 2 | `SOS Người Lớn` | Push Button / Switch | `sos_adult` | Test SOS người lớn |
| 3 | `Mức SOS` | Value | `sos_level` | Xem mức độ SOS |
| 4 | `Nội dung SOS` | Value / Messenger | `sos_message` | Xem nội dung cảnh báo SOS |

---

## 4.5. Nhóm 4 - Cảm biến chuyển động và khoảng cách

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `PIR phát hiện chuyển động` | Status / LED | `pir_detected` | Xem PIR có phát hiện chuyển động không |
| 2 | `Khoảng cách HC-SR04` | Gauge / Chart | `ultrasonic_distance` | Xem khoảng cách vật thể |
| 3 | `Có vật thể gần` | Status / LED | `object_near` | Xem kết luận có vật thể gần |
| 4 | `Vật nuôi được bỏ qua` | Status / LED | `pet_detected` | Xem hệ thống có nhận là vật nuôi không |

---

## 4.6. Nhóm 5 - Ánh sáng LDR

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Giá trị ánh sáng LDR` | Gauge / Chart | `ldr_value` | Xem giá trị ánh sáng |
| 2 | `Độ thay đổi ánh sáng` | Gauge / Value | `ldr_delta` | Xem mức ánh sáng thay đổi đột ngột |
| 3 | `Ánh sáng bất thường` | Status / LED | `light_abnormal` | Xem hệ thống có phát hiện ánh sáng bất thường không |
| 4 | `LDR bị che` | Status / LED | `ldr_covered` | Xem LDR có bị che bất thường không |

---

## 4.7. Nhóm 6 - Báo cháy và nhiệt độ bếp

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Giá trị Flame Sensor` | Gauge / Chart | `flame_value` | Xem tín hiệu thô từ cảm biến lửa |
| 2 | `Phát hiện ngọn lửa` | Status / LED | `flame_detected` | Xem cảm biến có phát hiện dấu hiệu lửa không |
| 3 | `Nhiệt độ bếp` | Gauge / Chart | `kitchen_temperature` | Xem nhiệt độ khu vực bếp |
| 4 | `Cảnh báo cháy` | Status / LED | `fire_alert` | Xem hệ thống đã xác nhận cháy chưa |

Ghi chú:

```text
Flame Sensor không đo nhiệt độ trực tiếp.
flame_value dùng để xem tín hiệu cảm biến lửa.
kitchen_temperature dùng để xem nhiệt độ bếp từ cảm biến nhiệt riêng.
```

---

## 4.8. Nhóm 7 - Đột nhập và nguy hiểm

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Cảnh báo đột nhập` | Status / LED | `intrusion_alert` | Xem hệ thống đã xác nhận đột nhập chưa |
| 2 | `Điểm nghi ngờ đột nhập` | Value / Gauge | `intrusion_score` | Xem điểm nghi ngờ từ thuật toán |
| 3 | `Mức nguy hiểm` | Value / Gauge | `threat_level` | Xem mức nguy hiểm tổng thể |
| 4 | `Cảnh báo phá hoại` | Status / LED | `sabotage_alert` | Xem có hành vi phá hoại thiết bị không |
| 5 | `Thiết bị bị can thiệp` | Status / LED | `device_tampered` | Xem thiết bị có dấu hiệu bị can thiệp không |

---

## 4.9. Nhóm 8 - Thời gian DS1307

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Thời gian hiện tại` | Value | `current_time` | Xem giờ hiện tại |
| 2 | `Giờ hiện tại` | Value | `current_hour` | Xem giờ dạng số |
| 3 | `Chế độ ban đêm` | Status / LED | `night_mode` | Xem hệ thống có đang ở night mode không |

---

## 4.10. Nhóm 9 - RFID

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `UID RFID vừa quét` | Value | `rfid_uid` | Xem UID thẻ vừa quét |
| 2 | `Thẻ hợp lệ` | Status / LED | `rfid_authorized` | Xem thẻ có hợp lệ không |
| 3 | `Trạng thái truy cập` | Value | `access_status` | Xem ACCESS_GRANTED hoặc ACCESS_DENIED |

---

## 4.11. Nhóm 10 - WiFi lạ và MAC whitelist

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Thiết bị người nhà gần đó` | Status / LED | `known_device_present` | Xem có thiết bị quen thuộc gần đó không |
| 2 | `Số lần phát hiện WiFi lạ` | Value / Gauge | `unknown_wifi_count` | Xem số lần bắt được thiết bị lạ |
| 3 | `Cảnh báo WiFi lạ` | Status / LED | `unknown_wifi_alert` | Xem hệ thống có xác nhận WiFi lạ đáng nghi không |
| 4 | `MAC lạ gần nhất` | Value | `last_unknown_mac` | Xem MAC lạ gần nhất |

---

## 4.12. Nhóm 11 - Camera ESP32-CAM

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Chụp ảnh` | Push Button / Switch | `capture_photo` | Yêu cầu camera chụp ảnh |
| 2 | `Trạng thái ảnh` | Value | `photo_status` | Xem camera chụp thành công hay lỗi |

---

## 4.13. Nhóm 12 - Còi và LED

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Còi đang kêu` | Status / LED | `buzzer_on` | Xem còi có đang bật không |
| 2 | `LED đỏ đang bật` | Status / LED | `led_red_on` | Xem LED đỏ có đang bật không |

---

## 4.14. Nhóm 13 - Email

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Yêu cầu gửi email` | Status / LED | `send_email_request` | Xem hệ thống có đang yêu cầu gửi email không |
| 2 | `Loại email cảnh báo` | Value | `email_event_type` | Xem loại email: SOS, fire, intrusion... |
| 3 | `Trạng thái gửi email` | Value | `email_sent_status` | Xem email gửi thành công hay lỗi |

---

## 4.15. Nhóm 14 - Log sự kiện

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Sự kiện gần nhất` | Value / Messenger | `last_event` | Xem sự kiện gần nhất |
| 2 | `Loại sự kiện gần nhất` | Value | `last_event_type` | Xem loại sự kiện |
| 3 | `Số sự kiện` | Value | `event_counter` | Xem tổng số sự kiện hệ thống đã ghi nhận |

---

## 4.16. Nhóm 15 - Chống spam cảnh báo

| STT | Tên widget hiển thị | Loại widget gợi ý | Gắn với biến | Mục đích |
|---|---|---|---|---|
| 1 | `Đang chống spam` | Status / LED | `cooldown_active` | Xem hệ thống có đang trong thời gian chống spam không |
| 2 | `Lần cảnh báo gần nhất` | Value | `last_alert_time` | Xem thời điểm gửi cảnh báo gần nhất |

---

# 5. Bố Cục Tổng Thể Cho Admin Demo Dashboard

Nên sắp theo thứ tự ưu tiên khi demo:

```text
Hàng 1: Điều khiển hệ thống
[alarm_enabled] [system_armed] [reset_alarm] [alarm_status]

Hàng 2: Demo
[demo_mode] [demo_scenario]

Hàng 3: SOS
[sos_child] [sos_adult] [sos_level] [sos_message]

Hàng 4: Cảm biến chuyển động
[pir_detected] [ultrasonic_distance] [object_near] [pet_detected]

Hàng 5: Ánh sáng
[ldr_value] [ldr_delta] [light_abnormal] [ldr_covered]

Hàng 6: Báo cháy
[flame_value] [flame_detected] [kitchen_temperature] [fire_alert]

Hàng 7: Đột nhập và nguy hiểm
[intrusion_alert] [intrusion_score] [threat_level] [sabotage_alert] [device_tampered]

Hàng 8: Thời gian
[current_time] [current_hour] [night_mode]

Hàng 9: RFID
[rfid_uid] [rfid_authorized] [access_status]

Hàng 10: WiFi lạ
[known_device_present] [unknown_wifi_count] [unknown_wifi_alert] [last_unknown_mac]

Hàng 11: Camera + Output
[capture_photo] [photo_status] [buzzer_on] [led_red_on]

Hàng 12: Email + Log
[send_email_request] [email_event_type] [email_sent_status] [last_event] [event_counter]

Hàng 13: Chống spam
[cooldown_active] [last_alert_time]
```

---

# 6. Tóm Tắt Nhanh 3 Dashboard

## 6.1. Child SOS Dashboard

```cpp
sos_child
alarm_status
last_event
```

Mục tiêu:

```text
Trẻ em chỉ có một nhiệm vụ: bấm SOS khi nguy hiểm.
```

---

## 6.2. Parent Control Dashboard

```cpp
alarm_enabled
sos_adult
reset_alarm
alarm_status
last_event
threat_level
fire_alert
flame_value
kitchen_temperature
email_sent_status
capture_photo
photo_status
```

Mục tiêu:

```text
Người lớn có đủ quyền xử lý hệ thống và theo dõi cảnh báo quan trọng.
```

---

## 6.3. Admin Demo Dashboard

```cpp
alarm_enabled
system_armed
reset_alarm
alarm_status

demo_mode
demo_scenario

sos_child
sos_adult
sos_level
sos_message

pir_detected
ultrasonic_distance
object_near
pet_detected

ldr_value
ldr_delta
light_abnormal
ldr_covered

flame_value
flame_detected
kitchen_temperature
fire_alert

intrusion_alert
intrusion_score
threat_level
sabotage_alert
device_tampered

current_time
current_hour
night_mode

rfid_uid
rfid_authorized
access_status

known_device_present
unknown_wifi_count
unknown_wifi_alert
last_unknown_mac

capture_photo
photo_status

buzzer_on
led_red_on

send_email_request
email_event_type
email_sent_status

last_event
last_event_type
event_counter

cooldown_active
last_alert_time
```

Mục tiêu:

```text
Admin/leader có thể quan sát toàn bộ hệ thống và linh hoạt chọn kịch bản demo.
```

---

# 7. Kết Luận

Thiết kế 3 Dashboard giúp hệ thống rõ vai trò hơn:

```text
Child Dashboard
→ đơn giản, an toàn, chỉ có SOS.

Parent Dashboard
→ đủ quyền xử lý cảnh báo và xem trạng thái quan trọng.

Admin Demo Dashboard
→ đầy đủ biến để debug, kiểm thử và bảo vệ đồ án.
```

Cách chia này giúp:

- Tránh trẻ em bấm nhầm chức năng nguy hiểm.
- Người lớn không bị rối bởi quá nhiều biến kỹ thuật.
- Admin có đủ công cụ để demo từng kịch bản.
- Dễ giải thích với giảng viên rằng hệ thống có phân chia giao diện theo vai trò.
