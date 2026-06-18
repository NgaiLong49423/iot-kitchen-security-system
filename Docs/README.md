# KẾ HOẠCH DỰ ÁN NHÓM 6 - LỚP SE2034



| Item         | Context                     |
| ------------ | --------------------------- |
| Project Name | IoT based anti-theft system |
| Version      | 2.0.4                       |
| Prepared By  | Group 6                     |
| Date         | Jun 5, 2026                 |

### Đề tài

**Hệ thống chống trộm IoT đa cảm biến cho khu vực phòng bếp**

### 1. Bảng Đánh Giá Mức Độ Đóng Góp Thành Viên

| STT | Họ và tên         | MSSV     | Vai trò             | Mức độ đóng góp |
| --- | ----------------- | -------- | ------------------- | --------------- |
| 1   | Ngô Gia Long      | SE190732 | Trưởng nhóm         | 100%            |
| 2   | Nguyễn Nhật Anh   | SE192077 | Phụ trách phần cứng | 100%            |
| 3   | Võ Trần Công Danh | SE190824 | Phụ trách phần mềm  | 100%            |
| 4   | Nguyễn An Vương   | SE190996 | Kiểm thử và hỗ trợ  | 100%            |

### 2. Các thiết bị và module được dùng đến trong đề tài

| STT | Thiết bị/ Module                          | Chức năng                                                                                                                                                                                                                                                                          |
| --- | ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1   | RFID RC522                                | Nhận dạng và xác thực người dùng thông qua thẻ RFID hoặc thẻ từ. Cho phép bật/tắt chế độ bảo vệ, vô hiệu hóa báo động đối với người được cấp quyền, đồng thời ghi nhận lịch sử truy cập (thời gian, UID thẻ, trạng thái truy cập) để phục vụ quản lý và theo dõi an ninh hệ thống. |
| 2   | ESP32-CAM Module Wifi ESP32 Camera OV2640 | Vi điều khiển chính của hệ thống, xử lý dữ liệu cảm biến, kết nối WiFi, gửi cảnh báo lên Internet và chụp ảnh hiện trường khi phát hiện xâm nhập.                                                                                                                                  |
| 3   | DS1307                                    | Lưu trữ và cung cấp thời gian thực cho hệ thống, vẫn hoạt động khi mất điện nhờ pin CR2032 dự phòng. Dùng để xác định ngày, giờ xảy ra sự kiện và chế độ bảo vệ ban đêm.                                                                                                           |
| 4   | Ultrasonic Range Finder (HC-SR04)         | Đo khoảng cách đến vật thể bằng sóng siêu âm. Hỗ trợ xác nhận có người xâm nhập và phân biệt người với vật nuôi dựa trên chiều cao hoặc khoảng cách.                                                                                                                               |
| 5   | Light Sensor (LDR)                        | Đo cường độ ánh sáng môi trường. Dùng để phát hiện ánh sáng bất thường (đèn pin của kẻ gian) và phát hiện hành vi che khuất cảm biến.                                                                                                                                              |
| 6   | Còi Thụ Động 1407                         | Phát âm thanh cảnh báo khi phát hiện nguy cơ đột nhập hoặc phá hoại hệ thống.                                                                                                                                                                                                      |
| 7   | PIR 5050                                  | Phát hiện chuyển động của con người dựa trên bức xạ hồng ngoại từ cơ thể. Là cảm biến chính dùng để nhận diện sự xuất hiện của người trong khu vực giám sát.                                                                                                                       |
| 8   | Led đỏ                                    | Hiển thị trạng thái cảnh báo, báo động hoặc sự kiện nguy hiểm đang xảy ra.                                                                                                                                                                                                         |
| 9   | Flame Sensor                              | Phát hiện ánh lửa để cảnh báo cháy                                                                                                                                                                                                                                                 |

### 2. Giới Thiệu Đề Tài

Dự án xây dựng một hệ thống an ninh thông minh ứng dụng công nghệ IoT nhằm giám sát khu vực phòng bếp trong gia đình. Hệ thống sử dụng nhiều loại cảm biến để phát hiện các hành vi bất thường, hỗ trợ gửi cảnh báo theo thời gian thực và tăng cường khả năng bảo vệ tài sản.

**Mục tiêu cốt lõi của thiết bị:**

* Phát hiện hành vi xâm nhập bằng nhiều cảm biến đồng thời.
* Giảm thiểu False Positive (Báo động giả) do môi trường hoặc vật nuôi.
* Tự động duy trì trạng thái bảo vệ sau sự cố mất điện.
* Hỗ trợ cảnh báo sớm các đối tượng khả nghi xuất hiện gần khu vực giám sát.
* Tự động kích hoạt hoặc vô hiệu hóa chế độ bảo vệ dựa trên sự hiện diện của người dùng.
* Phát hiện các hành vi cố tình phá hoại hoặc vô hiệu hóa thiết bị.

### 3. Danh Sách Chức Năng Hệ Thống

| STT | Chức năng                           | Mô tả                                                              |
| --- | ----------------------------------- | ------------------------------------------------------------------ |
| 1   | Nhận diện đột nhập đa cảm biến      | Kết hợp PIR, HC-SR04, và LDR để xác nhận hành vi xâm nhập          |
| 2   | Lọc nhiễu vật nuôi                  | Loại bỏ cảnh báo do thú nuôi gây ra                                |
| 3   | Khôi phục trạng thái sau mất điện   | Tự động phục hồi chế độ bảo vệ khi nguồn điện được cấp lại         |
| 4   | Cảnh báo thiết bị WiFi lạ lảng vảng | Phát hiện thiết bị WiFi lạ xuất hiện liên tục gần khu vực giám sát |
| 5   | Tự động kích hoạt/vô hiệu hóa       | Dựa trên sự hiện diện của các thiết bị đã được xác thực            |
| 6   | Phát hiện hành vi phá hoại thiết bị | Nhận diện việc che khuất hoặc can thiệp cảm biến                   |

### 4a. Kịch Bản Trình Diễn Chức Năng (Demo Scenarios)

#### 3.1 Chức năng 1 - Nhận diện đột nhập đa cảm biến

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ 02:15 AM, phòng bếp tối hoàn toàn. Kẻ gian cạy cửa sổ bếp(hoặc của phụ) và dùng đèn pin quét tìm đồ đạc.
  * _Thiết bị:_ ESP32-CAM, PIR (Cảm biến chuyển động), HC-SR04 (Cảm biến siêu âm), LDR (Cảm biến ánh sáng), DS1307 (Đồng hồ thời gian thực), Buzzer (Còi).
  * _Dữ liệu đầu vào:_ Tín hiệu chuyển động, Khoảng cách vật thể ($< 100\text{cm}$), Cường độ sáng thay đổi liên tục, Thời gian thực (02:15).
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Giả lập kẻ gian đứng trước cửa sổ bếp (đưa tay qua lại trước PIR và HC-SR04 $> 30\text{ giây}$), đồng thời rọi đèn pin lướt qua cảm biến LDR.
  * _Kết quả:_ ESP8266 ghi nhận ánh sáng dao động bất thường ($120 \rightarrow 700 \rightarrow 250$). Hệ thống xử lý logic AND cho cả 3 cảm biến, lập tức kích hoạt Buzzer và gửi Notification (Thông báo cảnh báo).
* **Nội dung thuyết trình:** > "Để giải quyết triệt để bài toán False Positive (Báo động giả), hệ thống sử dụng thuật toán Sensor Fusion (Kết hợp cảm biến). Việc kiểm tra song song độ dao động ánh sáng từ LDR và khoảng cách từ HC-SR04 giúp hệ thống xác nhận chính xác 100% đây là hành vi đột nhập dùng đèn pin, loại bỏ hoàn toàn sai số do người nhà bật đèn bếp uống nước."

#### 3.2 Chức năng 2 - Lọc nhiễu vật nuôi

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ 01:00 AM, chó nuôi trong nhà đi vòng quanh mặt sàn phòng bếp để tìm thức ăn.
  * _Thiết bị:_ ESP8266, PIR, HC-SR04, DS1307 (Lắp đặt tại bếp, tầm quét song song mặt đất ở độ cao $80\text{cm}$).
  * _Dữ liệu đầu vào:_ Thời gian (01:00 AM), Tín hiệu chuyển động nguồn nhiệt, Khoảng cách vật cản tầm thấp ($15 - 30\text{ cm}$ so với mặt sàn).
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Đưa một vật cản lướt qua dưới mặt sàn bếp (cách cảm biến $15 - 30\text{ cm}$) để giả lập chú chó đi ngang qua.
  * _Kết quả:_ Đèn LED và Buzzer hoàn toàn im lặng. ESP8266 bỏ qua cảnh báo và chỉ âm thầm ghi Log (Nhật ký hệ thống).

#### 3.3 Chức năng 3 - Khôi phục trạng thái sau mất điện

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ 00:45 AM, kẻ gian cắt cầu dao điện tổng của phòng bếp để vô hiệu hóa an ninh trước khi trèo vào.
  * _Thiết bị:_ ESP8266 (chạy nguồn 220V), DS1307 (chạy độc lập bằng pin dự phòng CR2032).
  * _Dữ liệu đầu vào:_ Trạng thái nguồn điện lưới (Mất/Có), Thời gian thực lấy ngầm từ DS1307.
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Rút nguồn điện chính của mạch (Giả lập cúp điện). Chờ 3 giây, sau đó cắm nguồn lại và lập tức quơ tay trước cảm biến PIR.
  * _Kết quả:_ Khi mất điện, ESP8266 tắt nhưng DS1307 vẫn đếm giờ. Ngay khi có điện, ESP8266 khởi động lại chớp nhoáng, truy vấn DS1307, nhận diện vẫn là ban đêm và lập tức khôi phục Security Mode (Chế độ an ninh), réo còi ngay khi có chuyển động.

#### 3.4 Chức năng 4 - Cảnh báo thiết bị WiFi lạ lảng vảng

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ Một kẻ gian lạ mặt đứng lỳ sát bên ngoài cửa sổ bếp để rình rập, dò xét hệ thống chốt cửa trước khi bẻ khóa.
  * _Thiết bị:_ ESP8266 kích hoạt Promiscuous Mode (Chế độ nhận toàn bộ gói tin trong mạng mà không cần lọc theo địa chỉ MAC).
  * _Dữ liệu đầu vào:_ Probe Requests (Gói tin thăm dò) phát ra từ điện thoại kẻ gian, tín hiệu cường độ sóng, bộ đếm thời gian.
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Dùng 1 điện thoại lạ (không có trong danh sách) đứng gần khu bếp. Mạch bắt liên tục các gói tin phát ra.
  * _Kết quả:_ Khi bộ đếm điện thoại lạ chạm mốc 5 lần xuất hiện trong thời gian quy định, ESP8266 lập tức đẩy một HTTP Request (Yêu cầu siêu văn bản) báo tin nhắn: “Có đối tượng lảng vảng khu vực bếp lúc \[Thời gian thực]”.

#### 3.5 Chức năng 5 - Tự động kích hoạt và vô hiệu hóa chế độ bảo vệ

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ Ban ngày, toàn bộ gia đình rời khỏi nhà, khu vực bếp không có ai. Hệ thống phải tự nhận biết để bảo vệ.
  * _Thiết bị:_ ESP8266 (Quét MAC WiFi).
  * _Dữ liệu đầu vào:_ Whitelist (Danh sách trắng) chứa MAC điện thoại của 3 thành viên, dữ liệu quét gói tin WiFi hiện tại trong bếp.
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Tắt WiFi trên điện thoại chủ nhà (Giả lập cả nhà đã rời đi). Quơ tay trước cảm biến PIR ban ngày. Sau đó, bật lại WiFi (Giả lập chủ nhà bước vào bếp).
  * _Kết quả:_ Sau 15 phút không bắt được MAC quen thuộc, hệ thống tự bật Vacation Mode (Chế độ đi vắng). Lúc này PIR bắt chuyển động ban ngày và hú còi ngay lập tức. Khi chủ nhà bật WiFi, hệ thống tóm được tín hiệu và tự động Disarm (Vô hiệu hóa báo động).

#### 3.6 Chức năng 6 - Phát hiện hành vi phá hoại thiết bị

* **Điều kiện tiền đề (Setup):**
  * _Tình huống:_ Kẻ gian lọt vào bếp lúc ban ngày, lén dùng áo trùm kín hộp thiết bị hoặc dùng băng keo đen bịt mắt cảm biến để tối hành động.
  * _Thiết bị:_ ESP8266, LDR, PIR.
  * _Dữ liệu đầu vào:_ Mức sụt giảm ánh sáng đột ngột, tín hiệu chuyển động tức thời.
* **Luồng trình diễn (Demo Flow):**
  * _Hành động:_ Dùng bàn tay úp chặt che tối hoàn toàn LDR một cách đột ngột vào ban ngày, sau đó quơ tay trước cảm biến PIR ngay lập tức.
  * _Kết quả:_ Hệ thống nhận diện sự thay đổi ánh sáng phi logic (từ sáng rực xuống tối đen trong 1 giây) kèm chuyển động. Sinh ra trạng thái Interrupt (Ngắt khẩn cấp), Buzzer hú liên tục không cần chờ tới giờ giới nghiêm ban đêm.

### 4b. Một Số Chức Năng Phụ Được Bổ Sung

Bên cạnh các chức năng chống đột nhập chính, hệ thống được bổ sung thêm một số chức năng hỗ trợ nhằm nâng cao khả năng bảo vệ con người, tài sản và tăng tính thực tế khi triển khai trong khu vực phòng bếp.

#### 4b.1. Cảnh báo nguy cơ cháy

**Mục đích:**

Phát hiện sớm dấu hiệu xuất hiện ngọn lửa trong khu vực phòng bếp và cảnh báo cho người dùng.

**Thiết bị sử dụng:**

Flame Sensor, ESP32-CAM, còi Buzzer, đèn LED đỏ và hệ thống gửi thông báo qua Internet.

**Nguyên lý hoạt động:**

* Flame Sensor liên tục giám sát tín hiệu ánh sáng hồng ngoại phát ra từ ngọn lửa.
* Khi tín hiệu vượt quá ngưỡng nguy hiểm trong một khoảng thời gian xác định, hệ thống ghi nhận dấu hiệu có ngọn lửa.
* ESP32-CAM kích hoạt còi Buzzer và đèn LED đỏ để cảnh báo tại chỗ.
* Hệ thống đồng thời gửi thông báo khẩn cấp đến điện thoại của chủ nhà, kèm theo thời gian phát hiện sự kiện.
* ESP32-CAM có thể chụp ảnh khu vực xảy ra cảnh báo để người dùng kiểm tra tình hình từ xa.

**Kết quả mong đợi:**

Hệ thống phát hiện sớm dấu hiệu ngọn lửa, cảnh báo bằng âm thanh, ánh sáng và gửi thông báo đến người dùng nhằm hạn chế nguy cơ cháy lan.

> **Lưu ý:** Flame Sensor chỉ phát hiện dấu hiệu của ngọn lửa, không thay thế hoàn toàn cảm biến khói hoặc cảm biến nhiệt độ chuyên dụng.

#### 4b.2. Chụp ảnh khi phát hiện đột nhập

**Mục đích:**

Ghi lại hình ảnh hiện trường khi hệ thống xác nhận có người xâm nhập trái phép.

**Thiết bị sử dụng:**

ESP32-CAM, PIR, HC-SR04, LDR, DS1307 và hệ thống gửi thông báo qua Internet.

**Nguyên lý hoạt động:**

* Hệ thống kết hợp dữ liệu từ PIR, HC-SR04 và LDR để xác nhận hành vi đột nhập.
* Khi điều kiện cảnh báo được thỏa mãn, ESP32-CAM tự động chụp ảnh khu vực giám sát.
* Hình ảnh được lưu lại hoặc gửi đến nền tảng Arduino IoT Cloud cùng với thời gian xảy ra sự kiện lấy từ DS1307 và hiển thị trên ứng dụng điện thoại.
* Còi Buzzer và đèn LED đỏ đồng thời được kích hoạt để cảnh báo tại chỗ.
* Hệ thống áp dụng thời gian chờ giữa các lần chụp để tránh chụp liên tục hoặc gửi quá nhiều thông báo.

**Kết quả mong đợi:**

Chủ nhà không chỉ nhận được thông báo có đột nhập mà còn có hình ảnh hiện trường để kiểm tra và làm dữ liệu đối chiếu khi cần thiết.

#### 4b.3. Chức năng cảnh báo khẩn cấp SOS trên điện thoại

**Mục đích:**

Cho phép chủ nhà chủ động kích hoạt cảnh báo khẩn cấp thông qua ứng dụng hoặc giao diện điều khiển trên điện thoại khi phát hiện nguy hiểm, cần cầu cứu hoặc muốn cảnh báo những người đang ở gần khu vực phòng bếp.

**Thiết bị và nền tảng sử dụng:**

Điện thoại của chủ nhà, nền tảng Arduino IoT Cloud (và ứng dụng Arduino IoT Cloud Remote), ESP32-CAM, còi Buzzer và đèn LED đỏ.

**Nguyên lý hoạt động:**

* Trên giao diện điều khiển của chủ nhà có một nút SOS dùng để kích hoạt chế độ cảnh báo khẩn cấp.
* Khi chủ nhà nhấn nút SOS, ứng dụng gửi lệnh thông qua Internet đến ESP32-CAM.
* Sau khi nhận được lệnh, ESP32-CAM lập tức kích hoạt còi Buzzer và đèn LED đỏ để phát tín hiệu cảnh báo tại khu vực phòng bếp.
* ESP32-CAM có thể chụp ảnh hiện trường tại thời điểm nhận lệnh SOS và gửi hình ảnh về ứng dụng để chủ nhà kiểm tra.
* Hệ thống ghi lại thời gian kích hoạt SOS vào nhật ký sự kiện.
* Chủ nhà có thể tắt cảnh báo thông qua nút Tắt SOS hoặc Dừng báo động trên ứng dụng sau khi tình huống nguy hiểm đã được xử lý.

**Luồng trình diễn:**

1. Người trình diễn mở giao diện điều khiển hệ thống trên điện thoại.
2. Nhấn nút SOS để giả lập tình huống chủ nhà phát hiện nguy hiểm.
3. Ứng dụng gửi lệnh điều khiển đến ESP32-CAM.
4. Còi Buzzer phát âm thanh cảnh báo và đèn LED đỏ nhấp nháy.
5. ESP32-CAM chụp ảnh khu vực giám sát và gửi ảnh về điện thoại.
6. Người trình diễn nhấn nút Tắt SOS để dừng cảnh báo.

**Kết quả mong đợi:**

Chủ nhà có thể chủ động kích hoạt hệ thống cảnh báo từ điện thoại mà không cần có mặt trực tiếp tại thiết bị. Chức năng này hỗ trợ cảnh báo người trong nhà, thu hút sự chú ý và ghi nhận hình ảnh hiện trường khi phát hiện tình huống nguy hiểm.

#### 4b.4. Ghi nhật ký hoạt động của cảm biến

**Mục đích:**

Ghi lại toàn bộ các sự kiện quan trọng do cảm biến phát hiện, giúp nhóm theo dõi hoạt động của hệ thống, kiểm tra nguyên nhân báo động và phục vụ quá trình kiểm thử.

**Thiết bị và nền tảng sử dụng:**

ESP32-CAM, DS1307, các cảm biến của hệ thống và nền tảng Arduino IoT Cloud (hoặc bộ nhớ lưu trữ).

**Nguyên lý hoạt động:**

* ESP32-CAM liên tục tiếp nhận dữ liệu từ các cảm biến như PIR, HC-SR04, LDR và Flame Sensor.
* Khi một cảm biến phát hiện sự thay đổi bất thường hoặc kích hoạt một sự kiện, hệ thống tạo một bản ghi nhật ký.
* Mỗi bản ghi bao gồm:
  * Thời gian xảy ra sự kiện lấy từ DS1307.
  * Tên cảm biến phát hiện sự kiện.
  * Giá trị hoặc trạng thái cảm biến tại thời điểm đó.
  * Mức độ cảnh báo của sự kiện.
  * Hành động mà hệ thống đã thực hiện, ví dụ: bật còi, bật đèn LED, chụp ảnh hoặc gửi thông báo.
* Nhật ký có thể được gửi lên nền tảng Arduino IoT Cloud để chủ nhà xem lại trên ứng dụng điện thoại.
* Trong trường hợp mất kết nối Internet, dữ liệu có thể được lưu tạm thời và gửi lại khi kết nối được khôi phục.
* Hệ thống chỉ ghi các sự kiện có ý nghĩa hoặc ghi theo chu kỳ nhất định để tránh tạo quá nhiều dữ liệu không cần thiết.

**Ví dụ bản ghi nhật ký:**

> 02:15:08 - PIR phát hiện chuyển động - HC-SR04: 72 cm - LDR: 685 - Mức cảnh báo: Nguy hiểm - Hành động: Bật còi, chụp ảnh và gửi thông báo.

**Luồng trình diễn:**

1. Người trình diễn tạo chuyển động trước cảm biến PIR.
2. Rọi đèn pin vào cảm biến LDR hoặc đưa vật thể đến gần HC-SR04.
3. ESP32-CAM tiếp nhận và xử lý dữ liệu cảm biến.
4. Hệ thống tạo bản ghi nhật ký kèm thời gian và giá trị cảm biến.
5. Người trình diễn mở giao diện trên điện thoại để xem lại lịch sử hoạt động.
6. Màn hình hiển thị thời gian, loại cảm biến, trạng thái phát hiện và hành động đã thực hiện.

**Kết quả mong đợi:**

Người dùng có thể theo dõi lịch sử hoạt động của hệ thống, biết cảm biến nào đã được kích hoạt, thời điểm xảy ra sự kiện và phản ứng tương ứng của thiết bị. Chức năng này giúp tăng khả năng kiểm tra, truy vết và đánh giá độ ổn định của hệ thống.

### 5. Phân Công Nhân Sự (Roles & Responsibilities)

#### Ngô Gia Long (Trưởng nhóm)

* Quản lý và điều phối tiến độ dự án.
* Thiết kế kiến trúc tổng thể của hệ thống.
* Xây dựng nền tảng Cloud (sử dụng Arduino IoT Cloud) và ứng dụng di động để lưu trữ dữ liệu và nhận cảnh báo.
* Thiết kế và tối ưu các thuật toán xử lý chính.
* Hoàn thiện báo cáo và slide thuyết trình.
* Tìm viết báo khoa học bằng công cụ LaTeX.

#### Nguyễn Nhật Anh (Phần cứng)

* Lắp ráp linh kiện và cảm biến.
* Thiết kế sơ đồ đấu nối phần cứng.
* Hàn mạch và gia công sản phẩm.
* Kiểm tra độ ổn định của thiết bị.

#### Võ Trần Công Danh (Phần mềm)

* Lập trình ESP8266 bằng C/C++.
* Xử lý dữ liệu từ các cảm biến.
* Xây dựng các thuật toán phát hiện sự kiện.
* Tối ưu hiệu năng và độ ổn định của hệ thống.

#### Nguyễn An Vương (Kiểm thử và hỗ trợ)

* Xây dựng các kịch bản kiểm thử.
* Thực hiện thử nghiệm thực tế.
* Hỗ trợ xây dựng sơ đồ và tài liệu.
* Quay video minh họa sản phẩm.

### 6. Kế Hoạch Thực Hiện Theo Tuần (Roadmap)

#### Tuần 3: Ghép nối phần cứng thô & Đọc tín hiệu cảm biến

**Mục tiêu:** Linh kiện được cắm chung lên một Testboard (Bo mạch thử nghiệm) và Cloud đã sẵn sàng nhận dữ liệu thô.

**Phân chia công việc:**

* Nhật Anh (Phần cứng): Cắm tất cả linh kiện (ESP8266, PIR, LDR, DS1307, HC-SR04) lên board dựa trên sơ đồ mạch điện.
* Công Danh (Phần mềm): Viết code cơ bản để đọc khoảng cách từ cảm biến siêu âm, đọc trạng thái chuyển động từ PIR để còi (Buzzer) kêu.
* Gia Long (Leader) + An Vương: Thiết lập nền tảng Cloud (sử dụng Arduino IoT Cloud) để chuẩn bị hứng và lưu trữ dữ liệu từ chip gửi lên, đồng thời cấu hình truyền về ứng dụng trên điện thoại.

#### Tuần 4: Xử lý Thuật toán đa cảm biến (Multi-Sensor Fusion)

**Mục tiêu:** Hệ thống nhận diện chính xác đột nhập, còi báo động kêu đúng lúc, loại bỏ báo động giả do môi trường.

**Phân chia công việc:**

* Công Danh (Phần mềm): Viết thêm logic kết hợp (AND/OR) cho PIR, Ultrasonic và độ dao động ánh sáng LDR; thêm code Wi-Fi để chip gửi dữ liệu cảnh báo lên mạng.
* Gia Long (Leader): Cấu hình Arduino IoT Cloud để tự động nhận dạng mức độ nguy hiểm, lưu trữ dữ liệu sự kiện và định dạng tin nhắn cảnh báo truyền về điện thoại.
* Nhật Anh + An Vương: Kiểm tra xem góc quét có điểm mù không, và đo xem mất bao lâu dữ liệu báo động mới nhảy lên điện thoại.

#### Tuần 5: Lọc nhiễu chống phá hoại & Spam tin nhắn

**Mục tiêu:** Thiết bị thông minh hơn, không bị lỗi spam tin nhắn khi kẻ gian đứng lỳ một chỗ, và phát hiện được hành vi che mắt cảm biến (phá hoại).

**Phân chia công việc:**

* Công Danh (Phần mềm): Sửa code để xử lý chức năng Anti-Sabotage (Chống phá hoại - báo động khẩn nếu ánh sáng LDR tụt đột ngột kết hợp PIR).
* Gia Long (Leader): Viết thuật toán Debounce (Chống dội tín hiệu) sao cho nếu kẻ gian di chuyển liên tục thì máy chỉ gửi 1 tin nhắn cảnh báo trong mỗi 1 phút để tránh treo mạng.
* An Vương (Kiểm thử): Đóng vai kẻ trộm quơ tay liên tục, dùng áo trùm kín cảm biến để xem máy có phát hiện ra lỗi và báo động đúng không.

#### Tuần 6: Tối ưu Edge Cases (Pet Immunity & Lưu trạng thái Offline)

**Mục tiêu:** Mất mạng hoặc mất điện thì hệ thống vẫn không sập; vật nuôi đi qua không gây hú còi.

**Phân chia công việc:**

* Công Danh (Phần mềm): Viết code để khi có chuyển động $< 30\text{cm}$ (thú nuôi) thì bỏ qua. Lưu trạng thái (Armed/Disarmed) vào EEPROM bên trong chip phòng khi mất điện.
* Nhật Anh (Phần cứng): Lắp pin CR2032 cho DS1307 để đảm bảo thời gian chạy độc lập; thiết kế lại đèn LED báo trạng thái "Đang lưu tạm/Mất mạng".
* Gia Long (Leader) + An Vương: Tắt thử router Wi-Fi và dập cầu dao điện để xem máy có khôi phục hoạt động Offline đúng như thiết kế không.

#### Tuần 7: Quét MAC lảng vảng & Chống đơ máy

**Mục tiêu:** Nhận diện được MAC điện thoại lạ lảng vảng quá 5 phút; nếu máy bị treo thì tự khởi động lại.

**Phân chia công việc:**

* Công Danh (Phần mềm): Viết code kích hoạt Promiscuous Mode (Chế độ giám sát mạng) để bắt các gói tin Wi-Fi; cài thêm tính năng tự reset (Watchdog Timer - Bộ định thời giám sát hệ thống) nếu chip bị đơ.
* Gia Long (Leader): Xử lý thuật toán Sliding Window (Cửa sổ thời gian trượt) đếm số lần xuất hiện MAC lạ, đẩy cảnh báo lên điện thoại.
* Nhật Anh (Phần cứng): Chuẩn bị sẵn sàng các dụng cụ hàn mạch để cố định dây cắm, tìm vị trí gắn cảm biến hoàn hảo trong hộp.

#### Tuần 8: Hàn mạch và Đóng vỏ hộp sản phẩm

**Mục tiêu:** Hoàn thiện thiết bị chắc chắn, đẹp đẽ, bỏ vào trong hộp bảo vệ (Mô phỏng vị trí phòng bếp).

**Phân chia công việc:**

* Nhật Anh (Phần cứng): Hàn cố định các linh kiện và gia công hộp đựng (cắt nhựa hoặc mica), đục lỗ chính xác cho mắt cảm biến siêu âm, LDR và PIR.
* Công Danh (Phần mềm): Hỗ trợ kiểm tra xem sau khi hàn xong thì tín hiệu cảm biến có bị nhiễu do lỏng dây không.
* An Vương + Gia Long (Leader): Cắm điện cho máy chạy thử cả ngày đêm (Stress Test - Kiểm thử áp lực/độ bền) xem chip có bị nóng hay bị lỗi gì không.

#### Tuần 9: Chạy thử nghiệm tổng thể & Quay video Demo

**Mục tiêu:** Có video thực tế chứng minh sản phẩm chạy hoàn hảo toàn bộ 6 kịch bản khắt khe nhất để nộp cho giảng viên.

**Phân chia công việc:**

* An Vương (Kiểm thử): Setup bối cảnh phòng bếp. Trực tiếp đóng vai kẻ trộm rọi đèn pin, dập cầu dao, giả làm vật nuôi đi qua. Quay clip lại toàn bộ quá trình (góc máy thấy rõ màn hình điện thoại nhận thông báo và cả người chuyển động) rồi dựng thành một clip demo ngắn gọn, súc tích.
* Cả nhóm: Kiểm tra lại thiết bị, lau chùi vỏ hộp sạch sẽ.

#### Tuần 10: Làm báo cáo, Slide & Đi bảo vệ

**Mục tiêu:** Nộp bài và lấy điểm cao, sẵn sàng phản biện kiến trúc hệ thống.

**Phân chia công việc:**

* Gia Long (Leader) + An Vương: Hoàn thiện cuốn báo cáo Word (bao gồm lưu đồ thuật toán kỹ thuật) và làm một bộ Slide thuyết trình thật đẹp, ngắn gọn.
* Nhật Anh + Công Danh: Chuẩn bị giải thích các câu hỏi về hàn phần cứng, đấu dây và code xử lý lõi C++ nếu thầy hỏi.
* Cả nhóm: Tập thuyết trình thử với nhau trước khi lên gặp thầy, tập trung làm nổi bật sự tối ưu và thực dụng của sản phẩm.
