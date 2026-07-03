Dựa trên **ESP32-S3 WROOM N16R8 Freenove** trong hình và **SRS hệ thống chống trộm** (PIR + siêu âm + LDR + RTC + LED + Buzzer), dưới đây là sơ đồ nối dây phù hợp.

> **Lưu ý**
>
> * ESP32-S3 sử dụng mức logic **3.3V**.
> * DS1307 và HY-SRF05 thường cấp **5V**.
> * Chân **Echo của HY-SRF05 là 5V**, nên cần giảm áp xuống 3.3V trước khi đưa vào ESP32 bằng cầu chia áp (khuyến nghị).
> * Tất cả GND của các module phải nối chung.

---

# 1. PIR HC-SR505

| PIR HC-SR505 | ESP32-S3 |
| ------------ | -------- |
| VCC          | 5V (VIN) |
| GND          | GND      |
| OUT          | GPIO4    |

GPIO4 chỉ làm Input nên rất phù hợp đọc tín hiệu PIR.

---

# 2. Cảm biến ánh sáng LDR

Dùng module LDR có chân AO và DO.

### Chỉ dùng AO

| LDR | ESP32-S3         |
| --- | ---------------- |
| VCC | 3V3              |
| GND | GND              |
| AO  | GPIO1 (ADC1_CH0) |

Không cần dùng DO.

---

# 3. RTC DS1307

Giao tiếp I2C.

| DS1307 | ESP32-S3 |
| ------ | -------- |
| VCC    | 5V       |
| GND    | GND      |
| SDA    | GPIO8    |
| SCL    | GPIO9    |

GPIO8 và GPIO9 hoàn toàn phù hợp làm I2C.

---

# 4. LED đỏ

Nối qua điện trở 220Ω.

| LED đỏ      | ESP32-S3                 |
| ----------- | ------------------------ |
| Anode (+)   | GPIO14 qua điện trở 220Ω |
| Cathode (-) | GND                      |

---

# 5. LED xanh

Qua điện trở 220Ω.

| LED xanh    | ESP32-S3                 |
| ----------- | ------------------------ |
| Anode (+)   | GPIO15 qua điện trở 220Ω |
| Cathode (-) | GND                      |

---

# 6. Buzzer 1407

Nếu là **Active Buzzer 3.3–5V**:

| Buzzer | ESP32-S3 |
| ------ | -------- |
| (+)    | GPIO16   |
| (-)    | GND      |

Nếu buzzer tiêu thụ dòng lớn (>20 mA), nên điều khiển qua transistor NPN hoặc MOSFET. Nếu là buzzer active nhỏ (5–15 mA) có thể nối trực tiếp.

---

# 7. HY-SRF05

| HY-SRF05 | ESP32-S3                             |
| -------- | ------------------------------------ |
| VCC      | 5V                                   |
| GND      | GND                                  |
| TRIG     | GPIO17                               |
| ECHO     | GPIO18 *(qua cầu chia áp 5V → 3.3V)* |

### Cầu chia áp Echo

```
Echo ----1kΩ-----+------ GPIO18
                 |
                2kΩ
                 |
                GND
```

Điện áp:

```
5V
 ↓
≈3.3V
```

An toàn cho ESP32-S3.

---

# 8. Tổng hợp chân sử dụng

| Module        | Chân ESP32-S3 |
| ------------- | ------------- |
| PIR HC-SR505  | GPIO4         |
| LDR AO        | GPIO1         |
| DS1307 SDA    | GPIO8         |
| DS1307 SCL    | GPIO9         |
| LED đỏ        | GPIO14        |
| LED xanh      | GPIO15        |
| Buzzer        | GPIO16        |
| HY-SRF05 TRIG | GPIO17        |
| HY-SRF05 ECHO | GPIO18        |

---

# 9. Nguồn cấp

| Module       | Điện áp                |
| ------------ | ---------------------- |
| ESP32-S3     | USB                    |
| PIR HC-SR505 | 5V                     |
| DS1307       | 5V                     |
| HY-SRF05     | 5V                     |
| LDR          | 3.3V                   |
| LED đỏ       | GPIO qua điện trở 220Ω |
| LED xanh     | GPIO qua điện trở 220Ω |
| Buzzer       | GPIO16                 |

---

## Đánh giá theo SRS

Sơ đồ trên đáp ứng đầy đủ các chức năng trong SRS:

* ✔ Phát hiện chuyển động bằng PIR HC-SR505.
* ✔ Đo khoảng cách bằng HY-SRF05.
* ✔ Phân biệt ngày/đêm bằng LDR (ADC).
* ✔ Quản lý thời gian bằng DS1307 (I2C).
* ✔ LED xanh báo trạng thái hệ thống (ví dụ đã kích hoạt/chế độ bình thường).
* ✔ LED đỏ báo cảnh báo.
* ✔ Buzzer phát còi khi có sự kiện cảnh báo.

Đồng thời vẫn còn rất nhiều GPIO chưa sử dụng (GPIO2, GPIO3, GPIO5–GPIO7, GPIO10–GPIO13, GPIO21, GPIO38–GPIO48...), thuận tiện nếu sau này bạn bổ sung Wi-Fi, camera, RFID, hoặc các cảm biến khác.
