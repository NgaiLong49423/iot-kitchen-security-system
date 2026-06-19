# Cách Nối Phần Cứng Cho Project IoT Based Anti-Theft System

> Tài liệu này mô tả cách nối phần cứng cho hệ thống **IoT Based Anti-Theft System** sử dụng **ESP32-S3 CAM OV2640** và các cảm biến/phần cứng đi kèm.

---

## 1. Danh Sách Thiết Bị Sử Dụng

| STT | Thiết bị | Vai trò |
|---:|---|---|
| 1 | ESP32-S3 CAM OV2640 | Vi điều khiển chính, tích hợp camera |
| 2 | RFID RC522 | Xác thực người dùng bằng thẻ RFID |
| 3 | DS1307 RTC | Cung cấp thời gian thực |
| 4 | HY-SRF05 Ultrasonic Sensor | Đo khoảng cách vật thể |
| 5 | PIR 5050 | Phát hiện chuyển động |
| 6 | Light Sensor / LDR | Phát hiện ánh sáng bất thường |
| 7 | Flame Sensor | Phát hiện lửa |
| 8 | Buzzer 1407 | Phát âm thanh báo động |
| 9 | LED đỏ | Hiển thị trạng thái báo động |

---

## 2. Nguyên Tắc Nối Dây Quan Trọng

Trước khi đấu nối, cần lưu ý các nguyên tắc sau:

- Tất cả module phải **nối chung GND**.
- Các module dùng **3.3V** không được cấp nhầm **5V**.
- Chân **ECHO** của cảm biến siêu âm **HY-SRF05** xuất mức tín hiệu **5V**, trong khi ESP32-S3 chỉ chịu được mức logic **3.3V**, nên bắt buộc phải dùng **mạch chia áp**.
- Nên dùng nguồn ngoài ổn định, khuyến nghị **Adapter 5V 2A**.
- Cần kiểm tra đúng **pinout** của phiên bản ESP32-S3 CAM OV2640 đang dùng để tránh xung đột chân với camera.
- Không cần đấu nối thêm camera vì **OV2640 đã tích hợp sẵn trên board ESP32-S3 CAM**.

---

## 3. Kết Nối Từng Thiết Bị

---

## 3.1. DS1307 RTC

### Giao tiếp

DS1307 sử dụng giao tiếp **I2C**.

> I2C là chuẩn giao tiếp dùng 2 dây tín hiệu chính: **SDA** để truyền dữ liệu và **SCL** để truyền xung clock.

### Bảng nối dây

| DS1307 RTC | ESP32-S3 CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO1 |
| SCL | GPIO2 |

### Chức năng

- Cung cấp thời gian thực cho hệ thống.
- Vẫn duy trì thời gian khi mất điện nhờ pin **CR2032**.

---

## 3.2. RFID RC522

### Giao tiếp

RFID RC522 sử dụng giao tiếp **SPI**.

> SPI là chuẩn giao tiếp tốc độ cao, thường dùng các chân **SCK**, **MOSI**, **MISO**, **SS/SDA** để truyền dữ liệu giữa vi điều khiển và module ngoại vi.

### Bảng nối dây

| RC522 | ESP32-S3 CAM |
|---|---|
| SDA / SS | GPIO10 |
| SCK | GPIO12 |
| MOSI | GPIO11 |
| MISO | GPIO13 |
| RST | GPIO14 |
| VCC | 3.3V |
| GND | GND |

### Chức năng

- Xác thực người dùng bằng thẻ RFID.
- Bật hoặc tắt chế độ bảo vệ của hệ thống.

---

## 3.3. PIR 5050

### Bảng nối dây

| PIR 5050 | ESP32-S3 CAM |
|---|---|
| VCC | 5V |
| GND | GND |
| OUT | GPIO4 |

### Chức năng

- Phát hiện chuyển động của con người.
- Dùng để nhận biết có người đi vào vùng giám sát.

---

## 3.4. HY-SRF05 Ultrasonic Sensor

### Bảng nối dây

| HY-SRF05 | ESP32-S3 CAM |
|---|---|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO5 |
| ECHO | GPIO6, nhưng phải qua mạch chia áp |

### Cảnh báo quan trọng

Chân **ECHO** của HY-SRF05 xuất tín hiệu mức **5V**.

ESP32-S3 chỉ chịu mức logic **3.3V**, vì vậy **không được nối trực tiếp ECHO vào GPIO6**.

### Mạch chia áp cho chân ECHO

```text
ECHO
 |
1kΩ
 |
--------> GPIO6
 |
2kΩ
 |
GND
```

### Chức năng

- Đo khoảng cách vật thể.
- Hỗ trợ phân biệt người và vật nuôi.

---

## 3.5. Light Sensor / LDR

### Bảng nối dây

| LDR | ESP32-S3 CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| AO | GPIO7 |

### Chức năng

- Phát hiện ánh sáng bất thường.
- Phát hiện tình huống cảm biến bị che khuất.

---

## 3.6. Flame Sensor

### Bảng nối dây

| Flame Sensor | ESP32-S3 CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DO | GPIO8 |

### Chức năng

- Phát hiện lửa trong khu vực bếp.
- Có thể kích hoạt cảnh báo khẩn cấp khi phát hiện cháy.

---

## 3.7. Buzzer 1407

### Bảng nối dây

| Buzzer 1407 | ESP32-S3 CAM |
|---|---|
| + | GPIO9 |
| - | GND |

### Chức năng

- Phát âm thanh báo động khi có sự kiện nguy hiểm.
- Có thể dùng cho cảnh báo xâm nhập, cảnh báo cháy hoặc trạng thái SOS.

---

## 3.8. LED Đỏ

### Yêu cầu điện trở

LED đỏ cần mắc nối tiếp với điện trở **220Ω** để hạn chế dòng điện, tránh làm cháy LED hoặc gây quá tải chân GPIO.

### Sơ đồ nối

```text
GPIO15 ---- 220Ω ---->|----- GND
                      LED đỏ
```

### Bảng nối dây

| LED đỏ | ESP32-S3 CAM |
|---|---|
| Anode / cực dương | GPIO15 |
| Cathode / cực âm | GND |

### Chức năng

- Hiển thị trạng thái báo động.
- Có thể dùng để báo trạng thái đang bật bảo vệ, phát hiện xâm nhập hoặc lỗi hệ thống.

---

## 3.9. Camera OV2640

Camera **OV2640** đã được tích hợp sẵn trên board **ESP32-S3 CAM**.

### Chức năng

- Chụp ảnh khi phát hiện xâm nhập.
- Gửi ảnh về điện thoại hoặc hệ thống giám sát.

### Lưu ý

- Không cần đấu nối thêm camera ngoài.
- Cần kiểm tra pinout của board để tránh dùng nhầm các GPIO đã bị camera chiếm dụng.

---

## 4. Sơ Đồ Kết Nối Tổng Thể

```text
ESP32-S3 CAM
------------------

DS1307
 SDA  ---------------- GPIO1
 SCL  ---------------- GPIO2

PIR 5050
 OUT  ---------------- GPIO4

HY-SRF05
 TRIG ---------------- GPIO5
 ECHO ---------------- GPIO6, qua mạch chia áp

LDR
 AO   ---------------- GPIO7

Flame Sensor
 DO   ---------------- GPIO8

Buzzer 1407
 +    ---------------- GPIO9

RFID RC522
 SDA  ---------------- GPIO10
 MOSI ---------------- GPIO11
 SCK  ---------------- GPIO12
 MISO ---------------- GPIO13
 RST  ---------------- GPIO14

LED đỏ
 +    ---------------- GPIO15
```

---

## 5. Tổng Hợp Chân GPIO Được Sử Dụng

| GPIO | Thiết bị | Chức năng |
|---:|---|---|
| GPIO1 | DS1307 | SDA |
| GPIO2 | DS1307 | SCL |
| GPIO4 | PIR 5050 | OUT |
| GPIO5 | HY-SRF05 | TRIG |
| GPIO6 | HY-SRF05 | ECHO qua chia áp |
| GPIO7 | LDR | AO |
| GPIO8 | Flame Sensor | DO |
| GPIO9 | Buzzer 1407 | Cực dương |
| GPIO10 | RFID RC522 | SDA / SS |
| GPIO11 | RFID RC522 | MOSI |
| GPIO12 | RFID RC522 | SCK |
| GPIO13 | RFID RC522 | MISO |
| GPIO14 | RFID RC522 | RST |
| GPIO15 | LED đỏ | Anode qua điện trở 220Ω |

---

## 6. Nguồn Cấp

### Nguồn khuyến nghị

Sử dụng **Adapter 5V 2A** để cấp nguồn ổn định cho toàn bộ hệ thống.

```text
Adapter 5V 2A
 |
 ├── ESP32-S3 CAM
 ├── PIR 5050
 ├── HY-SRF05
 ├── Buzzer
 └── LED đỏ
```

### Nguồn 3.3V từ ESP32-S3 CAM

Nguồn **3.3V** từ ESP32-S3 CAM dùng để cấp cho các module sau:

```text
3.3V từ ESP32-S3 CAM
 |
 ├── DS1307
 ├── RC522
 ├── LDR
 └── Flame Sensor
```

---

## 7. Bảng Tổng Hợp Điện Áp Theo Thiết Bị

| Thiết bị | Điện áp cấp | Ghi chú |
|---|---:|---|
| ESP32-S3 CAM | 5V | Nên cấp bằng adapter 5V 2A |
| DS1307 RTC | 3.3V | Giao tiếp I2C |
| RFID RC522 | 3.3V | Không cấp 5V cho RC522 |
| PIR 5050 | 5V | OUT nối GPIO4 |
| HY-SRF05 | 5V | ECHO bắt buộc qua chia áp |
| LDR | 3.3V | AO nối GPIO7 |
| Flame Sensor | 3.3V | DO nối GPIO8 |
| Buzzer 1407 | GPIO/GND | Nên kiểm tra dòng tiêu thụ |
| LED đỏ | GPIO/GND | Phải có điện trở 220Ω |

---

## 8. Checklist Trước Khi Cắm Nguồn

Trước khi cấp nguồn cho mạch, nên kiểm tra lần lượt:

- [ ] Tất cả module đã nối chung GND.
- [ ] RC522 được cấp 3.3V, không cấp nhầm 5V.
- [ ] DS1307 được cấp 3.3V.
- [ ] LDR được cấp 3.3V.
- [ ] Flame Sensor được cấp 3.3V.
- [ ] HY-SRF05 được cấp 5V.
- [ ] Chân ECHO của HY-SRF05 đã đi qua mạch chia áp.
- [ ] LED đỏ đã mắc nối tiếp điện trở 220Ω.
- [ ] Không có dây nào chạm chập giữa 5V và GND.
- [ ] Đã kiểm tra đúng pinout của board ESP32-S3 CAM OV2640.
- [ ] Dùng nguồn đủ dòng, khuyến nghị Adapter 5V 2A.

---

## 9. Lưu Ý An Toàn Khi Test

- Không nên cắm tất cả module cùng lúc ngay lần đầu.
- Nên test từng module riêng, ví dụ:
  1. ESP32-S3 CAM.
  2. RC522.
  3. DS1307.
  4. PIR.
  5. HY-SRF05 với mạch chia áp.
  6. LDR.
  7. Flame Sensor.
  8. Buzzer và LED.
- Khi test cảm biến HY-SRF05, tuyệt đối không nối trực tiếp chân ECHO 5V vào ESP32-S3.
- Nếu thấy board nóng bất thường, có mùi khét hoặc máy tính ngắt USB, phải rút nguồn ngay.
- Khi chưa chắc chắn mạch đúng, nên cấp nguồn qua adapter ngoài thay vì lấy toàn bộ nguồn từ cổng USB laptop.

---

## 10. Ghi Chú Kỹ Thuật

- **GPIO** là chân vào/ra số của vi điều khiển, dùng để đọc cảm biến hoặc điều khiển thiết bị.
- **AO** là ngõ ra analog, tức tín hiệu thay đổi theo mức điện áp.
- **DO** là ngõ ra digital, tức tín hiệu chỉ có hai trạng thái chính: HIGH hoặc LOW.
- **GND chung** nghĩa là tất cả thiết bị phải có cùng mốc điện áp 0V để tín hiệu đọc được chính xác.
- **Mạch chia áp** là mạch dùng điện trở để hạ mức điện áp tín hiệu, ví dụ từ 5V xuống khoảng 3.3V để bảo vệ ESP32-S3.

---

## 11. Kết Luận

Sơ đồ trên cho phép ESP32-S3 CAM kết nối với nhiều cảm biến để xây dựng hệ thống chống trộm IoT gồm:

- Xác thực bằng RFID.
- Phát hiện chuyển động.
- Đo khoảng cách.
- Nhận biết ánh sáng bất thường.
- Phát hiện lửa.
- Báo động bằng buzzer và LED.
- Chụp ảnh bằng camera OV2640 khi có sự kiện xâm nhập.

Điểm cần đặc biệt chú ý là **nguồn cấp**, **GND chung**, **điện áp 3.3V/5V**, và **mạch chia áp cho chân ECHO của HY-SRF05**.
