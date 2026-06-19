# Cách Nối Phần Cứng Cho Project IoT Based Anti-Theft System

> Tài liệu này mô tả cách nối phần cứng cho hệ thống **IoT Based Anti-Theft System** sử dụng **ESP32-S3 CAM OV2640** và các cảm biến/phần cứng đi kèm.

---
| Item         | Context                     |
| ------------ | --------------------------- |
| Project Name | IoT based anti-theft system |
| Version      | 2.0.2                       |
| Prepared By  | Group 6                     |
| Date         | June 19, 2026                 |

# CÁCH NỐI PHẦN CỨNG

## ESP32-CAM-MB (AI Thinker OV2640)

## Thiết bị sử dụng

* ESP32-CAM-MB + ESP32-CAM AI Thinker OV2640
* RFID RC522
* DS1307 RTC
* HY-SRF05 Ultrasonic Sensor
* PIR 5050
* Light Sensor (LDR)
* Flame Sensor
* Còi thụ động 1407 (Buzzer)
* LED đỏ

---

# 1. DS1307 RTC (I2C)

| DS1307 | ESP32-CAM |
| ------ | --------- |
| VCC    | 3.3V      |
| GND    | GND       |
| SDA    | GPIO14    |
| SCL    | GPIO15    |

Chức năng:

* Lưu thời gian thực.
* Vẫn hoạt động khi mất điện nhờ pin CR2032.

---

# 2. RFID RC522 (SPI)

| RC522    | ESP32-CAM |
| -------- | --------- |
| SDA (SS) | GPIO13    |
| SCK      | GPIO12    |
| MOSI     | GPIO2     |
| MISO     | GPIO16    |
| RST      | GPIO0     |
| VCC      | 3.3V      |
| GND      | GND       |

Chức năng:

* Xác thực người dùng bằng thẻ RFID.
* Arm/Disarm hệ thống.

---

# 3. PIR 5050

| PIR | ESP32-CAM |
| --- | --------- |
| VCC | 5V        |
| GND | GND       |
| OUT | GPIO3     |

Chức năng:

* Phát hiện chuyển động của người.

---

# 4. HY-SRF05 Ultrasonic Sensor

| HY-SRF05 | ESP32-CAM |
| -------- | --------- |
| VCC      | 5V        |
| GND      | GND       |
| TRIG     | GPIO1     |
| ECHO     | GPIO13    |

## Chú ý

Chân ECHO xuất mức 5V.

Cần chia áp:

```text
ECHO
 |
1kΩ
 |
--------> GPIO13
 |
2kΩ
 |
GND
```

Chức năng:

* Đo khoảng cách vật thể.
* Hỗ trợ phân biệt người và vật nuôi.

---

# 5. Light Sensor (LDR)

| LDR Module | ESP32-CAM |
| ---------- | --------- |
| VCC        | 3.3V      |
| GND        | GND       |
| AO         | GPIO33    |

Chức năng:

* Đo cường độ sáng.
* Phát hiện ánh sáng bất thường.

---

# 6. Flame Sensor

| Flame Sensor | ESP32-CAM |
| ------------ | --------- |
| VCC          | 3.3V      |
| GND          | GND       |
| DO           | GPIO4     |

Chức năng:

* Cảnh báo cháy.

---

# 7. Buzzer 1407

| Buzzer | ESP32-CAM |
| ------ | --------- |
| +      | GPIO2     |
| -      | GND       |

Chức năng:

* Phát âm thanh báo động.

---

# 8. LED đỏ

Sử dụng điện trở 220Ω.

```text
GPIO12 ----220Ω---->|----- GND
                  LED đỏ
```

| LED đỏ      | ESP32-CAM |
| ----------- | --------- |
| Anode (+)   | GPIO12    |
| Cathode (-) | GND       |

Chức năng:

* Hiển thị trạng thái báo động.

---

# CAMERA OV2640

Camera đã tích hợp sẵn trên ESP32-CAM AI Thinker.

Không cần đấu nối thêm.

Chức năng:

* Chụp ảnh khi phát hiện đột nhập.
* Gửi ảnh lên Telegram hoặc Cloud.

---

# SƠ ĐỒ TỔNG THỂ

```text
                 ESP32-CAM AI Thinker
              -------------------------

DS1307
 SDA ---------------- GPIO14
 SCL ---------------- GPIO15

PIR
 OUT ---------------- GPIO3

HY-SRF05
 TRIG --------------- GPIO1
 ECHO --------------- GPIO13

LDR
 AO ----------------- GPIO33

Flame Sensor
 DO ----------------- GPIO4

Buzzer
 + ------------------ GPIO2

RFID RC522
 SDA ---------------- GPIO13
 MOSI --------------- GPIO2
 SCK ---------------- GPIO12
 MISO --------------- GPIO16
 RST ---------------- GPIO0

LED đỏ
 + ------------------ GPIO12

3.3V ------------- DS1307, RC522, LDR, Flame Sensor

5V --------------- PIR, HY-SRF05

GND -------------- Chung tất cả
```

---

# NGUỒN CẤP

Khuyến nghị:

```text
Adapter 5V 2A
      │
      ├── ESP32-CAM
      ├── PIR
      ├── HY-SRF05
      ├── Buzzer
      └── LED đỏ

Nguồn 3.3V từ ESP32-CAM

      ├── DS1307
      ├── RC522
      ├── LDR
      └── Flame Sensor
```

---

# LƯU Ý QUAN TRỌNG

* Tất cả module phải nối chung GND.
* Chân ECHO của HY-SRF05 bắt buộc phải qua mạch chia áp trước khi vào ESP32-CAM.
* GPIO0 dùng cho chế độ nạp chương trình, hạn chế sử dụng khi chạy thực tế.
* GPIO1 và GPIO3 là UART, nên không nên dùng Serial Monitor liên tục khi cảm biến đang hoạt động.
* Camera OV2640 chiếm nhiều GPIO, cần kiểm tra xung đột chân trước khi viết chương trình.
* Nguồn khuyến nghị: Adapter 5V 2A.
