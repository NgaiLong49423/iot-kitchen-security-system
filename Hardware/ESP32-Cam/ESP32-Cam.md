# Tài liệu tham khảo sơ đồ chân ESP32-CAM

**Tác giả:** Amrit Prabhu
**Cập nhật lần cuối:** Ngày 20 tháng 1 năm 2026

---

## Mục lục

* [Giới thiệu](#giới-thiệu)
* [Sơ đồ chân ESP32-CAM](#sơ-đồ-chân-esp32-cam)
* [Chân GPIO](#chân-gpio)
* [Các chân cắm thẻ MicroSD](#các-chân-cắm-thẻ-microsd)
* [Các chân ADC](#các-chân-adc)
* [Chân cảm ứng](#chân-cảm-ứng)
* [Chân SPI](#chân-spi)
* [Các chân UART](#các-chân-uart)
* [Chân PWM](#chân-pwm)
* [Các chân GPIO của RTC](#các-chân-gpio-của-rtc)
* [Chân nguồn](#chân-nguồn)

---

## Giới thiệu

Không thể phủ nhận, ESP32-CAM là một thiết bị mạnh mẽ với camera tích hợp và hỗ trợ WiFi. Tuy nhiên, ESP32-CAM có ít chân I/O hơn, một số chân được dùng chung với thẻ SD và do đó không thể sử dụng khi thẻ được kết nối, gây khó khăn trong việc thiết kế dự án xoay quanh nó.

Có một vài điều bạn cần lưu ý, vì vậy vui lòng đọc kỹ sơ đồ chân cắm.

> [!NOTE]
> Sơ đồ chân cắm trong tài liệu này dành cho bo mạch ESP32-CAM phổ biến của AI-Thinker.

---

## Sơ đồ chân ESP32-CAM

ESP32-CAM có tổng cộng 16 chân. Để thuận tiện, các chân có chức năng tương tự được nhóm lại với nhau.

Sơ đồ chân như sau:

> **Hình ảnh:** Sơ đồ chân cắm ESP32-CAM.

Tìm hiểu sâu hơn về chức năng của từng chân cắm.

---

## Chân GPIO

Chip ESP32-S có tổng cộng 32 chân GPIO, nhưng vì nhiều chân trong số đó được sử dụng nội bộ cho camera và PSRAM nên ESP32-CAM chỉ có 10 chân GPIO khả dụng.

Các chân này có thể được gán nhiều chức năng ngoại vi khác nhau, chẳng hạn như:

* UART
* SPI
* ADC
* Touch

Việc gán chức năng được thực hiện bằng cách lập trình các thanh ghi thích hợp.

> **Hình ảnh:** Các chân GPIO của ESP32-CAM.
![alt text](<Screenshot 2026-06-12 153957.png>)

### Các chân GPIO nào của ESP32-CAM an toàn để sử dụng?

Mặc dù ESP32-CAM có 10 chân GPIO với nhiều chức năng khác nhau, một số chân có thể không phù hợp với dự án của bạn.

Có thể chia các chân GPIO thành ba nhóm:

* **Ưu tiên sử dụng:** Các chân hoàn toàn an toàn để sử dụng.
* **Sử dụng thận trọng:** Hành vi của các chân, đặc biệt trong quá trình khởi động, có thể khó đoán trước. Một số chân GPIO còn được dùng chung với thẻ nhớ microSD.
* **Nên tránh sử dụng:** Những chân có thể gây ảnh hưởng đến quá trình khởi động, nạp chương trình hoặc hoạt động của bo mạch.

| Nhãn | GPIO | Có an toàn để sử dụng không? | Lý do                                                                                                                                                        |
| ---- | ---: | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| D0   |    0 | Sử dụng thận trọng           | Phải ở mức **HIGH** trong quá trình khởi động và mức **LOW** khi nạp chương trình.                                                                           |
| TX0  |    1 | Sử dụng thận trọng           | Chân TX, được sử dụng để nạp firmware và gỡ lỗi.                                                                                                             |
| D2   |    2 | Sử dụng thận trọng           | Phải ở mức **LOW** trong quá trình khởi động và không thể sử dụng khi có thẻ microSD.                                                                        |
| RX0  |    3 | Sử dụng thận trọng           | Chân RX, được sử dụng để nạp firmware và gỡ lỗi.                                                                                                             |
| D4   |    4 | Sử dụng thận trọng           | Được kết nối với đèn LED flash tích hợp và không thể sử dụng khi có thẻ nhớ microSD.                                                                         |
| D12  |   12 | Sử dụng thận trọng           | Phải ở mức **LOW** trong quá trình khởi động và không thể sử dụng khi có thẻ microSD.                                                                        |
| D13  |   13 | Sử dụng thận trọng           | Không thể sử dụng khi có thẻ nhớ microSD.                                                                                                                    |
| D14  |   14 | Sử dụng thận trọng           | Không thể sử dụng khi có thẻ nhớ microSD.                                                                                                                    |
| D15  |   15 | Sử dụng thận trọng           | Phải ở mức **HIGH** trong quá trình khởi động. Nếu kéo xuống mức **LOW**, chân này sẽ ngăn việc ghi nhật ký khởi động. Không thể sử dụng khi có thẻ microSD. |
| RX2  |   16 | An toàn                      | Có thể sử dụng làm chân GPIO thông thường hoặc chân RX của UART2.                                                                                            |

> **Hình ảnh:** Các chân GPIO của ESP32-CAM an toàn để sử dụng.

---

### Chân GPIO 0

GPIO 0 là một trong những chân quan trọng nhất của ESP32-CAM. Chân này quyết định ESP32 đang hoạt động ở chế độ bình thường hay chế độ nạp chương trình.

GPIO 0 phải ở:

* Mức **HIGH** trong quá trình khởi động bình thường.
* Mức **LOW** trong quá trình nạp chương trình.

Chân này được kéo lên bên trong bằng một điện trở `10 kΩ`.

Khi kết nối GPIO 0 với GND, ESP32 sẽ vào chế độ nạp chương trình và bạn có thể tải mã nguồn lên bo mạch. Sau khi hoàn tất việc lập trình, bạn phải ngắt kết nối GPIO 0 khỏi GND.

> [!WARNING]
> Bạn phải kết nối GPIO 0 với GND mỗi khi muốn tải một chương trình mới lên ESP32-CAM. Sau khi tải xong, cần ngắt kết nối và khởi động lại bo mạch.

---

### GPIO 33 – Đèn LED đỏ tích hợp

Ở mặt sau của ESP32-CAM có một đèn LED nhỏ màu đỏ được sử dụng để báo trạng thái. Đèn LED này có thể được lập trình và được kết nối với chân GPIO 33.

Đèn LED hoạt động theo nguyên lý **logic đảo ngược**:

* Gửi tín hiệu `LOW` để bật đèn LED.
* Gửi tín hiệu `HIGH` để tắt đèn LED.

---

### GPIO 4 – Đèn flash máy ảnh

ESP32-CAM có một đèn LED trắng với độ sáng cao. Đèn này được thiết kế để sử dụng làm đèn flash của camera, nhưng cũng có thể được sử dụng để chiếu sáng thông thường.

Đèn LED flash được kết nối nội bộ với chân GPIO 4.

---

## Các chân cắm thẻ MicroSD

Các chân sau đây được sử dụng để giao tiếp với thẻ nhớ microSD.

Nếu không sử dụng thẻ nhớ microSD, bạn có thể sử dụng các chân này như những chân đầu vào và đầu ra thông thường.

| Chức năng MicroSD |    GPIO |
| ----------------- | ------: |
| CLK               | GPIO 14 |
| CMD               | GPIO 15 |
| DATA0             |  GPIO 2 |
| DATA1             |  GPIO 4 |
| DATA2             | GPIO 12 |
| DATA3             | GPIO 13 |

> **Hình ảnh:** Các chân cắm thẻ microSD của ESP32-CAM.

> [!WARNING]
> Khi sử dụng thẻ nhớ microSD, các chân GPIO 2, 4, 12, 13, 14 và 15 sẽ được dùng cho việc giao tiếp với thẻ nhớ và không nên sử dụng cho thiết bị ngoại vi khác.

---

## Các chân ADC

Trên ESP32-CAM, chỉ có các chân ADC2 được đưa ra ngoài.

Tuy nhiên, các chân ADC2 được sử dụng nội bộ bởi trình điều khiển WiFi. Vì vậy, chúng không thể được sử dụng để đọc tín hiệu analog khi WiFi đang được bật.

> **Hình ảnh:** Các chân ADC của ESP32-CAM.

> [!CAUTION]
> Không nên sử dụng ADC2 để đọc cảm biến analog trong khi ESP32-CAM đang kết nối WiFi vì có thể xảy ra xung đột tài nguyên.

---

## Chân cảm ứng

ESP32-CAM có 7 chân GPIO hỗ trợ cảm ứng điện dung.

Khi một tải điện dung, chẳng hạn như ngón tay người, ở gần chân GPIO, ESP32 có thể phát hiện sự thay đổi điện dung.

> **Hình ảnh:** Các chân cảm ứng của ESP32-CAM.

Bạn có thể tạo một bàn di chuột bằng cách gắn vật liệu dẫn điện vào các chân cảm ứng, chẳng hạn như:

* Giấy nhôm
* Vải dẫn điện
* Sơn dẫn điện
* Miếng kim loại

Nhờ thiết kế ít nhiễu và độ nhạy cao của mạch, có thể tạo ra các bàn di chuột có kích thước tương đối nhỏ.

Ngoài ra, các chân cảm ứng điện dung còn có thể được sử dụng để đánh thức ESP32 khỏi chế độ ngủ sâu.

---

## Chân SPI

ESP32-CAM chỉ có một giao tiếp SPI là VSPI, có thể hoạt động ở cả chế độ slave và master.

SPI là giao thức giao tiếp nối tiếp đồng bộ, thường được sử dụng để kết nối vi điều khiển với cảm biến, màn hình hoặc module ngoại vi.

ESP32-CAM hỗ trợ các tính năng SPI sau:

* Hỗ trợ 4 chế độ định thời truyền dữ liệu SPI.
* Tần số hoạt động lên đến 80 MHz.
* Hỗ trợ các mức xung nhịp được chia từ 80 MHz.
* Bộ đệm FIFO có dung lượng lên đến 64 byte.

> **Hình ảnh:** Các chân SPI của ESP32-CAM.

Các chân SPI thường được sử dụng gồm:

| Chức năng SPI |    GPIO |
| ------------- | ------: |
| MOSI          | GPIO 13 |
| MISO          | GPIO 12 |
| SCK           | GPIO 14 |
| CS            | GPIO 15 |

> [!NOTE]
> Các chân SPI này cũng được sử dụng cho thẻ nhớ microSD. Nếu thẻ microSD đang hoạt động, các chân này không nên được sử dụng cho thiết bị SPI khác nếu chưa có thiết kế chia sẻ bus phù hợp.

---

## Các chân UART

Chip ESP32-S có hai giao diện UART:

* UART0
* UART2

> **Hình ảnh:** Các chân UART của ESP32-CAM.

Tuy nhiên, chỉ có chân RX của UART2, tức GPIO 16, được đưa ra ngoài. Điều này khiến UART0 trở thành giao diện UART đầy đủ duy nhất có thể sử dụng trực tiếp trên ESP32-CAM.

Các chân UART0 gồm:

| Chức năng |   GPIO |
| --------- | -----: |
| TX0       | GPIO 1 |
| RX0       | GPIO 3 |

Do ESP32-CAM không có cổng USB tích hợp, GPIO 1 và GPIO 3 phải được sử dụng để:

* Nạp firmware.
* Gỡ lỗi qua Serial Monitor.
* Kết nối với các thiết bị UART.

Một số thiết bị UART có thể kết nối với ESP32-CAM gồm:

* Module GPS.
* Cảm biến vân tay.
* Cảm biến khoảng cách.
* Module Bluetooth.
* Các vi điều khiển khác.

> [!WARNING]
> Khi sử dụng GPIO 1 và GPIO 3 cho thiết bị UART bên ngoài, thiết bị có thể gây cản trở quá trình nạp chương trình hoặc hiển thị thông tin gỡ lỗi.

---

## Chân PWM

Bo mạch ESP32-CAM có 10 kênh PWM, tương ứng với các chân GPIO khả dụng và được điều khiển bởi bộ điều khiển PWM.

PWM là phương pháp điều chỉnh độ rộng xung, thường được sử dụng để điều khiển độ sáng đèn LED, tốc độ động cơ hoặc vị trí động cơ servo.

Đầu ra PWM có thể được sử dụng để:

* Điều khiển động cơ.
* Điều chỉnh độ sáng đèn LED.
* Điều khiển servo.
* Tạo dạng sóng tín hiệu.

> **Hình ảnh:** Các chân PWM của ESP32-CAM.

Bộ điều khiển PWM bao gồm:

* Các bộ định thời PWM.
* Bộ điều khiển PWM.
* Mô-đun con thu tín hiệu chuyên dụng.

Mỗi bộ định thời cung cấp thời gian ở dạng đồng bộ hoặc độc lập. Mỗi bộ điều khiển PWM tạo ra một dạng sóng cho một kênh PWM.

Mô-đun con thu tín hiệu chuyên dụng có khả năng thu tín hiệu và xác định chính xác thời gian của các sự kiện bên ngoài.

---

## Các chân GPIO của RTC

Các chân GPIO được kết nối với hệ thống con RTC công suất thấp được gọi là GPIO RTC.

RTC là khối thời gian thực và công suất thấp của ESP32, có thể tiếp tục hoạt động trong khi phần lớn vi điều khiển đang ở chế độ ngủ sâu.

Các chân GPIO RTC có thể được sử dụng để đánh thức ESP32 khỏi chế độ ngủ sâu khi bộ đồng xử lý công suất cực thấp ULP đang hoạt động.

Các chân GPIO được đánh dấu trong sơ đồ có thể được sử dụng làm nguồn đánh thức bên ngoài.

> **Hình ảnh:** Các chân GPIO RTC của ESP32-CAM.

---

## Chân nguồn

ESP32-CAM có hai chân cấp nguồn chính:

* `5V`
* `3V3`

ESP32-CAM có thể được cấp nguồn thông qua chân 3.3V hoặc chân 5V.

Tuy nhiên, nhiều người dùng đã báo cáo thiết bị hoạt động không ổn định khi được cấp nguồn bằng 3.3V. Vì vậy, khuyến cáo nên cấp nguồn cho ESP32-CAM thông qua chân 5V.

Chân VCC thường xuất điện áp 3.3V từ bộ điều chỉnh điện áp tích hợp trên bo mạch. Tuy nhiên, chân này có thể được cấu hình để xuất ra 5V bằng cách sử dụng đường dẫn không trở kháng nằm gần chân VCC.

Chân GND là chân nối đất và được sử dụng làm mức điện áp tham chiếu chung cho toàn bộ mạch.

> **Hình ảnh:** Các chân nguồn của ESP32-CAM.

> [!IMPORTANT]
> Nên sử dụng nguồn 5V ổn định và có khả năng cung cấp đủ dòng điện. Nguồn điện yếu hoặc không ổn định có thể khiến ESP32-CAM khởi động lại, mất kết nối WiFi hoặc không thể sử dụng camera.

---

## Tài liệu tham khảo

Tài liệu được lấy từ:
https://lastminuteengineers.com/esp32-cam-pinout-reference/
