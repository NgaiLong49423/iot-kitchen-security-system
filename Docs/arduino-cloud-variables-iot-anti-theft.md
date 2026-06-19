# Danh Sách Biến Arduino Cloud Cho Dự Án IoT Anti-Theft System

| Item | Nội dung |
|---|---|
| Project | IoT Based Anti-Theft System |
| Nhóm | Group 6 - SE2034 |
| Mục đích tài liệu | Liệt kê toàn bộ Cloud Variables cần tạo trong Arduino IoT Cloud |
| Nội dung cập nhật | Bổ sung quyền biến, kiểu cập nhật, lý do chọn Read & Write / Read Only, On Change / Periodic |
| Mô hình đề xuất | 1 Device → 1 Thing → 3 Dashboard |
| Thing đề xuất | `IoT_Anti_Theft_System` |
| Version | 2.1 |
| Updated | June 19, 2026 |

---

## 1. Tổng Quan Thiết Kế Cloud

Dự án nên dùng mô hình:

```text
1 board thật
→ 1 Device trên Arduino Cloud
→ 1 Thing chứa toàn bộ biến
→ 3 Dashboard theo vai trò người dùng
```

Trong đó:

- **Device** là board thật, ví dụ ESP32-CAM hoặc ESP8266.
- **Thing** là nơi chứa các biến Cloud Variables.
- **Dashboard** là giao diện điều khiển và hiển thị trên app/web Arduino Cloud.
- **Cloud Variable** là biến được đồng bộ giữa board và Arduino Cloud.

Không nên tạo nhiều Thing cho cùng một Device. Cách hợp lý hơn là tạo **1 Thing duy nhất**, rồi chia ra nhiều Dashboard theo vai trò.

---

## 2. Giải Thích Quyền Và Kiểu Cập Nhật Biến

Khi tạo biến trong Arduino Cloud, cần chọn đúng **Permission** và **Update Policy**.

### 2.1. Permission - Quyền của biến

| Quyền | Nghĩa | Khi nào dùng |
|---|---|---|
| `Read & Write` | Dashboard/app được đọc và được ghi giá trị xuống board | Dùng cho nút bấm, công tắc, lệnh điều khiển |
| `Read Only` | Dashboard/app chỉ được xem, board là bên cập nhật giá trị | Dùng cho cảm biến, trạng thái hệ thống, kết quả xử lý |

Ví dụ:

```text
sos_child = Read & Write
vì trẻ em cần bấm nút SOS từ app xuống board.

pir_detected = Read Only
vì PIR là cảm biến do board đọc, người dùng không nên tự sửa.
```

---

### 2.2. Update Policy - Kiểu cập nhật

| Kiểu cập nhật | Nghĩa | Khi nào dùng |
|---|---|---|
| `On Change` | Chỉ gửi lên Cloud khi giá trị thay đổi | Dùng cho biến trạng thái, cảnh báo, nút bấm, log |
| `Periodic` | Gửi lên Cloud theo chu kỳ, ví dụ mỗi 1s, 2s, 5s | Dùng cho giá trị đo liên tục như khoảng cách, ánh sáng, nhiệt độ |

Ví dụ:

```text
flame_detected = On Change
vì nó chỉ có true/false, chỉ cần gửi khi đổi trạng thái.

kitchen_temperature = Periodic
vì nhiệt độ thay đổi liên tục, dashboard cần xem giá trị theo thời gian.
```

---

### 2.3. Quy tắc chọn nhanh

```text
Biến nào app cần bấm / điều khiển board
→ Read & Write + On Change

Biến nào board tự đọc từ cảm biến rồi gửi lên app
→ Read Only + Periodic hoặc On Change

Biến nào là trạng thái/cảnh báo chỉ đổi khi có sự kiện
→ Read Only + On Change

Biến nào là số đo liên tục
→ Read Only + Periodic
```

---

## 3. Số Dashboard Nên Có

Nên có **3 Dashboard**:

| Dashboard | Người dùng | Mục đích |
|---|---|---|
| `Child SOS Dashboard` | Trẻ em | Chỉ có nút SOS trẻ em và trạng thái cảnh báo |
| `Parent Control Dashboard` | Người lớn | Điều khiển cơ bản: SOS người lớn, reset, bật/tắt bảo vệ, xem trạng thái |
| `Admin Demo Dashboard` | Người demo/leader | Xem và điều khiển toàn bộ biến quan trọng để demo với giảng viên |

---

# 4. Bảng Tổng Hợp Biến Cần Tạo Trong Thing

Tất cả biến dưới đây nên được tạo trong **1 Thing duy nhất**:

```text
Thing: IoT_Anti_Theft_System
```

---

## 4.1. Nhóm Biến Điều Khiển Chính

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `alarm_enabled` | `bool` | Read & Write | On Change | Không cần | Parent, Admin | App cần bật/tắt chế độ bảo vệ, nên dashboard phải ghi được xuống board |
| `system_armed` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Đây là trạng thái thực tế do board tự quyết định sau khi xét thời gian, RFID, WiFi whitelist |
| `reset_alarm` | `bool` | Read & Write | On Change | Không cần | Parent, Admin | Người lớn/admin cần bấm reset từ app để dừng còi và đưa hệ thống về trạng thái an toàn |
| `alarm_status` | `String` | Read Only | On Change | Không cần | Child, Parent, Admin | Board tự cập nhật trạng thái như `SAFE`, `ARMED`, `FIRE_ALERT`, dashboard chỉ cần xem |

### Giải thích từng biến

#### `alarm_enabled`

```cpp
bool alarm_enabled;
```

**Lưu gì?**

```text
true  = người dùng muốn bật bảo vệ
false = người dùng muốn tắt bảo vệ
```

**Tại sao cần?**

Đây là công tắc tổng của hệ thống. Không có biến này thì hệ thống không biết người dùng đang muốn bật hay tắt chế độ chống trộm.

---

#### `system_armed`

```cpp
bool system_armed;
```

**Lưu gì?**

```text
true  = hệ thống đang thật sự bảo vệ
false = hệ thống chưa bảo vệ hoặc đang được vô hiệu hóa
```

**Tại sao cần?**

`alarm_enabled` là mong muốn của người dùng.  
`system_armed` là trạng thái thực tế sau khi board xét thêm các điều kiện như ban đêm, RFID hợp lệ, người nhà có ở gần không.

Ví dụ:

```text
alarm_enabled = true
nhưng phát hiện điện thoại người nhà đang ở gần
→ system_armed = false
```

---

#### `reset_alarm`

```cpp
bool reset_alarm;
```

**Lưu gì?**

```text
true = người dùng yêu cầu dừng báo động hiện tại
```

**Tại sao cần?**

Khi còi đang kêu, người lớn/admin cần có nút để tắt còi, tắt LED và reset trạng thái cảnh báo.

---

#### `alarm_status`

```cpp
String alarm_status;
```

**Lưu gì?**

Ví dụ:

```text
"SAFE"
"ARMED"
"INTRUSION_ALERT"
"FIRE_ALERT"
"SOS_CHILD_ALERT"
"SABOTAGE_ALERT"
"PET_IGNORED"
```

**Tại sao cần?**

Dashboard sẽ dễ nhìn hơn. Người dùng không cần hiểu nhiều số 0/1, chỉ cần đọc trạng thái dạng chữ.

---

## 4.2. Nhóm Biến SOS Theo Vai Trò

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `sos_child` | `bool` | Read & Write | On Change | Không cần | Child, Admin | Trẻ em cần bấm nút SOS trên app, nên dashboard phải ghi được xuống board |
| `sos_adult` | `bool` | Read & Write | On Change | Không cần | Parent, Admin | Người lớn cần bấm SOS trên app, nên dashboard phải ghi được xuống board |
| `sos_level` | `int` | Read Only | On Change | Không cần | Admin | Mức SOS nên do board tự tính, tránh người dùng tự sửa lung tung |
| `sos_message` | `String` | Read Only | On Change | Không cần | Parent, Admin | Nội dung cảnh báo do board tạo ra để hiển thị/gửi email |

### Giải thích từng biến

#### `sos_child`

```cpp
bool sos_child;
```

**Lưu gì?**

```text
true  = trẻ em đã bấm SOS
false = chưa bấm
```

**Tại sao cần?**

Đây là biến quan trọng nhất cho ý tưởng phân quyền SOS. Khi `sos_child = true`, hệ thống hiểu đây là tín hiệu khẩn cấp từ trẻ em.

Hành động đề xuất:

```text
Bật còi
Bật LED đỏ
Gửi email khẩn cấp cho người lớn
Ghi log sự kiện
Có thể chụp ảnh hiện trường
```

---

#### `sos_adult`

```cpp
bool sos_adult;
```

**Lưu gì?**

```text
true  = người lớn đã bấm SOS
false = chưa bấm
```

**Tại sao cần?**

Dùng để phân biệt với SOS của trẻ em. Theo thiết kế của dự án, trẻ em bấm SOS thì gửi email khẩn cấp cho người lớn, còn người lớn bấm SOS thì có thể chỉ báo động tại chỗ hoặc ghi log.

Hành động đề xuất:

```text
Bật còi
Bật LED đỏ
Ghi log
Không gửi email khẩn cấp cho người lớn
```

---

#### `sos_level`

```cpp
int sos_level;
```

**Lưu gì?**

```text
0 = không có SOS
1 = chỉ báo động tại chỗ
2 = gửi email cho người lớn
3 = gửi email cho người lớn + nhóm giả lập công an/cứu hộ
```

**Tại sao cần?**

Nếu sau này muốn làm chức năng trẻ em bấm nhiều lần để tăng mức cảnh báo, biến này rất hữu ích.

Ví dụ:

```text
Bấm 1 lần trong 3 giây → sos_level = 1
Bấm 2 lần trong 3 giây → sos_level = 2
Bấm 3 lần trong 3 giây → sos_level = 3
```

---

#### `sos_message`

```cpp
String sos_message;
```

**Lưu gì?**

Ví dụ:

```text
"Tre em da kich hoat SOS tai khu vuc bep"
```

**Tại sao cần?**

Dùng để hiển thị trên dashboard hoặc gửi nội dung email rõ ràng hơn.

---

## 4.3. Nhóm Biến Cảm Biến Chuyển Động Và Khoảng Cách

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `pir_detected` | `bool` | Read Only | On Change | Không cần | Admin | PIR chỉ có trạng thái có/không, chỉ cần cập nhật khi đổi trạng thái |
| `ultrasonic_distance` | `float` | Read Only | Periodic | 1s khi demo, 5s khi chạy ổn định | Admin | Khoảng cách thay đổi liên tục nên cần gửi theo chu kỳ |
| `object_near` | `bool` | Read Only | On Change | Không cần | Admin | Board tự kết luận có vật thể gần hay không |
| `pet_detected` | `bool` | Read Only | On Change | Không cần | Admin | Board tự kết luận có phải vật nuôi hay không |

### Giải thích từng biến

#### `pir_detected`

```cpp
bool pir_detected;
```

**Lưu gì?**

```text
true  = có chuyển động
false = không có chuyển động
```

**Tại sao cần?**

PIR là cảm biến chính để phát hiện có người hoặc vật di chuyển trong khu vực phòng bếp.

---

#### `ultrasonic_distance`

```cpp
float ultrasonic_distance;
```

**Lưu gì?**

Khoảng cách đo được từ cảm biến HC-SR04, đơn vị thường dùng là cm.

Ví dụ:

```text
72.5
```

**Tại sao cần?**

Dùng để xác nhận vật thể có đang ở gần khu vực nguy hiểm hay không. Cũng dùng để hỗ trợ lọc vật nuôi.

---

#### `object_near`

```cpp
bool object_near;
```

**Lưu gì?**

```text
true  = có vật thể trong vùng nguy hiểm
false = không có vật thể gần
```

**Tại sao cần?**

Giúp dashboard dễ hiểu hơn thay vì chỉ nhìn số khoảng cách.

---

#### `pet_detected`

```cpp
bool pet_detected;
```

**Lưu gì?**

```text
true  = nghi là vật nuôi
false = không phải vật nuôi
```

**Tại sao cần?**

Dự án có chức năng lọc nhiễu vật nuôi. Nếu PIR có chuyển động nhưng khoảng cách/chiều cao cho thấy vật thể thấp, hệ thống không hú còi.

---

## 4.4. Nhóm Biến Ánh Sáng LDR

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `ldr_value` | `int` | Read Only | Periodic | 1s khi demo, 5s khi chạy ổn định | Admin | Ánh sáng thay đổi liên tục nên cần gửi theo chu kỳ |
| `ldr_delta` | `int` | Read Only | Periodic | 1s khi demo, 5s khi chạy ổn định | Admin | Độ thay đổi ánh sáng cũng biến thiên liên tục |
| `light_abnormal` | `bool` | Read Only | On Change | Không cần | Admin | Board tự kết luận ánh sáng bất thường hay không |
| `ldr_covered` | `bool` | Read Only | On Change | Không cần | Admin | Board tự kết luận LDR có bị che hay không |

### Giải thích từng biến

#### `ldr_value`

```cpp
int ldr_value;
```

**Lưu gì?**

Giá trị ánh sáng đọc từ LDR.

Ví dụ:

```text
120 = tối
700 = sáng mạnh
```

**Tại sao cần?**

Dùng để phát hiện ánh sáng bất thường, đèn pin hoặc hành vi che cảm biến.

---

#### `ldr_delta`

```cpp
int ldr_delta;
```

**Lưu gì?**

Độ thay đổi ánh sáng giữa lần đọc trước và lần đọc hiện tại.

Ví dụ:

```text
ldr trước = 700
ldr sau   = 100
ldr_delta = 600
```

**Tại sao cần?**

Không chỉ cần biết sáng hay tối, mà cần biết ánh sáng có thay đổi đột ngột hay không. Đây là phần giúp phát hiện đèn pin hoặc hành vi che LDR.

---

#### `light_abnormal`

```cpp
bool light_abnormal;
```

**Lưu gì?**

```text
true  = ánh sáng bất thường
false = ánh sáng bình thường
```

**Tại sao cần?**

Dùng trong thuật toán kết hợp cảm biến:

```text
PIR + khoảng cách + ánh sáng bất thường
→ nghi đột nhập
```

---

#### `ldr_covered`

```cpp
bool ldr_covered;
```

**Lưu gì?**

```text
true  = LDR bị che bất thường
false = bình thường
```

**Tại sao cần?**

Dùng cho chức năng chống phá hoại thiết bị. Ví dụ ban ngày đang sáng, tự nhiên LDR tụt mạnh xuống tối đen thì có thể ai đó đang che cảm biến.

---

## 4.5. Nhóm Biến Báo Cháy Và Nhiệt Độ Bếp

> Lưu ý quan trọng: **Flame Sensor không đo nhiệt độ trực tiếp**. Flame Sensor chủ yếu phát hiện tín hiệu ánh sáng/hồng ngoại từ ngọn lửa. Vì vậy cần tách rõ:
>
> - `flame_value`: giá trị tín hiệu thô từ Flame Sensor.
> - `flame_detected`: kết luận có phát hiện dấu hiệu ngọn lửa hay không.
> - `kitchen_temperature`: nhiệt độ khu vực bếp, cần thêm cảm biến nhiệt độ như DHT11, DHT22, DS18B20 hoặc MLX90614.
> - `fire_alert`: kết luận cuối cùng của hệ thống sau khi đối chiếu tín hiệu lửa và nhiệt độ.

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `flame_value` | `int` | Read Only | Periodic | 1s hoặc 2s | Parent, Admin | Giá trị thô từ cảm biến lửa thay đổi liên tục nên cần xem theo chu kỳ |
| `flame_detected` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự kết luận có dấu hiệu lửa hay không, chỉ gửi khi đổi trạng thái |
| `kitchen_temperature` | `float` | Read Only | Periodic | 2s hoặc 5s | Parent, Admin | Nhiệt độ bếp thay đổi liên tục, người dùng cần xem để kiểm tra báo cháy có đáng tin không |
| `fire_alert` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự xác nhận báo cháy sau khi xử lý chống nhiễu |

### Giải thích từng biến

#### `flame_value`

```cpp
int flame_value;
```

**Lưu gì?**

Lưu giá trị thô đọc được từ Flame Sensor.

Ví dụ:

```text
flame_value = 850  → bình thường
flame_value = 320  → có thể có lửa
```

Giá trị cao/thấp còn tùy loại module và cách đấu chân analog.

**Tại sao cần?**

Vì `flame_detected` chỉ là true/false. Nếu có thêm `flame_value`, người xem có thể biết cảm biến lửa đang đọc tín hiệu bao nhiêu. Điều này giúp demo thuyết phục hơn.

---

#### `flame_detected`

```cpp
bool flame_detected;
```

**Lưu gì?**

```text
true  = phát hiện dấu hiệu ngọn lửa
false = không phát hiện ngọn lửa
```

**Tại sao cần?**

Đây là kết luận sơ bộ từ Flame Sensor. Khi biến này chuyển thành true, hệ thống có thể kiểm tra thêm nhiệt độ bếp trước khi xác nhận báo cháy.

---

#### `kitchen_temperature`

```cpp
float kitchen_temperature;
```

**Lưu gì?**

Lưu nhiệt độ khu vực bếp, đơn vị °C.

Ví dụ:

```text
31.5°C
45.2°C
68.0°C
```

**Tại sao cần?**

Biến này giúp người dùng kiểm tra xem cảnh báo cháy có hợp lý không.

Ví dụ:

```text
flame_detected = true
kitchen_temperature = 72.5°C
→ cảnh báo cháy đáng tin hơn
```

Nếu:

```text
flame_detected = true
kitchen_temperature = 30°C
```

thì có thể chỉ ghi nhận là tín hiệu nghi ngờ, chưa vội xác nhận cháy lớn.

---

#### `fire_alert`

```cpp
bool fire_alert;
```

**Lưu gì?**

```text
true  = hệ thống xác nhận nguy cơ cháy
false = chưa có cảnh báo cháy
```

**Tại sao cần?**

`flame_detected` là dữ liệu sơ bộ.  
`fire_alert` là kết luận cuối cùng sau khi xử lý chống nhiễu.

Logic gợi ý:

```cpp
if (flame_detected && kitchen_temperature >= 55.0) {
  fire_alert = true;
  alarm_status = "FIRE_ALERT";
  threat_level = 4;
}
```

Nếu nhóm chưa có cảm biến nhiệt độ thật, vẫn có thể tạo `kitchen_temperature` để chuẩn bị cho bản nâng cấp, nhưng khi demo cần nói rõ nhiệt độ là dữ liệu từ cảm biến nhiệt độ rời, không phải từ Flame Sensor.

---

## 4.6. Nhóm Biến Nhận Diện Đột Nhập

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `intrusion_alert` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự xác nhận có đột nhập hay không |
| `intrusion_score` | `int` | Read Only | On Change | Không cần | Admin | Điểm nghi ngờ do board tự tính từ nhiều cảm biến |
| `threat_level` | `int` | Read Only | On Change | Không cần | Parent, Admin | Mức nguy hiểm tổng thể do board tự tính |

### Giải thích từng biến

#### `intrusion_alert`

```cpp
bool intrusion_alert;
```

**Lưu gì?**

```text
true  = có đột nhập
false = không có đột nhập
```

**Tại sao cần?**

Đây là biến chính để kích hoạt còi, LED, chụp ảnh, gửi thông báo hoặc gửi email.

---

#### `intrusion_score`

```cpp
int intrusion_score;
```

**Lưu gì?**

Điểm nghi ngờ đột nhập.

Ví dụ:

```text
PIR phát hiện chuyển động       +1
Khoảng cách dưới 100 cm         +1
Ánh sáng thay đổi bất thường    +1
Đang ban đêm                    +1
```

Nếu tổng điểm cao:

```text
intrusion_score >= 3
→ intrusion_alert = true
```

**Tại sao cần?**

Dùng score, tức là điểm đánh giá nguy cơ. Cách này dễ demo và dễ giải thích hơn so với chỉ dùng điều kiện cứng.

---

#### `threat_level`

```cpp
int threat_level;
```

**Lưu gì?**

Mức nguy hiểm tổng thể.

```text
0 = an toàn
1 = nghi ngờ nhẹ
2 = cảnh báo
3 = nguy hiểm
4 = khẩn cấp
```

**Tại sao cần?**

Dùng để quyết định hành động:

```text
level 1 → chỉ ghi log
level 2 → bật LED
level 3 → bật còi
level 4 → gửi email
```

---

## 4.7. Nhóm Biến Chống Phá Hoại Thiết Bị

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `sabotage_alert` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự xác nhận có hành vi phá hoại hay không |
| `device_tampered` | `bool` | Read Only | On Change | Không cần | Admin | Board tự xác nhận thiết bị có bị can thiệp vật lý hay không |

### Giải thích từng biến

#### `sabotage_alert`

```cpp
bool sabotage_alert;
```

**Lưu gì?**

```text
true  = nghi bị phá hoại
false = bình thường
```

**Tại sao cần?**

Dự án có chức năng phát hiện hành vi che cảm biến, bịt mắt LDR hoặc can thiệp thiết bị.

---

#### `device_tampered`

```cpp
bool device_tampered;
```

**Lưu gì?**

```text
true  = thiết bị có dấu hiệu bị can thiệp
false = bình thường
```

**Tại sao cần?**

Biến này rộng hơn `sabotage_alert`. Sau này nếu thêm cảm biến rung, công tắc mở hộp hoặc phát hiện thiết bị bị rơi thì vẫn dùng được.

---

## 4.8. Nhóm Biến Thời Gian DS1307

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `current_time` | `String` | Read Only | Periodic | 5s hoặc 10s | Parent, Admin | Thời gian thay đổi liên tục nên gửi theo chu kỳ, nhưng không cần quá nhanh |
| `current_hour` | `int` | Read Only | Periodic | 60s | Admin | Giờ hiện tại chỉ cần cập nhật mỗi phút |
| `night_mode` | `bool` | Read Only | On Change | Không cần | Admin | Chỉ cần gửi khi chuyển ngày/đêm hoặc khi trạng thái thay đổi |

### Giải thích từng biến

#### `current_time`

```cpp
String current_time;
```

**Lưu gì?**

Thời gian hiện tại lấy từ DS1307.

Ví dụ:

```text
"02:15:08"
```

**Tại sao cần?**

Dùng để hiển thị trên dashboard và ghi log sự kiện.

---

#### `current_hour`

```cpp
int current_hour;
```

**Lưu gì?**

Giờ hiện tại.

Ví dụ:

```text
2
```

**Tại sao cần?**

Dùng để kiểm tra ban ngày/ban đêm nhanh hơn trong code.

---

#### `night_mode`

```cpp
bool night_mode;
```

**Lưu gì?**

```text
true  = ban đêm
false = ban ngày
```

**Tại sao cần?**

Ban đêm hệ thống nên nhạy hơn. Ban ngày có thể giảm độ nhạy để tránh báo động giả.

---

## 4.9. Nhóm Biến RFID

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `rfid_uid` | `String` | Read Only | On Change | Không cần | Admin | Chỉ gửi khi có thẻ mới được quét |
| `rfid_authorized` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự kiểm tra thẻ hợp lệ hay không |
| `access_status` | `String` | Read Only | On Change | Không cần | Parent, Admin | Chỉ đổi khi có sự kiện quét thẻ |

### Giải thích từng biến

#### `rfid_uid`

```cpp
String rfid_uid;
```

**Lưu gì?**

UID thẻ RFID vừa quét.

Ví dụ:

```text
"13 A7 5C 2B"
```

**Tại sao cần?**

Để biết thẻ nào vừa được đưa vào hệ thống.

---

#### `rfid_authorized`

```cpp
bool rfid_authorized;
```

**Lưu gì?**

```text
true  = thẻ hợp lệ
false = thẻ lạ
```

**Tại sao cần?**

Nếu thẻ hợp lệ thì có thể tắt báo động hoặc vô hiệu hóa bảo vệ. Nếu thẻ lạ thì ghi log hoặc cảnh báo.

---

#### `access_status`

```cpp
String access_status;
```

**Lưu gì?**

Ví dụ:

```text
"ACCESS_GRANTED"
"ACCESS_DENIED"
```

**Tại sao cần?**

Dashboard hiển thị rõ hơn thay vì chỉ hiện true/false.

---

## 4.10. Nhóm Biến WiFi Lạ Và MAC Whitelist

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `known_device_present` | `bool` | Read Only | On Change | Không cần | Admin | Board tự phát hiện điện thoại người nhà trong whitelist |
| `unknown_wifi_count` | `int` | Read Only | Periodic | 5s | Admin | Số lần phát hiện WiFi lạ có thể tăng liên tục |
| `unknown_wifi_alert` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Chỉ cần gửi khi xác nhận có thiết bị lạ đáng nghi |
| `last_unknown_mac` | `String` | Read Only | On Change | Không cần | Admin | Chỉ gửi khi phát hiện MAC lạ mới |

### Giải thích từng biến

#### `known_device_present`

```cpp
bool known_device_present;
```

**Lưu gì?**

```text
true  = có điện thoại người nhà gần đó
false = không thấy thiết bị quen
```

**Tại sao cần?**

Dùng cho chức năng tự động bật/tắt bảo vệ dựa trên sự hiện diện của người nhà.

---

#### `unknown_wifi_count`

```cpp
int unknown_wifi_count;
```

**Lưu gì?**

Số lần phát hiện thiết bị WiFi lạ.

Ví dụ:

```text
5
```

**Tại sao cần?**

Dự án có kịch bản thiết bị WiFi lạ lảng vảng. Nếu xuất hiện nhiều lần trong thời gian ngắn thì hệ thống cảnh báo.

---

#### `unknown_wifi_alert`

```cpp
bool unknown_wifi_alert;
```

**Lưu gì?**

```text
true  = có thiết bị lạ lảng vảng
false = bình thường
```

**Tại sao cần?**

Tách dữ liệu thô `unknown_wifi_count` khỏi kết luận cuối cùng.

---

#### `last_unknown_mac`

```cpp
String last_unknown_mac;
```

**Lưu gì?**

MAC thiết bị lạ gần nhất.

Ví dụ:

```text
"AA:BB:CC:11:22:33"
```

**Tại sao cần?**

Để log và demo rõ: hệ thống phát hiện được thiết bị lạ cụ thể, không chỉ đếm số lượng.

---

## 4.11. Nhóm Biến Camera ESP32-CAM

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `capture_photo` | `bool` | Read & Write | On Change | Không cần | Parent, Admin | Người lớn/admin có thể bấm chụp ảnh từ dashboard |
| `photo_status` | `String` | Read Only | On Change | Không cần | Parent, Admin | Board tự báo trạng thái chụp ảnh |

### Giải thích từng biến

#### `capture_photo`

```cpp
bool capture_photo;
```

**Lưu gì?**

```text
true  = cần chụp ảnh
false = không cần chụp
```

**Tại sao cần?**

Khi có SOS, cháy hoặc đột nhập, hệ thống có thể bật biến này để ESP32-CAM chụp ảnh. Người lớn/admin cũng có thể chủ động yêu cầu chụp ảnh từ dashboard.

---

#### `photo_status`

```cpp
String photo_status;
```

**Lưu gì?**

Ví dụ:

```text
"PHOTO_IDLE"
"PHOTO_CAPTURING"
"PHOTO_DONE"
"PHOTO_FAILED"
```

**Tại sao cần?**

Để dashboard biết camera có chụp thành công không.

---

## 4.12. Nhóm Biến Còi Và LED

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `buzzer_on` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự bật/tắt còi theo cảnh báo, người dùng không nên bật còi trực tiếp |
| `led_red_on` | `bool` | Read Only | On Change | Không cần | Parent, Admin | Board tự bật/tắt LED theo cảnh báo |

### Giải thích từng biến

#### `buzzer_on`

```cpp
bool buzzer_on;
```

**Lưu gì?**

```text
true  = còi đang kêu
false = còi tắt
```

**Tại sao cần?**

Để dashboard biết còi đang bật hay không. Không nên để Read & Write trong bản chính, vì người dùng bấm nhầm có thể làm còi kêu lung tung. Muốn tắt còi thì dùng `reset_alarm`.

---

#### `led_red_on`

```cpp
bool led_red_on;
```

**Lưu gì?**

```text
true  = LED đỏ bật
false = LED đỏ tắt
```

**Tại sao cần?**

LED đỏ dùng để hiển thị trạng thái nguy hiểm tại chỗ.

---

## 4.13. Nhóm Biến Gửi Email Google Apps Script

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `send_email_request` | `bool` | Read Only | On Change | Không cần | Admin | Board tự quyết định khi nào cần gửi email, tránh người dùng tự kích hoạt email lung tung |
| `email_event_type` | `String` | Read Only | On Change | Không cần | Admin | Board tự xác định loại sự kiện gửi email |
| `email_sent_status` | `String` | Read Only | On Change | Không cần | Parent, Admin | Board tự báo gửi email thành công hay thất bại |

### Giải thích từng biến

#### `send_email_request`

```cpp
bool send_email_request;
```

**Lưu gì?**

```text
true  = cần gửi email
false = không gửi
```

**Tại sao cần?**

Khi SOS trẻ em, cháy hoặc đột nhập xảy ra, biến này được bật để code gọi Google Apps Script.

---

#### `email_event_type`

```cpp
String email_event_type;
```

**Lưu gì?**

Ví dụ:

```text
"sos_child"
"fire"
"intrusion"
"sabotage"
```

**Tại sao cần?**

Google Apps Script cần biết loại sự kiện để chọn nội dung email và người nhận phù hợp.

---

#### `email_sent_status`

```cpp
String email_sent_status;
```

**Lưu gì?**

Ví dụ:

```text
"EMAIL_IDLE"
"EMAIL_SENDING"
"EMAIL_SENT"
"EMAIL_FAILED"
```

**Tại sao cần?**

Để demo biết email đã gửi thành công hay lỗi.

---

## 4.14. Nhóm Biến Log Sự Kiện

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `last_event` | `String` | Read Only | On Change | Không cần | Parent, Admin | Board tự ghi sự kiện gần nhất |
| `last_event_type` | `String` | Read Only | On Change | Không cần | Admin | Board tự phân loại sự kiện gần nhất |
| `event_counter` | `int` | Read Only | On Change | Không cần | Admin | Board tự tăng số khi có sự kiện mới |

### Giải thích từng biến

#### `last_event`

```cpp
String last_event;
```

**Lưu gì?**

Ví dụ:

```text
"02:15:08 - INTRUSION - PIR + HC-SR04 + LDR"
```

**Tại sao cần?**

Dashboard admin và dashboard người lớn cần xem nhanh chuyện gì vừa xảy ra.

---

#### `last_event_type`

```cpp
String last_event_type;
```

**Lưu gì?**

Ví dụ:

```text
"SOS_CHILD"
"FIRE"
"INTRUSION"
"PET_IGNORED"
"RFID_ACCESS"
```

**Tại sao cần?**

Dùng để lọc log và xử lý hành động tiếp theo.

---

#### `event_counter`

```cpp
int event_counter;
```

**Lưu gì?**

Tổng số sự kiện đã xảy ra.

Ví dụ:

```text
0, 1, 2, 3...
```

**Tại sao cần?**

Khi demo, mỗi lần có sự kiện thì số này tăng lên. Nhìn vào biến này có thể biết hệ thống có ghi nhận sự kiện hay không.

---

## 4.15. Nhóm Biến Chống Spam Cảnh Báo

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `cooldown_active` | `bool` | Read Only | On Change | Không cần | Admin | Board tự xác định hệ thống đang trong thời gian chống spam |
| `last_alert_time` | `String` | Read Only | On Change | Không cần | Admin | Chỉ đổi khi có cảnh báo mới được gửi |

### Giải thích từng biến

#### `cooldown_active`

```cpp
bool cooldown_active;
```

**Lưu gì?**

```text
true  = đang cooldown
false = có thể gửi cảnh báo mới
```

**Cooldown** nghĩa là thời gian nghỉ giữa hai lần gửi cảnh báo.

**Tại sao cần?**

Nếu kẻ gian đứng trước cảm biến 1 phút, hệ thống không nên gửi quá nhiều email hoặc thông báo liên tục.

---

#### `last_alert_time`

```cpp
String last_alert_time;
```

**Lưu gì?**

Thời điểm gửi cảnh báo gần nhất.

Ví dụ:

```text
"02:15:08"
```

**Tại sao cần?**

Dùng để so sánh thời gian và quyết định có được gửi cảnh báo mới chưa.

---

## 4.16. Nhóm Biến Chế Độ Demo

| Tên biến | Kiểu dữ liệu | Permission | Update Policy | Chu kỳ gợi ý | Dashboard dùng | Tại sao chọn như vậy |
|---|---|---|---|---|---|---|
| `demo_mode` | `bool` | Read & Write | On Change | Không cần | Admin | Admin cần bật/tắt chế độ demo từ dashboard |
| `demo_scenario` | `int` | Read & Write | On Change | Không cần | Admin | Admin cần chọn nhanh kịch bản demo số 1, 2, 3... |

### Giải thích từng biến

#### `demo_mode`

```cpp
bool demo_mode;
```

**Lưu gì?**

```text
true  = đang demo
false = chạy thực tế
```

**Tại sao cần?**

Khi `demo_mode = false`, hệ thống chạy thật theo cảm biến thật.

Khi `demo_mode = true`, admin có thể dùng `demo_scenario` để chọn kịch bản cần trình diễn hoặc kiểm thử.

Biến này giúp demo ổn định hơn, tránh phụ thuộc hoàn toàn vào nhiễu cảm biến.

---

#### `demo_scenario`

```cpp
int demo_scenario;
```

**Lưu gì?**

Kịch bản demo mà nhóm muốn chạy.

```text
0 = chạy bình thường
1 = demo đột nhập
2 = demo vật nuôi
3 = demo mất điện
4 = demo WiFi lạ
5 = demo tự động bật/tắt
6 = demo phá hoại thiết bị
7 = demo cháy
8 = demo SOS trẻ em
```

**Tại sao cần?**

Giúp người demo linh hoạt chọn kịch bản khi bảo vệ.

Ví dụ:

```text
demo_mode = true
demo_scenario = 7
→ hệ thống chạy nhánh demo báo cháy

demo_mode = true
demo_scenario = 8
→ hệ thống chạy nhánh demo SOS trẻ em
```

Lưu ý khi thuyết trình:

```text
demo_mode và demo_scenario chỉ dùng cho kiểm thử/trình diễn.
Khi chạy thực tế, demo_mode = false thì hệ thống vẫn xử lý theo cảm biến thật.
```

---

# 5. Bản Chốt Danh Sách Biến Để Tạo Trong Thing

Bạn có thể copy danh sách này để tạo biến trong Arduino Cloud.

```cpp
// Điều khiển hệ thống
bool alarm_enabled;             // Read & Write, On Change
bool system_armed;              // Read Only, On Change
bool reset_alarm;               // Read & Write, On Change
String alarm_status;            // Read Only, On Change

// SOS
bool sos_child;                 // Read & Write, On Change
bool sos_adult;                 // Read & Write, On Change
int sos_level;                  // Read Only, On Change
String sos_message;             // Read Only, On Change

// Cảm biến chuyển động và khoảng cách
bool pir_detected;              // Read Only, On Change
float ultrasonic_distance;      // Read Only, Periodic, 1s demo / 5s ổn định
bool object_near;               // Read Only, On Change
bool pet_detected;              // Read Only, On Change

// Ánh sáng LDR
int ldr_value;                  // Read Only, Periodic, 1s demo / 5s ổn định
int ldr_delta;                  // Read Only, Periodic, 1s demo / 5s ổn định
bool light_abnormal;            // Read Only, On Change
bool ldr_covered;               // Read Only, On Change

// Báo cháy và nhiệt độ bếp
int flame_value;                // Read Only, Periodic, 1s hoặc 2s
bool flame_detected;            // Read Only, On Change
float kitchen_temperature;      // Read Only, Periodic, 2s hoặc 5s
bool fire_alert;                // Read Only, On Change

// Nhận diện đột nhập
bool intrusion_alert;           // Read Only, On Change
int intrusion_score;            // Read Only, On Change
int threat_level;               // Read Only, On Change

// Chống phá hoại thiết bị
bool sabotage_alert;            // Read Only, On Change
bool device_tampered;           // Read Only, On Change

// Thời gian DS1307
String current_time;            // Read Only, Periodic, 5s hoặc 10s
int current_hour;               // Read Only, Periodic, 60s
bool night_mode;                // Read Only, On Change

// RFID
String rfid_uid;                // Read Only, On Change
bool rfid_authorized;           // Read Only, On Change
String access_status;           // Read Only, On Change

// WiFi lạ và MAC whitelist
bool known_device_present;      // Read Only, On Change
int unknown_wifi_count;         // Read Only, Periodic, 5s
bool unknown_wifi_alert;        // Read Only, On Change
String last_unknown_mac;        // Read Only, On Change

// Camera ESP32-CAM
bool capture_photo;             // Read & Write, On Change
String photo_status;            // Read Only, On Change

// Output còi và LED
bool buzzer_on;                 // Read Only, On Change
bool led_red_on;                // Read Only, On Change

// Email Google Apps Script
bool send_email_request;        // Read Only, On Change
String email_event_type;        // Read Only, On Change
String email_sent_status;       // Read Only, On Change

// Log sự kiện
String last_event;              // Read Only, On Change
String last_event_type;         // Read Only, On Change
int event_counter;              // Read Only, On Change

// Chống spam cảnh báo
bool cooldown_active;           // Read Only, On Change
String last_alert_time;         // Read Only, On Change

// Demo
bool demo_mode;                 // Read & Write, On Change
int demo_scenario;              // Read & Write, On Change
```

Tổng cộng: **39 biến**.

---

# 6. Các Biến Read & Write

Các biến này dashboard/app được phép ghi xuống board:

```cpp
bool alarm_enabled;
bool reset_alarm;
bool sos_child;
bool sos_adult;
bool capture_photo;
bool demo_mode;
int demo_scenario;
```

**Tại sao chỉ những biến này là Read & Write?**

Vì đây là các biến điều khiển trực tiếp từ người dùng:

- Bật/tắt bảo vệ.
- Reset báo động.
- Bấm SOS.
- Yêu cầu chụp ảnh.
- Bật demo mode.
- Chọn kịch bản demo.

Không nên cho người dùng ghi trực tiếp vào các biến cảm biến hoặc biến cảnh báo như `pir_detected`, `fire_alert`, `intrusion_alert`, vì như vậy sẽ làm sai bản chất hệ thống.

---

# 7. Các Biến Read Only

Các biến còn lại nên để **Read Only**.

Lý do:

```text
Các biến này là dữ liệu cảm biến, kết quả xử lý, trạng thái hệ thống hoặc log.
Chúng phải do board tự cập nhật để đảm bảo tính đúng kỹ thuật.
Dashboard chỉ nên xem, không nên sửa.
```

Ví dụ:

```cpp
bool pir_detected;         // cảm biến PIR đọc được
bool fire_alert;           // board xác nhận có cháy
bool intrusion_alert;      // board xác nhận có đột nhập
String last_event;         // board ghi log sự kiện
```

---

# 8. Các Biến Periodic

Các biến nên để **Periodic**:

```cpp
float ultrasonic_distance; // 1s demo / 5s ổn định
int ldr_value;             // 1s demo / 5s ổn định
int ldr_delta;             // 1s demo / 5s ổn định
int flame_value;           // 1s hoặc 2s
float kitchen_temperature; // 2s hoặc 5s
String current_time;       // 5s hoặc 10s
int current_hour;          // 60s
int unknown_wifi_count;    // 5s
```

**Tại sao là Periodic?**

Vì đây là các giá trị có thể thay đổi liên tục theo thời gian. Dashboard cần được cập nhật định kỳ để người xem thấy dữ liệu đang thay đổi.

---

# 9. Các Biến On Change

Các biến còn lại nên để **On Change**.

**Tại sao là On Change?**

Vì các biến này chỉ cần cập nhật khi có sự kiện hoặc khi trạng thái thay đổi.

Ví dụ:

```cpp
bool fire_alert;
bool intrusion_alert;
bool sos_child;
bool reset_alarm;
String last_event;
String alarm_status;
```

Nếu gửi các biến này theo chu kỳ thì sẽ tốn tài nguyên Cloud không cần thiết.

---

# 10. Logic Gợi Ý Cho Báo Cháy Có Đối Chiếu Nhiệt Độ

Vì Flame Sensor không đo nhiệt độ trực tiếp, nên không nên nói:

```text
Flame Sensor đo được nhiệt độ bếp
```

Cách nói đúng là:

```text
Flame Sensor phát hiện dấu hiệu ngọn lửa.
Cảm biến nhiệt độ riêng đo nhiệt độ khu vực bếp.
Hệ thống đối chiếu hai nguồn dữ liệu để xác nhận báo cháy.
```

Logic gợi ý:

```cpp
if (flame_detected && kitchen_temperature >= 55.0) {
  fire_alert = true;
  alarm_status = "FIRE_ALERT";
  threat_level = 4;
  buzzer_on = true;
  led_red_on = true;
  email_event_type = "fire";
}
```

Nếu:

```text
flame_detected = true
kitchen_temperature = 30°C
```

thì hệ thống có thể ghi:

```text
Nghi ngờ tín hiệu lửa, cần kiểm tra thêm
```

Nếu:

```text
flame_detected = true
kitchen_temperature = 72°C
```

thì hệ thống có thể ghi:

```text
Xác nhận cảnh báo cháy
```

---

# 11. Gợi Ý Biến Theo Từng Dashboard

## 11.1. Child SOS Dashboard

Chỉ nên có:

| Widget | Biến | Lý do |
|---|---|---|
| Nút SOS trẻ em | `sos_child` | Trẻ em chỉ cần bấm SOS |
| Trạng thái báo động | `alarm_status` | Cho trẻ em biết hệ thống đã nhận lệnh hay chưa |

Mục tiêu:

```text
Trẻ em chỉ thấy đúng chức năng cần dùng, tránh bấm nhầm.
```

---

## 11.2. Parent Control Dashboard

Nên có:

| Widget | Biến | Lý do |
|---|---|---|
| SOS người lớn | `sos_adult` | Người lớn có thể kích hoạt cảnh báo tại chỗ |
| Reset báo động | `reset_alarm` | Dừng còi/LED khi đã xử lý xong |
| Bật/tắt bảo vệ | `alarm_enabled` | Cho phép người lớn bật/tắt hệ thống |
| Trạng thái hệ thống | `alarm_status` | Xem trạng thái hiện tại |
| Sự kiện gần nhất | `last_event` | Biết chuyện gì vừa xảy ra |
| Mức nguy hiểm | `threat_level` | Biết mức độ rủi ro |
| Kết quả gửi email | `email_sent_status` | Kiểm tra email có gửi thành công không |
| Trạng thái báo cháy | `fire_alert` | Biết hệ thống đã xác nhận cháy chưa |
| Giá trị lửa | `flame_value` | Xem tín hiệu cảm biến lửa |
| Nhiệt độ bếp | `kitchen_temperature` | Kiểm tra nhiệt độ thực tế khu vực bếp |

Mục tiêu:

```text
Người lớn có quyền xử lý hệ thống nhưng không cần thấy toàn bộ biến kỹ thuật.
```

---

## 11.3. Admin Demo Dashboard

Nên có các biến quan trọng:

| Nhóm | Biến |
|---|---|
| Điều khiển | `alarm_enabled`, `system_armed`, `reset_alarm`, `alarm_status` |
| SOS | `sos_child`, `sos_adult`, `sos_level`, `sos_message` |
| Cảm biến chuyển động | `pir_detected`, `ultrasonic_distance`, `object_near`, `pet_detected` |
| Ánh sáng | `ldr_value`, `ldr_delta`, `light_abnormal`, `ldr_covered` |
| Báo cháy | `flame_value`, `flame_detected`, `kitchen_temperature`, `fire_alert` |
| Xử lý nguy hiểm | `intrusion_alert`, `intrusion_score`, `sabotage_alert`, `device_tampered`, `threat_level` |
| Thời gian | `current_time`, `current_hour`, `night_mode` |
| RFID | `rfid_uid`, `rfid_authorized`, `access_status` |
| WiFi | `known_device_present`, `unknown_wifi_count`, `unknown_wifi_alert`, `last_unknown_mac` |
| Camera | `capture_photo`, `photo_status` |
| Output | `buzzer_on`, `led_red_on` |
| Email | `send_email_request`, `email_event_type`, `email_sent_status` |
| Log | `last_event`, `last_event_type`, `event_counter` |
| Chống spam | `cooldown_active`, `last_alert_time` |
| Demo | `demo_mode`, `demo_scenario` |

Mục tiêu:

```text
Admin/leader có thể xem toàn bộ hệ thống và ép chạy từng kịch bản demo.
```

---

# 12. Kết Luận

Cấu trúc đề xuất cuối cùng:

```text
1 Device thật
→ 1 Thing: IoT_Anti_Theft_System
→ 39 Cloud Variables
→ 3 Dashboard:
   1. Child SOS Dashboard
   2. Parent Control Dashboard
   3. Admin Demo Dashboard
```

Cách này giúp dự án:

- Dễ code.
- Dễ demo.
- Không bị rối Thing/Device.
- Phân biệt được vai trò trẻ em và người lớn.
- Có bảng quyền rõ ràng cho từng biến.
- Biết biến nào dùng `Read & Write`, biến nào dùng `Read Only`.
- Biết biến nào dùng `On Change`, biến nào dùng `Periodic`.
- Bổ sung được dữ liệu `flame_value` và `kitchen_temperature` để báo cháy thuyết phục hơn.
- Giải thích được với giảng viên là hệ thống có thiết kế phân quyền theo dashboard.
- Hỗ trợ đầy đủ các chức năng: chống trộm, báo cháy, SOS, gửi email, log, chống phá hoại và demo kịch bản.
