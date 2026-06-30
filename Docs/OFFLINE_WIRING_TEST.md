# OFFLINE_WIRING_TEST.md

# Hướng dẫn test offline toàn bộ wiring ESP32-CAM

> Mục đích file: giúp test phần cắm dây phần cứng trước khi làm WiFi, Arduino Cloud, Telegram hoặc Google Apps Script.  
> File này chỉ kiểm tra board có chạy được không, cảm biến có đọc được không, LED/còi có phản hồi không.

---

## 1. Phạm vi test

File này dùng để test các thiết bị đang cắm với ESP32-CAM:

| Thiết bị | Chân ESP32-CAM |
|---|---|
| PIR HC-SR501 | `GPIO16` |
| HY-SRF05 `Trig` | `GPIO13` |
| HY-SRF05 `Echo` | qua mạch chia áp rồi vào `GPIO12` |
| LDR DO | `GPIO2` |
| RTC DS1307 SDA | `GPIO14` |
| RTC DS1307 SCL | `GPIO15` |
| LED đỏ | `GPIO4` |
| LED xanh | `GPIO1` |
| Buzzer | `GPIO3` |

---

## 2. Cảnh báo quan trọng trước khi bật nguồn

### 2.1 Echo HY-SRF05 không được cắm thẳng vào ESP32-CAM

Không cắm:

```text
HY-SRF05 Echo -> GPIO12 ESP32-CAM
```

Phải cắm qua mạch chia áp.

Nếu dùng cách không có điện trở 2.2kΩ:

```text
R1 = 1kΩ
R2 = 5 điện trở 330Ω nối tiếp = 1650Ω
```

Sơ đồ:

```text
HY-SRF05 Echo
     |
    1kΩ
     |
     +-----------------> GPIO12 ESP32-CAM
     |
   330Ω
     |
   330Ω
     |
   330Ω
     |
   330Ω
     |
   330Ω
     |
    GND
```

Điểm giữa sau điện trở `1kΩ` gọi là `ECHO_SAFE`.

---

### 2.2 Tất cả GND phải nối chung

Bắt buộc:

```text
GND ESP32-CAM
GND PIR
GND HY-SRF05
GND LDR
GND RTC
GND LED
GND buzzer
GND nguồn ngoài

=> nối chung với nhau
```

Nếu không nối chung GND, cảm biến có thể đọc sai hoặc không hoạt động.

---

### 2.3 Không dùng GPIO0 khi chạy bình thường

`GPIO0` chỉ dùng khi upload code.

Khi upload:

```text
GPIO0 -> GND
```

Sau khi upload xong:

```text
Tháo GPIO0 khỏi GND
Reset lại ESP32-CAM
```

---

### 2.4 GPIO1 và GPIO3 có thể ảnh hưởng upload và Serial

Trong wiring hiện tại:

```text
GPIO1 -> LED xanh
GPIO3 -> buzzer
```

Nhưng `GPIO1` và `GPIO3` cũng là chân UART.

**UART** là giao tiếp nạp code và xem log Serial.

Nếu upload lỗi, hãy tháo tạm:

```text
LED xanh khỏi GPIO1
Buzzer khỏi GPIO3
```

Upload xong rồi cắm lại.

---

## 3. Thứ tự test khuyến nghị

Nên test theo 2 bước.

| Bước | Mục tiêu | Có dùng Serial Monitor không? |
|---|---|---|
| Test 1 | Test cảm biến: PIR, LDR, HY-SRF05, RTC, LED đỏ | Có |
| Test 2 | Test full output: LED đỏ, LED xanh, buzzer | Không bắt buộc |
| Test 3 | Test full logic offline: cảm biến điều khiển LED/còi | Có thể có, nhưng nhìn LED/còi là chính |

Lý do tách bước: `GPIO1` và `GPIO3` đang dùng cho LED xanh/buzzer, có thể ảnh hưởng Serial.

---

# 4. Test 1 - Test cảm biến offline bằng Serial Monitor

## 4.1 Trước khi upload Test 1

Tạm tháo:

```text
LED xanh khỏi GPIO1
Buzzer khỏi GPIO3
```

Giữ nguyên:

```text
PIR
HY-SRF05
LDR
RTC
LED đỏ
Nguồn
GND chung
```

---

## 4.2 Code Test 1

Tạo file Arduino mới, ví dụ:

```text
offline_sensor_test.ino
```

Dán code sau:

```cpp
#include <Wire.h>

#define PIN_PIR_OUT             16

#define PIN_ULTRASONIC_TRIG     13
#define PIN_ULTRASONIC_ECHO     12

#define PIN_LDR_DO              2

#define PIN_RTC_SDA             14
#define PIN_RTC_SCL             15

#define PIN_LED_RED             4

float readUltrasonicDistanceCm() {
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

bool checkI2CDevice(byte address) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  return error == 0;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(PIN_PIR_OUT, INPUT);
  pinMode(PIN_LDR_DO, INPUT);

  pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);

  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);

  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);

  Serial.println();
  Serial.println("=== ESP32-CAM OFFLINE SENSOR TEST ===");
  Serial.println("No WiFi, no Cloud, no Telegram.");
  Serial.println("Testing: PIR, LDR, HY-SRF05, RTC, LED red.");
  Serial.println("GPIO1/GPIO3 should be disconnected in this test.");
}

void loop() {
  int pirValue = digitalRead(PIN_PIR_OUT);
  int ldrValue = digitalRead(PIN_LDR_DO);
  float distanceCm = readUltrasonicDistanceCm();

  bool rtcFound = checkI2CDevice(0x68);

  bool objectNear = distanceCm > 0 && distanceCm < 20;

  if (pirValue == HIGH || objectNear) {
    digitalWrite(PIN_LED_RED, HIGH);
  } else {
    digitalWrite(PIN_LED_RED, LOW);
  }

  Serial.println("------------------------------");

  Serial.print("PIR GPIO16: ");
  Serial.println(pirValue == HIGH ? "MOTION DETECTED" : "NO MOTION");

  Serial.print("LDR GPIO2: ");
  Serial.println(ldrValue == HIGH ? "HIGH" : "LOW");

  Serial.print("HY-SRF05 distance: ");
  if (distanceCm < 0) {
    Serial.println("NO ECHO / TIMEOUT");
  } else {
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  Serial.print("RTC DS1307 I2C 0x68: ");
  Serial.println(rtcFound ? "FOUND" : "NOT FOUND");

  delay(1000);
}
```

---

## 4.3 Cách upload Test 1 cho ESP32-CAM

Trước khi upload:

```text
GPIO0 -> GND
GND USB/FTDI -> GND ESP32-CAM
TX USB/FTDI -> GPIO3 / U0R ESP32-CAM
RX USB/FTDI -> GPIO1 / U0T ESP32-CAM
5V nguồn ngoài -> 5V ESP32-CAM
GND nguồn ngoài -> GND ESP32-CAM
```

Nếu dùng nguồn ngoài riêng, không nên cấp 5V từ hai nguồn khác nhau cùng lúc.

Sau khi upload xong:

```text
Tháo GPIO0 khỏi GND
Reset ESP32-CAM
Mở Serial Monitor 115200
```

---

## 4.4 Kết quả mong đợi Test 1

Serial Monitor phải hiện dạng:

```text
=== ESP32-CAM OFFLINE SENSOR TEST ===
No WiFi, no Cloud, no Telegram.
Testing: PIR, LDR, HY-SRF05, RTC, LED red.
GPIO1/GPIO3 should be disconnected in this test.
------------------------------
PIR GPIO16: NO MOTION
LDR GPIO2: HIGH
HY-SRF05 distance: 35.42 cm
RTC DS1307 I2C 0x68: FOUND
```

Ý nghĩa:

| Kết quả | Ý nghĩa |
|---|---|
| `PIR GPIO16: MOTION DETECTED` khi có người đi qua | PIR hoạt động |
| `PIR GPIO16: NO MOTION` khi không có chuyển động | PIR hoạt động |
| `LDR GPIO2: HIGH/LOW` thay đổi khi che hoặc chiếu sáng | LDR hoạt động |
| `HY-SRF05 distance: số cm` | Siêu âm hoạt động |
| `HY-SRF05 distance: NO ECHO / TIMEOUT` | Siêu âm chưa đọc được |
| `RTC DS1307 I2C 0x68: FOUND` | RTC phản hồi |
| `RTC DS1307 I2C 0x68: NOT FOUND` | RTC chưa phản hồi |

---

## 4.5 Nếu HY-SRF05 báo `NO ECHO / TIMEOUT`

Kiểm tra:

```text
[ ] VCC HY-SRF05 đã vào 5V chưa?
[ ] GND HY-SRF05 đã nối chung GND ESP32-CAM chưa?
[ ] Trig đã vào GPIO13 chưa?
[ ] Echo đã qua mạch chia áp chưa?
[ ] GPIO12 có nối đúng vào điểm ECHO_SAFE chưa?
[ ] OUT của HY-SRF05 có đang bỏ trống không?
[ ] Có vật cản trước cảm biến trong khoảng 5 cm đến 100 cm không?
```

---

## 4.6 Nếu RTC báo `NOT FOUND`

Kiểm tra:

```text
[ ] RTC VCC đã vào 3.3V chưa?
[ ] RTC GND đã nối chung chưa?
[ ] SDA đã vào GPIO14 chưa?
[ ] SCL đã vào GPIO15 chưa?
[ ] Có cắm nhầm SDA/SCL không?
```

Ghi chú: DS1307 thường chạy ổn hơn ở 5V, nhưng nếu cấp 5V thì SDA/SCL có thể kéo lên 5V và không an toàn cho ESP32 nếu không có level shifter. Vì vậy bản test này vẫn giữ RTC ở 3.3V theo wiring hiện tại.

---

# 5. Test 2 - Test full LED xanh, LED đỏ và buzzer

## 5.1 Trước khi upload Test 2

Nếu upload bị lỗi, tháo tạm:

```text
LED xanh khỏi GPIO1
Buzzer khỏi GPIO3
```

Upload xong thì cắm lại.

---

## 5.2 Code Test 2

Tạo file Arduino mới:

```text
offline_output_test.ino
```

Dán code sau:

```cpp
#define PIN_LED_RED             4
#define PIN_LED_GREEN           1
#define PIN_BUZZER              3

void beepOnce() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(150);
  digitalWrite(PIN_BUZZER, LOW);
}

void setup() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_GREEN, LOW);
  delay(500);

  digitalWrite(PIN_LED_RED, HIGH);
  beepOnce();
  delay(1000);
  digitalWrite(PIN_LED_RED, LOW);
  delay(1000);
}
```

---

## 5.3 Kết quả mong đợi Test 2

Nếu đúng:

```text
LED xanh sáng 1 giây
LED xanh tắt
LED đỏ sáng
Buzzer kêu bíp ngắn
LED đỏ tắt
Lặp lại
```

Nếu LED không sáng:

```text
[ ] Kiểm tra chân dài LED có qua điện trở rồi vào GPIO không?
[ ] Chân ngắn LED đã về GND chưa?
[ ] Điện trở có phải 220Ω hoặc 330Ω không?
[ ] Có cắm ngược cực LED không?
```

Nếu buzzer không kêu:

```text
[ ] Chân + buzzer đã vào GPIO3 chưa?
[ ] Chân - buzzer đã vào GND chưa?
[ ] Buzzer có phải loại active buzzer không?
[ ] Nếu buzzer cần dòng lớn, nên dùng transistor thay vì kéo trực tiếp từ GPIO.
```

---

# 6. Test 3 - Test full logic offline không cần WiFi

## 6.1 Mục tiêu

Test này chạy logic gần giống demo offline:

| Điều kiện | Phản ứng |
|---|---|
| PIR phát hiện chuyển động | Cộng nghi ngờ |
| Vật gần HY-SRF05 dưới 20 cm | Cộng nghi ngờ |
| LDR đổi HIGH/LOW | In trạng thái debug |
| Nếu PIR hoặc vật gần | LED đỏ bật, buzzer kêu ngắn |
| Nếu bình thường | LED xanh bật |

Ghi chú: Test này dùng `GPIO1` và `GPIO3`, nên Serial Monitor có thể bị ảnh hưởng. Nếu Serial log bị lỗi, chỉ cần nhìn LED/còi.

---

## 6.2 Code Test 3

Tạo file Arduino mới:

```text
offline_full_logic_test.ino
```

Dán code sau:

```cpp
#include <Wire.h>

#define PIN_PIR_OUT             16

#define PIN_ULTRASONIC_TRIG     13
#define PIN_ULTRASONIC_ECHO     12

#define PIN_LDR_DO              2

#define PIN_RTC_SDA             14
#define PIN_RTC_SCL             15

#define PIN_LED_RED             4
#define PIN_LED_GREEN           1
#define PIN_BUZZER              3

unsigned long lastBeepTime = 0;
const unsigned long BEEP_COOLDOWN_MS = 2000;

float readUltrasonicDistanceCm() {
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

bool checkI2CDevice(byte address) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  return error == 0;
}

void beepShortWithCooldown() {
  unsigned long now = millis();

  if (now - lastBeepTime < BEEP_COOLDOWN_MS) {
    return;
  }

  lastBeepTime = now;

  digitalWrite(PIN_BUZZER, HIGH);
  delay(120);
  digitalWrite(PIN_BUZZER, LOW);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(PIN_PIR_OUT, INPUT);
  pinMode(PIN_LDR_DO, INPUT);

  pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);

  Serial.println();
  Serial.println("=== ESP32-CAM OFFLINE FULL LOGIC TEST ===");
  Serial.println("No WiFi, no Cloud, no Telegram.");
  Serial.println("Normal: green LED ON.");
  Serial.println("Alert: red LED ON and buzzer beep.");
}

void loop() {
  int pirValue = digitalRead(PIN_PIR_OUT);
  int ldrValue = digitalRead(PIN_LDR_DO);
  float distanceCm = readUltrasonicDistanceCm();
  bool rtcFound = checkI2CDevice(0x68);

  bool pirDetected = pirValue == HIGH;
  bool objectNear = distanceCm > 0 && distanceCm < 20;

  bool alert = pirDetected || objectNear;

  if (alert) {
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    beepShortWithCooldown();
  } else {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
  }

  Serial.println("------------------------------");

  Serial.print("PIR: ");
  Serial.println(pirDetected ? "MOTION" : "NO MOTION");

  Serial.print("LDR: ");
  Serial.println(ldrValue == HIGH ? "HIGH" : "LOW");

  Serial.print("Distance: ");
  if (distanceCm < 0) {
    Serial.println("NO ECHO");
  } else {
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  Serial.print("Object near < 20cm: ");
  Serial.println(objectNear ? "YES" : "NO");

  Serial.print("RTC: ");
  Serial.println(rtcFound ? "FOUND" : "NOT FOUND");

  Serial.print("Alert output: ");
  Serial.println(alert ? "RED LED + BUZZER" : "GREEN LED");

  delay(500);
}
```

---

## 6.3 Kết quả mong đợi Test 3

Trạng thái bình thường:

```text
LED xanh sáng
LED đỏ tắt
Buzzer không kêu
```

Khi đưa tay/người qua PIR hoặc đưa vật gần siêu âm dưới 20 cm:

```text
LED xanh tắt
LED đỏ sáng
Buzzer kêu bíp ngắn
```

Khi hết chuyển động và vật không còn gần:

```text
LED đỏ tắt
LED xanh sáng lại
```

---

# 7. Checklist test cuối

Trước khi kết luận wiring ổn, cần tick được:

```text
[ ] ESP32-CAM upload code được.
[ ] ESP32-CAM boot được sau khi tháo GPIO0 khỏi GND.
[ ] Serial Monitor 115200 đọc được log ở Test 1.
[ ] PIR đổi trạng thái khi có chuyển động.
[ ] LDR đổi HIGH/LOW khi che hoặc thay đổi ánh sáng.
[ ] HY-SRF05 trả khoảng cách cm.
[ ] RTC DS1307 trả FOUND hoặc nếu NOT FOUND thì đã kiểm tra dây SDA/SCL.
[ ] LED đỏ sáng/tắt được.
[ ] LED xanh sáng/tắt được.
[ ] Buzzer kêu được.
[ ] Khi chạy Test 3, bình thường LED xanh sáng.
[ ] Khi có chuyển động hoặc vật gần, LED đỏ sáng và buzzer kêu.
```

---

# 8. Kết luận test

Nếu Test 1, Test 2 và Test 3 đều đạt, có thể kết luận:

```text
Phần wiring cơ bản đã ổn.
Có thể chuyển sang bước code logic chính.
Sau đó mới thêm WiFi, Arduino Cloud, Telegram hoặc Google Apps Script.
```

Nếu còn lỗi, sửa theo đúng thiết bị báo lỗi trước. Không nên thêm WiFi/Cloud khi wiring chưa ổn.
