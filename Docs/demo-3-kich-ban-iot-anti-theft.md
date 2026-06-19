# Tài liệu Demo 3 Kịch Bản IoT Anti-Theft System  
## Bản cập nhật: Arduino IoT Cloud + SOS điện thoại + Google Apps Script gửi Gmail

> Mục tiêu của bản demo này là trình bày hệ thống **IoT Anti-Theft System** theo 3 kịch bản dễ kiểm chứng nhất, đồng thời bổ sung phần **đồng bộ Arduino IoT Cloud**, **bấm SOS từ điện thoại**, **module tự cảnh báo bằng LED/Buzzer**, và **gửi Gmail qua Google Apps Script** khi có cảnh báo SOS.

---

# 1. Tổng quan demo

Trong bản demo này, nhóm trình bày 3 kịch bản chính:

| STT | Kịch bản | Ý nghĩa chính | Module / Dịch vụ chính |
|---|---|---|---|
| 1 | SOS từ điện thoại + gửi Gmail | Người dùng bấm SOS trên Arduino IoT Cloud, module tự báo động và gửi Gmail | ESP32-CAM, WiFi, Arduino IoT Cloud, Google Apps Script, Gmail, LED, Buzzer |
| 2 | Cảnh báo cháy | Flame Sensor phát hiện lửa, module tự báo động | Flame Sensor, ESP32-CAM, LED, Buzzer |
| 3 | Chống phá hoại thiết bị | LDR bị che kết hợp PIR phát hiện chuyển động, module báo động | LDR, PIR, ESP32-CAM, LED, Buzzer |

---

# 2. Mục tiêu của bản demo

Bản demo cần chứng minh được các ý sau:

1. **Thiết bị được đồng bộ với Arduino IoT Cloud**.  
   ESP32-CAM có thể kết nối WiFi và đồng bộ trạng thái với dashboard trên điện thoại.

2. **Điện thoại có thể điều khiển module từ xa**.  
   Người dùng có thể bấm nút SOS trên Arduino IoT Cloud.

3. **Module có thể tự cảnh báo tại chỗ**.  
   Khi có SOS hoặc cảnh báo cảm biến, ESP32-CAM tự bật LED đỏ và buzzer.

4. **Hệ thống có thể gửi Gmail cảnh báo**.  
   Khi có người bấm SOS trên điện thoại, ESP32-CAM gọi Google Apps Script để gửi Gmail.

5. **Cảm biến vẫn hoạt động độc lập**.  
   Ngoài SOS thủ công, hệ thống vẫn có thể phát hiện cháy và chống phá hoại cảm biến.

---

# 3. Kiến trúc tổng quan

```text
Điện thoại / Arduino IoT Cloud Dashboard
        ↓
Arduino IoT Cloud
        ↓
WiFi
        ↓
ESP32-CAM AI Thinker OV2640
        ↓
LED đỏ + Buzzer
        ↓
Google Apps Script Web App
        ↓
Gmail cảnh báo
```

Giải thích:

- **Arduino IoT Cloud** là nền tảng cloud dùng để đồng bộ dữ liệu giữa điện thoại và ESP32-CAM.
- **Cloud** nghĩa là nền tảng trực tuyến, thiết bị có thể gửi/nhận dữ liệu qua Internet.
- **ESP32-CAM** là vi điều khiển chính, nhận lệnh từ cloud và đọc cảm biến.
- **Google Apps Script** là dịch vụ trung gian để gửi Gmail.
- **Gmail** là nơi nhận thông báo cảnh báo.

---

# 4. Module cần dùng trong bản demo

| Module / Dịch vụ | Có dùng không? | Lý do |
|---|---:|---|
| ESP32-CAM AI Thinker OV2640 | Có | Vi điều khiển chính, kết nối WiFi, xử lý logic |
| Arduino IoT Cloud | Có | Đồng bộ biến giữa điện thoại và ESP32-CAM |
| Điện thoại | Có | Dùng để bấm SOS và theo dõi trạng thái |
| Google Apps Script | Có | Nhận request từ ESP32-CAM và gửi Gmail |
| Gmail | Có | Nhận thông báo cảnh báo SOS |
| Flame Sensor | Có | Demo cảnh báo cháy |
| LDR Light Sensor | Có | Demo phát hiện che cảm biến |
| HC-SR501 | Có | Demo phát hiện chuyển động |
| LED đỏ | Có | Hiển thị cảnh báo bằng ánh sáng |
| Buzzer 1407 | Có | Phát còi cảnh báo |
| RC522 | Không | Không cần cho 3 demo này |
| DS1307 | Không bắt buộc | Demo này có thể bỏ để giảm dây |
| HY-SRF05 / HC-SR04 | Không | Không cần cho 3 demo này |

---

# 5. Bảng nối dây

## 5.1 Nguồn chính

| Nguồn | ESP32-CAM |
|---|---|
| 5V | 5V |
| GND | GND |

> Tất cả module phải nối chung GND.

**GND chung** nghĩa là chân âm của tất cả module phải được nối chung với GND của ESP32-CAM. Nếu không nối chung GND, tín hiệu cảm biến có thể đọc sai hoặc không hoạt động.

---

## 5.2 Flame Sensor

| Flame Sensor | ESP32-CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DO | GPIO15 |

> `DO` là **Digital Output**, nghĩa là ngõ ra số, chỉ có 2 trạng thái: HIGH hoặc LOW.

---

## 5.3 LDR Light Sensor

| LDR | ESP32-CAM |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DO | GPIO14 |
| AO | Không nối |

> Không dùng `AO -> GPIO33` vì ESP32-CAM AI Thinker không có GPIO33 đưa ra ngoài.  
> Với demo, dùng `DO` là đủ. Chỉnh biến trở xanh trên module LDR để chọn ngưỡng sáng/tối.

**AO** là **Analog Output**, nghĩa là ngõ ra tương tự, có thể trả về giá trị thay đổi liên tục theo ánh sáng.  
**DO** là **Digital Output**, nghĩa là ngõ ra số, chỉ báo sáng/tối theo ngưỡng đã chỉnh.

---

## 5.4 PIR 5050

| PIR | ESP32-CAM |
|---|---|
| VCC | 5V |
| GND | GND |
| OUT | GPIO13 |

> Trước khi nối OUT vào ESP32-CAM, nên đo điện áp OUT so với GND. Nếu OUT gần 5V thì cần chia áp xuống 3.3V.

**PIR** là cảm biến chuyển động hồng ngoại, dùng để phát hiện người hoặc vật thể chuyển động trong vùng quan sát.

---

## 5.5 LED đỏ

| LED đỏ | ESP32-CAM |
|---|---|
| Anode (+) | GPIO2 qua điện trở 220Ω hoặc 330Ω |
| Cathode (-) | GND |

> LED bắt buộc có điện trở để tránh cháy LED hoặc làm hỏng chân GPIO.

---

## 5.6 Buzzer 1407

### Cách an toàn nhất: dùng transistor

| Buzzer / mạch đệm | ESP32-CAM |
|---|---|
| Chân điều khiển transistor | GPIO16 |
| Buzzer + | 5V |
| Buzzer - | Qua transistor xuống GND |

### Cách test nhanh nếu chưa có transistor

| Buzzer | ESP32-CAM |
|---|---|
| + | GPIO16 |
| - | GND |

> Không khuyến khích nối trực tiếp buzzer lâu dài vì chân GPIO của ESP32 chịu dòng nhỏ.

---

# 6. Pin mapping dùng trong code

```cpp
#define PIN_FLAME   15
#define PIN_LDR     14
#define PIN_PIR     13
#define PIN_LED     2
#define PIN_BUZZER  16
```

| Tên trong code | Chân ESP32-CAM | Tác dụng |
|---|---:|---|
| `PIN_FLAME` | GPIO15 | Đọc Flame Sensor |
| `PIN_LDR` | GPIO14 | Đọc LDR DO |
| `PIN_PIR` | GPIO13 | Đọc PIR OUT |
| `PIN_LED` | GPIO2 | Điều khiển LED đỏ |
| `PIN_BUZZER` | GPIO16 | Điều khiển buzzer |

---

# 7. Biến cần tạo trong Arduino IoT Cloud

Các biến dưới đây nên tạo trong **Thing** của Arduino IoT Cloud.

## 7.1 Nhóm biến điều khiển từ điện thoại

| Tên biến | Kiểu dữ liệu | Quyền | Cập nhật | Tác dụng |
|---|---|---|---|---|
| `sosButton` | Boolean | Read & Write | On Change | Nút SOS trên điện thoại |
| `resetAlarm` | Boolean | Read & Write | On Change | Nút reset/tắt cảnh báo từ điện thoại |

Giải thích:

- `sosButton` cần **Read & Write** vì điện thoại phải ghi giá trị xuống ESP32-CAM.
- `resetAlarm` cũng cần **Read & Write** vì người dùng cần bấm nút để tắt cảnh báo.
- **On Change** nghĩa là chỉ cập nhật khi giá trị thay đổi, phù hợp với nút bấm.

---

## 7.2 Nhóm biến trạng thái cảnh báo

| Tên biến | Kiểu dữ liệu | Quyền | Cập nhật | Tác dụng |
|---|---|---|---|---|
| `alarmStatus` | Boolean | Read Only | On Change | Cho biết hệ thống đang báo động hay không |
| `alertType` | String | Read Only | On Change | Loại cảnh báo hiện tại: SOS, FIRE, SABOTAGE |
| `lastAlertMessage` | String | Read Only | On Change | Nội dung cảnh báo gần nhất |
| `gmailSent` | Boolean | Read Only | On Change | Cho biết Gmail đã gửi thành công hay chưa |

Giải thích:

- Các biến này nên để **Read Only** vì đây là trạng thái do thiết bị tạo ra.
- Người dùng chỉ nên xem, không nên tự sửa thủ công trên dashboard.
- `alertType` giúp khi demo biết cảnh báo đến từ SOS, cháy hay chống phá hoại.

---

## 7.3 Nhóm biến cảm biến

| Tên biến | Kiểu dữ liệu | Quyền | Cập nhật | Tác dụng |
|---|---|---|---|---|
| `flameDetected` | Boolean | Read Only | On Change | Cho biết có phát hiện lửa hay không |
| `ldrBlocked` | Boolean | Read Only | On Change | Cho biết LDR có bị che hay không |
| `pirDetected` | Boolean | Read Only | On Change | Cho biết PIR có phát hiện chuyển động hay không |

Giải thích:

- Các biến cảm biến nên để **Read Only** vì chúng phản ánh trạng thái thật từ phần cứng.
- **On Change** phù hợp vì các trạng thái này chỉ cần báo khi thay đổi.

---

# 8. Kịch bản demo 1: SOS từ điện thoại + module tự cảnh báo + gửi Gmail

## 8.1 Mục tiêu

Cho phép người dùng bấm nút SOS trên điện thoại thông qua Arduino IoT Cloud. Khi SOS được kích hoạt, ESP32-CAM tự bật LED đỏ, buzzer và gọi Google Apps Script để gửi Gmail cảnh báo.

---

## 8.2 Luồng hoạt động

```text
Người dùng mở Arduino IoT Cloud Dashboard trên điện thoại
        ↓
Người dùng bấm nút SOS
        ↓
Biến sosButton trên Cloud đổi thành true
        ↓
ESP32-CAM nhận giá trị sosButton = true
        ↓
ESP32-CAM bật LED đỏ và buzzer
        ↓
ESP32-CAM cập nhật alarmStatus = true
        ↓
ESP32-CAM cập nhật alertType = "SOS"
        ↓
ESP32-CAM gọi Google Apps Script Web App
        ↓
Google Apps Script gửi Gmail cảnh báo
        ↓
Người dùng bấm resetAlarm để tắt cảnh báo
```

---

## 8.3 Điều kiện kích hoạt

```text
Nếu sosButton == true
→ Kích hoạt cảnh báo SOS
```

---

## 8.4 Hành động của module

Khi có SOS:

1. LED đỏ nháy.
2. Buzzer kêu.
3. `alarmStatus = true`.
4. `alertType = "SOS"`.
5. `lastAlertMessage = "SOS button pressed from phone"`.
6. ESP32-CAM gọi Google Apps Script để gửi Gmail.
7. Nếu gửi Gmail thành công, `gmailSent = true`.

---

## 8.5 Nội dung Gmail đề xuất

**Subject:**

```text
[SOS ALERT] IoT Anti-Theft System
```

**Body:**

```text
Emergency alert from IoT Anti-Theft System.

Alert type: SOS
Device: ESP32-CAM Kitchen Security Module
Status: Someone pressed the SOS button from Arduino IoT Cloud.

Please check the situation immediately.
```

---

## 8.6 Cách trình bày khi demo

Người thuyết trình có thể nói:

> “Ở kịch bản đầu tiên, nhóm em demo chức năng SOS từ điện thoại. ESP32-CAM đã được đồng bộ với Arduino IoT Cloud. Khi người dùng bấm nút SOS trên dashboard, biến `sosButton` trên Cloud thay đổi. ESP32-CAM nhận lệnh này, tự bật LED đỏ và buzzer để cảnh báo tại chỗ. Đồng thời, ESP32-CAM gọi Google Apps Script để gửi Gmail cảnh báo cho người nhận.”

---

## 8.7 Điểm cần nhấn mạnh

Kịch bản này có đủ 3 phần quan trọng của một hệ thống IoT:

```text
Cloud control + Local alarm + Email notification
```

Giải thích:

- **Cloud control**: điều khiển từ điện thoại qua Arduino IoT Cloud.
- **Local alarm**: thiết bị tự hú còi và nháy LED tại chỗ.
- **Email notification**: gửi Gmail để thông báo cho người khác.

---

# 9. Kịch bản demo 2: Cảnh báo cháy

## 9.1 Mục tiêu

Dùng Flame Sensor để phát hiện lửa hoặc nguồn hồng ngoại. Khi phát hiện cháy, ESP32-CAM kích hoạt cảnh báo bằng LED đỏ và buzzer.

---

## 9.2 Luồng hoạt động

```text
Người trình diễn đưa nguồn lửa nhỏ hoặc nguồn hồng ngoại lại gần Flame Sensor
        ↓
Flame Sensor đổi trạng thái DO
        ↓
ESP32-CAM đọc tín hiệu từ GPIO15
        ↓
ESP32-CAM xác nhận có cháy
        ↓
LED đỏ nháy
        ↓
Buzzer kêu
        ↓
Cập nhật flameDetected = true
        ↓
Cập nhật alarmStatus = true
        ↓
Cập nhật alertType = "FIRE"
```

---

## 9.3 Điều kiện kích hoạt

```text
Nếu Flame Sensor phát hiện lửa
→ Kích hoạt cảnh báo cháy
```

Lưu ý: Tùy module, Flame Sensor có thể báo ngược logic.

Ví dụ:

```text
Có lửa = LOW
Không có lửa = HIGH
```

hoặc:

```text
Có lửa = HIGH
Không có lửa = LOW
```

Vì vậy cần test trước bằng Serial Monitor hoặc Arduino Cloud để biết module của nhóm đang dùng logic nào.

---

## 9.4 Hành động của module

Khi phát hiện cháy:

1. LED đỏ nháy.
2. Buzzer kêu.
3. `flameDetected = true`.
4. `alarmStatus = true`.
5. `alertType = "FIRE"`.
6. `lastAlertMessage = "Fire detected by flame sensor"`.

---

## 9.5 Cách trình bày khi demo

Người thuyết trình có thể nói:

> “Ở kịch bản thứ hai, nhóm em demo cảnh báo cháy. Khi Flame Sensor phát hiện lửa hoặc nguồn hồng ngoại, ESP32-CAM sẽ đọc tín hiệu từ cảm biến, sau đó kích hoạt LED và buzzer để cảnh báo tại chỗ. Đồng thời trạng thái `flameDetected` và `alertType` được cập nhật lên Arduino IoT Cloud.”

---

## 9.6 Lưu ý an toàn

Không dí lửa sát:

- Breadboard.
- Dây điện.
- ESP32-CAM.
- Module cảm biến.
- Vật dễ cháy.

Nên đưa lửa ở khoảng cách vừa đủ để cảm biến nhận tín hiệu. Nếu có thể, dùng remote hồng ngoại để test an toàn hơn.

---

# 10. Kịch bản demo 3: Chống phá hoại thiết bị bằng LDR + PIR

## 10.1 Mục tiêu

Phát hiện trường hợp có người cố tình che cảm biến hoặc phá hoại thiết bị.

Hệ thống không chỉ dựa vào LDR, mà kết hợp thêm PIR để giảm báo động giả.

---

## 10.2 Luồng hoạt động

```text
Ban đầu LDR ở môi trường sáng
        ↓
Người trình diễn dùng tay hoặc khăn che LDR
        ↓
LDR DO đổi trạng thái
        ↓
ESP32-CAM cập nhật ldrBlocked = true
        ↓
Người trình diễn quơ tay trước PIR
        ↓
PIR phát hiện chuyển động
        ↓
ESP32-CAM cập nhật pirDetected = true
        ↓
ESP32-CAM xác nhận có hành vi che cảm biến kèm chuyển động
        ↓
LED đỏ nháy
        ↓
Buzzer kêu
        ↓
Cập nhật alarmStatus = true
        ↓
Cập nhật alertType = "SABOTAGE"
```

---

## 10.3 Điều kiện kích hoạt

```text
Nếu LDR bị che và PIR phát hiện chuyển động
→ Kích hoạt cảnh báo chống phá hoại
```

Viết theo logic đơn giản:

```cpp
if (ldrBlocked == true && pirDetected == true) {
    triggerSabotageAlarm();
}
```

---

## 10.4 Vì sao phải dùng cả LDR và PIR?

Nếu chỉ dùng LDR, hệ thống dễ báo nhầm khi:

- Trời tối.
- Tắt đèn.
- Có bóng người đi ngang.
- Ánh sáng môi trường thay đổi.
- Người trình diễn vô tình che cảm biến.

Vì vậy cần thêm PIR để xác nhận có chuyển động gần thiết bị.

Logic của nhóm là:

```text
LDR bị che + PIR có chuyển động = nghi ngờ có người cố tình phá hoại
```

Đây là dạng **sensor fusion**.

**Sensor fusion** nghĩa là kết hợp nhiều cảm biến để ra quyết định chính xác hơn. Thay vì chỉ tin một cảm biến, hệ thống kiểm tra nhiều nguồn tín hiệu cùng lúc để giảm báo động sai.

---

## 10.5 Hành động của module

Khi phát hiện chống phá hoại:

1. LED đỏ nháy.
2. Buzzer kêu.
3. `ldrBlocked = true`.
4. `pirDetected = true`.
5. `alarmStatus = true`.
6. `alertType = "SABOTAGE"`.
7. `lastAlertMessage = "LDR blocked and motion detected"`.

---

## 10.6 Cách trình bày khi demo

Người thuyết trình có thể nói:

> “Ở kịch bản thứ ba, nhóm em demo chức năng chống phá hoại thiết bị. Nếu chỉ dùng LDR thì hệ thống dễ báo nhầm khi ánh sáng thay đổi. Vì vậy nhóm em kết hợp LDR với PIR. Khi LDR bị che và PIR đồng thời phát hiện chuyển động, ESP32-CAM xác định đây là hành vi đáng ngờ và kích hoạt cảnh báo.”

---

# 11. Thứ tự demo đề xuất trên lớp

Nên demo theo thứ tự sau để dễ hiểu và ít lỗi nhất:

## Bước 1: Kiểm tra Arduino IoT Cloud

Mở dashboard trên điện thoại hoặc laptop.

Kiểm tra:

- Device đang online.
- Có nút `sosButton`.
- Có nút `resetAlarm`.
- Có trạng thái `alarmStatus`.
- Có trạng thái `alertType`.
- Có trạng thái `flameDetected`, `ldrBlocked`, `pirDetected`.
- Có trạng thái `gmailSent`.

Câu nói gợi ý:

> “Trước tiên, nhóm em cho thấy ESP32-CAM đã được đồng bộ với Arduino IoT Cloud. Các biến trên dashboard sẽ thay đổi theo trạng thái thật của thiết bị.”

---

## Bước 2: Demo SOS từ điện thoại

1. Bấm nút SOS trên điện thoại.
2. Quan sát LED đỏ và buzzer.
3. Kiểm tra `alarmStatus = true`.
4. Kiểm tra `alertType = SOS`.
5. Mở Gmail để kiểm tra email đã gửi.
6. Bấm `resetAlarm` để tắt cảnh báo.

Câu nói gợi ý:

> “Khi em bấm SOS trên điện thoại, ESP32-CAM nhận lệnh qua Cloud, tự cảnh báo tại chỗ bằng LED và buzzer, đồng thời gửi Gmail thông báo qua Google Apps Script.”

---

## Bước 3: Demo cảnh báo cháy

1. Đưa nguồn lửa nhỏ hoặc nguồn hồng ngoại gần Flame Sensor.
2. Quan sát LED đỏ và buzzer.
3. Kiểm tra `flameDetected = true`.
4. Kiểm tra `alertType = FIRE`.
5. Bấm reset nếu cần.

Câu nói gợi ý:

> “Tiếp theo là cảnh báo cháy. Khi Flame Sensor phát hiện lửa, hệ thống tự kích hoạt cảnh báo và cập nhật trạng thái lên Cloud.”

---

## Bước 4: Demo chống phá hoại

1. Che LDR bằng tay hoặc khăn.
2. Quơ tay trước PIR.
3. Quan sát LED đỏ và buzzer.
4. Kiểm tra `ldrBlocked = true`.
5. Kiểm tra `pirDetected = true`.
6. Kiểm tra `alertType = SABOTAGE`.
7. Bấm reset nếu cần.

Câu nói gợi ý:

> “Cuối cùng là chống phá hoại. Khi cảm biến ánh sáng bị che và có chuyển động gần thiết bị, hệ thống xác nhận có hành vi đáng ngờ và kích hoạt báo động.”

---

# 12. Checklist trước khi cấp nguồn

- [ ] Chưa nối RC522, DS1307, HY-SRF05 nếu không demo các module đó.
- [ ] ESP32-CAM nhận 5V đúng chân 5V.
- [ ] Tất cả module nối chung GND.
- [ ] Flame Sensor cấp 3.3V.
- [ ] LDR cấp 3.3V.
- [ ] LDR dùng DO, không dùng AO.
- [ ] LED đỏ có điện trở 220Ω hoặc 330Ω.
- [ ] PIR OUT đã được kiểm tra điện áp, không vượt quá 3.3V nếu nối thẳng vào ESP32-CAM.
- [ ] Buzzer không nóng bất thường.
- [ ] Không dùng GPIO0 cho module demo.
- [ ] Không dùng GPIO1/GPIO3 để tránh ảnh hưởng Serial Monitor.
- [ ] WiFi đã nhập đúng SSID và password.
- [ ] Arduino IoT Cloud device đang online.
- [ ] Google Apps Script Web App URL đã đúng.
- [ ] Gmail người nhận đã đúng.
- [ ] Đã test gửi Gmail trước khi demo.
- [ ] Đã test nút resetAlarm để tắt còi khi cần.

---

# 13. Rủi ro thường gặp và cách xử lý

## 13.1 Buzzer không kêu

Nguyên nhân có thể:

- Buzzer là loại passive, cần phát xung.
- GPIO16 không đủ dòng.
- Cắm sai cực buzzer.
- Buzzer cần transistor.
- Dây GND chưa nối chung.

Cách xử lý:

- Test buzzer riêng bằng code đơn giản.
- Nếu buzzer passive, dùng `tone()` hoặc PWM.
- Nếu buzzer active, chỉ cần HIGH/LOW.
- Nếu vẫn yếu hoặc không kêu, dùng transistor để cấp dòng riêng cho buzzer.

**PWM** là Pulse Width Modulation, nghĩa là điều chế độ rộng xung. Hiểu đơn giản là tạo tín hiệu bật/tắt rất nhanh để điều khiển thiết bị như buzzer hoặc LED.

---

## 13.2 LDR không đổi trạng thái

Nguyên nhân có thể:

- Chưa chỉnh biến trở xanh.
- Che chưa đủ kín.
- Đang đọc ngược HIGH/LOW.
- Cấp sai nguồn.
- Dây DO cắm sai chân.

Cách xử lý:

- Chỉnh biến trở xanh trên module.
- In giá trị LDR ra Serial Monitor.
- Hiển thị `ldrBlocked` lên Arduino Cloud để kiểm tra.
- Test trong môi trường sáng rõ rồi che kín bằng tay.

---

## 13.3 PIR luôn báo 1 hoặc báo lung tung

Nguyên nhân có thể:

- PIR cần thời gian ổn định sau khi cấp nguồn.
- Người đứng quá gần cảm biến.
- Dây OUT nhiễu.
- OUT của PIR có mức 5V gây không ổn định.

Cách xử lý:

- Chờ 30-60 giây sau khi cấp nguồn rồi mới demo.
- Đứng yên vài giây trước khi test.
- Kiểm tra điện áp OUT.
- Nếu OUT gần 5V, dùng chia áp xuống 3.3V.

---

## 13.4 Flame Sensor báo ngược

Nguyên nhân:

- Một số module có logic ngược.
- Biến trở trên module chỉnh ngưỡng khác nhau.

Cách xử lý:

- Test trước xem có lửa là HIGH hay LOW.
- Đổi điều kiện trong code nếu cần.
- Hiển thị `flameDetected` lên Cloud để dễ quan sát.

---

## 13.5 Arduino Cloud không cập nhật

Nguyên nhân có thể:

- WiFi sai tên hoặc mật khẩu.
- Device Secret sai.
- Board chưa online.
- Code chưa gọi hàm cập nhật cloud.
- Mạng yếu.

Cách xử lý:

- Kiểm tra Serial Monitor.
- Kiểm tra trạng thái online trong Arduino IoT Cloud.
- Kiểm tra lại WiFi SSID, password và Device Secret.
- Test bằng biến đơn giản trước như bật/tắt LED.

---

## 13.6 Không gửi được Gmail

Nguyên nhân có thể:

- Google Apps Script chưa deploy đúng dạng Web App.
- Web App URL sai.
- Chưa cấp quyền gửi Gmail.
- ESP32-CAM chưa gọi được URL.
- Mạng WiFi không có Internet.
- Google Apps Script bị lỗi tham số.

Cách xử lý:

- Mở Web App URL trên trình duyệt để test.
- Kiểm tra quyền Gmail trong Google Apps Script.
- In HTTP response code ra Serial Monitor.
- Kiểm tra Gmail người nhận.
- Kiểm tra thư mục Spam.

**HTTP response code** là mã phản hồi khi thiết bị gọi một đường link. Ví dụ `200` thường nghĩa là gọi thành công.

---

# 14. Câu thuyết trình ngắn

Có thể dùng đoạn này khi trình bày:

> “Trong bản demo này, nhóm em chọn ba tình huống dễ kiểm chứng nhất: SOS từ điện thoại, cảnh báo cháy và chống phá hoại cảm biến. ESP32-CAM được đồng bộ với Arduino IoT Cloud, nên người dùng có thể theo dõi trạng thái và bấm SOS từ điện thoại. Khi SOS được kích hoạt, thiết bị tự bật LED đỏ và buzzer để cảnh báo tại chỗ, đồng thời gọi Google Apps Script để gửi Gmail thông báo. Ngoài SOS thủ công, hệ thống còn có thể tự phát hiện cháy bằng Flame Sensor và phát hiện hành vi che cảm biến bằng cách kết hợp LDR với PIR.”

---

# 15. Câu trả lời khi thầy hỏi

## 15.1 Vì sao dùng Arduino IoT Cloud?

> “Dạ vì Arduino IoT Cloud giúp nhóm em đồng bộ dữ liệu giữa ESP32-CAM và điện thoại. Nhờ đó người dùng có thể xem trạng thái cảm biến, trạng thái cảnh báo và bấm SOS từ xa.”

---

## 15.2 Vì sao bấm SOS rồi module vẫn phải tự hú còi?

> “Dạ vì Gmail hoặc Internet có thể bị trễ, nên thiết bị vẫn cần cảnh báo tại chỗ bằng LED và buzzer. Như vậy người ở gần có thể nhận biết nguy hiểm ngay lập tức.”

---

## 15.3 Vì sao dùng Google Apps Script để gửi Gmail?

> “Dạ vì ESP32-CAM không phù hợp để đăng nhập Gmail trực tiếp. Google Apps Script đóng vai trò trung gian: ESP32-CAM chỉ cần gọi một Web App URL, còn Apps Script sẽ gửi Gmail giúp hệ thống.”

---

## 15.4 Vì sao không dùng AO của LDR?

> “Dạ vì ESP32-CAM AI Thinker không đưa GPIO33 ra ngoài nên nhóm em không nối được chân AO của LDR vào GPIO33. Với demo này nhóm em chỉ cần phát hiện sáng hoặc bị che, nên dùng DO là đủ.”

---

## 15.5 Vì sao chống phá hoại phải dùng cả LDR và PIR?

> “Dạ nếu chỉ dùng LDR thì dễ báo nhầm khi môi trường tối hoặc ánh sáng thay đổi. Vì vậy nhóm em dùng thêm PIR để xác nhận có chuyển động gần thiết bị. Khi vừa có LDR bị che vừa có PIR phát hiện chuyển động, hệ thống mới xác định là hành vi đáng ngờ.”

---

# 16. Kết luận

Sau khi cập nhật, 3 kịch bản demo của nhóm gồm:

1. **SOS từ điện thoại + module tự cảnh báo + gửi Gmail**  
   Người dùng bấm SOS trên Arduino IoT Cloud, ESP32-CAM tự bật LED/Buzzer và gọi Google Apps Script để gửi Gmail.

2. **Cảnh báo cháy**  
   Flame Sensor phát hiện lửa, ESP32-CAM bật LED/Buzzer và cập nhật trạng thái lên Cloud.

3. **Chống phá hoại thiết bị**  
   LDR phát hiện bị che, PIR xác nhận có chuyển động, ESP32-CAM bật LED/Buzzer và cập nhật trạng thái lên Cloud.

Bản demo này thể hiện rõ tính chất IoT vì có đủ:

```text
Thiết bị thật + Cloud + Điện thoại + Cảm biến + Cảnh báo tại chỗ + Gửi Gmail
```
