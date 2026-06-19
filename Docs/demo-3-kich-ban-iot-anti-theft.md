# Tài liệu Demo 3 Kịch Bản IoT Anti-Theft System  
## Bản sửa chân LDR cho ESP32-CAM AI Thinker OV2640

> Lý do sửa: ESP32-CAM AI Thinker OV2640 không có chân GPIO33 đưa ra ngoài, nên không thể nối `AO` của LDR vào GPIO33.  
> Với demo dùng WiFi / Arduino IoT Cloud, nên dùng chân `DO` của module LDR thay vì `AO`.

---

## 1. Ba kịch bản demo

| STT | Kịch bản | Module chính |
|---|---|---|
| 1 | SOS trên điện thoại | ESP32-CAM, WiFi, LED, Buzzer |
| 2 | Cảnh báo cháy | Flame Sensor, LED, Buzzer |
| 3 | Chống phá hoại thiết bị | LDR dùng chân DO, PIR, LED, Buzzer |

---

## 2. Module cần nối

| Module | Có dùng không? | Lý do |
|---|---:|---|
| ESP32-CAM AI Thinker OV2640 | Có | Vi điều khiển chính, WiFi, xử lý logic |
| Flame Sensor | Có | Demo cảnh báo cháy |
| LDR Light Sensor | Có | Demo phát hiện che cảm biến |
| HC-SR501  | Có | Demo phát hiện chuyển động |
| LED đỏ | Có | Hiển thị cảnh báo |
| Buzzer 1407 | Có | Phát còi cảnh báo |


---

## 3. Bảng nối dây bản sửa

### 3.1 Nguồn chính

| Nguồn | ESP32-CAM |
|---|---|
| 5V | 5V |
| GND | GND |

Tất cả module phải nối chung GND.

---

### 3.2 Flame Sensor

| Flame Sensor | ESP32-CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DO | GPIO15 |

> `DO` là Digital Output, nghĩa là ngõ ra số, chỉ có trạng thái HIGH/LOW.

---

### 3.3 LDR Light Sensor

| LDR | ESP32-CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DO | GPIO14 |
| AO | Không nối |

> Không dùng `AO -> GPIO33` vì ESP32-CAM AI Thinker không có GPIO33.  
> Với demo, dùng `DO` là đủ. Chỉnh biến trở xanh trên module LDR để chọn ngưỡng sáng/tối.

---

### 3.4 PIR 5050

| PIR | ESP32-CAM |
|---|---|
| VCC | 5V |
| GND | GND |
| OUT | GPIO13 |

> Trước khi nối OUT vào ESP32-CAM, nên đo điện áp OUT so với GND. Nếu OUT gần 5V thì cần chia áp xuống 3.3V.

---

### 3.5 LED đỏ

| LED đỏ | ESP32-CAM |
|---|---|
| Anode (+) | GPIO2 qua điện trở 220Ω |
| Cathode (-) | GND |

> LED bắt buộc có điện trở 220Ω hoặc 330Ω.

---

### 3.6 Buzzer 1407

Cách an toàn nhất là dùng transistor.

| Buzzer / mạch đệm | ESP32-CAM |
|---|---|
| Chân điều khiển transistor | GPIO16 |
| Buzzer + | 5V |
| Buzzer - | Qua transistor xuống GND |

Nếu chưa có transistor và chỉ test nhanh buzzer nhỏ:

| Buzzer | ESP32-CAM |
|---|---|
| + | GPIO16 |
| - | GND |

> Không khuyến khích nối trực tiếp buzzer lâu dài vì GPIO ESP32 chịu dòng nhỏ.

---

## 4. Pin mapping dùng trong code

```cpp
#define PIN_FLAME 15
#define PIN_LDR   14
#define PIN_PIR   13
#define PIN_LED   2
#define PIN_BUZZER 16
```

---

## 5. Logic demo 3 kịch bản

### Kịch bản 1: SOS trên điện thoại

1. Người trình diễn bấm nút SOS trên Arduino IoT Cloud.
2. ESP32-CAM nhận lệnh.
3. LED đỏ nháy.
4. Buzzer kêu.
5. Có thể gửi email cảnh báo qua Google Apps Script.

---

### Kịch bản 2: Cảnh báo cháy

1. Đưa nguồn lửa nhỏ hoặc nguồn hồng ngoại lại gần Flame Sensor.
2. Flame Sensor đổi trạng thái DO.
3. ESP32-CAM phát hiện cháy.
4. LED đỏ nháy.
5. Buzzer kêu.
6. Có thể gửi email cảnh báo cháy.

Lưu ý: Không dí lửa sát breadboard, dây điện hoặc ESP32-CAM.

---

### Kịch bản 3: Chống phá hoại bằng LDR + PIR

1. Ban đầu LDR ở môi trường sáng.
2. Người trình diễn dùng tay hoặc khăn che LDR đột ngột.
3. LDR DO đổi trạng thái.
4. Người trình diễn quơ tay trước PIR.
5. ESP32-CAM xác nhận có hành vi che cảm biến kèm chuyển động.
6. LED đỏ nháy và buzzer kêu.

---

## 6. Checklist trước khi cấp nguồn

- [ ] RC522, DS1307, HY-SRF05 chưa nối khi test 3 demo này.
- [ ] ESP32-CAM nhận 5V đúng chân 5V.
- [ ] Tất cả GND nối chung.
- [ ] Flame Sensor cấp 3.3V, không cấp 5V.
- [ ] LDR cấp 3.3V, dùng DO, không dùng AO.
- [ ] LED có điện trở 220Ω.
- [ ] PIR OUT đã được kiểm tra điện áp.
- [ ] Buzzer không nóng bất thường.
- [ ] Không dùng GPIO0 cho module demo.
- [ ] Không dùng GPIO1/GPIO3 để tránh ảnh hưởng Serial Monitor.

---

## 7. Câu thuyết trình ngắn

“Trong bản demo này, nhóm em chọn ba tình huống dễ kiểm chứng nhất: SOS từ điện thoại, cảnh báo cháy và chống phá hoại cảm biến.  
Để mạch ổn định hơn trên ESP32-CAM AI Thinker, nhóm em không dùng chân AO của LDR vì board không có GPIO33 và WiFi có thể ảnh hưởng đọc analog trên ADC2. Thay vào đó, nhóm em dùng chân DO của LDR để phát hiện trạng thái sáng/tối, đủ cho kịch bản che cảm biến khi demo.”
