# WIRING_BY_DEVICE.md

# Sơ đồ gắn dây theo từng thiết bị với ESP32-CAM

> File này trình bày theo đúng cách cắm từng thiết bị vào ESP32-CAM.  
> Mỗi mục là một thiết bị riêng.  
> Trong từng thiết bị sẽ ghi rõ từng dây cắm vào chân nào của ESP32-CAM.

---

## 1. Quy ước chung

Trong tài liệu này:

- `VCC` là chân cấp nguồn cho thiết bị.
- `GND` là mass/cực âm chung.
- `GPIO` là chân vào/ra tín hiệu của ESP32-CAM.
- Tất cả thiết bị phải nối chung `GND` với ESP32-CAM.
- Không dùng thẻ microSD trong bản wiring này.
- Không dùng `GPIO0` cho thiết bị ngoại vi.

---

# 2. PIR HC-SR501 cắm với ESP32-CAM

## 2.1 Bảng cắm dây

| PIR HC-SR501 | ESP32-CAM |
|---|---|
| `VCC` | `5V` |
| `GND` | `GND` |
| `OUT` | `GPIO16` |

## 2.2 Sơ đồ chữ

```text
PIR HC-SR501        ESP32-CAM
-----------         ---------
VCC       --------> 5V
GND       --------> GND
OUT       --------> GPIO16
```

## 2.3 Chân dùng trong code

```cpp
#define PIN_PIR_OUT 16
```

---

# 3. Cảm biến siêu âm HY-SRF05 cắm với ESP32-CAM

## 3.1 Bảng cắm dây

| HY-SRF05 | ESP32-CAM |
|---|---|
| `VCC` | `5V` |
| `GND` | `GND` |
| `Trig` | `GPIO13` |
| `Echo` | Qua mạch chia áp rồi vào `GPIO12` |
| `OUT` | Không cắm |

## 3.2 Sơ đồ chữ

```text
HY-SRF05             ESP32-CAM
--------             ---------
VCC        --------> 5V
GND        --------> GND
Trig       --------> GPIO13
Echo       --------> chia áp --------> GPIO12
OUT        --------> không cắm
```

## 3.3 Mạch chia áp cho Echo

Không cắm `Echo` trực tiếp vào ESP32-CAM.

`Echo` của HY-SRF05 có thể trả tín hiệu mức 5V. GPIO của ESP32-CAM dùng mức 3.3V, vì vậy phải dùng mạch chia áp trước khi đưa tín hiệu vào `GPIO12`.

**Chia áp** là cách dùng điện trở để hạ điện áp tín hiệu, giúp tín hiệu từ cảm biến 5V đi vào ESP32-CAM an toàn hơn.

---

### Cách 1 - Cách chuẩn nếu có điện trở 2.2kΩ

Dùng:

```text
R1 = 1kΩ
R2 = 2.2kΩ
```

Bảng cắm:

| Thành phần | Cắm với |
|---|---|
| `Echo` HY-SRF05 | Một đầu `R1 1kΩ` |
| Đầu còn lại của `R1 1kΩ` | Điểm `ECHO_SAFE` |
| Điểm `ECHO_SAFE` | `GPIO12` ESP32-CAM |
| Điểm `ECHO_SAFE` | Một đầu `R2 2.2kΩ` |
| Đầu còn lại của `R2 2.2kΩ` | `GND` ESP32-CAM |

Sơ đồ chữ:

```text
HY-SRF05 Echo
   |
   |--- R1 1kΩ ---+--- GPIO12 ESP32-CAM
                  |
                R2 2.2kΩ
                  |
                 GND
```

---

### Cách 2 - Nếu không có điện trở 2.2kΩ

Dùng:

```text
R1 = 1kΩ
R2 = 5 điện trở 330Ω nối tiếp = 1650Ω
```

Bảng cắm:

| Thành phần | Cắm với |
|---|---|
| `Echo` HY-SRF05 | Một đầu `R1 1kΩ` |
| Đầu còn lại của `R1 1kΩ` | Điểm `ECHO_SAFE` |
| Điểm `ECHO_SAFE` | `GPIO12` ESP32-CAM |
| Điểm `ECHO_SAFE` | Một đầu điện trở `330Ω số 1` |
| Điện trở `330Ω số 1` | Điện trở `330Ω số 2` |
| Điện trở `330Ω số 2` | Điện trở `330Ω số 3` |
| Điện trở `330Ω số 3` | Điện trở `330Ω số 4` |
| Điện trở `330Ω số 4` | Điện trở `330Ω số 5` |
| Điện trở `330Ω số 5` | `GND` ESP32-CAM |

Sơ đồ chữ:

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

Cách 2 tạo điện áp vào `GPIO12` khoảng 3.1V khi `Echo` ở mức 5V. Mức này phù hợp hơn cho ESP32-CAM so với cắm `Echo` trực tiếp.

## 3.4 Chân dùng trong code

```cpp
#define PIN_ULTRASONIC_TRIG 13
#define PIN_ULTRASONIC_ECHO 12
```

---

# 4. LDR MH-Sensor-Series cắm với ESP32-CAM

## 4.1 Bảng cắm dây

| LDR Module | ESP32-CAM |
|---|---|
| `VCC` | `3.3V` |
| `GND` | `GND` |
| `DO` | `GPIO2` |
| `AO` | Không cắm |

## 4.2 Sơ đồ chữ

```text
LDR Module          ESP32-CAM
----------          ---------
VCC       --------> 3.3V
GND       --------> GND
DO        --------> GPIO2
AO        --------> không cắm
```

## 4.3 Chân dùng trong code

```cpp
#define PIN_LDR_DO 2
```

**DO** là Digital Output, nghĩa là tín hiệu số dạng HIGH/LOW.

---

# 5. Tiny RTC DS1307 cắm với ESP32-CAM

## 5.1 Bảng cắm dây

| Tiny RTC DS1307 | ESP32-CAM |
|---|---|
| `VCC` | `3.3V` |
| `GND` | `GND` |
| `SDA` | `GPIO14` |
| `SCL` | `GPIO15` |
| `SQ` | Không cắm |
| `DS` | Không cắm |
| `BAT` | Không cắm nếu chưa dùng pin backup |

## 5.2 Sơ đồ chữ

```text
Tiny RTC DS1307      ESP32-CAM
---------------      ---------
VCC        --------> 3.3V
GND        --------> GND
SDA        --------> GPIO14
SCL        --------> GPIO15
SQ         --------> không cắm
DS         --------> không cắm
BAT        --------> không cắm nếu chưa dùng pin backup
```

## 5.3 Chân dùng trong code

```cpp
#define PIN_RTC_SDA 14
#define PIN_RTC_SCL 15
```

**I2C** là giao tiếp hai dây.  
`SDA` là dây dữ liệu.  
`SCL` là dây xung clock.

---

# 6. LED đỏ cắm với ESP32-CAM

## 6.1 Bảng cắm dây

| LED đỏ | ESP32-CAM |
|---|---|
| Anode / chân dài | Qua điện trở rồi vào `GPIO4` |
| Cathode / chân ngắn | `GND` |

## 6.2 Sơ đồ chữ

```text
ESP32-CAM GPIO4
      |
      | 
Điện trở 220Ω hoặc 330Ω
      |
      |
Anode LED đỏ
Cathode LED đỏ
      |
ESP32-CAM GND
```

## 6.3 Chân dùng trong code

```cpp
#define PIN_LED_RED 4
```

**Anode** là cực dương của LED, thường là chân dài.  
**Cathode** là cực âm của LED, thường là chân ngắn.

---

# 7. LED xanh cắm với ESP32-CAM

## 7.1 Bảng cắm dây

| LED xanh | ESP32-CAM |
|---|---|
| Anode / chân dài | Qua điện trở rồi vào `GPIO1` |
| Cathode / chân ngắn | `GND` |

## 7.2 Sơ đồ chữ

```text
ESP32-CAM GPIO1
      |
      |
Điện trở 220Ω hoặc 330Ω
      |
      |
Anode LED xanh
Cathode LED xanh
      |
ESP32-CAM GND
```

## 7.3 Chân dùng trong code

```cpp
#define PIN_LED_GREEN 1
```

---

# 8. Buzzer 2 chân cắm với ESP32-CAM

## 8.1 Bảng cắm dây

| Buzzer 2 chân | ESP32-CAM |
|---|---|
| `+` | `GPIO3` |
| `-` | `GND` |

## 8.2 Sơ đồ chữ

```text
Buzzer              ESP32-CAM
------              ---------
+         --------> GPIO3
-         --------> GND
```

## 8.3 Chân dùng trong code

```cpp
#define PIN_BUZZER 3
```

---

# 9. Nguồn ngoài cắm với ESP32-CAM

## 9.1 Bảng cắm dây

| Nguồn ngoài | ESP32-CAM |
|---|---|
| `5V` | `5V` |
| `GND` | `GND` |

## 9.2 Sơ đồ chữ

```text
Nguồn ngoài         ESP32-CAM
-----------         ---------
5V        --------> 5V
GND       --------> GND
```

Nên dùng nguồn ngoài 5V ổn định vì ESP32-CAM dùng WiFi và camera.

---

# 10. Bảng tổng hợp từng thiết bị

| Thiết bị | Cắm vào ESP32-CAM |
|---|---|
| PIR HC-SR501 | `VCC -> 5V`, `GND -> GND`, `OUT -> GPIO16` |
| HY-SRF05 | `VCC -> 5V`, `GND -> GND`, `Trig -> GPIO13`, `Echo -> chia áp -> GPIO12` |
| LDR Module | `VCC -> 3.3V`, `GND -> GND`, `DO -> GPIO2` |
| Tiny RTC DS1307 | `VCC -> 3.3V`, `GND -> GND`, `SDA -> GPIO14`, `SCL -> GPIO15` |
| LED đỏ | `Anode -> điện trở -> GPIO4`, `Cathode -> GND` |
| LED xanh | `Anode -> điện trở -> GPIO1`, `Cathode -> GND` |
| Buzzer | `+ -> GPIO3`, `- -> GND` |
| Nguồn ngoài | `5V -> 5V ESP32-CAM`, `GND -> GND ESP32-CAM` |

---

# 11. Khai báo pin tổng hợp cho code

```cpp
#define PIN_PIR_OUT             16

#define PIN_ULTRASONIC_TRIG     13
#define PIN_ULTRASONIC_ECHO     12

#define PIN_LDR_DO              2

#define PIN_RTC_SDA             14
#define PIN_RTC_SCL             15

#define PIN_LED_RED             4
#define PIN_LED_GREEN           1

#define PIN_BUZZER              3
```

---

# 12. Lưu ý khi cắm thật

## 12.1 Không dùng GPIO0

Không cắm thiết bị nào vào `GPIO0`.

`GPIO0` dùng cho chế độ upload code của ESP32-CAM.

## 12.2 Không dùng microSD

Không cắm thẻ microSD trong bản wiring này vì một số chân đang được dùng cho cảm biến và LED.

## 12.3 GPIO1 và GPIO3 có thể ảnh hưởng upload code

`GPIO1` và `GPIO3` là chân UART.

**UART** là giao tiếp nạp code và xem log Serial.

Trong sơ đồ này:

```text
GPIO1 -> LED xanh
GPIO3 -> Buzzer
```

Nếu upload code bị lỗi, tháo tạm dây ở `GPIO1` và `GPIO3`, upload xong rồi cắm lại.

## 12.4 Echo của HY-SRF05 phải qua chia áp

Không cắm `Echo` trực tiếp vào ESP32-CAM.

Phải dùng mạch chia áp trước khi đưa vào `GPIO12`.

---

# 13. Checklist cắm dây theo từng thiết bị

```text
[ ] Nguồn ngoài: 5V -> ESP32-CAM 5V, GND -> ESP32-CAM GND.

[ ] PIR: VCC -> 5V, GND -> GND, OUT -> GPIO16.

[ ] HY-SRF05: VCC -> 5V, GND -> GND, Trig -> GPIO13, Echo -> chia áp -> GPIO12.

[ ] LDR: VCC -> 3.3V, GND -> GND, DO -> GPIO2.

[ ] RTC DS1307: VCC -> 3.3V, GND -> GND, SDA -> GPIO14, SCL -> GPIO15.

[ ] LED đỏ: GPIO4 -> điện trở -> Anode, Cathode -> GND.

[ ] LED xanh: GPIO1 -> điện trở -> Anode, Cathode -> GND.

[ ] Buzzer: + -> GPIO3, - -> GND.

[ ] Không có thiết bị nào cắm vào GPIO0.

[ ] Không cắm microSD.
```

---

# 14. Bản chốt ngắn nhất

```text
PIR HC-SR501:
VCC -> 5V
GND -> GND
OUT -> GPIO16

HY-SRF05:
VCC -> 5V
GND -> GND
Trig -> GPIO13
Echo -> chia áp -> GPIO12
OUT -> không cắm

LDR:
VCC -> 3.3V
GND -> GND
DO -> GPIO2
AO -> không cắm

RTC DS1307:
VCC -> 3.3V
GND -> GND
SDA -> GPIO14
SCL -> GPIO15

LED đỏ:
GPIO4 -> điện trở -> chân dài LED
chân ngắn LED -> GND

LED xanh:
GPIO1 -> điện trở -> chân dài LED
chân ngắn LED -> GND

Buzzer:
+ -> GPIO3
- -> GND
```
