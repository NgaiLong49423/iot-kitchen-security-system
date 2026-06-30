# HARDWARE_DEVICE_PINS.md

# Danh sách thiết bị và chân phần cứng

> Mục đích tài liệu: mô tả **mỗi thiết bị có bao nhiêu chân, tên các chân là gì, và ý nghĩa từng chân**.  
> Tài liệu này **chưa mô tả cách nối mạch**, **chưa đề xuất GPIO**, và **chưa đưa cảnh báo điện áp**. Các phần đó sẽ được làm ở bước thiết kế sơ đồ cắm mạch riêng.

---

## 1. Danh sách thiết bị trong dự án

| STT | Thiết bị | Số chân chính | Vai trò trong dự án |
|---:|---|---:|---|
| 1 | ESP32-CAM | 16 chân header chính | Board điều khiển chính, camera, WiFi |
| 2 | Module cảm biến ánh sáng LDR MH-Sensor-Series | 4 chân | Phát hiện ánh sáng / che cảm biến |
| 3 | Cảm biến siêu âm HY-SRF05 | 5 chân | Đo khoảng cách vật thể |
| 4 | Cảm biến chuyển động PIR HC-SR501 | 3 chân | Phát hiện chuyển động |
| 5 | Tiny RTC DS1307 Module | 7 chân ở cụm P1, 5 chân ở cụm P2 | Cung cấp thời gian thực |
| 6 | LED đỏ | 2 chân | Báo trạng thái cảnh báo |
| 7 | LED xanh | 2 chân | Báo trạng thái bình thường |
| 8 | Điện trở LED | 2 chân | Linh kiện giới hạn dòng cho LED |
| 9 | Còi buzzer 2 chân | 2 chân | Phát âm thanh cảnh báo |

---

# 2. ESP32-CAM

## 2.1 Vai trò

ESP32-CAM là board chính của hệ thống. Board này dùng để xử lý logic, kết nối WiFi, giao tiếp với cảm biến và dùng camera tích hợp.

## 2.2 Số chân

Theo sơ đồ chân được cung cấp, ESP32-CAM có **16 chân header chính**.

## 2.3 Danh sách chân

| Vị trí theo sơ đồ | Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|---|
| Bên trái | `5V` | Power | Chân nguồn 5V |
| Bên trái | `GND` | Ground | Chân mass / cực âm |
| Bên trái | `GPIO12` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_5`, `HSPI_Q`, `Touch5` |
| Bên trái | `GPIO13` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_4`, `HSPI_ID`, `Touch4` |
| Bên trái | `GPIO15` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_3`, `HSPI_CS0`, `Touch3` |
| Bên trái | `GPIO14` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_6`, `HSPI_CLK`, `Touch6` |
| Bên trái | `GPIO2` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_2`, `HSPI_WP`, `Touch2` |
| Bên trái | `GPIO4` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_0`, `HSPI_HD`, `Touch0` |
| Bên phải | `3.3V` | Power | Chân nguồn 3.3V |
| Bên phải | `GPIO16` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `U2_RXD` |
| Bên phải | `GPIO0` | GPIO | Chân vào/ra số; trên sơ đồ có chức năng phụ `ADC2_1`, `Touch1`, `CLK1` |
| Bên phải | `GND` | Ground | Chân mass / cực âm |
| Bên phải | `VCC` | Power | Chân nguồn theo thiết kế board |
| Bên phải | `GPIO3` | GPIO / UART | Chân vào/ra số; trên sơ đồ có chức năng phụ `U0_RXD`, `CLK2` |
| Bên phải | `GPIO1` | GPIO / UART | Chân vào/ra số; trên sơ đồ có chức năng phụ `U0_TXD`, `CLK3` |
| Bên phải | `GND` | Ground | Chân mass / cực âm |

## 2.4 Thuật ngữ chân

| Thuật ngữ | Ý nghĩa |
|---|---|
| `GPIO` | General Purpose Input/Output, chân vào/ra đa dụng |
| `Power` | Chân cấp nguồn |
| `Ground` | Chân mass / cực âm |
| `UART` | Giao tiếp nối tiếp dùng cho truyền/nhận dữ liệu |
| `ADC` | Analog-to-Digital Converter, bộ đọc tín hiệu analog thành số |
| `Touch` | Chân có khả năng đọc cảm ứng điện dung |
| `HSPI` | Một nhóm chân SPI phần cứng trên ESP32 |

---

# 3. Module cảm biến ánh sáng LDR MH-Sensor-Series

## 3.1 Vai trò

Module LDR dùng để nhận biết cường độ ánh sáng. Trong dự án, module này phục vụ các logic liên quan đến ánh sáng bất thường hoặc tình huống cảm biến bị che.

## 3.2 Số chân

Module trong ảnh có **4 chân**.

## 3.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `VCC` | Power | Chân cấp nguồn cho module |
| `GND` | Ground | Chân mass / cực âm |
| `DO` | Digital Signal | Chân tín hiệu số |
| `AO` | Analog Signal | Chân tín hiệu analog |

## 3.4 Thành phần nhận diện trên module

| Thành phần | Ý nghĩa |
|---|---|
| Quang trở LDR | Linh kiện thay đổi giá trị theo ánh sáng |
| Biến trở chỉnh ngưỡng | Núm chỉnh ngưỡng để thay đổi trạng thái tín hiệu `DO` |
| Mạch so sánh | Mạch tạo tín hiệu số `DO` dựa trên ngưỡng ánh sáng |

## 3.5 Thuật ngữ chân

| Thuật ngữ | Ý nghĩa |
|---|---|
| `DO` | Digital Output, tín hiệu đầu ra dạng số |
| `AO` | Analog Output, tín hiệu đầu ra dạng analog |
| Digital Signal | Tín hiệu thường có hai mức logic, ví dụ HIGH/LOW |
| Analog Signal | Tín hiệu thay đổi liên tục theo giá trị vật lý |

---

# 4. Cảm biến siêu âm HY-SRF05

## 4.1 Vai trò

HY-SRF05 dùng để đo khoảng cách từ cảm biến đến vật thể phía trước.

Trong dự án, cảm biến này phục vụ logic phát hiện vật thể hoặc người ở gần khu vực giám sát.

## 4.2 Số chân

Module trong ảnh có **5 chân**.

## 4.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `VCC` | Power | Chân cấp nguồn cho module |
| `Trig` | Trigger Input | Chân nhận xung kích hoạt đo |
| `Echo` | Echo Output | Chân trả tín hiệu phản hồi sau khi đo |
| `OUT` | Signal / Mode | Chân tín hiệu/phụ thuộc chế độ module |
| `GND` | Ground | Chân mass / cực âm |

## 4.4 Thành phần nhận diện trên module

| Thành phần | Ý nghĩa |
|---|---|
| Đầu phát siêu âm | Bộ phát sóng siêu âm |
| Đầu thu siêu âm | Bộ nhận sóng siêu âm phản hồi |
| Thạch anh `Y1` | Linh kiện tạo dao động cho mạch |
| Ký hiệu `T` | Thường chỉ phía transmitter, tức đầu phát |
| Ký hiệu `R` | Thường chỉ phía receiver, tức đầu thu |

## 4.5 Thuật ngữ chân

| Thuật ngữ | Ý nghĩa |
|---|---|
| `Trig` | Trigger, chân kích hoạt quá trình đo |
| `Echo` | Echo, chân phản hồi kết quả đo bằng độ dài xung |
| Ultrasonic | Sóng siêu âm, tần số cao hơn khả năng nghe của người |
| Transmitter | Bộ phát tín hiệu |
| Receiver | Bộ nhận tín hiệu |

---

# 5. Cảm biến chuyển động PIR HC-SR501

## 5.1 Vai trò

PIR HC-SR501 dùng để phát hiện chuyển động của người hoặc vật thể có thay đổi bức xạ hồng ngoại trong vùng quan sát.

Trong dự án, cảm biến này phục vụ logic phát hiện chuyển động trong khu vực giám sát.

## 5.2 Số chân

Module trong ảnh có **3 chân**.

## 5.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `GND` | Ground | Chân mass / cực âm |
| `Output` / `OUT` | Digital Signal | Chân tín hiệu phát hiện chuyển động |
| `VCC` | Power | Chân cấp nguồn cho module |

## 5.4 Thành phần nhận diện trên module

| Thành phần | Ý nghĩa |
|---|---|
| PIR sensor | Cảm biến hồng ngoại thụ động |
| Fresnel lens | Chụp nhựa trắng giúp mở rộng vùng phát hiện |
| Distance / Sensitivity adjustment | Biến trở chỉnh độ nhạy hoặc khoảng cách phát hiện |
| Delay time adjustment | Biến trở chỉnh thời gian giữ tín hiệu sau khi phát hiện |
| Jumper mode | Chân/cầu chọn chế độ hoạt động của một số module HC-SR501 |

## 5.5 Thuật ngữ chân

| Thuật ngữ | Ý nghĩa |
|---|---|
| `PIR` | Passive Infrared Sensor, cảm biến hồng ngoại thụ động |
| `OUT` | Output, chân tín hiệu đầu ra |
| Digital Signal | Tín hiệu số, thường biểu diễn phát hiện hoặc không phát hiện |

---

# 6. Tiny RTC DS1307 Module

## 6.1 Vai trò

Tiny RTC DS1307 Module dùng để cung cấp thời gian thực cho hệ thống.

Trong dự án, RTC phục vụ logic thời gian, log sự kiện và lịch tự động.

## 6.2 Số chân

Theo ảnh, module có hai cụm chân chính:

| Cụm chân | Số chân có nhãn |
|---|---:|
| P1 | 7 chân |
| P2 | 5 chân |

## 6.3 Danh sách chân cụm P1

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `SQ` | Signal Output | Chân xuất xung vuông từ RTC |
| `DS` | Data / Extension | Chân mở rộng liên quan DS trên module |
| `SCL` | I2C Clock | Chân clock của giao tiếp I2C |
| `SDA` | I2C Data | Chân dữ liệu của giao tiếp I2C |
| `VCC` | Power | Chân cấp nguồn |
| `GND` | Ground | Chân mass / cực âm |
| `BAT` | Battery | Chân pin dự phòng / nguồn backup |

## 6.4 Danh sách chân cụm P2

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `DS` | Data / Extension | Chân mở rộng liên quan DS trên module |
| `SCL` | I2C Clock | Chân clock của giao tiếp I2C |
| `SDA` | I2C Data | Chân dữ liệu của giao tiếp I2C |
| `VCC` | Power | Chân cấp nguồn |
| `GND` | Ground | Chân mass / cực âm |

## 6.5 Thành phần nhận diện trên module

| Thành phần | Ý nghĩa |
|---|---|
| `DS1307ZN` | Chip đồng hồ thời gian thực |
| `AT24C32N` | Chip EEPROM lưu trữ nhỏ đi kèm trên module |
| Khe pin / battery holder | Vị trí gắn pin dự phòng cho RTC |
| Thạch anh | Linh kiện dao động thời gian |
| Cụm điện trở/ký hiệu R | Linh kiện phụ trợ trên mạch |

## 6.6 Thuật ngữ chân

| Thuật ngữ | Ý nghĩa |
|---|---|
| `RTC` | Real-Time Clock, đồng hồ thời gian thực |
| `I2C` | Giao tiếp hai dây gồm SDA và SCL |
| `SDA` | Serial Data, đường dữ liệu I2C |
| `SCL` | Serial Clock, đường xung clock I2C |
| `SQ` | Square Wave, tín hiệu xung vuông |
| `BAT` | Battery, nguồn pin dự phòng |
| `EEPROM` | Bộ nhớ nhỏ có thể lưu dữ liệu khi mất điện |

---

# 7. LED đỏ

## 7.1 Vai trò

LED đỏ dùng để biểu diễn trạng thái cảnh báo.

## 7.2 Số chân

LED đỏ có **2 chân**.

## 7.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| Anode | Positive | Cực dương của LED |
| Cathode | Negative | Cực âm của LED |

## 7.4 Cách nhận diện vật lý

| Dấu hiệu | Ý nghĩa thường gặp |
|---|---|
| Chân dài hơn | Anode |
| Chân ngắn hơn | Cathode |
| Cạnh vát/phẳng trên thân LED | Thường gần phía Cathode |

---

# 8. LED xanh

## 8.1 Vai trò

LED xanh dùng để biểu diễn trạng thái hệ thống bình thường.

## 8.2 Số chân

LED xanh có **2 chân**.

## 8.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| Anode | Positive | Cực dương của LED |
| Cathode | Negative | Cực âm của LED |

## 8.4 Cách nhận diện vật lý

| Dấu hiệu | Ý nghĩa thường gặp |
|---|---|
| Chân dài hơn | Anode |
| Chân ngắn hơn | Cathode |
| Cạnh vát/phẳng trên thân LED | Thường gần phía Cathode |

---

# 9. Điện trở LED

## 9.1 Vai trò

Điện trở dùng kèm LED để giới hạn dòng điện trong mạch LED.

## 9.2 Số chân

Mỗi điện trở có **2 chân**.

## 9.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| Chân 1 | Passive terminal | Một đầu của điện trở |
| Chân 2 | Passive terminal | Đầu còn lại của điện trở |

## 9.4 Ghi chú nhận diện

Điện trở là linh kiện không phân cực.

**Không phân cực** nghĩa là hai chân điện trở không có chiều âm/dương cố định.

---

# 10. Còi buzzer 2 chân

## 10.1 Vai trò

Buzzer dùng để tạo âm thanh cảnh báo tại chỗ.

## 10.2 Số chân

Buzzer trong dự án là loại **2 chân**.

## 10.3 Danh sách chân

| Tên chân | Nhóm chân | Ý nghĩa |
|---|---|---|
| `+` | Positive | Cực dương của buzzer |
| `-` | Negative | Cực âm của buzzer |

## 10.4 Cách nhận diện vật lý

| Dấu hiệu | Ý nghĩa thường gặp |
|---|---|
| Ký hiệu `+` trên thân buzzer | Chân dương |
| Chân còn lại | Chân âm |

---

# 11. Tóm tắt nhanh chân của từng thiết bị

| Thiết bị | Các chân |
|---|---|
| ESP32-CAM | `5V`, `GND`, `GPIO12`, `GPIO13`, `GPIO15`, `GPIO14`, `GPIO2`, `GPIO4`, `3.3V`, `GPIO16`, `GPIO0`, `GND`, `VCC`, `GPIO3`, `GPIO1`, `GND` |
| LDR MH-Sensor-Series | `VCC`, `GND`, `DO`, `AO` |
| HY-SRF05 | `VCC`, `Trig`, `Echo`, `OUT`, `GND` |
| PIR HC-SR501 | `GND`, `Output/OUT`, `VCC` |
| Tiny RTC DS1307 P1 | `SQ`, `DS`, `SCL`, `SDA`, `VCC`, `GND`, `BAT` |
| Tiny RTC DS1307 P2 | `DS`, `SCL`, `SDA`, `VCC`, `GND` |
| LED đỏ | `Anode`, `Cathode` |
| LED xanh | `Anode`, `Cathode` |
| Điện trở LED | `Chân 1`, `Chân 2` |
| Buzzer 2 chân | `+`, `-` |

---

# 12. Quy tắc phạm vi cho tài liệu này

Tài liệu này chỉ trả lời câu hỏi:

```text
Thiết bị này có những chân gì?
Mỗi chân có ý nghĩa gì?
```

Tài liệu này không trả lời các câu hỏi sau:

```text
Chân này nối vào đâu?
Nên dùng GPIO nào?
Có cần điện trở không?
Có cần level shifter không?
Có nguy hiểm điện áp không?
Sơ đồ breadboard nên vẽ thế nào?
```

Các nội dung trên sẽ được viết trong tài liệu thiết kế wiring/schematic riêng ở bước tiếp theo.
