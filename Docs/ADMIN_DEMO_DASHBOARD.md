# ADMIN_DEMO_DASHBOARD.md

# Hướng dẫn cấu hình Admin Demo Dashboard

> **Tên tài liệu:** ADMIN_DEMO_DASHBOARD.md  
> **Phiên bản:** v1.0.0  
> **Ngày tạo:** 2026-06-30  
> **Sửa đổi gần nhất:** 2026-06-30  
> **Dự án:** Hệ Thống Chống Trộm IoT  
> **Dashboard:** Admin Demo Dashboard  
> **Mục đích:** Hướng dẫn cấu hình dashboard dành cho Admin / người demo / người kiểm thử trên Arduino Cloud.

---

## 1. Mục đích dashboard

`Admin Demo Dashboard` là dashboard dùng để:

- kiểm tra toàn bộ biến Arduino Cloud,
- test từng cảm biến sau khi cắm dây,
- debug trạng thái hệ thống,
- xem trạng thái cảnh báo,
- kiểm tra còi / LED / camera,
- chạy các kịch bản demo bằng `demo_mode` và `demo_scenario`.

Dashboard này không phải giao diện chính cho trẻ em hoặc phụ huynh.  
Dashboard này dành cho người làm dự án, người thuyết trình, người test và người code.

---

## 2. Tên dashboard cần tạo

Đặt tên dashboard trong Arduino Cloud là:

```text
Admin Demo Dashboard
```

---

## 3. Nguyên tắc cấu hình chung

Khi tạo widget trong Arduino Cloud:

- biến `Read Only` chỉ dùng để xem trạng thái,
- biến `Read & Write` có thể dùng Switch, Button, Slider hoặc Input,
- không tự tạo thêm biến ngoài danh sách đã chốt,
- không dùng lại biến cũ đã bị loại như `flame_value`, `fire_alert`, `rfid_uid`, `capture_photo`, `email_sent_status`,
- tên hiển thị trên dashboard nên viết tiếng Việt để khi demo dễ hiểu.

**Widget** là ô điều khiển hoặc ô hiển thị trên dashboard.  
Ví dụ: Switch, Button, Value, Gauge, Text, Status, LED.

---

## 4. Cấu trúc layout đề xuất

Nên chia dashboard thành các khu vực sau:

```text
1. System Control
2. Sensor Monitor
3. Intrusion Detection
4. Anti-Sabotage
5. Output Devices
6. Camera
7. SOS / Emergency
8. Notification
9. WiFi Detection
10. Event Log
11. Demo Control
```

Nên đặt các khu vực quan trọng ở phía trên:

```text
System Control
Sensor Monitor
Intrusion Detection
Output Devices
```

Các khu vực debug hoặc demo có thể đặt phía dưới:

```text
Notification
WiFi Detection
Event Log
Demo Control
```

---

# 5. Khu vực 1: System Control

Khu vực này dùng để điều khiển chế độ chống trộm, reset cảnh báo và xem trạng thái tổng thể.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Bật / Tắt chống trộm | `alarm_enabled` | Switch | Read & Write | Công tắc bật/tắt chức năng chống trộm |
| Trạng thái chống trộm thực tế | `system_armed` | Status / LED | Read Only | Hệ thống tự tính, người dùng không chỉnh trực tiếp |
| Reset cảnh báo | `reset_alarm` | Push Button | Read & Write | Nút reset báo động |
| Trạng thái hệ thống | `alarm_status` | Text | Read Only | Hiển thị trạng thái như `ARMED`, `DISARMED`, `INTRUSION_ALERT` |
| Thời gian tự bật lại | `rearm_delay_seconds` | Value / Input / Slider | Read & Write | Số giây chờ trước khi tự bật lại chống trộm |
| Đếm ngược tự bật lại | `rearm_countdown_remaining` | Value | Read Only | Hiển thị số giây còn lại |

## 5.1 Tên button / switch nên đặt

```text
Bật / Tắt chống trộm
Reset cảnh báo
```

## 5.2 Gợi ý cấu hình `rearm_delay_seconds`

Nếu dùng Slider:

```text
Tên hiển thị: Thời gian tự bật lại
Min: 5
Max: 600
Step: 5
Đơn vị: giây
```

Nếu Arduino Cloud không có Push Button, có thể dùng Switch cho `reset_alarm`.  
Trong code sau này cần tự đặt `reset_alarm = false` sau khi xử lý xong.

---

# 6. Khu vực 2: Sensor Monitor

Khu vực này dùng để test cảm biến thật sau khi cắm dây.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| PIR phát hiện chuyển động | `pir_detected` | Status / LED | Read Only | Test cảm biến PIR |
| Khoảng cách siêu âm | `ultrasonic_distance` | Value / Gauge | Read Only | Test cảm biến HY-SRF05 |
| Có vật ở gần | `object_near` | Status / LED | Read Only | Dựa trên khoảng cách siêu âm |
| Giá trị LDR | `ldr_value` | Value / Gauge | Read Only | Test cảm biến ánh sáng |
| Độ lệch ánh sáng | `ldr_delta` | Value | Read Only | Độ thay đổi ánh sáng |
| Ánh sáng bất thường | `light_abnormal` | Status / LED | Read Only | Dùng cho logic nghi ngờ |
| LDR bị che | `ldr_covered` | Status / LED | Read Only | Dùng cho anti-sabotage |

## 6.1 Tên widget nên đặt

```text
PIR phát hiện chuyển động
Khoảng cách siêu âm
Có vật ở gần
Giá trị LDR
Độ lệch ánh sáng
Ánh sáng bất thường
LDR bị che
```

## 6.2 Gợi ý hiển thị

- `ultrasonic_distance`: dùng Value hoặc Gauge.
- `ldr_value`: dùng Value hoặc Gauge.
- các biến bool như `pir_detected`, `object_near`, `ldr_covered`: dùng Status hoặc LED.

**Bool** là kiểu đúng/sai, thường hiển thị bằng bật/tắt.  
**Float** là số thập phân, ví dụ khoảng cách `25.4 cm`.  
**Int** là số nguyên.

---

# 7. Khu vực 3: Intrusion Detection

Khu vực này dùng để xem logic phát hiện đột nhập.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Cảnh báo đột nhập | `intrusion_alert` | Status / LED | Read Only | Bật khi hệ thống xác nhận có đột nhập |
| Điểm nghi ngờ đột nhập | `intrusion_score` | Value | Read Only | Điểm tổng từ PIR, siêu âm, LDR, night mode, WiFi lạ |
| Mức nguy hiểm | `threat_level` | Value / Gauge | Read Only | Mức nguy hiểm tổng thể |
| Nghi vật nuôi / báo nhầm | `pet_detected` | Status / LED | Read Only | Chỉ là nghi ngờ, không phải xác nhận chắc chắn |
| Mức nhạy phát hiện | `sensitivity_level` | Slider / Input | Read & Write | Cho chỉnh mức nhạy 1–3 |

## 7.1 Tên widget nên đặt

```text
Cảnh báo đột nhập
Điểm nghi ngờ đột nhập
Mức nguy hiểm
Nghi vật nuôi / báo nhầm
Mức nhạy phát hiện
```

## 7.2 Cấu hình `sensitivity_level`

Nếu dùng Slider:

```text
Tên hiển thị: Mức nhạy phát hiện
Min: 1
Max: 3
Step: 1
```

Ý nghĩa đề xuất:

```text
1 = Ít nhạy
2 = Bình thường
3 = Nhạy cao
```

---

# 8. Khu vực 4: Anti-Sabotage

Khu vực này dùng để xem cảnh báo phá hoại hoặc can thiệp thiết bị.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Cảnh báo phá hoại | `sabotage_alert` | Status / LED | Read Only | Cảnh báo chính về phá hoại |
| Thiết bị bị can thiệp | `device_tampered` | Status / LED | Read Only | Trạng thái thiết bị bị can thiệp |
| LDR bị che | `ldr_covered` | Status / LED | Read Only | Có thể dùng lại từ Sensor Monitor |
| Có vật quá gần cảm biến | `object_near` | Status / LED | Read Only | Có thể dùng lại từ Sensor Monitor |

## 8.1 Tên widget nên đặt

```text
Cảnh báo phá hoại
Thiết bị bị can thiệp
LDR bị che
Có vật quá gần cảm biến
```

Nếu dashboard bị chật, có thể bỏ `LDR bị che` và `Có vật quá gần cảm biến` ở khu vực này vì đã có trong Sensor Monitor.

---

# 9. Khu vực 5: Output Devices

Khu vực này dùng để xem trạng thái đầu ra vật lý.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Còi đang bật | `buzzer_on` | Status / LED | Read Only | Trạng thái buzzer |
| LED đỏ cảnh báo | `led_red_on` | Status / LED | Read Only | LED đỏ báo cảnh báo |
| LED xanh bình thường | `led_green_on` | Status / LED | Read Only | LED xanh báo hệ thống bình thường |

## 9.1 Tên widget nên đặt

```text
Còi đang bật
LED đỏ cảnh báo
LED xanh bình thường
```

---

# 10. Khu vực 6: Camera

Khu vực này dùng để test chụp ảnh thủ công và xem trạng thái chụp ảnh tự động.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Chụp ảnh thủ công | `manual_capture_photo` | Push Button | Read & Write | Admin bấm để yêu cầu chụp ảnh |
| Yêu cầu chụp ảnh tự động | `auto_capture_photo_request` | Status / LED | Read Only | Hệ thống bật khi có cảnh báo đột nhập |
| Trạng thái chụp ảnh | `photo_status` | Text | Read Only | Hiển thị `IDLE`, `CAPTURING`, `CAPTURED`, `FAILED` |

## 10.1 Tên button nên đặt

```text
Chụp ảnh thủ công
```

Nếu Arduino Cloud không có Push Button, dùng Switch.  
Trong code sau này cần tự đặt `manual_capture_photo = false` sau khi xử lý xong.

---

# 11. Khu vực 7: SOS / Emergency

Khu vực này dùng để test SOS và escalation khẩn cấp.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| SOS trẻ em | `sos_child` | Push Button | Read & Write | Nút SOS của trẻ em |
| SOS người lớn | `sos_adult` | Push Button | Read & Write | Nút SOS của người lớn |
| Đã gửi yêu cầu xác nhận khẩn cấp | `emergency_confirmation_requested` | Status / LED | Read Only | Đã gửi yêu cầu xác nhận |
| Đã xác nhận khẩn cấp | `emergency_confirmed` | Status / LED | Read Only | Đã có xác nhận |
| Trạng thái gửi khẩn cấp | `emergency_escalation_status` | Text | Read Only | Trạng thái escalation |
| Nội dung SOS | `sos_message` | Text | Read Only | Nội dung SOS do hệ thống tạo |

## 11.1 Tên button nên đặt

```text
SOS trẻ em
SOS người lớn
```

Nếu dùng Switch thay Push Button, code cần tự reset `sos_child = false` và `sos_adult = false` sau khi xử lý.

---

# 12. Khu vực 8: Notification

Khu vực này dùng để xem trạng thái gửi notification / Telegram.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Yêu cầu gửi thông báo | `send_notification_request` | Status / LED | Read Only | Hệ thống yêu cầu gửi notification |
| Loại sự kiện thông báo | `notification_event_type` | Text | Read Only | Ví dụ `intrusion_alert`, `sos_alert` |
| Trạng thái gửi thông báo | `notification_sent_status` | Text | Read Only | `IDLE`, `PENDING`, `SENT`, `FAILED` |
| Kênh thông báo | `notification_channel` | Text | Read Only | Mặc định là `telegram` |

## 12.1 Tên widget nên đặt

```text
Yêu cầu gửi thông báo
Loại sự kiện thông báo
Trạng thái gửi thông báo
Kênh thông báo
```

Không dùng chữ `email` cho nhóm này, vì hệ thống hiện tại dùng nhóm biến `notification_*`.

---

# 13. Khu vực 9: WiFi Detection

Khu vực này dùng để debug thiết bị đáng tin và WiFi/MAC lạ.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Phát hiện WiFi/MAC lạ | `unknown_wifi_detection_enabled` | Switch | Read & Write | Bật/tắt phát hiện WiFi/MAC lạ |
| Có thiết bị đáng tin | `known_device_present` | Status / LED | Read Only | Có thiết bị đáng tin gần khu vực |
| Cảnh báo WiFi/MAC lạ | `unknown_wifi_alert` | Status / LED | Read Only | Pre-warning, chưa phải báo động chính |
| Số lần WiFi/MAC lạ | `unknown_wifi_count` | Value | Read Only | Số lần hoặc số lượng phát hiện |
| MAC lạ gần nhất | `last_unknown_mac` | Text | Read Only | Địa chỉ MAC lạ gần nhất |

## 13.1 Tên switch nên đặt

```text
Phát hiện WiFi/MAC lạ
```

**MAC** là địa chỉ định danh của thiết bị mạng, ví dụ điện thoại hoặc laptop khi xuất hiện trong mạng/WiFi.

---

# 14. Khu vực 10: Event Log

Khu vực này dùng để xem sự kiện gần nhất.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Sự kiện gần nhất | `last_event` | Text | Read Only | Mô tả sự kiện gần nhất |
| Loại sự kiện gần nhất | `last_event_type` | Text | Read Only | Loại sự kiện gần nhất |
| Tổng số sự kiện | `event_counter` | Value | Read Only | Số sự kiện có ý nghĩa |
| Đang cooldown | `cooldown_active` | Status / LED | Read Only | Hệ thống đang trong thời gian chống spam |
| Thời điểm cảnh báo gần nhất | `last_alert_time` | Text | Read Only | Thời điểm gửi cảnh báo gần nhất |

## 14.1 Tên widget nên đặt

```text
Sự kiện gần nhất
Loại sự kiện gần nhất
Tổng số sự kiện
Đang cooldown
Thời điểm cảnh báo gần nhất
```

**Cooldown** là thời gian chờ để tránh gửi cảnh báo liên tục.

---

# 15. Khu vực 11: Demo Control

Khu vực này dùng để bật chế độ demo và chọn kịch bản demo.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Bật chế độ demo | `demo_mode` | Switch | Read & Write | Bật/tắt chế độ demo |
| Chọn kịch bản demo | `demo_scenario` | Slider / Input | Read & Write | Chọn kịch bản demo |

## 15.1 Tên switch / input nên đặt

```text
Bật chế độ demo
Chọn kịch bản demo
```

## 15.2 Cấu hình `demo_scenario`

Nếu dùng Slider:

```text
Tên hiển thị: Chọn kịch bản demo
Min: 0
Max: 6
Step: 1
```

Ý nghĩa đề xuất:

| Giá trị | Kịch bản |
|---:|---|
| 0 | Không chọn |
| 1 | Demo đột nhập |
| 2 | Demo SOS trẻ em |
| 3 | Demo SOS người lớn |
| 4 | Demo phá hoại thiết bị |
| 5 | Demo WiFi/MAC lạ |
| 6 | Demo reset toàn hệ thống |

## 15.3 Nguyên tắc demo mode

Khi `demo_mode = false`:

```text
Hệ thống chạy theo cảm biến thật và logic thật.
```

Khi `demo_mode = true`:

```text
Admin có thể chọn demo_scenario để ép hệ thống hiển thị/chạy một tình huống demo.
```

Không được để chức năng thật phụ thuộc hoàn toàn vào `demo_mode`.  
SOS, reset, chống phá hoại, cập nhật dashboard và cảm biến thật vẫn phải có logic riêng.

---

# 16. Checklist kéo widget nhanh

Dùng checklist này khi tạo dashboard trên Arduino Cloud.

```text
[ ] System Control
    [ ] Bật / Tắt chống trộm -> alarm_enabled -> Switch
    [ ] Trạng thái chống trộm thực tế -> system_armed -> Status/LED
    [ ] Reset cảnh báo -> reset_alarm -> Button
    [ ] Trạng thái hệ thống -> alarm_status -> Text
    [ ] Thời gian tự bật lại -> rearm_delay_seconds -> Input/Slider
    [ ] Đếm ngược tự bật lại -> rearm_countdown_remaining -> Value

[ ] Sensor Monitor
    [ ] PIR phát hiện chuyển động -> pir_detected -> Status/LED
    [ ] Khoảng cách siêu âm -> ultrasonic_distance -> Value/Gauge
    [ ] Có vật ở gần -> object_near -> Status/LED
    [ ] Giá trị LDR -> ldr_value -> Value/Gauge
    [ ] Độ lệch ánh sáng -> ldr_delta -> Value
    [ ] Ánh sáng bất thường -> light_abnormal -> Status/LED
    [ ] LDR bị che -> ldr_covered -> Status/LED

[ ] Intrusion Detection
    [ ] Cảnh báo đột nhập -> intrusion_alert -> Status/LED
    [ ] Điểm nghi ngờ đột nhập -> intrusion_score -> Value
    [ ] Mức nguy hiểm -> threat_level -> Value/Gauge
    [ ] Nghi vật nuôi / báo nhầm -> pet_detected -> Status/LED
    [ ] Mức nhạy phát hiện -> sensitivity_level -> Slider/Input

[ ] Anti-Sabotage
    [ ] Cảnh báo phá hoại -> sabotage_alert -> Status/LED
    [ ] Thiết bị bị can thiệp -> device_tampered -> Status/LED
    [ ] LDR bị che -> ldr_covered -> Status/LED
    [ ] Có vật quá gần cảm biến -> object_near -> Status/LED

[ ] Output Devices
    [ ] Còi đang bật -> buzzer_on -> Status/LED
    [ ] LED đỏ cảnh báo -> led_red_on -> Status/LED
    [ ] LED xanh bình thường -> led_green_on -> Status/LED

[ ] Camera
    [ ] Chụp ảnh thủ công -> manual_capture_photo -> Button
    [ ] Yêu cầu chụp ảnh tự động -> auto_capture_photo_request -> Status/LED
    [ ] Trạng thái chụp ảnh -> photo_status -> Text

[ ] SOS / Emergency
    [ ] SOS trẻ em -> sos_child -> Button
    [ ] SOS người lớn -> sos_adult -> Button
    [ ] Đã gửi yêu cầu xác nhận khẩn cấp -> emergency_confirmation_requested -> Status/LED
    [ ] Đã xác nhận khẩn cấp -> emergency_confirmed -> Status/LED
    [ ] Trạng thái gửi khẩn cấp -> emergency_escalation_status -> Text
    [ ] Nội dung SOS -> sos_message -> Text

[ ] Notification
    [ ] Yêu cầu gửi thông báo -> send_notification_request -> Status/LED
    [ ] Loại sự kiện thông báo -> notification_event_type -> Text
    [ ] Trạng thái gửi thông báo -> notification_sent_status -> Text
    [ ] Kênh thông báo -> notification_channel -> Text

[ ] WiFi Detection
    [ ] Phát hiện WiFi/MAC lạ -> unknown_wifi_detection_enabled -> Switch
    [ ] Có thiết bị đáng tin -> known_device_present -> Status/LED
    [ ] Cảnh báo WiFi/MAC lạ -> unknown_wifi_alert -> Status/LED
    [ ] Số lần WiFi/MAC lạ -> unknown_wifi_count -> Value
    [ ] MAC lạ gần nhất -> last_unknown_mac -> Text

[ ] Event Log
    [ ] Sự kiện gần nhất -> last_event -> Text
    [ ] Loại sự kiện gần nhất -> last_event_type -> Text
    [ ] Tổng số sự kiện -> event_counter -> Value
    [ ] Đang cooldown -> cooldown_active -> Status/LED
    [ ] Thời điểm cảnh báo gần nhất -> last_alert_time -> Text

[ ] Demo Control
    [ ] Bật chế độ demo -> demo_mode -> Switch
    [ ] Chọn kịch bản demo -> demo_scenario -> Slider/Input
```

---

# 17. Thứ tự nên kéo widget trên dashboard

Nên kéo widget theo thứ tự sau để dashboard dễ nhìn:

```text
1. System Control
2. Sensor Monitor
3. Intrusion Detection
4. Output Devices
5. Camera
6. SOS / Emergency
7. Anti-Sabotage
8. Notification
9. WiFi Detection
10. Event Log
11. Demo Control
```

Nếu màn hình bị chật, ưu tiên giữ lại các nhóm:

```text
System Control
Sensor Monitor
Intrusion Detection
Output Devices
SOS / Emergency
Demo Control
```

Các nhóm có thể thu gọn hoặc đặt xuống dưới:

```text
Notification
WiFi Detection
Event Log
Anti-Sabotage
```

---

# 18. Kết luận

Sau khi tạo xong `Admin Demo Dashboard`, dashboard này phải giúp người làm dự án:

- bật/tắt chống trộm,
- reset cảnh báo,
- xem cảm biến có hoạt động không,
- xem điểm nghi ngờ đột nhập,
- xem còi/LED/camera,
- bấm SOS thử,
- xem notification,
- xem log sự kiện,
- bật demo mode và chọn kịch bản demo.

Khi dashboard này hoàn tất, bước tiếp theo là tạo:

```text
Parent Control Dashboard
```

Dashboard Parent sẽ đơn giản hơn Admin, chỉ giữ các nút và trạng thái quan trọng cho người lớn sử dụng trong demo.
