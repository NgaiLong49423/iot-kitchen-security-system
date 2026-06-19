# TÀI LIỆU DEMO 3 KỊCH BẢN

**Project:** IoT Based Anti-Theft System  
**Nhóm:** Group 6 - SE2034  
**Phiên bản tài liệu:** v1.0  
**Mục tiêu:** Chuẩn bị demo 3 kịch bản dễ triển khai, dễ thuyết trình, ít rủi ro phần cứng cho buổi báo cáo thứ 2.

---

## 1. Mục tiêu demo

Tài liệu này mô tả 3 kịch bản demo chính của hệ thống chống trộm IoT đa cảm biến cho khu vực phòng bếp.

Ba kịch bản được chọn gồm:

| STT | Kịch bản | Mục đích |
| --- | --- | --- |
| 1 | SOS trên điện thoại | Cho phép chủ nhà chủ động kích hoạt cảnh báo khẩn cấp từ xa |
| 2 | Cảnh báo cháy bằng Flame Sensor | Phát hiện sớm dấu hiệu có lửa trong khu vực bếp |
| 3 | Chống phá hoại bằng LDR + PIR | Phát hiện hành vi cố tình che cảm biến hoặc vô hiệu hóa thiết bị |

Lý do chọn 3 kịch bản này:

- Ít module hơn so với demo full hệ thống.
- Dễ dựng tình huống thực tế trước lớp.
- Giảm rủi ro lỗi dây, lỗi nguồn và lỗi cảm biến.
- Dễ giải thích cho giảng viên vì mỗi kịch bản có đầu vào, xử lý và đầu ra rõ ràng.

---

## 2. Danh sách module cần nối

### 2.1. Module bắt buộc

| STT | Module | Vai trò trong demo |
| --- | --- | --- |
| 1 | ESP32-CAM | Vi điều khiển chính, xử lý cảm biến, kết nối WiFi/Cloud |
| 2 | Flame Sensor | Phát hiện dấu hiệu lửa trong kịch bản cảnh báo cháy |
| 3 | LDR Light Sensor | Phát hiện ánh sáng bị che đột ngột trong kịch bản chống phá hoại |
| 4 | PIR 5050 | Phát hiện chuyển động trong kịch bản chống phá hoại |
| 5 | LED đỏ | Hiển thị trạng thái cảnh báo |
| 6 | Buzzer 1407 | Phát âm thanh cảnh báo |
| 7 | Nguồn 5V 2A | Cấp nguồn ổn định cho ESP32-CAM và các module cần 5V |

### 2.2. Module chưa cần nối trong demo này

| Module | Lý do tạm thời chưa dùng |
| --- | --- |
| RFID RC522 | Không nằm trong 3 kịch bản demo đã chọn |
| DS1307 RTC | Chưa bắt buộc nếu demo không cần thời gian thực chính xác |
| HY-SRF05 / HC-SR04 | Không dùng trong SOS, báo cháy và chống phá hoại tối giản |
| Camera gửi ảnh | Có thể để làm chức năng phụ nếu còn thời gian |

> Ghi chú: Việc giảm số lượng module giúp mạch gọn hơn, dễ kiểm tra hơn và giảm nguy cơ lỗi khi demo trước giảng viên.

---

## 3. Bảng nối dây đề xuất

> Lưu ý: Cần kiểm tra lại pinout thực tế của board ESP32-CAM đang sử dụng trước khi cắm dây. Không cắm/rút dây khi mạch đang có điện.

### 3.1. Nguồn chung

| Nguồn | Nối đến |
| --- | --- |
| Adapter 5V 2A cực dương | Chân 5V của ESP32-CAM |
| Adapter 5V 2A cực âm | Chân GND của ESP32-CAM |
| GND của các module | Nối chung với GND của ESP32-CAM |

Tất cả module phải dùng chung GND để tín hiệu đọc được chính xác.

---

### 3.2. Flame Sensor

| Flame Sensor | ESP32-CAM |
| --- | --- |
| VCC | 3.3V |
| GND | GND |
| DO | GPIO4 |

**DO** là `Digital Output`, nghĩa là ngõ ra số. Cảm biến chỉ trả về hai trạng thái chính: có phát hiện lửa hoặc không phát hiện lửa.

---

### 3.3. LDR Light Sensor

| LDR | ESP32-CAM |
| --- | --- |
| VCC | 3.3V |
| GND | GND |
| AO | GPIO33 |

**AO** là `Analog Output`, nghĩa là ngõ ra tương tự. Giá trị thay đổi theo mức sáng môi trường, phù hợp để phát hiện ánh sáng bị che đột ngột.

Nếu board không có GPIO33 hoặc không đọc analog được, có thể dùng chân `DO` của module LDR thay cho `AO`. Khi đó cần chỉnh biến trở trên module LDR để đặt ngưỡng sáng/tối.

---

### 3.4. PIR 5050

| PIR 5050 | ESP32-CAM |
| --- | --- |
| VCC | 5V |
| GND | GND |
| OUT | GPIO13 |

Trước khi nối chân OUT vào ESP32-CAM, nên đo điện áp OUT bằng đồng hồ đo điện:

- Nếu OUT khoảng 3.3V: có thể nối trực tiếp.
- Nếu OUT gần 5V: cần dùng mạch chia áp trước khi đưa vào GPIO.

---

### 3.5. LED đỏ

| LED đỏ | ESP32-CAM |
| --- | --- |
| Anode (+) | GPIO12 qua điện trở 220Ω |
| Cathode (-) | GND |

Điện trở 220Ω là bắt buộc để giới hạn dòng, tránh làm hỏng LED hoặc GPIO.

---

### 3.6. Buzzer 1407

#### Phương án test nhanh

| Buzzer | ESP32-CAM |
| --- | --- |
| + | GPIO15 |
| - | GND |

Phương án này chỉ nên dùng nếu buzzer nhỏ và dòng tiêu thụ thấp.

#### Phương án an toàn hơn

Nên dùng transistor để điều khiển buzzer:

```text
GPIO15 -> điện trở 1kΩ -> chân B transistor
Buzzer + -> 5V
Buzzer - -> chân C transistor
Chân E transistor -> GND
GND chung toàn mạch
```

**Transistor** là linh kiện đóng/ngắt dòng điện. Nói dễ hiểu: GPIO chỉ đưa tín hiệu điều khiển, còn transistor chịu dòng cho buzzer, giúp bảo vệ ESP32-CAM.

---

## 4. Kịch bản demo 1: SOS trên điện thoại

### 4.1. Mục đích

Cho phép chủ nhà chủ động kích hoạt cảnh báo khẩn cấp từ điện thoại khi phát hiện tình huống nguy hiểm.

### 4.2. Module sử dụng

| Module | Vai trò |
| --- | --- |
| ESP32-CAM | Nhận lệnh SOS từ Cloud |
| Arduino IoT Cloud / App điện thoại | Giao diện bấm nút SOS |
| LED đỏ | Hiển thị cảnh báo |
| Buzzer | Phát âm thanh cảnh báo |
| WiFi | Kết nối ESP32-CAM với Cloud |

### 4.3. Điều kiện trước khi demo

- ESP32-CAM đã kết nối WiFi thành công.
- Thiết bị đã Online trên Arduino IoT Cloud.
- Giao diện điện thoại có nút SOS.
- LED và buzzer đã test riêng trước đó.

### 4.4. Luồng demo

```text
Người trình diễn mở app Arduino IoT Cloud
        ↓
Nhấn nút SOS
        ↓
ESP32-CAM nhận lệnh từ Cloud
        ↓
LED đỏ nhấp nháy
        ↓
Buzzer phát cảnh báo
        ↓
Nếu có cấu hình email: hệ thống gửi email cảnh báo SOS
        ↓
Người trình diễn nhấn nút Tắt SOS
        ↓
LED và buzzer dừng hoạt động
```

### 4.5. Kết quả mong đợi

- Khi nhấn SOS, thiết bị phản hồi gần như ngay lập tức.
- LED đỏ nháy rõ ràng.
- Buzzer phát âm thanh cảnh báo.
- Nếu có tích hợp Google Apps Script, email cảnh báo được gửi đến người nhận đã cấu hình.

### 4.6. Câu thuyết trình gợi ý

> “Ở kịch bản đầu tiên, nhóm em mô phỏng tình huống chủ nhà phát hiện nguy hiểm và chủ động kích hoạt SOS từ điện thoại. Lệnh SOS được gửi qua Arduino IoT Cloud đến ESP32-CAM. Sau khi nhận lệnh, thiết bị kích hoạt còi và đèn LED để cảnh báo tại chỗ. Nếu hệ thống có Internet, thiết bị cũng có thể gửi email cảnh báo đến người nhà.”

---

## 5. Kịch bản demo 2: Cảnh báo cháy bằng Flame Sensor

### 5.1. Mục đích

Phát hiện sớm dấu hiệu xuất hiện ngọn lửa trong khu vực phòng bếp và cảnh báo người dùng.

### 5.2. Module sử dụng

| Module | Vai trò |
| --- | --- |
| ESP32-CAM | Đọc tín hiệu từ Flame Sensor và xử lý cảnh báo |
| Flame Sensor | Phát hiện dấu hiệu lửa |
| LED đỏ | Hiển thị trạng thái nguy hiểm |
| Buzzer | Phát âm thanh cảnh báo |
| WiFi / Email | Gửi cảnh báo từ xa nếu có cấu hình |

### 5.3. Điều kiện trước khi demo

- Flame Sensor đã được cấp 3.3V.
- Chân DO của Flame Sensor đã đọc được trạng thái HIGH/LOW.
- LED và buzzer đã hoạt động.
- Không đặt lửa sát dây điện, breadboard hoặc ESP32-CAM.

### 5.4. Luồng demo

```text
Người trình diễn đưa nguồn lửa nhỏ hoặc nguồn hồng ngoại lại gần Flame Sensor
        ↓
Flame Sensor thay đổi trạng thái DO
        ↓
ESP32-CAM đọc tín hiệu cảnh báo cháy
        ↓
LED đỏ bật hoặc nhấp nháy
        ↓
Buzzer phát âm thanh cảnh báo
        ↓
Nếu có cấu hình email: gửi email cảnh báo cháy
        ↓
Đưa nguồn lửa ra xa
        ↓
Hệ thống trở về trạng thái bình thường sau vài giây
```

### 5.5. Kết quả mong đợi

- Khi cảm biến phát hiện lửa, LED và buzzer kích hoạt.
- Hệ thống có thể gửi cảnh báo qua Cloud/email.
- Khi không còn tín hiệu lửa, hệ thống có thể dừng cảnh báo hoặc giữ trạng thái cảnh báo tùy logic code.

### 5.6. Lưu ý an toàn khi demo

- Không dí bật lửa quá gần cảm biến.
- Không để lửa gần dây điện, breadboard, pin hoặc module ESP32-CAM.
- Có thể dùng remote hồng ngoại hoặc nguồn sáng hồng ngoại để thay thế bật lửa nếu muốn an toàn hơn.
- Chuẩn bị sẵn thao tác tắt nguồn nếu có mùi khét hoặc module nóng bất thường.

### 5.7. Câu thuyết trình gợi ý

> “Ở kịch bản thứ hai, nhóm em bổ sung chức năng cảnh báo cháy vì hệ thống được đặt trong khu vực phòng bếp. Flame Sensor sẽ phát hiện dấu hiệu ánh sáng hồng ngoại từ ngọn lửa. Khi có nguy cơ cháy, ESP32-CAM kích hoạt còi, đèn LED và có thể gửi cảnh báo qua Internet để người dùng xử lý kịp thời.”

---

## 6. Kịch bản demo 3: Chống phá hoại bằng LDR + PIR

### 6.1. Mục đích

Phát hiện hành vi cố tình che cảm biến hoặc vô hiệu hóa thiết bị, ví dụ dùng tay, áo hoặc băng keo đen che cảm biến ánh sáng.

### 6.2. Module sử dụng

| Module | Vai trò |
| --- | --- |
| ESP32-CAM | Xử lý logic chống phá hoại |
| LDR | Phát hiện ánh sáng tụt đột ngột |
| PIR 5050 | Xác nhận có chuyển động gần thiết bị |
| LED đỏ | Hiển thị trạng thái cảnh báo |
| Buzzer | Phát âm thanh cảnh báo |

### 6.3. Điều kiện trước khi demo

- LDR đọc được giá trị ánh sáng bình thường.
- PIR đọc được chuyển động khi có người quơ tay.
- Hệ thống đã có ngưỡng phát hiện ánh sáng giảm mạnh.
- LED và buzzer đã hoạt động.

### 6.4. Logic xử lý đề xuất

Hệ thống chỉ báo động khi có đủ 2 điều kiện:

```text
1. LDR phát hiện ánh sáng giảm đột ngột
2. PIR phát hiện có chuyển động gần thiết bị
```

Cách này giúp giảm báo động giả. Ví dụ, nếu ánh sáng phòng giảm từ từ do tắt đèn thì chưa chắc là phá hoại. Nhưng nếu ánh sáng tụt mạnh trong thời gian rất ngắn và có chuyển động gần thiết bị, hệ thống có cơ sở để xác định có hành vi can thiệp.

### 6.5. Luồng demo

```text
Ban đầu LDR đang nhận ánh sáng bình thường
        ↓
Người trình diễn dùng tay hoặc khăn che LDR đột ngột
        ↓
LDR ghi nhận ánh sáng giảm mạnh
        ↓
Người trình diễn quơ tay trước PIR
        ↓
PIR phát hiện chuyển động
        ↓
ESP32-CAM xác nhận hành vi phá hoại
        ↓
LED đỏ nhấp nháy
        ↓
Buzzer hú cảnh báo
        ↓
Nếu có cấu hình Cloud/email: gửi cảnh báo phá hoại thiết bị
```

### 6.6. Kết quả mong đợi

- Khi chỉ che LDR nhẹ hoặc ánh sáng thay đổi từ từ, hệ thống không báo động ngay.
- Khi LDR bị che đột ngột và PIR phát hiện chuyển động, hệ thống kích hoạt cảnh báo.
- LED và buzzer phản hồi rõ ràng để người xem dễ quan sát.

### 6.7. Câu thuyết trình gợi ý

> “Ở kịch bản thứ ba, nhóm em mô phỏng tình huống kẻ gian cố tình che cảm biến để vô hiệu hóa hệ thống. Nếu chỉ dựa vào một cảm biến thì dễ báo động giả, nên nhóm em kết hợp LDR và PIR. Khi ánh sáng giảm đột ngột và đồng thời có chuyển động gần thiết bị, hệ thống xác định đây là hành vi phá hoại và kích hoạt cảnh báo.”

---

## 7. Thứ tự test trước ngày demo

Để tránh lỗi khi lên trình bày, nên test theo thứ tự sau:

| Thứ tự | Nội dung test | Kết quả cần đạt |
| --- | --- | --- |
| 1 | ESP32-CAM chạy độc lập | Board lên nguồn, kết nối WiFi/Cloud ổn định |
| 2 | LED đỏ | LED bật/tắt đúng theo code |
| 3 | Buzzer | Buzzer kêu/tắt đúng theo code, không làm ESP32 reset |
| 4 | Flame Sensor | Phát hiện lửa hoặc nguồn hồng ngoại đúng |
| 5 | LDR | Đọc được sáng/tối hoặc ánh sáng thay đổi |
| 6 | PIR | Phát hiện chuyển động ổn định |
| 7 | SOS | Bấm trên điện thoại, thiết bị phản hồi đúng |
| 8 | Cảnh báo cháy | Flame Sensor kích hoạt LED/buzzer/email |
| 9 | Chống phá hoại | Che LDR + quơ tay PIR thì báo động |
| 10 | Test tổng hợp | Chạy liên tục cả 3 kịch bản không reset, không treo |

---

## 8. Checklist trước khi lên demo

### 8.1. Phần cứng

- [ ] ESP32-CAM đã nạp code mới nhất.
- [ ] Nguồn 5V 2A hoạt động ổn định.
- [ ] Tất cả module đã nối chung GND.
- [ ] Flame Sensor dùng 3.3V, không dùng 5V.
- [ ] LDR dùng 3.3V.
- [ ] PIR OUT đã kiểm tra mức điện áp an toàn.
- [ ] LED đỏ có điện trở 220Ω.
- [ ] Buzzer không làm ESP32-CAM reset khi kêu.
- [ ] Không có dây lỏng hoặc dây chạm nhau.
- [ ] Module không nóng bất thường sau khi chạy thử.

### 8.2. Phần mềm

- [ ] ESP32-CAM kết nối WiFi ổn định.
- [ ] Arduino IoT Cloud hiển thị thiết bị Online.
- [ ] Nút SOS trên app hoạt động.
- [ ] Code đọc Flame Sensor ổn định.
- [ ] Code đọc LDR ổn định.
- [ ] Code đọc PIR ổn định.
- [ ] Có cơ chế chống spam cảnh báo.
- [ ] Có biến tắt báo động sau khi demo xong.
- [ ] Nếu dùng email, Google Apps Script Web App hoạt động.

### 8.3. Khi trình diễn

- [ ] Có điện thoại mở sẵn app điều khiển.
- [ ] Có nguồn lửa/remote hồng ngoại để test Flame Sensor.
- [ ] Có khăn hoặc tay để che LDR.
- [ ] Có người quơ tay trước PIR đúng vị trí.
- [ ] Có phương án dự phòng nếu WiFi yếu.
- [ ] Có thể demo bằng LED nếu buzzer gặp lỗi.

---

## 9. Rủi ro và phương án dự phòng

| Rủi ro | Nguyên nhân có thể | Cách xử lý nhanh |
| --- | --- | --- |
| ESP32-CAM Offline | WiFi yếu, nguồn yếu, sai cấu hình Cloud | Reset board, kiểm tra WiFi, dùng hotspot điện thoại |
| Buzzer làm board reset | Buzzer ăn dòng cao | Tắt buzzer, demo bằng LED hoặc dùng transistor |
| Flame Sensor không nhận lửa | Sai ngưỡng, khoảng cách quá xa, sai chân DO | Chỉnh biến trở trên module, kiểm tra Serial Monitor |
| PIR không nhận chuyển động | Cảm biến cần thời gian ổn định | Chờ 30-60 giây sau khi cấp nguồn |
| LDR đọc không thay đổi | Sai chân AO/DO hoặc sai ngưỡng | Kiểm tra Serial Monitor, chỉnh biến trở nếu dùng DO |
| Email không gửi | Lỗi WiFi hoặc Google Apps Script URL | Demo cảnh báo tại chỗ bằng LED/buzzer trước |

---

## 10. Kịch bản thuyết trình tổng thể

Có thể trình bày theo thứ tự sau:

```text
1. Giới thiệu mục tiêu hệ thống
2. Giới thiệu các module đang dùng trong bản demo
3. Demo SOS trên điện thoại
4. Demo cảnh báo cháy bằng Flame Sensor
5. Demo chống phá hoại bằng LDR + PIR
6. Giải thích vì sao dùng nhiều cảm biến để giảm báo động giả
7. Kết luận: hệ thống có khả năng cảnh báo tại chỗ và cảnh báo từ xa
```

### Đoạn mở đầu gợi ý

> “Dự án của nhóm em là hệ thống chống trộm IoT đa cảm biến cho khu vực phòng bếp. Trong buổi demo hôm nay, nhóm em chọn 3 kịch bản có tính thực tế cao và dễ quan sát: kích hoạt SOS từ điện thoại, cảnh báo cháy bằng Flame Sensor và phát hiện hành vi phá hoại thiết bị bằng LDR kết hợp PIR.”

### Đoạn kết luận gợi ý

> “Qua 3 kịch bản trên, hệ thống cho thấy khả năng phản ứng với cả tình huống chủ động từ người dùng và tình huống tự động từ cảm biến. Thiết bị không chỉ phát cảnh báo tại chỗ bằng LED và buzzer mà còn có thể mở rộng để gửi cảnh báo qua Cloud hoặc email, phù hợp với mục tiêu xây dựng hệ thống an ninh IoT cho khu vực phòng bếp.”

---

## 11. Ghi chú kỹ thuật

- `GPIO` là chân vào/ra của ESP32-CAM, dùng để đọc cảm biến hoặc điều khiển LED, buzzer.
- `Digital Output` là tín hiệu số, thường chỉ có HIGH/LOW tương ứng với 1/0.
- `Analog Output` là tín hiệu tương tự, giá trị thay đổi liên tục theo môi trường.
- `Cloud` là nền tảng đám mây, giúp điện thoại và ESP32-CAM giao tiếp qua Internet.
- `Debounce` là kỹ thuật chống nhiễu/chống kích hoạt liên tục, giúp hệ thống không gửi quá nhiều cảnh báo trong thời gian ngắn.
- `Sensor Fusion` là kết hợp nhiều cảm biến để đưa ra quyết định chính xác hơn, ví dụ LDR + PIR để xác nhận hành vi phá hoại.

---

## 12. Kết luận

Bản demo 3 kịch bản này ưu tiên tính ổn định, dễ hiểu và an toàn phần cứng. Nhóm không cần nối toàn bộ module ngay từ đầu, mà tập trung vào các thành phần phục vụ trực tiếp cho phần trình diễn.

Ba kịch bản được chọn giúp thể hiện rõ các năng lực chính của hệ thống:

1. Chủ động cảnh báo từ xa qua điện thoại.
2. Tự động phát hiện nguy cơ cháy trong phòng bếp.
3. Tự động phát hiện hành vi phá hoại thiết bị bằng cách kết hợp cảm biến.

Nếu còn thời gian, nhóm có thể mở rộng thêm chức năng gửi email, chụp ảnh bằng ESP32-CAM hoặc ghi log hoạt động để tăng điểm thuyết phục khi bảo vệ.
