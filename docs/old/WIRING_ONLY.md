# WIRING_ONLY.md

# Sơ đồ cắm mạch mới và lộ trình test module cho dự án IoT Anti-Theft

> **Dự án:** Hệ thống chống trộm IoT  
> **Board chính:** Freenove ESP32-S3 WROOM + Camera OV3660  
> **Mục tiêu:** Cắm lại phần cứng để tránh xung đột với camera, sau đó test từng module trước khi ghép code Arduino Cloud theo SRS.  
> **Trạng thái:** Bản cập nhật sau khi chốt ưu tiên camera OV3660; PIR và RTC được chuyển sang GPIO không xung đột camera.  
> **Cập nhật pin map:** Giữ nguyên camera OV3660; chuyển `PIR OUT` sang `GPIO40`, `RTC SDA` sang `GPIO41`, `RTC SCL` sang `GPIO42`.  
> **Cập nhật RTC hiện tại:** Nhóm **chưa có level shifter**, nên RTC DS1307 sẽ được cắm tạm bằng **3V3** để tránh đưa tín hiệu SDA/SCL 5V vào ESP32-S3. Nếu sau này nâng cấp sang **DS3231** thì vẫn dùng GPIO41/GPIO42, chỉ đổi module RTC và code khởi tạo RTC.

---

## 0. Cập nhật nhanh sau khi chốt lỗi pin map

| Hạng mục | Pin cũ trong tài liệu trước | Pin mới dùng hiện tại | Lý do |
|---|---:|---:|---|
| PIR OUT | GPIO12 | GPIO40 | GPIO12 đang thuộc nhóm chân camera trong firmware hiện tại |
| RTC SDA | GPIO8 | GPIO41 | GPIO8 đang thuộc nhóm chân camera trong firmware hiện tại |
| RTC SCL | GPIO13 | GPIO42 | GPIO13 đang thuộc nhóm chân camera/PCLK trong firmware hiện tại |
| Camera OV3660 | Giữ nguyên | Giữ nguyên | Camera là phần bắt buộc theo SRS, không chuyển dây camera |

---

## 1. Lý do cần cắm lại từ đầu

Sơ đồ cũ đang dùng nhiều chân GPIO bị camera OV3660 chiếm. Nếu vẫn giữ cách cắm cũ rồi bật camera, hệ thống có nguy cơ gặp các lỗi sau:

- Camera không khởi tạo được.
- Cảm biến đọc sai hoặc nhiễu.
- Board reset bất thường.
- Arduino Cloud chạy nhưng camera/chân ngoại vi bị xung đột.
- Sau này phải sửa lại cả dây và code.

Vì SRS hiện tại yêu cầu camera OV3660 là chức năng bắt buộc, nên nguyên tắc đúng là:

```text
Giữ nguyên chân camera
→ Né toàn bộ GPIO camera
→ Gắn cảm biến vào các GPIO còn lại
→ Test từng module
→ Sau đó mới ghép code tổng
```

---

## 2. Các GPIO không nên dùng cho module ngoài

### 2.1 Nhóm GPIO liên quan camera OV3660

Không dùng các chân sau cho PIR, LDR, RTC, LED, buzzer hoặc HY-SRF05:

```text
GPIO3, GPIO4, GPIO5, GPIO6, GPIO7,
GPIO8, GPIO9, GPIO10, GPIO11,
GPIO12, GPIO13,
GPIO15, GPIO16, GPIO17, GPIO18,
GPIO46
```

Lý do: đây là nhóm chân đang liên quan hoặc có nguy cơ liên quan đến camera OV3660 trên board Freenove ESP32-S3 WROOM. Đặc biệt, firmware hiện tại đang cấu hình camera dùng `GPIO8`, `GPIO12` và `GPIO13`, nên không cắm PIR/RTC vào các chân này.

### 2.2 Nhóm GPIO nên tránh nếu không thật sự cần

```text
GPIO0   : Boot, liên quan chế độ nạp chương trình
GPIO45  : Strap, ảnh hưởng trạng thái khởi động
GPIO19  : USB_D-
GPIO20  : USB_D+
GPIO35  : PSRAM
GPIO36  : PSRAM
GPIO37  : PSRAM
GPIO43  : UART TX / Serial
GPIO44  : UART RX / Serial
```

Giải thích thuật ngữ:

- `GPIO`: General Purpose Input/Output, chân vào/ra đa dụng.
- `Boot`: chế độ khởi động/nạp chương trình.
- `Strap`: chân cấu hình trạng thái khởi động của chip.
- `PSRAM`: RAM mở rộng, thường liên quan trực tiếp đến phần cứng board.
- `UART`: giao tiếp nối tiếp, thường dùng cho Serial Monitor hoặc nạp/chẩn đoán.

---

## 3. Sơ đồ cắm mạch mới khuyến nghị

> Lưu ý quan trọng: sơ đồ này ưu tiên né camera và các chân nhạy cảm. Nếu sau này dùng thêm thẻ SD hoặc ngoại vi khác chiếm GPIO38/GPIO39 thì cần phân bổ lại.

| Module | Chân module | Cắm vào ESP32-S3 | Ghi chú |
|---|---|---|---|
| LDR | VCC | 3V3 | Cấp 3.3V |
| LDR | GND | GND | Nối chung GND |
| LDR | AO | GPIO1 | Đọc analog |
| LDR | DO | Không cắm | Không cần dùng |
| PIR HC-SR501 / HC-SR505 | VCC | 5V | PIR thường dùng 5V |
| PIR HC-SR501 / HC-SR505 | GND | GND | Nối chung GND |
| PIR HC-SR501 / HC-SR505 | OUT | GPIO40 | Đọc chuyển động; đã chuyển khỏi GPIO12 để né chân camera |
| RTC DS1307 — phương án hiện tại khi chưa có level shifter | VCC | 3V3 | Cắm tạm để SDA/SCL không lên 5V; an toàn hơn cho ESP32-S3 nhưng DS1307 có thể không chạy ổn |
| RTC DS1307 — phương án hiện tại khi chưa có level shifter | GND | GND | Nối chung GND |
| RTC DS1307 — phương án hiện tại khi chưa có level shifter | SDA | GPIO41 trực tiếp | I2C data, dây dữ liệu; không cắm qua cầu chia áp điện trở |
| RTC DS1307 — phương án hiện tại khi chưa có level shifter | SCL | GPIO42 trực tiếp | I2C clock, dây xung; không cắm qua cầu chia áp điện trở |
| RTC DS1307 — phương án chuẩn nếu sau này có level shifter | VCC | 5V | Chỉ dùng 5V khi SDA/SCL đi qua level shifter I2C 3.3V ↔ 5V |
| RTC DS1307 — phương án chuẩn nếu sau này có level shifter | SDA | GPIO41 qua level shifter | Bên ESP32 vào LV, bên DS1307 vào HV |
| RTC DS1307 — phương án chuẩn nếu sau này có level shifter | SCL | GPIO42 qua level shifter | Bên ESP32 vào LV, bên DS1307 vào HV |
| RTC DS3231 — phương án nâng cấp khuyến nghị | VCC | 3V3 | Nếu đổi sang DS3231, cấp 3.3V để không cần level shifter |
| RTC DS3231 — phương án nâng cấp khuyến nghị | GND | GND | Nối chung GND |
| RTC DS3231 — phương án nâng cấp khuyến nghị | SDA | GPIO41 trực tiếp | Dùng lại pin I2C đã né camera |
| RTC DS3231 — phương án nâng cấp khuyến nghị | SCL | GPIO42 trực tiếp | Dùng lại pin I2C đã né camera |
| LED đỏ | Anode (+) | GPIO14 qua điện trở 220Ω | Báo cảnh báo |
| LED đỏ | Cathode (-) | GND | Cực âm |
| LED xanh | Anode (+) | GPIO21 qua điện trở 220Ω | Báo trạng thái bình thường |
| LED xanh | Cathode (-) | GND | Cực âm |
| Buzzer active nhỏ | (+) | GPIO47 | Nếu buzzer ăn dòng lớn thì dùng transistor |
| Buzzer active nhỏ | (-) | GND | Cực âm |
| HY-SRF05 | VCC | 5V | Cấp 5V |
| HY-SRF05 | GND | GND | Nối chung GND |
| HY-SRF05 | TRIG | GPIO38 | Chân kích hoạt đo |
| HY-SRF05 | ECHO | GPIO39 qua cầu chia áp | Echo thường là 5V, phải hạ xuống 3.3V |

---

## 4. Pin map chuẩn để dùng trong code

```cpp
#define PIN_LDR_AO       1
#define PIN_PIR_OUT      40

#define PIN_RTC_SDA      41
#define PIN_RTC_SCL      42

#define PIN_LED_RED      14
#define PIN_LED_GREEN    21
#define PIN_BUZZER       47

#define PIN_US_TRIG      38
#define PIN_US_ECHO      39
```

> Ghi chú: Pin map trên đang khớp với firmware chính hiện tại. Firmware đã khai báo `PIN_PIR_OUT = 40`, `PIN_RTC_SDA = 41`, `PIN_RTC_SCL = 42`, nên nếu bạn đã cắm dây theo bảng này thì **không cần đổi code cho lỗi pin map**.

---

## 5. Cầu chia áp cho Echo HY-SRF05

Chân `ECHO` của HY-SRF05 thường xuất tín hiệu 5V. ESP32-S3 dùng logic 3.3V, nên không đưa thẳng Echo vào GPIO.

Cắm cầu chia áp như sau:

```text
HY-SRF05 Echo ---- 1kΩ ----+---- GPIO39
                           |
                          2kΩ
                           |
                          GND
```

Điện áp từ Echo 5V sẽ được hạ xuống khoảng 3.3V, an toàn hơn cho ESP32-S3.

---

## 6. Lưu ý với RTC DS1307 và phương án nâng cấp RTC

### 6.1 Quyết định hiện tại: chưa có level shifter nên cấp DS1307 bằng 3V3

Hiện tại nhóm **chưa có level shifter**, nên không dùng phương án DS1307 cấp 5V rồi nối SDA/SCL trực tiếp vào ESP32-S3.

Phương án cắm hiện tại:

```text
DS1307 VCC → 3V3 của ESP32-S3
DS1307 GND → GND
DS1307 SDA → GPIO41
DS1307 SCL → GPIO42
```

Lý do chọn phương án này:

```text
ESP32-S3 dùng logic 3.3V.
Nếu DS1307 cấp 5V và module kéo SDA/SCL lên 5V,
GPIO41/GPIO42 có thể bị quá áp nếu kéo lên 5V.
Cấp DS1307 bằng 3V3 giúp SDA/SCL chỉ quanh mức 3.3V,
an toàn hơn cho ESP32-S3.
```

Hạn chế của phương án này:

```text
DS1307 thường phù hợp 5V hơn 3.3V.
Vì vậy RTC có thể không được nhận, giờ có thể chạy không ổn định,
hoặc lúc nhận lúc không.
```

Nếu smoke test hiện `RTC DS1307: NOT FOUND`, không được tự ý đổi sang 5V rồi nối thẳng SDA/SCL vào ESP32-S3. Khi đó chọn một trong hai hướng:

```text
1. Bỏ RTC tạm thời, tiếp tục test các module khác.
2. Đổi sang DS3231 cấp 3V3.
3. Hoặc mua level shifter I2C rồi dùng lại DS1307 ở 5V.
```

### 6.2 Không dùng cầu chia áp điện trở cho SDA/SCL

Không dùng cầu chia áp kiểu Echo HY-SRF05 cho SDA/SCL của RTC.

Lý do:

```text
I2C là giao tiếp 2 dây.
SDA là đường dữ liệu hai chiều.
SCL là đường xung clock.
Cầu chia áp điện trở chỉ phù hợp hơn với tín hiệu một chiều,
ví dụ Echo của HY-SRF05 đi từ cảm biến về ESP32.
```

### 6.3 Nếu sau này có level shifter và vẫn dùng DS1307

Khi có mạch chuyển mức logic I2C 3.3V ↔ 5V, có thể chuyển sang phương án chuẩn hơn cho DS1307:

```text
ESP32-S3 3V3 → LV của level shifter
ESP32-S3 5V  → HV của level shifter
GND          → GND chung

GPIO41       → LV1
GPIO42       → LV2

DS1307 VCC   → 5V
DS1307 GND   → GND
DS1307 SDA   → HV1
DS1307 SCL   → HV2
```

`Level shifter` là mạch chuyển mức tín hiệu giữa 3.3V và 5V, không phải mạch hạ áp nguồn.

### 6.4 Nếu sau này nâng cấp sang DS3231

Nếu không mua được level shifter, phương án nâng cấp gọn hơn là đổi RTC DS1307 sang **DS3231**. Nếu trong ghi chú cũ có viết `DS3207`, cần hiểu lại là **DS3231**; nếu đúng là một module khác tên DS3207 thì phải kiểm tra datasheet/module thực tế trước khi cắm.

Cách cắm DS3231 khuyến nghị:

```text
DS3231 VCC → 3V3 của ESP32-S3
DS3231 GND → GND
DS3231 SDA → GPIO41
DS3231 SCL → GPIO42
```

Khi cấp DS3231 bằng 3V3, SDA/SCL sẽ ở mức 3.3V nên không cần level shifter.

Khi đổi code từ DS1307 sang DS3231 với thư viện RTClib, đổi phần khai báo RTC:

```cpp
// DS1307 hiện tại
RTC_DS1307 rtc;

// DS3231 sau này
// RTC_DS3231 rtc;
```

Với DS3231, đoạn kiểm tra pin/bộ nhớ thời gian nên dùng `lostPower()` thay vì `isrunning()`:

```cpp
if (rtc.lostPower()) {
  Serial.println("RTC lost power. Setting time from compile time...");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
```

---

## 7. Lưu ý với buzzer

Nếu là `active buzzer`, chỉ cần đưa GPIO HIGH/LOW là kêu/tắt.

Nếu là `passive buzzer`, cần dùng hàm phát xung như `tone()` hoặc PWM.

Nếu buzzer tiêu thụ dòng lớn hơn khoảng 20mA, không nên kéo trực tiếp từ GPIO. Nên dùng transistor NPN hoặc MOSFET để điều khiển.

---

## 8. Thứ tự cắm và test từng module

Không cắm tất cả một lần. Làm theo thứ tự sau:

| Bước | Cắm module | Mục tiêu test | Kết quả đúng |
|---:|---|---|---|
| 1 | Chưa cắm module ngoài | Test upload code | Serial Monitor chạy |
| 2 | LED đỏ, LED xanh, buzzer | Test output | LED nháy, buzzer kêu |
| 3 | LDR | Test ánh sáng | Che sáng hoặc chiếu đèn thì giá trị thay đổi rõ |
| 4 | PIR | Test chuyển động | Có người đi qua thì PIR chuyển `1` |
| 5 | HY-SRF05 | Test khoảng cách | Đưa tay gần thì khoảng cách giảm |
| 6 | RTC DS1307 | Test thời gian | In được ngày/giờ/phút/giây |
| 7 | Camera OV3660 | Test camera riêng | Camera khởi tạo và chụp/stream được |
| 8 | Ghép cảm biến + output | Test cục bộ | Đọc ổn nhiều module cùng lúc |
| 9 | Ghép Arduino Cloud | Test dashboard | Cloud đọc/ghi biến được |
| 10 | Ghép logic SRS | Test state machine | `ARMED`, `INTRUSION_ALERT`, `SOS_ALERT`, `SABOTAGE_ALERT` chạy đúng |

---

## 9. Code smoke test phần cứng

Cài thư viện trước:

```text
RTClib by Adafruit
```

Code dưới đây dùng để test:

- LED đỏ.
- LED xanh.
- Buzzer.
- LDR AO.
- PIR.
- HY-SRF05.
- RTC DS1307 theo phương án hiện tại VCC 3V3.

```cpp
#include <Wire.h>
#include "RTClib.h"

// =======================
// PIN MAP MỚI - NÉ CAMERA
// =======================
#define PIN_LDR_AO       1
#define PIN_PIR_OUT      40

#define PIN_RTC_SDA      41
#define PIN_RTC_SCL      42

#define PIN_LED_RED      14
#define PIN_LED_GREEN    21
#define PIN_BUZZER       47

#define PIN_US_TRIG      38
#define PIN_US_ECHO      39

RTC_DS1307 rtc;
bool rtcOk = false;

const bool ENABLE_BUZZER_TEST = true;

// =======================
// ĐỌC SIÊU ÂM
// =======================
float readUltrasonicDistanceCm() {
  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_US_ECHO, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

// =======================
// TEST LED + BUZZER
// =======================
void testOutputsBlink() {
  static unsigned long lastToggle = 0;
  static bool alertState = false;

  if (millis() - lastToggle >= 1000) {
    lastToggle = millis();
    alertState = !alertState;

    digitalWrite(PIN_LED_RED, alertState ? HIGH : LOW);
    digitalWrite(PIN_LED_GREEN, alertState ? LOW : HIGH);

    if (ENABLE_BUZZER_TEST) {
      digitalWrite(PIN_BUZZER, alertState ? HIGH : LOW);
    } else {
      digitalWrite(PIN_BUZZER, LOW);
    }
  }
}

// =======================
// IN DỮ LIỆU SENSOR
// =======================
void printSensorData() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint < 1000) {
    return;
  }

  lastPrint = millis();

  int ldrValue = analogRead(PIN_LDR_AO);
  int pirDetected = digitalRead(PIN_PIR_OUT);
  float distanceCm = readUltrasonicDistanceCm();

  Serial.println("================================");

  Serial.print("LDR value: ");
  Serial.println(ldrValue);

  Serial.print("PIR detected: ");
  Serial.println(pirDetected);

  Serial.print("Ultrasonic distance cm: ");
  if (distanceCm < 0) {
    Serial.println("NO_ECHO");
  } else {
    Serial.println(distanceCm);
  }

  if (rtcOk) {
    DateTime now = rtc.now();

    Serial.print("RTC time: ");
    Serial.print(now.year());
    Serial.print("-");
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
  } else {
    Serial.println("RTC status: NOT_FOUND");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("ESP32-S3 IoT Anti-Theft Hardware Smoke Test");
  Serial.println("Using camera-safe pin map.");
  Serial.println("RTC wiring note: DS1307 is expected to be powered from 3V3 in the current no-level-shifter setup.");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_PIR_OUT, INPUT);

  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_US_TRIG, LOW);

  analogReadResolution(12);

  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);

  rtcOk = rtc.begin();

  if (rtcOk) {
    Serial.println("RTC DS1307: FOUND");

    if (!rtc.isrunning()) {
      Serial.println("RTC is not running. Setting time from compile time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    Serial.println("RTC DS1307: NOT FOUND");
  }

  Serial.println("PIR needs about 30-60 seconds to stabilize after power on.");
}

void loop() {
  testOutputsBlink();
  printSensorData();
}
```

---

## 10. Cách đọc kết quả smoke test

### 10.1 LED và buzzer

Kết quả đúng:

```text
LED đỏ nháy 1 giây/lần
LED xanh sáng ngược trạng thái với LED đỏ
Buzzer kêu theo LED đỏ
```

Nếu buzzer không kêu:

```text
1. Kiểm tra buzzer là active hay passive.
2. Kiểm tra chân + và -.
3. Nếu buzzer ăn dòng lớn, cần dùng transistor.
```

### 10.2 LDR

Serial Monitor sẽ in dạng:

```text
LDR value: 1234
```

Kết quả đúng:

```text
Che cảm biến hoặc chiếu đèn vào thì giá trị thay đổi rõ.
```

Nếu không đổi:

```text
1. Kiểm tra đã cắm AO vào GPIO1 chưa.
2. Không dùng DO trong test này.
3. Kiểm tra VCC LDR là 3V3.
4. Kiểm tra GND đã nối chung chưa.
```

### 10.3 PIR

Serial Monitor sẽ in dạng:

```text
PIR detected: 0
```

Khi có chuyển động:

```text
PIR detected: 1
```

Lưu ý:

```text
PIR cần khoảng 30-60 giây sau khi cấp nguồn để ổn định.
```

### 10.4 HY-SRF05

Serial Monitor sẽ in dạng:

```text
Ultrasonic distance cm: 25.30
```

Kết quả đúng:

```text
Đưa tay lại gần cảm biến thì số khoảng cách giảm.
Đưa tay ra xa thì số khoảng cách tăng.
```

Nếu hiện:

```text
NO_ECHO
```

Kiểm tra:

```text
1. TRIG có đúng GPIO38 không.
2. ECHO có qua cầu chia áp vào GPIO39 không.
3. VCC HY-SRF05 là 5V chưa.
4. GND đã nối chung chưa.
```

### 10.5 RTC DS1307

Phương án hiện tại của nhóm là DS1307 cấp **3V3** vì chưa có level shifter.

Nếu đúng:

```text
RTC DS1307: FOUND
RTC time: yyyy-mm-dd hh:mm:ss
```

Nếu lỗi:

```text
RTC DS1307: NOT FOUND
```

Kiểm tra:

```text
1. SDA có vào GPIO41 không.
2. SCL có vào GPIO42 không.
3. VCC DS1307 hiện tại có đang vào 3V3 không.
4. GND đã nối chung chưa.
5. Nếu vẫn NOT_FOUND, khả năng module DS1307 không chạy ổn ở 3.3V.
```

Không xử lý lỗi bằng cách cắm DS1307 VCC vào 5V rồi nối thẳng SDA/SCL vào ESP32-S3. Nếu cần dùng DS1307 ở 5V, phải thêm level shifter I2C. Nếu không mua được level shifter, đổi sang DS3231 cấp 3V3 là hướng dễ hơn.

---

## 11. Checklist trước khi chuyển sang code chính

Chỉ chuyển sang code Arduino Cloud/SRS khi tất cả mục sau PASS:

```text
[ ] Upload code thành công.
[ ] Serial Monitor chạy ở baud 115200.
[ ] LED đỏ nháy đúng.
[ ] LED xanh hoạt động đúng.
[ ] Buzzer kêu/tắt đúng.
[ ] LDR đọc được giá trị analog và thay đổi khi che/chiếu sáng.
[ ] PIR phát hiện chuyển động sau thời gian ổn định.
[ ] HY-SRF05 đo được khoảng cách, không bị NO_ECHO liên tục.
[ ] Echo HY-SRF05 đã qua cầu chia áp.
[ ] RTC DS1307 được tìm thấy qua I2C khi cấp 3V3, hoặc đã ghi nhận `RTC_PENDING` để đổi sang DS3231 / thêm level shifter sau.
[ ] GND của tất cả module đã nối chung.
[ ] Camera OV3660 vẫn test được riêng.
```

---

## 12. Lộ trình code chính sau khi phần cứng PASS

### Phase 1 — Hardware Layer

Tạo các hàm đọc/ghi phần cứng:

```text
readLdr()
readPir()
readUltrasonicDistanceCm()
readRtcTime()
setLedRed()
setLedGreen()
setBuzzer()
```

### Phase 2 — Security Logic

Tạo logic lõi theo SRS:

```text
updateSystemArmed()
updateIntrusionScore()
updateSabotageDetection()
updateSosLatch()
updateAlarmStatus()
updateLocalOutputs()
```

### Phase 3 — Arduino Cloud Variables

Ghép các biến Cloud tối thiểu trước:

```text
alarm_enabled
system_armed
reset_alarm
alarm_status
sos_child
sos_adult
pir_detected
ultrasonic_distance
object_near
ldr_value
light_abnormal
intrusion_alert
intrusion_score
threat_level
sabotage_alert
buzzer_on
led_red_on
led_green_on
last_event
last_event_type
event_counter
```

### Phase 4 — State Machine

Không để từng cảm biến tự bật/tắt còi riêng. Còi và LED phải chạy theo trạng thái tổng:

```text
DISARMED
DISARMED_BY_TRUSTED_DEVICE
ARMED
INTRUSION_ALERT
SOS_ALERT
SABOTAGE_ALERT
ALARM_RESET
ERROR
```

`State machine` nghĩa là mô hình điều khiển theo trạng thái. Ví dụ khi hệ thống vào `SOS_ALERT`, còi và LED đỏ phải duy trì cho đến khi người lớn/admin bấm `reset_alarm`.

### Phase 5 — Camera

Thêm sau khi logic chính chạy ổn:

```text
manual_capture_photo
auto_capture_photo_request
photo_status
```

### Phase 6 — Notification

Thêm Telegram/notification:

```text
send_notification_request
notification_event_type
notification_sent_status
notification_channel
cooldown_active
last_alert_time
```

### Phase 7 — Google Apps Script SOS Escalation

Thêm sau khi SOS local chạy ổn:

```text
emergency_confirmation_requested
emergency_confirmed
emergency_escalation_status
home_address_configured
sos_authority_note
emergency_authority_message_status
```

Không hardcode địa chỉ nhà hoặc contact cơ quan chức năng/công an trực tiếp trong firmware nếu không cần thiết. Các thông tin nhạy cảm nên để trong Google Apps Script hoặc cấu hình triển khai.

### Phase 8 — BLE Trusted Device

Thêm cuối cùng:

```text
trusted_ble_detection_enabled
trusted_ble_present
trusted_device_source
trusted_ble_rssi
trusted_ble_last_seen_seconds
known_device_present
```

Lý do làm BLE cuối cùng: BLE scanning có thể ảnh hưởng WiFi/Arduino Cloud nếu triển khai chưa tốt. Ban đầu có thể dùng `known_device_present` từ dashboard làm controlled demo input để chứng minh logic.

---

## 13. Quy tắc quan trọng khi code theo SRS

Không code sai các điểm sau:

```text
1. alarm_enabled không phải công tắc tắt toàn hệ thống.
2. system_armed là trạng thái hệ thống tự tính, không phải biến để dashboard ép trực tiếp.
3. SOS không phụ thuộc alarm_enabled.
4. Anti-sabotage không phụ thuộc alarm_enabled.
5. Intrusion alert không tự tắt khi intrusion_score giảm.
6. Cảnh báo active chỉ tắt khi reset_alarm.
7. ACK trong SOS escalation không phải reset.
8. SOS_ALERT phải giữ còi/LED đỏ cho đến khi reset_alarm.
9. WiFi/MAC lạ chỉ là pre-warning, không tự tạo intrusion alert độc lập.
10. BLE trusted device không được tự xóa cảnh báo đang active.
```


---

## 14. Ghi chú quyết định RTC hiện tại

Quyết định phần cứng hiện tại:

```text
Chưa có level shifter.
Tạm thời cắm DS1307 VCC vào 3V3.
SDA dùng GPIO41.
SCL dùng GPIO42.
Không cắm DS1307 VCC 5V nếu SDA/SCL nối thẳng ESP32-S3.
```

Nếu DS1307 chạy ổn ở 3V3:

```text
Giữ phương án hiện tại để code/test tiếp.
```

Nếu DS1307 không chạy ổn ở 3V3:

```text
Không sửa dây sang 5V trực tiếp.
Đánh dấu RTC_PENDING.
Tiếp tục test phần chống trộm chính.
Sau đó mua DS3231 hoặc level shifter.
```

Phương án nâng cấp khuyến nghị:

```text
Đổi sang DS3231.
DS3231 VCC → 3V3
DS3231 GND → GND
DS3231 SDA → GPIO41
DS3231 SCL → GPIO42
```

---

## 15. Kết luận triển khai

Thứ tự đúng của nhóm nên là:

```text
1. Cắm lại dây theo sơ đồ mới.
2. Chạy smoke test phần cứng.
3. Xác nhận từng module PASS.
4. Test camera riêng.
5. Ghép code Arduino Cloud tối thiểu.
6. Viết state machine theo SRS.
7. Thêm camera, notification, Google Apps Script, BLE sau cùng.
```

Không nên ghép full code ngay khi phần cứng chưa PASS, vì khi lỗi sẽ rất khó biết lỗi đến từ dây, GPIO, module, camera, Cloud hay logic.
