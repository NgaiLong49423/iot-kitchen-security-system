# PARENT_CONTROL_DASHBOARD.md

# Hướng dẫn cấu hình Parent Control Dashboard

> **Tên tài liệu:** PARENT_CONTROL_DASHBOARD.md  
> **Phiên bản:** v1.0.0  
> **Ngày tạo:** 2026-06-30  
> **Sửa đổi gần nhất:** 2026-06-30  
> **Dự án:** Hệ Thống Chống Trộm IoT  
> **Dashboard:** Parent Control Dashboard  
> **Mục đích:** Hướng dẫn cấu hình dashboard dành cho phụ huynh / người lớn trên Arduino Cloud.

---

## 1. Mục đích dashboard

`Parent Control Dashboard` là dashboard dành cho người lớn hoặc phụ huynh trong nhà.

Dashboard này dùng để:

- bật / tắt chức năng chống trộm,
- reset cảnh báo,
- xem trạng thái cảnh báo quan trọng,
- bấm SOS khẩn cấp,
- yêu cầu chụp ảnh thủ công,
- xem trạng thái gửi thông báo,
- xem sự kiện gần nhất,
- cấu hình thời gian tự bật lại chống trộm.

Dashboard này không dùng để debug chi tiết cảm biến.  
Các biến debug như PIR, LDR, siêu âm, `intrusion_score`, WiFi/MAC lạ chi tiết nên để ở `Admin Demo Dashboard`.

---

## 2. Tên dashboard cần tạo

Đặt tên dashboard trong Arduino Cloud là:

```text
Parent Control Dashboard
```

---

## 3. Nguyên tắc cấu hình chung

Khi tạo widget trong Arduino Cloud:

- chỉ đưa các biến cần thiết cho người lớn,
- không đưa quá nhiều biến debug gây rối,
- các nút thao tác quan trọng nên đặt phía trên,
- các trạng thái cảnh báo nên hiển thị rõ ràng,
- tên hiển thị nên viết tiếng Việt để dễ demo.

**Widget** là ô điều khiển hoặc ô hiển thị trên dashboard.  
Ví dụ: Switch, Button, Value, Gauge, Text, Status, LED.

**Switch** là công tắc bật/tắt.  
**Push Button** là nút bấm một lần rồi nhả.  
**Status / LED** là ô hiển thị trạng thái đúng/sai.  
**Text** là ô hiển thị chữ.  
**Value** là ô hiển thị số.  
**Slider** là thanh kéo chọn số.

---

## 4. Cấu trúc layout đề xuất

Nên chia dashboard thành các khu vực sau:

```text
1. Main Control
2. Alert Status
3. Emergency / SOS
4. Camera Control
5. Re-arm Setting
6. Notification Status
7. Event Summary
```

Nên đặt các khu vực quan trọng ở phía trên:

```text
Main Control
Alert Status
Emergency / SOS
```

Các khu vực ít dùng hơn có thể đặt phía dưới:

```text
Camera Control
Re-arm Setting
Notification Status
Event Summary
```

---

# 5. Khu vực 1: Main Control

Khu vực này dùng để điều khiển chức năng chống trộm và reset cảnh báo.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Bật / Tắt chống trộm | `alarm_enabled` | Switch | Read & Write | Người lớn bật/tắt chức năng chống trộm |
| Trạng thái chống trộm | `system_armed` | Status / LED | Read Only | Trạng thái chống trộm thực tế do hệ thống tính |
| Reset cảnh báo | `reset_alarm` | Push Button | Read & Write | Xóa cảnh báo hiện tại |
| Trạng thái hệ thống | `alarm_status` | Text | Read Only | Hiển thị trạng thái tổng của hệ thống |

## 5.1 Tên button / switch nên đặt

```text
Bật / Tắt chống trộm
Trạng thái chống trộm
Reset cảnh báo
Trạng thái hệ thống
```

## 5.2 Lưu ý với `reset_alarm`

Nếu Arduino Cloud không có Push Button, có thể dùng Switch cho `reset_alarm`.

Trong code sau này cần tự đặt lại:

```cpp
reset_alarm = false;
```

sau khi xử lý reset xong.

---

# 6. Khu vực 2: Alert Status

Khu vực này dùng để phụ huynh biết hệ thống có đang cảnh báo hay không.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Cảnh báo đột nhập | `intrusion_alert` | Status / LED | Read Only | Bật khi hệ thống xác nhận có đột nhập |
| Cảnh báo phá hoại | `sabotage_alert` | Status / LED | Read Only | Bật khi có dấu hiệu phá hoại thiết bị |
| Mức nguy hiểm | `threat_level` | Value / Gauge | Read Only | Mức nguy hiểm tổng thể |
| Còi đang bật | `buzzer_on` | Status / LED | Read Only | Trạng thái còi cảnh báo |
| LED đỏ cảnh báo | `led_red_on` | Status / LED | Read Only | LED đỏ báo cảnh báo |
| LED xanh bình thường | `led_green_on` | Status / LED | Read Only | LED xanh báo hệ thống bình thường |

## 6.1 Tên widget nên đặt

```text
Cảnh báo đột nhập
Cảnh báo phá hoại
Mức nguy hiểm
Còi đang bật
LED đỏ cảnh báo
LED xanh bình thường
```

## 6.2 Không cần đưa `intrusion_score`

`intrusion_score` là điểm debug nội bộ.  
Người lớn chỉ cần xem `threat_level`, `intrusion_alert` và `alarm_status`.

---

# 7. Khu vực 3: Emergency / SOS

Khu vực này dùng để người lớn bấm SOS và xem trạng thái xử lý khẩn cấp.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| SOS khẩn cấp | `sos_adult` | Push Button | Read & Write | Nút SOS của người lớn |
| Đã gửi yêu cầu xác nhận khẩn cấp | `emergency_confirmation_requested` | Status / LED | Read Only | Hệ thống đã gửi yêu cầu xác nhận |
| Đã xác nhận khẩn cấp | `emergency_confirmed` | Status / LED | Read Only | Đã có xác nhận khẩn cấp |
| Trạng thái gửi khẩn cấp | `emergency_escalation_status` | Text | Read Only | Trạng thái gửi khẩn cấp |
| Nội dung SOS | `sos_message` | Text | Read Only | Nội dung SOS do hệ thống tạo |

## 7.1 Tên button nên đặt

```text
SOS khẩn cấp
```

## 7.2 Lưu ý với `sos_adult`

Nếu Arduino Cloud không có Push Button, có thể dùng Switch.

Trong code sau này cần tự đặt lại:

```cpp
sos_adult = false;
```

sau khi xử lý SOS xong.

---

# 8. Khu vực 4: Camera Control

Khu vực này dùng để phụ huynh yêu cầu chụp ảnh thủ công và xem trạng thái chụp ảnh.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Chụp ảnh thủ công | `manual_capture_photo` | Push Button | Read & Write | Firmware chốt một lệnh chụp nội bộ khi button chuyển sang `true` |
| Trạng thái chụp ảnh | `photo_status` | Text | Read Only | Hiển thị trạng thái chụp ảnh |

## 8.1 Tên button nên đặt

```text
Chụp ảnh thủ công
```

## 8.2 Giá trị trạng thái ảnh dự kiến

`photo_status` hiển thị câu tiếng Việt theo tiến trình thực tế, ví dụ:

```text
Đã nhận yêu cầu chụp ảnh thủ công
Đang chụp ảnh
Đã chụp và gửi ảnh
Chụp ảnh thất bại
```

| Giá trị | Ý nghĩa |
|---|---|
| `Đã nhận yêu cầu chụp ảnh thủ công` | Firmware đã chốt lệnh button và đang chờ vòng camera xử lý |
| `Đang chụp ảnh` | Camera đang lấy khung hình |
| `Đã chụp và gửi ảnh` | Ảnh đã gửi Telegram thành công |
| `Chụp ảnh thất bại` | Camera không tạo được ảnh; kiểm tra camera hoặc kết nối |

Nếu Arduino Cloud không có Push Button, có thể dùng Switch cho `manual_capture_photo`.  
Firmware tự đặt lại biến Cloud ngay sau khi chốt lệnh để button sẵn sàng cho lần tiếp theo; yêu cầu thật được giữ bằng cờ nội bộ:

```cpp
manual_capture_photo = false;
```

sau khi xử lý chụp ảnh xong.

---

# 9. Khu vực 5: Re-arm Setting

Khu vực này dùng để cấu hình thời gian tự bật lại chống trộm.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Thời gian tự bật lại | `rearm_delay_seconds` | Slider / Input | Read & Write | Số giây chờ trước khi tự bật lại chống trộm |
| Đếm ngược tự bật lại | `rearm_countdown_remaining` | Value | Read Only | Số giây còn lại trước khi tự bật lại |

## 9.1 Tên widget nên đặt

```text
Thời gian tự bật lại
Đếm ngược tự bật lại
```

## 9.2 Cấu hình `rearm_delay_seconds`

Nếu dùng Slider:

```text
Tên hiển thị: Thời gian tự bật lại
Min: 5
Max: 600
Step: 5
Đơn vị: giây
```

Ý nghĩa:

```text
5 giây = tự bật lại rất nhanh, phù hợp test demo
600 giây = 10 phút
```

---

# 10. Khu vực 6: Notification Status

Khu vực này dùng để phụ huynh biết hệ thống đã gửi thông báo hay chưa.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Trạng thái gửi thông báo | `notification_sent_status` | Text | Read Only | Trạng thái gửi thông báo |
| Kênh thông báo | `notification_channel` | Text | Read Only | Kênh thông báo, ví dụ `telegram` |

## 10.1 Tên widget nên đặt

```text
Trạng thái gửi thông báo
Kênh thông báo
```

## 10.2 Giá trị trạng thái gửi thông báo dự kiến

`notification_sent_status` có thể hiển thị:

```text
IDLE
PENDING
SENT
FAILED
```

| Giá trị | Ý nghĩa |
|---|---|
| `IDLE` | Chưa có yêu cầu gửi |
| `PENDING` | Đang chờ gửi |
| `SENT` | Đã gửi thành công |
| `FAILED` | Gửi thất bại |

Không nên dùng chữ `email` cho nhóm này, vì hệ thống hiện tại dùng nhóm biến `notification_*`.

---

# 11. Khu vực 7: Event Summary

Khu vực này dùng để xem sự kiện gần nhất.

| Tên hiển thị trên dashboard | Biến gắn vào | Loại widget nên dùng | Quyền | Ghi chú |
|---|---|---|---|---|
| Sự kiện gần nhất | `last_event` | Text | Read Only | Mô tả sự kiện gần nhất |
| Thời điểm cảnh báo gần nhất | `last_alert_time` | Text | Read Only | Thời điểm gửi cảnh báo gần nhất |

## 11.1 Tên widget nên đặt

```text
Sự kiện gần nhất
Thời điểm cảnh báo gần nhất
```

---

# 12. Checklist kéo widget nhanh

Dùng checklist này khi tạo `Parent Control Dashboard` trên Arduino Cloud.

```text
[ ] Main Control
    [ ] Bật / Tắt chống trộm -> alarm_enabled -> Switch
    [ ] Trạng thái chống trộm -> system_armed -> Status/LED
    [ ] Reset cảnh báo -> reset_alarm -> Button
    [ ] Trạng thái hệ thống -> alarm_status -> Text

[ ] Alert Status
    [ ] Cảnh báo đột nhập -> intrusion_alert -> Status/LED
    [ ] Cảnh báo phá hoại -> sabotage_alert -> Status/LED
    [ ] Mức nguy hiểm -> threat_level -> Value/Gauge
    [ ] Còi đang bật -> buzzer_on -> Status/LED
    [ ] LED đỏ cảnh báo -> led_red_on -> Status/LED
    [ ] LED xanh bình thường -> led_green_on -> Status/LED

[ ] Emergency / SOS
    [ ] SOS khẩn cấp -> sos_adult -> Button
    [ ] Đã gửi yêu cầu xác nhận khẩn cấp -> emergency_confirmation_requested -> Status/LED
    [ ] Đã xác nhận khẩn cấp -> emergency_confirmed -> Status/LED
    [ ] Trạng thái gửi khẩn cấp -> emergency_escalation_status -> Text
    [ ] Nội dung SOS -> sos_message -> Text

[ ] Camera Control
    [ ] Chụp ảnh thủ công -> manual_capture_photo -> Button
    [ ] Trạng thái chụp ảnh -> photo_status -> Text

[ ] Re-arm Setting
    [ ] Thời gian tự bật lại -> rearm_delay_seconds -> Slider/Input
    [ ] Đếm ngược tự bật lại -> rearm_countdown_remaining -> Value

[ ] Notification Status
    [ ] Trạng thái gửi thông báo -> notification_sent_status -> Text
    [ ] Kênh thông báo -> notification_channel -> Text

[ ] Event Summary
    [ ] Sự kiện gần nhất -> last_event -> Text
    [ ] Thời điểm cảnh báo gần nhất -> last_alert_time -> Text
```

---

# 13. Thứ tự nên kéo widget trên dashboard

Nên kéo theo thứ tự sau:

```text
1. Main Control
2. Alert Status
3. Emergency / SOS
4. Camera Control
5. Re-arm Setting
6. Notification Status
7. Event Summary
```

Nếu dashboard bị chật, ưu tiên giữ lại:

```text
Main Control
Alert Status
Emergency / SOS
Camera Control
Event Summary
```

Có thể đặt xuống dưới hoặc thu gọn:

```text
Re-arm Setting
Notification Status
```

---

# 14. Những biến không nên đưa vào Parent Dashboard

Để dashboard gọn, không nên đưa các biến debug sau vào Parent Dashboard:

```text
pir_detected
ultrasonic_distance
object_near
ldr_value
ldr_delta
light_abnormal
ldr_covered
intrusion_score
device_tampered
unknown_wifi_detection_enabled
unknown_wifi_count
unknown_wifi_alert
last_unknown_mac
demo_mode
demo_scenario
event_counter
last_event_type
auto_capture_photo_request
send_notification_request
notification_event_type
cooldown_active
current_time
current_hour
night_mode
```

Các biến này nên để trong `Admin Demo Dashboard`.

---

# 15. Khác nhau giữa Parent Dashboard và Admin Dashboard

| Nội dung | Parent Control Dashboard | Admin Demo Dashboard |
|---|---|---|
| Người dùng chính | Người lớn / phụ huynh | Leader / người demo / người code |
| Mục tiêu | Điều khiển và xem trạng thái quan trọng | Test toàn bộ hệ thống và debug |
| Có biến cảm biến thô không | Không cần | Có |
| Có demo mode không | Không | Có |
| Có WiFi/MAC debug không | Không cần | Có |
| Có `intrusion_score` không | Không cần | Có |
| Có SOS không | Có, dùng `sos_adult` | Có cả `sos_child` và `sos_adult` |

---

# 16. Kết luận

Sau khi tạo xong `Parent Control Dashboard`, dashboard này phải giúp người lớn:

- bật/tắt chống trộm,
- reset cảnh báo,
- xem hệ thống có đang cảnh báo không,
- bấm SOS khẩn cấp,
- chụp ảnh thủ công,
- xem trạng thái gửi thông báo,
- xem sự kiện gần nhất,
- cấu hình thời gian tự bật lại chống trộm.

Khi dashboard này hoàn tất, bước tiếp theo là tạo:

```text
Child SOS Dashboard
```

Dashboard Child sẽ đơn giản hơn nhiều, chỉ cần nút SOS trẻ em và vài trạng thái cơ bản.
