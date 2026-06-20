# Demo 3 Kịch Bản IoT Anti-Theft System  
## Bản viết lại theo kịch bản đã sửa + đối chiếu đúng biến Arduino Cloud thật

| Mục | Nội dung |
|---|---|
| Project | IoT Based Anti-Theft System |
| Nhóm | Group 6 - SE2034 |
| Thing đề xuất | `IoT_Anti_Theft_System` |
| Mục tiêu | Demo 3 kịch bản thực tế: SOS, cảnh báo cháy, chống phá hoại thiết bị |
| Nguyên tắc viết lại | Chỉ dùng các biến có trong file danh sách biến Arduino Cloud thật |
| Version tài liệu | 3.0 |

---

# 1. Nguyên tắc quan trọng của bản demo

Bản demo này được viết lại theo đúng ý kịch bản đã sửa, nhưng phải bám theo biến thật trong Arduino Cloud.

## 1.1. Không dùng biến bịa thêm

Các biến cũ như sau **không dùng trong bản mới** vì không có trong file biến thật:

```cpp
sosButton
resetAlarm
alertType
gmailSent
lastAlertMessage
flameDetected
ldrBlocked
pirDetected
```

Thay vào đó, bản mới dùng đúng các biến thật:

```cpp
sos_child
sos_adult
sos_level
sos_message
reset_alarm
alarm_status
send_email_request
email_event_type
email_sent_status
flame_detected
fire_alert
ldr_covered
pir_detected
sabotage_alert
last_event
last_event_type
event_counter
buzzer_on
led_red_on
```

---

## 1.2. Cách hiểu luồng thực tế

Trong dự án này, ESP32-CAM là board chính xử lý logic.

Luồng chung:

```text
Cảm biến hoặc người dùng trên điện thoại
        ↓
Arduino IoT Cloud đồng bộ biến
        ↓
ESP32-CAM đọc biến / đọc cảm biến
        ↓
ESP32-CAM xử lý logic cảnh báo
        ↓
ESP32-CAM bật LED + Buzzer
        ↓
ESP32-CAM cập nhật trạng thái lên Cloud
        ↓
ESP32-CAM gọi Google Apps Script nếu cần gửi Gmail
        ↓
Google Apps Script gửi Gmail cho người nhận
```

---

# 2. Tổng quan 3 kịch bản demo

| STT | Kịch bản | Ý nghĩa | Biến chính dùng |
|---|---|---|---|
| 1 | SOS từ điện thoại theo mức độ | Trẻ em/người lớn bấm SOS, hệ thống cảnh báo tại chỗ và gửi email theo mức nguy hiểm | `sos_child`, `sos_adult`, `sos_level`, `sos_message`, `alarm_status`, `send_email_request`, `email_event_type`, `email_sent_status` |
| 2 | Cảnh báo cháy | Flame Sensor phát hiện lửa, đối chiếu nhiệt độ bếp, cảnh báo và gửi Gmail | `flame_value`, `flame_detected`, `kitchen_temperature`, `fire_alert`, `alarm_status`, `send_email_request`, `email_event_type`, `email_sent_status` |
| 3 | Chống phá hoại thiết bị | LDR bị che + PIR phát hiện chuyển động, hệ thống báo động chống phá hoại | `ldr_value`, `ldr_delta`, `ldr_covered`, `pir_detected`, `sabotage_alert`, `device_tampered`, `alarm_status` |

---

# 3. Các module và dịch vụ dùng trong demo

| Thành phần | Vai trò |
|---|---|
| ESP32-CAM AI Thinker OV2640 | Board chính, xử lý logic, kết nối WiFi |
| Arduino IoT Cloud | Đồng bộ biến giữa board và điện thoại |
| Điện thoại | Bấm SOS, xem trạng thái cảnh báo |
| Google Apps Script | Web App trung gian để gửi Gmail |
| Gmail | Nhận email cảnh báo |
| Flame Sensor | Phát hiện dấu hiệu ngọn lửa |
| Cảm biến nhiệt độ bếp | Cung cấp `kitchen_temperature` để đối chiếu báo cháy |
| LDR Light Sensor | Phát hiện ánh sáng bất thường hoặc bị che |
| HC-SR501 / PIR | Phát hiện chuyển động |
| LED đỏ | Cảnh báo tại chỗ bằng ánh sáng |
| Buzzer 1407 | Cảnh báo tại chỗ bằng âm thanh |

---

# 4. Nhóm biến Cloud dùng trong 3 kịch bản

## 4.1. Biến điều khiển chính

| Biến | Kiểu | Quyền | Cập nhật | Dùng trong demo |
|---|---|---|---|---|
| `alarm_enabled` | `bool` | Read & Write | On Change | Bật/tắt chế độ bảo vệ |
| `system_armed` | `bool` | Read Only | On Change | Cho biết hệ thống thật sự đang canh gác |
| `reset_alarm` | `bool` | Read & Write | On Change | Người lớn/admin bấm để tắt cảnh báo |
| `alarm_status` | `String` | Read Only | On Change | Hiển thị trạng thái chính của hệ thống |

Ví dụ giá trị `alarm_status` trong 3 demo:

```text
"SAFE"
"SOS_CHILD_ALERT"
"SOS_ADULT_ALERT"
"FIRE_ALERT"
"SABOTAGE_ALERT"
"EMAIL_SENDING"
"EMAIL_SENT"
"EMAIL_FAILED"
```

---

## 4.2. Biến SOS

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `sos_child` | `bool` | Read & Write | On Change | Trẻ em bấm SOS từ điện thoại |
| `sos_adult` | `bool` | Read & Write | On Change | Người lớn bấm SOS từ điện thoại |
| `sos_level` | `int` | Read Only | On Change | Board tự tính mức SOS |
| `sos_message` | `String` | Read Only | On Change | Nội dung SOS hiển thị/gửi email |

Lưu ý quan trọng:

```text
sos_level là Read Only.
Vì vậy dashboard không nên tự ghi sos_level.
Board phải tự tính sos_level dựa trên hành vi bấm SOS.
```

Ví dụ quy ước:

```text
sos_level = 0 → Không có SOS
sos_level = 1 → Chỉ báo động tại chỗ
sos_level = 2 → Gửi Gmail cho người lớn trong gia đình
sos_level = 3 → Gửi Gmail cho người lớn + yêu cầu xác nhận gửi mail cho công an/cứu hộ giả lập
```

---

## 4.3. Biến báo cháy

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `flame_value` | `int` | Read Only | Periodic | Giá trị thô từ Flame Sensor |
| `flame_detected` | `bool` | Read Only | On Change | Board kết luận có dấu hiệu lửa |
| `kitchen_temperature` | `float` | Read Only | Periodic | Nhiệt độ khu vực bếp |
| `fire_alert` | `bool` | Read Only | On Change | Board xác nhận nguy cơ cháy |

Lưu ý:

```text
Flame Sensor không đo nhiệt độ.
flame_value / flame_detected là dữ liệu về dấu hiệu ngọn lửa.
kitchen_temperature phải đến từ cảm biến nhiệt độ riêng.
```

---

## 4.4. Biến chống phá hoại

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `ldr_value` | `int` | Read Only | Periodic | Giá trị ánh sáng LDR |
| `ldr_delta` | `int` | Read Only | Periodic | Độ thay đổi ánh sáng |
| `light_abnormal` | `bool` | Read Only | On Change | Board kết luận ánh sáng bất thường |
| `ldr_covered` | `bool` | Read Only | On Change | Board kết luận LDR bị che |
| `pir_detected` | `bool` | Read Only | On Change | PIR phát hiện chuyển động |
| `sabotage_alert` | `bool` | Read Only | On Change | Board xác nhận phá hoại thiết bị |
| `device_tampered` | `bool` | Read Only | On Change | Thiết bị có dấu hiệu bị can thiệp |

---

## 4.5. Biến output LED/Buzzer

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `buzzer_on` | `bool` | Read Only | On Change | Cho biết còi đang kêu |
| `led_red_on` | `bool` | Read Only | On Change | Cho biết LED đỏ đang bật |

Lưu ý:

```text
buzzer_on và led_red_on là Read Only.
Người dùng không bật trực tiếp 2 biến này.
Muốn tắt báo động thì dùng reset_alarm.
```

---

## 4.6. Biến gửi Gmail

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `send_email_request` | `bool` | Read Only | On Change | Board đánh dấu cần gửi email |
| `email_event_type` | `String` | Read Only | On Change | Loại email cần gửi |
| `email_sent_status` | `String` | Read Only | On Change | Trạng thái gửi email |

Ví dụ `email_event_type`:

```text
"sos_child"
"sos_adult"
"fire"
"sabotage"
```

Ví dụ `email_sent_status`:

```text
"EMAIL_IDLE"
"EMAIL_SENDING"
"EMAIL_SENT"
"EMAIL_FAILED"
```

---

## 4.7. Biến log sự kiện

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `last_event` | `String` | Read Only | On Change | Ghi sự kiện gần nhất |
| `last_event_type` | `String` | Read Only | On Change | Loại sự kiện gần nhất |
| `event_counter` | `int` | Read Only | On Change | Tăng số lần có sự kiện |

---

## 4.8. Biến chống spam cảnh báo

| Biến | Kiểu | Quyền | Cập nhật | Dùng để làm gì |
|---|---|---|---|---|
| `cooldown_active` | `bool` | Read Only | On Change | Đang trong thời gian chống spam cảnh báo |
| `last_alert_time` | `String` | Read Only | On Change | Thời điểm cảnh báo gần nhất |

**Cooldown** nghĩa là thời gian nghỉ giữa hai lần gửi cảnh báo.  
Mục đích là tránh gửi quá nhiều Gmail liên tục khi cảm biến vẫn đang bị kích hoạt.

---

# 5. Kịch bản 1: SOS từ điện thoại theo mức độ + cảnh báo + gửi Gmail

## 5.1. Mục tiêu

Kịch bản này demo việc người dùng bấm SOS trên điện thoại bằng Arduino IoT Cloud.

Hệ thống cần xử lý theo mức độ SOS:

```text
Mức 1 → Module tự cảnh báo tại chỗ bằng LED + buzzer
Mức 2 → Gửi Gmail cho người lớn trong gia đình
Mức 3 → Gửi Gmail cho người lớn, sau đó yêu cầu xác nhận để gửi tiếp Gmail cho công an/cứu hộ giả lập
```

---

## 5.2. Luồng thực tế đề xuất

```text
Người dùng mở Arduino IoT Cloud Dashboard trên điện thoại
        ↓
Người dùng bấm SOS trẻ em hoặc SOS người lớn
        ↓
Cloud cập nhật biến sos_child hoặc sos_adult
        ↓
ESP32-CAM nhận thay đổi từ Arduino Cloud
        ↓
ESP32-CAM kiểm tra alarm_enabled và trạng thái hệ thống
        ↓
ESP32-CAM tính sos_level
        ↓
ESP32-CAM cập nhật sos_message
        ↓
ESP32-CAM bật LED đỏ và buzzer
        ↓
ESP32-CAM cập nhật led_red_on = true
        ↓
ESP32-CAM cập nhật buzzer_on = true
        ↓
ESP32-CAM cập nhật alarm_status theo mức SOS
        ↓
Nếu sos_level >= 2 thì ESP32-CAM bật send_email_request
        ↓
ESP32-CAM cập nhật email_event_type
        ↓
ESP32-CAM gọi Google Apps Script Web App
        ↓
Google Apps Script gửi Gmail theo email_event_type
        ↓
ESP32-CAM cập nhật email_sent_status
        ↓
ESP32-CAM ghi last_event, last_event_type, event_counter
        ↓
Người lớn/admin bấm reset_alarm để tắt cảnh báo
        ↓
ESP32-CAM tắt LED, tắt buzzer, đưa hệ thống về SAFE
```

---

## 5.3. Logic mức SOS

Vì file biến thật có `sos_level` là Read Only, nên `sos_level` phải do board tự tính.

### Cách demo dễ hiểu nhất

Trong lúc demo, có thể quy ước:

```text
Bấm sos_child 1 lần trong 3 giây → sos_level = 1
Bấm sos_child 2 lần trong 3 giây → sos_level = 2
Bấm sos_child 3 lần trong 3 giây → sos_level = 3
```

Hoặc nếu nhóm chưa code được bấm nhiều lần, có thể dùng:

```text
demo_mode = true
demo_scenario = 8
```

để chạy nhanh nhánh demo SOS trẻ em.

Lưu ý khi thuyết trình:

```text
demo_mode và demo_scenario chỉ dùng để hỗ trợ trình diễn.
Khi chạy thật, hệ thống xử lý dựa trên sos_child hoặc sos_adult.
```

---

## 5.4. Hành động theo từng mức SOS

| Mức | Điều kiện | Hành động tại module | Hành động email |
|---|---|---|---|
| `sos_level = 1` | SOS nhẹ | Bật LED đỏ, bật buzzer | Chưa bắt buộc gửi Gmail |
| `sos_level = 2` | SOS cần người lớn hỗ trợ | Bật LED đỏ, bật buzzer, ghi log | Gửi Gmail cho người lớn trong gia đình |
| `sos_level = 3` | SOS khẩn cấp | Bật LED đỏ, bật buzzer liên tục, ghi log khẩn cấp | Gửi Gmail cho người lớn, Gmail có phần xác nhận gửi tiếp cho công an/cứu hộ giả lập |

---

## 5.5. Biến cập nhật khi SOS mức 1

```cpp
sos_level = 1;
sos_message = "SOS level 1: local alarm only";
alarm_status = "SOS_CHILD_ALERT";
led_red_on = true;
buzzer_on = true;
last_event_type = "SOS_CHILD";
last_event = "SOS level 1 triggered from Arduino IoT Cloud";
event_counter++;
```

Không cần bật:

```cpp
send_email_request = false;
email_sent_status = "EMAIL_IDLE";
```

---

## 5.6. Biến cập nhật khi SOS mức 2

```cpp
sos_level = 2;
sos_message = "SOS level 2: notify adults by Gmail";
alarm_status = "SOS_CHILD_ALERT";
led_red_on = true;
buzzer_on = true;
send_email_request = true;
email_event_type = "sos_child";
email_sent_status = "EMAIL_SENDING";
last_event_type = "SOS_CHILD";
last_event = "SOS level 2: email sent to adults";
event_counter++;
```

Sau khi gọi Google Apps Script:

```cpp
email_sent_status = "EMAIL_SENT";
last_alert_time = current_time;
cooldown_active = true;
```

Nếu lỗi:

```cpp
email_sent_status = "EMAIL_FAILED";
```

---

## 5.7. Biến cập nhật khi SOS mức 3

```cpp
sos_level = 3;
sos_message = "SOS level 3: emergency confirmation required";
alarm_status = "SOS_CHILD_ALERT";
led_red_on = true;
buzzer_on = true;
send_email_request = true;
email_event_type = "sos_child";
email_sent_status = "EMAIL_SENDING";
last_event_type = "SOS_CHILD";
last_event = "SOS level 3: adults must confirm emergency escalation";
event_counter++;
```

Sau khi Gmail gửi thành công:

```cpp
email_sent_status = "EMAIL_SENT";
last_alert_time = current_time;
cooldown_active = true;
```

---

## 5.8. Về xác nhận gửi mail cho công an/cứu hộ

Trong ý tưởng của bạn có đoạn:

```text
Mức 3 yêu cầu người lớn xác nhận gửi mail báo công an/cứu hộ.
Chỉ cần 1 người lớn xác nhận là gửi ngay.
Sau khi đã gửi thì khóa quyền xác nhận của những người còn lại.
```

Luồng này **không nên bịa thêm biến Arduino Cloud**, vì file biến thật hiện chưa có biến kiểu:

```cpp
police_confirmed
firefighter_confirmed
hospital_confirmed
emergency_confirm_locked
```

Vì vậy bản demo thực tế nên làm như sau:

```text
ESP32-CAM chỉ gửi email SOS level 3 cho Google Apps Script.
Google Apps Script tạo Gmail có nút/link xác nhận.
Người lớn bấm link xác nhận trong Gmail.
Google Apps Script tự xử lý việc:
- Chỉ nhận xác nhận đầu tiên.
- Gửi mail tiếp cho công an/cứu hộ giả lập.
- Khóa các xác nhận sau.
- Gửi mail báo lại cho người lớn rằng mail khẩn cấp đã được gửi.
```

Như vậy phần xác nhận nằm ở **Google Apps Script**, không nằm ở Arduino Cloud.

Cách này hợp lý hơn vì:

```text
Arduino Cloud hiện chưa có biến xác nhận riêng.
Google Apps Script phù hợp để xử lý link xác nhận trong Gmail.
Không cần tạo thêm biến ngoài danh sách thật.
```

---

## 5.9. Cách demo kịch bản 1 trên lớp

### Bước 1: Cho thầy thấy dashboard

Mở Arduino IoT Cloud Dashboard và chỉ các biến:

```cpp
sos_child
sos_adult
sos_level
sos_message
alarm_status
email_event_type
email_sent_status
buzzer_on
led_red_on
```

Nói:

> “Đây là dashboard đã đồng bộ với ESP32-CAM. Những biến người dùng được bấm là `sos_child`, `sos_adult`, `reset_alarm`. Các biến còn lại là trạng thái do board tự cập nhật.”

---

### Bước 2: Bấm SOS

Bấm `sos_child`.

Quan sát:

```text
LED đỏ bật
Buzzer kêu
sos_level đổi
sos_message đổi
alarm_status đổi
buzzer_on = true
led_red_on = true
```

---

### Bước 3: Kiểm tra Gmail

Nếu `sos_level >= 2`, mở Gmail và kiểm tra email.

Quan sát trên dashboard:

```text
send_email_request = true
email_event_type = "sos_child"
email_sent_status = "EMAIL_SENT"
```

---

### Bước 4: Reset

Bấm:

```cpp
reset_alarm = true;
```

Board xử lý:

```cpp
led_red_on = false;
buzzer_on = false;
alarm_status = "SAFE";
sos_child = false;
sos_adult = false;
sos_level = 0;
send_email_request = false;
email_sent_status = "EMAIL_IDLE";
```

---

# 6. Kịch bản 2: Cảnh báo cháy + Gmail + hiển thị trên Arduino Cloud

## 6.1. Mục tiêu

Kịch bản này demo việc hệ thống tự phát hiện nguy cơ cháy bằng Flame Sensor và nhiệt độ bếp.

Hệ thống cần:

```text
Phát hiện dấu hiệu lửa
Đối chiếu nhiệt độ bếp
Bật LED + buzzer
Cập nhật dashboard Arduino Cloud
Gửi Gmail cho người lớn
Nếu cần, Gmail có link xác nhận gọi cứu hỏa giả lập
```

---

## 6.2. Luồng thực tế đề xuất

```text
Người trình diễn đưa nguồn lửa nhỏ hoặc nguồn hồng ngoại gần Flame Sensor
        ↓
ESP32-CAM đọc giá trị flame_value
        ↓
ESP32-CAM cập nhật flame_value định kỳ lên Arduino Cloud
        ↓
Nếu tín hiệu lửa vượt ngưỡng, ESP32-CAM cập nhật flame_detected = true
        ↓
ESP32-CAM đọc kitchen_temperature
        ↓
ESP32-CAM đối chiếu flame_detected và kitchen_temperature
        ↓
Nếu đủ điều kiện nguy hiểm, ESP32-CAM cập nhật fire_alert = true
        ↓
ESP32-CAM bật LED đỏ và buzzer
        ↓
ESP32-CAM cập nhật led_red_on = true, buzzer_on = true
        ↓
ESP32-CAM cập nhật alarm_status = "FIRE_ALERT"
        ↓
ESP32-CAM bật send_email_request = true
        ↓
ESP32-CAM cập nhật email_event_type = "fire"
        ↓
ESP32-CAM gọi Google Apps Script Web App
        ↓
Google Apps Script gửi Gmail cảnh báo cháy cho người lớn
        ↓
Gmail có thể chứa link xác nhận gọi cứu hỏa giả lập
        ↓
Người lớn xác nhận hoặc bỏ qua trong Gmail
        ↓
Người lớn/admin bấm reset_alarm để kết thúc cảnh báo trên module
```

---

## 6.3. Điều kiện kích hoạt đề xuất

Nếu nhóm có cảm biến nhiệt độ thật:

```cpp
if (flame_detected == true && kitchen_temperature >= 55.0) {
    fire_alert = true;
}
```

Nếu nhóm chưa có cảm biến nhiệt độ thật, demo có thể dùng mức đơn giản:

```cpp
if (flame_detected == true) {
    fire_alert = true;
}
```

Nhưng khi thuyết trình phải nói rõ:

```text
Flame Sensor phát hiện lửa.
kitchen_temperature là biến dành cho cảm biến nhiệt độ rời.
Nếu chưa gắn cảm biến nhiệt độ, nhóm chỉ demo theo tín hiệu Flame Sensor.
```

---

## 6.4. Biến cập nhật khi cảnh báo cháy

```cpp
flame_detected = true;
fire_alert = true;
alarm_status = "FIRE_ALERT";
led_red_on = true;
buzzer_on = true;
send_email_request = true;
email_event_type = "fire";
email_sent_status = "EMAIL_SENDING";
last_event_type = "FIRE";
last_event = "Fire alert detected by flame sensor";
event_counter++;
```

Sau khi gửi Gmail thành công:

```cpp
email_sent_status = "EMAIL_SENT";
last_alert_time = current_time;
cooldown_active = true;
```

Nếu gửi lỗi:

```cpp
email_sent_status = "EMAIL_FAILED";
```

---

## 6.5. Về xác nhận gọi cứu hỏa

Tương tự SOS mức 3, file biến thật hiện chưa có biến Cloud để lưu xác nhận cứu hỏa.

Vì vậy luồng hợp lý là:

```text
Google Apps Script gửi Gmail cảnh báo cháy.
Trong Gmail có link xác nhận gọi cứu hỏa giả lập.
Người lớn bấm xác nhận.
Google Apps Script gửi tiếp Gmail cho địa chỉ cứu hỏa giả lập + địa chỉ nhà.
Google Apps Script gửi lại Gmail thông báo cho người lớn rằng yêu cầu đã được gửi.
```

Phần này do Google Apps Script quản lý, không cần thêm biến Arduino Cloud mới.

---

## 6.6. Cách demo kịch bản 2 trên lớp

### Bước 1: Cho thấy giá trị cảm biến

Mở dashboard và chỉ các biến:

```cpp
flame_value
flame_detected
kitchen_temperature
fire_alert
alarm_status
email_event_type
email_sent_status
```

Nói:

> “Flame Sensor không đo nhiệt độ trực tiếp. Vì vậy nhóm em tách riêng `flame_value`, `flame_detected` và `kitchen_temperature`. `fire_alert` là kết luận cuối cùng sau khi board xử lý.”

---

### Bước 2: Đưa nguồn lửa/hồng ngoại gần cảm biến

Quan sát:

```text
flame_value thay đổi
flame_detected = true
fire_alert = true
alarm_status = "FIRE_ALERT"
LED đỏ bật
Buzzer kêu
```

---

### Bước 3: Kiểm tra Gmail

Quan sát:

```text
send_email_request = true
email_event_type = "fire"
email_sent_status = "EMAIL_SENT"
```

Mở Gmail để kiểm tra email cảnh báo cháy.

---

### Bước 4: Reset

Bấm:

```cpp
reset_alarm = true;
```

Board đưa về:

```cpp
fire_alert = false;
flame_detected = false;
led_red_on = false;
buzzer_on = false;
alarm_status = "SAFE";
send_email_request = false;
email_sent_status = "EMAIL_IDLE";
```

---

# 7. Kịch bản 3: Chống phá hoại thiết bị bằng LDR + PIR

## 7.1. Mục tiêu

Kịch bản này demo việc phát hiện người cố tình che cảm biến hoặc can thiệp thiết bị.

Hệ thống không chỉ dùng LDR, vì LDR có thể báo nhầm khi môi trường tối.  
Do đó cần kết hợp:

```text
LDR bị che bất thường
+
PIR phát hiện chuyển động
=
Nghi có người phá hoại thiết bị
```

---

## 7.2. Luồng thực tế đề xuất

```text
Ban đầu thiết bị hoạt động bình thường
        ↓
ESP32-CAM đọc ldr_value định kỳ
        ↓
ESP32-CAM tính ldr_delta
        ↓
Người trình diễn dùng tay hoặc khăn che LDR
        ↓
ldr_value thay đổi mạnh
        ↓
ESP32-CAM cập nhật light_abnormal = true
        ↓
ESP32-CAM cập nhật ldr_covered = true
        ↓
Người trình diễn quơ tay trước PIR
        ↓
ESP32-CAM cập nhật pir_detected = true
        ↓
ESP32-CAM xác nhận sabotage_alert = true
        ↓
ESP32-CAM cập nhật device_tampered = true
        ↓
ESP32-CAM bật LED đỏ và buzzer
        ↓
ESP32-CAM cập nhật led_red_on = true, buzzer_on = true
        ↓
ESP32-CAM cập nhật alarm_status = "SABOTAGE_ALERT"
        ↓
ESP32-CAM ghi last_event, last_event_type, event_counter
        ↓
Nếu cần gửi Gmail, ESP32-CAM bật send_email_request
        ↓
ESP32-CAM cập nhật email_event_type = "sabotage"
        ↓
Google Apps Script gửi Gmail cho người lớn trong nhà
        ↓
Người lớn/admin bấm reset_alarm để tắt cảnh báo
```

---

## 7.3. Điều kiện kích hoạt đề xuất

```cpp
if (ldr_covered == true && pir_detected == true) {
    sabotage_alert = true;
    device_tampered = true;
}
```

Nếu muốn chắc hơn, dùng thêm `light_abnormal`:

```cpp
if (light_abnormal == true && ldr_covered == true && pir_detected == true) {
    sabotage_alert = true;
    device_tampered = true;
}
```

---

## 7.4. Biến cập nhật khi chống phá hoại

```cpp
light_abnormal = true;
ldr_covered = true;
pir_detected = true;
sabotage_alert = true;
device_tampered = true;
alarm_status = "SABOTAGE_ALERT";
led_red_on = true;
buzzer_on = true;
last_event_type = "SABOTAGE";
last_event = "LDR covered and PIR motion detected";
event_counter++;
```

Nếu nhóm muốn gửi Gmail cho người lớn:

```cpp
send_email_request = true;
email_event_type = "sabotage";
email_sent_status = "EMAIL_SENDING";
```

Sau khi gửi thành công:

```cpp
email_sent_status = "EMAIL_SENT";
last_alert_time = current_time;
cooldown_active = true;
```

---

## 7.5. Vì sao luồng này thực tế hơn?

Nếu chỉ dùng:

```text
LDR bị tối → báo phá hoại
```

thì dễ sai vì có thể chỉ là tắt đèn hoặc trời tối.

Luồng mới dùng:

```text
LDR thay đổi bất thường
+
LDR bị che
+
PIR có chuyển động
```

nên hợp lý hơn. Có người ở gần thiết bị và ánh sáng bị che đột ngột thì mới kết luận là đáng nghi.

---

## 7.6. Cách demo kịch bản 3 trên lớp

### Bước 1: Cho thấy trạng thái ban đầu

Dashboard nên hiển thị:

```cpp
ldr_value
ldr_delta
light_abnormal
ldr_covered
pir_detected
sabotage_alert
device_tampered
alarm_status
```

Ban đầu:

```text
light_abnormal = false
ldr_covered = false
pir_detected = false
sabotage_alert = false
device_tampered = false
alarm_status = "SAFE"
```

---

### Bước 2: Che LDR

Dùng tay hoặc khăn che LDR.

Quan sát:

```text
ldr_value thay đổi
ldr_delta tăng
light_abnormal = true
ldr_covered = true
```

Lúc này chưa nhất thiết phải hú còi ngay nếu chưa có PIR.

---

### Bước 3: Quơ tay trước PIR

Quan sát:

```text
pir_detected = true
sabotage_alert = true
device_tampered = true
alarm_status = "SABOTAGE_ALERT"
LED đỏ bật
Buzzer kêu
```

---

### Bước 4: Kiểm tra thông báo/Gmail nếu có bật gửi mail

Quan sát:

```text
email_event_type = "sabotage"
email_sent_status = "EMAIL_SENT"
```

---

### Bước 5: Reset

Bấm:

```cpp
reset_alarm = true;
```

Board đưa về:

```cpp
sabotage_alert = false;
device_tampered = false;
led_red_on = false;
buzzer_on = false;
alarm_status = "SAFE";
send_email_request = false;
email_sent_status = "EMAIL_IDLE";
```

---

# 8. Thứ tự demo đề xuất

## Bước 1: Kiểm tra Cloud

Mở Arduino IoT Cloud Dashboard.

Nói:

> “Nhóm em dùng 1 Thing tên `IoT_Anti_Theft_System`. Các biến điều khiển như `sos_child`, `sos_adult`, `reset_alarm` là Read & Write. Các biến cảm biến và trạng thái như `fire_alert`, `sabotage_alert`, `alarm_status` là Read Only vì do board tự cập nhật.”

---

## Bước 2: Demo kịch bản 1 - SOS

1. Bấm `sos_child`.
2. Quan sát `sos_level`, `sos_message`, `alarm_status`.
3. Quan sát LED/Buzzer.
4. Kiểm tra `email_sent_status`.
5. Mở Gmail nếu mức SOS có gửi email.
6. Bấm `reset_alarm`.

---

## Bước 3: Demo kịch bản 2 - Cháy

1. Đưa nguồn lửa/hồng ngoại gần Flame Sensor.
2. Quan sát `flame_value`.
3. Quan sát `flame_detected`.
4. Quan sát `fire_alert`.
5. Kiểm tra Gmail.
6. Bấm `reset_alarm`.

---

## Bước 4: Demo kịch bản 3 - Chống phá hoại

1. Che LDR.
2. Quan sát `ldr_value`, `ldr_delta`, `ldr_covered`.
3. Quơ tay trước PIR.
4. Quan sát `pir_detected`, `sabotage_alert`, `device_tampered`.
5. Kiểm tra LED/Buzzer.
6. Bấm `reset_alarm`.

---

# 9. Câu thuyết trình ngắn

> “Trong bản demo này, nhóm em trình bày 3 kịch bản chính: SOS từ điện thoại, cảnh báo cháy và chống phá hoại thiết bị. ESP32-CAM được đồng bộ với Arduino IoT Cloud thông qua các biến trong Thing `IoT_Anti_Theft_System`. Người dùng có thể bấm SOS từ điện thoại bằng biến `sos_child` hoặc `sos_adult`. Board sẽ tự tính `sos_level`, bật LED, bật buzzer và nếu cần thì gửi Gmail qua Google Apps Script bằng nhóm biến `send_email_request`, `email_event_type`, `email_sent_status`. Với báo cháy, board dùng `flame_detected`, `kitchen_temperature` và `fire_alert`. Với chống phá hoại, board kết hợp `ldr_covered` và `pir_detected` để xác nhận `sabotage_alert`.”

---

# 10. Câu trả lời khi thầy hỏi

## 10.1. Vì sao không cho dashboard sửa `fire_alert`, `sabotage_alert`?

> “Dạ vì đây là kết luận do board tự xử lý từ cảm biến. Nếu cho dashboard sửa trực tiếp thì dữ liệu sẽ không còn phản ánh trạng thái thật của phần cứng.”

---

## 10.2. Vì sao `sos_level` là Read Only?

> “Dạ vì mức SOS nên do board tự tính từ hành vi bấm SOS. Người dùng chỉ bấm `sos_child` hoặc `sos_adult`, còn board quyết định mức 1, 2 hay 3 để tránh người dùng sửa sai mức cảnh báo.”

---

## 10.3. Vì sao cần `email_event_type`?

> “Dạ vì Google Apps Script cần biết sự kiện nào đang xảy ra. Ví dụ `sos_child`, `fire`, hoặc `sabotage`, từ đó chọn nội dung Gmail và người nhận phù hợp.”

---

## 10.4. Vì sao xác nhận công an/cứu hỏa không đưa vào Arduino Cloud?

> “Dạ vì trong danh sách biến hiện tại chưa có biến xác nhận riêng. Để không tạo thêm biến ngoài thiết kế, nhóm em để phần xác nhận nằm trong Google Apps Script thông qua link trong Gmail. Apps Script sẽ xử lý ai xác nhận trước, gửi mail tiếp và khóa các xác nhận sau.”

---

## 10.5. Vì sao chống phá hoại cần cả LDR và PIR?

> “Dạ nếu chỉ dùng LDR thì dễ báo nhầm khi tắt đèn hoặc môi trường tối. Vì vậy nhóm em kết hợp LDR bị che với PIR phát hiện chuyển động. Khi vừa có ánh sáng bất thường vừa có người ở gần thiết bị, hệ thống mới xác nhận là phá hoại.”

---

# 11. Checklist trước demo

- [ ] Arduino IoT Cloud Device đang online.
- [ ] Thing `IoT_Anti_Theft_System` có đúng biến thật.
- [ ] Dashboard có `sos_child`, `sos_adult`, `reset_alarm`.
- [ ] Dashboard có `alarm_status`, `sos_level`, `sos_message`.
- [ ] Dashboard có `flame_value`, `flame_detected`, `kitchen_temperature`, `fire_alert`.
- [ ] Dashboard có `ldr_value`, `ldr_delta`, `ldr_covered`, `pir_detected`, `sabotage_alert`.
- [ ] Dashboard có `send_email_request`, `email_event_type`, `email_sent_status`.
- [ ] LED đỏ hoạt động.
- [ ] Buzzer hoạt động.
- [ ] Google Apps Script Web App URL đúng.
- [ ] Gmail người nhận đúng.
- [ ] Đã test `reset_alarm` để tắt cảnh báo.
- [ ] Nếu chưa có cảm biến nhiệt độ thật, phải nói rõ khi demo báo cháy.
- [ ] Nếu xác nhận công an/cứu hỏa làm qua Gmail, phải nói rõ phần đó do Google Apps Script xử lý.

---

# 12. Kết luận

Bản demo sau khi viết lại gồm 3 kịch bản:

1. **SOS từ điện thoại theo mức độ**  
   Dùng `sos_child`, `sos_adult`, `sos_level`, `sos_message`, `alarm_status`, `send_email_request`, `email_event_type`, `email_sent_status`.

2. **Cảnh báo cháy**  
   Dùng `flame_value`, `flame_detected`, `kitchen_temperature`, `fire_alert`, `alarm_status`, `send_email_request`, `email_event_type`, `email_sent_status`.

3. **Chống phá hoại thiết bị**  
   Dùng `ldr_value`, `ldr_delta`, `light_abnormal`, `ldr_covered`, `pir_detected`, `sabotage_alert`, `device_tampered`, `alarm_status`.

Điểm quan trọng nhất của bản mới:

```text
Không bịa thêm biến Arduino Cloud.
Luồng xác nhận công an/cứu hỏa được đưa sang Google Apps Script.
Arduino Cloud chỉ dùng đúng các biến đã có trong file thật.
```
