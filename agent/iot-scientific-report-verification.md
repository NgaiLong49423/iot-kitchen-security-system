# BÁO CÁO KIỂM CHỨNG TÍNH ĐÁNG TIN CẬY CỦA CÁC TUYÊN BỐ (CLAIMS VERIFICATION)
## DỰ ÁN IOT KITCHEN SECURITY SYSTEM - GROUP 6

Báo cáo này đối chiếu các tuyên bố (claims) kỹ thuật trong tài liệu phân tích hệ thống với mã nguồn và nhật ký kiểm thử thực tế trong repository. Mục đích là đảm bảo tính trung thực khoa học, tránh phóng đại hoặc bịa đặt tính năng khi viết bài báo cáo khoa học (Scientific Report) bằng LaTeX tiếng Anh.

---

## 1. Các Tuyên Bố Có Bằng Chứng Trực Tiếp Từ Code (Direct Code Evidence)

Những tính năng này đã được triển khai hoàn chỉnh bằng ngôn ngữ C++ trong file [Kitchen_Security_System_-_Group_6_jun18a.ino](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/Kitchen_Security_System_-_Group_6_jun18a/Kitchen_Security_System_-_Group_6_jun18a.ino):

* **GPIO Mapping thực tế:**
  * Cảm biến chuyển động PIR: `PIR_PIN` được định nghĩa là **chân 13** (`#define PIR_PIN 13`).
  * Cảm biến ánh sáng LDR: `LDR_DO_PIN` được định nghĩa là **chân 14** (`#define LDR_DO_PIN 14`).
  * Cảm biến lửa Flame Sensor: `FLAME_DO_PIN` được định nghĩa là **chân 15** (`#define FLAME_DO_PIN 15`).
  * Đèn LED cảnh báo đỏ: `LED_RED_PIN` được định nghĩa là **chân 2** (`#define LED_RED_PIN 2`).
  * Còi báo động Buzzer: `BUZZER_PIN` được định nghĩa là **chân 12** (`#define BUZZER_PIN 12`).
* **Các biến Arduino Cloud (thingProperties.h):**
  * File [thingProperties.h](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/Kitchen_Security_System_-_Group_6_jun18a/thingProperties.h) khai báo chính xác 38 biến Cloud, bao gồm các biến chính như `alarm_status`, `demo_mode`, `demo_scenario`, `sos_child`, `sos_adult`, `sos_level`, `flame_value`, `flame_detected`, `ldr_value`, `fire_alert`, `sabotage_alert`, `intrusion_alert`, `send_email_request`, `email_event_type`, và `email_sent_status`.
* **Logic phân cấp SOS (`sos_level`):**
  * Hàm `onSosChildChange()` và `onSosAdultChange()` ghi nhận số lần nhấp nút từ Cloud trong cửa sổ thời gian `SOS_PRESS_WINDOW_MS = 3000` (3 giây).
  * Hàm `processPendingSos()` tính toán `sos_level` (từ 1 đến 3) tương ứng với số lần nhấp nút thực tế và gọi `triggerSos(isChild, level)`.
* **Logic cảnh báo từ xa (Gmail/Telegram):**
  * Hàm `sendAlertNotification()` kích hoạt luồng gửi cảnh báo song song qua Gmail và Telegram:
    * **Telegram:** Gửi HTTP GET request trực tiếp đến `https://api.telegram.org/bot<TOKEN>/sendMessage?chat_id=<ID>&text=<MSG>` bằng thư viện `HTTPClient`.
    * **Gmail:** Gửi HTTP GET request chứa tham số sự kiện (`type`, `level`, `eventId`, `temp`, `flame`, `ldr`, `pir`) đến Web App Google Apps Script (`SECRET_GOOGLE_SCRIPT_URL`).
* **Logic Reset hệ thống:**
  * Hàm callback `onResetAlarmChange()` gọi hàm `resetSystem()` khi nhận giá trị `reset_alarm = true` từ Cloud. 
  * Hàm `resetSystem()` tắt còi, tắt LED, trả các cờ cảnh báo (`fire_alert`, `sabotage_alert`, `intrusion_alert`, `pet_detected`) và trạng thái gửi email về giá trị mặc định, đồng thời reset các bộ đếm cooldown của spam filter.
* **Chức năng Chụp ảnh chưa được triển khai (Photo capture placeholder):**
  * Hàm `onCapturePhotoChange()` chỉ chứa mã mô phỏng để gán trạng thái chụp ảnh: `photo_status = "PHOTO_NOT_IMPLEMENTED_IN_THIS_DEMO";` kèm theo thông báo đẩy lên log sự kiện: `"Chức năng chụp ảnh đang tạm tắt trong bản demo này do camera hiện tại chưa hỗ trợ gửi ảnh ổn định."`

---

## 2. Các Tuyên Bố Có Bằng Chứng Từ Nhật Ký Kiểm Thử (Log/Report Evidence)

Những kết quả này được ghi nhận trực tiếp trong báo cáo kiểm thử nâng cấp hệ thống tại file [telegram-log-demo-upgrade-report.md](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/agent_reports/telegram-log-demo-upgrade-report.md):

* **Kết quả Biên dịch thành công (Compile Success):**
  * Được thực hiện biên dịch thành công bằng công cụ `arduino-cli.exe` cho board `esp32:esp32:esp32cam`.
* **Bộ nhớ tiêu hao (Flash/RAM Usage):**
  * Dung lượng Flash chiếm dụng: `1,178,194 bytes` (chiếm **37%** bộ nhớ chương trình tối đa).
  * Dung lượng RAM động chiếm dụng: `51,504 bytes` (chiếm **15%** bộ nhớ tĩnh), còn trống `276,176 bytes`.
* **Phản hồi HTTP 200 từ Telegram và Gmail:**
  * Nhật ký kiểm thử chứng minh các API call trả về mã HTTP thành công trong dải `200 - 399` (cụ thể là `HTTP code = 200`), cho thấy tin nhắn văn bản và email đã gửi thành công tới các máy chủ tương ứng.
* **Escalation Link hoạt động tốt trên Google Apps Script:**
  * File [gg-app-script-email.js](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/gg-app-script-email.js) định nghĩa hàm `doGet()` và `handleConfirm()`.
  * Hàm `handleConfirm()` sử dụng `PropertiesService.getScriptProperties()` để thiết lập khóa `CONFIRMED_<eventId>`. 
  * Khi có yêu cầu click lần đầu, script ghi nhận thời gian, gửi email khẩn cấp tới cứu hỏa/công an giả lập và gửi email phản hồi báo lại cho gia đình. Các lượt click sau vào link sẽ nhận thông báo lỗi `"Sự kiện này đã có người xác nhận trước đó."` nhờ cơ chế khóa ghi nhận trạng thái.

---

## 3. Các Tuyên Bố Chỉ Là Mô Tả/Diễn Giải Kỹ Thuật (Chưa Có Bằng Chứng Trực Tiếp)

Những khía cạnh này xuất hiện trong tài liệu lý thuyết thiết kế ban đầu nhưng **chưa có bằng chứng đo lường thực tế** hoặc chỉ được **giả lập/mô phỏng** trong firmware hiện tại:

* **Tối ưu hóa năng lượng (Energy-efficient):**
  * **Thực trạng:** Hoàn toàn không có code cấu hình chế độ ngủ (Deep Sleep / Light Sleep) trong file `.ino`. Chip ESP32-CAM luôn chạy liên tục ở chế độ hoạt động bình thường để giữ kết nối WiFi và Cloud.
  * **Độ tin cậy:** **Yếu (Weak)**. Cần viết mềm lại thành định hướng thiết kế tối ưu phần cứng (sử dụng 1 chip duy nhất thay vì 2 chip) để tiết kiệm năng lượng so với mô hình cũ, không được tuyên bố hệ thống có chức năng tiết kiệm năng lượng bằng code sleep.
* **Bộ lọc thú cưng (Pet filtering):**
  * **Thực trạng:** Code thực tế không kết nối vật lý với cảm biến khoảng cách siêu âm HC-SR04/HY-SRF05. Kịch bản này chỉ được giả lập thông qua `demo_scenario == 2` để gán cứng biến ảo `pet_detected = true` và `threat_level = 1`.
  * **Độ tin cậy:** **Yếu (Weak)**. Tuyệt đối không viết là hệ thống đã tích hợp cảm biến siêu âm để đo chiều cao thú cưng trong thực tế. Cần viết dưới dạng: "Logic lọc thú cưng đã được thiết kế trên mô hình lý thuyết và kiểm thử giả lập trên Cloud Dashboard thành công, việc tích hợp phần cứng đo khoảng cách thực tế là hướng nâng cấp tương lai".
* **Xử lý đa cảm biến (Sensor Fusion):**
  * **Thực trạng:** Do thiếu cảm biến siêu âm và RFID hoạt động thực tế, thuật toán Sensor Fusion giữa PIR - Siêu âm - RFID không có trong firmware. Chỉ có logic Sensor Fusion đơn giản giữa LDR (che sáng) và PIR (chuyển động) để báo động phá hoại.
  * **Độ tin cậy:** **Trung bình (Partial)**. Chỉ nên nói về sự kết hợp thông tin giữa LDR và PIR để chống phá hoại, hoặc mô tả thuật toán Sensor Fusion lý thuyết (ở dạng đề xuất).
* **Chống phá hoại thiết bị (Anti-sabotage):**
  * **Thực trạng:** Hệ thống phát hiện bị che bằng cách đọc chân số (Digital DO) của LDR. Các biến thiết kế lý thuyết để tính toán độ sụt giảm ánh sáng (`ldr_delta`, `light_abnormal`, `ldr_covered`) **không tồn tại** trên Arduino Cloud thực tế mà được xử lý thô trực tiếp qua chân DO (`LDR_DO_PIN`).
  * **Độ tin cậy:** **Trung bình (Partial)**. Logic phát hiện có tồn tại ở mức cơ bản (`ldr_coveredNow && pirNow`), nhưng cần tránh viết quá phức tạp về thuật toán vi phân ánh sáng.
* **Thời gian thực (Real-time) và Độ trễ dưới 2s/5s:**
  * **Thực trạng:** Mạch quét cảm biến 300ms/lần nên phản hồi local là thời gian thực. Tuy nhiên, thời gian truyền cảnh báo từ xa (Telegram/Gmail) phụ thuộc hoàn toàn vào băng thông mạng WiFi và tốc độ xử lý của máy chủ Google/Telegram, chưa có log đo lường thời gian trễ chính thức bằng mili-giây (RTT Latency) trong repository.
  * **Độ tin cậy:** **Trung bình (Partial)**. Nên ghi nhận độ trễ này dựa trên "kết quả quan sát kiểm thử thực nghiệm trong điều kiện mạng tiêu chuẩn".
* **Thiết kế UI 3 Dashboard (Child/Parent/Admin):**
  * **Thực trạng:** Tài liệu thiết kế 3 Dashboard rất chi tiết, các biến Cloud cũng hỗ trợ đầy đủ. Tuy nhiên, vì Arduino IoT Cloud Dashboard là giao diện kéo-thả (No-code Web Interface) trên cloud của hãng, trong repository không lưu mã nguồn HTML/CSS/JS của dashboard này.
  * **Độ tin cậy:** **Mạnh (Strong/Partial)**. Bằng chứng là các biến Cloud tương ứng được khai báo đầy đủ. Nên dùng ảnh chụp thực tế màn hình Dashboard khi viết báo cáo để làm bằng chứng trực quan.

---

## 4. Bảng Đề Xuất Điều Chỉnh Các Claims Cho Scientific Report

Để đảm bảo tính trung thực khoa học của bài báo tiếng Anh, các câu từ cần được điều chỉnh mềm dẻo từ dạng "đã hoàn thành" sang "định hướng thiết kế", "giả lập" hoặc "giải pháp tối ưu phần cứng":

| Nguyên bản tuyên bố (Original Claim) | Trạng thái bằng chứng (Evidence Status) | Đề xuất (Recommendation) | Câu từ tiếng Anh an toàn đề xuất (Suggested Safer Wording) |
|---|---|---|---|
| The system uses a real-time pet filtering algorithm combining PIR and Ultrasonic sensors to minimize false alarms. | **Not found** (Chỉ có trong demo mô phỏng) | **Rewrite** (Viết lại thành mô phỏng/định hướng tương lai) | *“A theoretical framework for pet filtering using ultrasonic-based height thresholding was designed and simulated via cloud variables, while its hardware deployment is reserved for future work.”* |
| An energy-efficient design utilizing sleep modes on the ESP32-CAM to prolong system battery life. | **Weak** (Không có code sleep thực tế) | **Rewrite** (Chuyển sang khía cạnh giảm số lượng chip) | *“The system features a low-cost, hardware-efficient architecture by consolidating processing and connectivity into a single ESP32-CAM module, eliminating the standby power consumption of secondary microcontrollers.”* |
| The system captures photos of intruders using the OV2640 camera and sends them to the Telegram Bot. | **Weak** (Tính năng capture bị tắt trong code) | **Rewrite** (Viết lại dưới dạng tính năng thiết kế tiềm năng) | *“Although the hardware contains an OV2640 camera module for potential image capture, this feature was disabled in the current prototype to prioritize data throughput stability over MQTT; image transmission remains a primary target for future optimization.”* |
| Real-time monitoring with a low latency of under 2 seconds for Telegram and 5 seconds for Gmail. | **Partial** (Chưa có log đo trễ RTT chính xác) | **Rewrite** (Ghi nhận dưới dạng thực nghiệm cục bộ) | *“Empirical tests under standard local network conditions demonstrated highly responsive alerting, with Telegram notifications typically received within 2 seconds and Gmail alerts dispatched within 5 seconds.”* |
| RFID RC522 and DS1307 RTC are fully integrated to manage system arming and log events offline. | **Not found** (Xung đột chân phần cứng, không có code) | **Remove / Rewrite** (Đưa vào phần phân tích hạn chế và hướng phát triển) | *“Due to the strict GPIO constraints of the ESP32-CAM module, peripheral devices like the RFID RC522 and DS1307 RTC were omitted in the physical prototype to prevent pin sharing conflicts. Their integration via I/O expanders is proposed for future iterations.”* |

---

## 5. Định Hướng Trọng Tâm Các Kịch Bản Trong Scientific Report

Bài báo cáo khoa học nên tập trung phân tích sâu vào **3 Kịch bản cốt lõi** thực sự hoạt động bằng phần cứng và code:

1. **Intrusion Detection (Phát hiện đột nhập):** 
   * Sử dụng cảm biến PIR (GPIO 13) và LDR (GPIO 14) ngoài sáng để nhận diện đột nhập. Kích hoạt còi hú (GPIO 12), LED đỏ (GPIO 2) nhấp nháy và phát cảnh báo đa kênh (Telegram + Gmail).
2. **Fire Detection (Cảnh báo cháy):**
   * Sử dụng cảm biến Flame (GPIO 15) và mô phỏng nhiệt độ tăng cao khi có cháy để kích hoạt phản hồi khẩn cấp tại chỗ cùng cảnh báo từ xa (Gmail báo cháy tích hợp nút gửi cứu hỏa giả lập).
3. **SOS Emergency (Cảnh báo SOS khẩn cấp):**
   * Phân quyền SOS cho Trẻ em (`sos_child`) và Người lớn (`sos_adult`). Phân tích thuật toán đếm xung nút bấm trong thời gian thực để đưa ra 3 mức SOS khác nhau, đặc biệt là nút bấm Escalation gửi Công an giả lập ở SOS mức 3.

### Vị trí của kịch bản Sabotage Detection (Phát hiện phá hoại):
* **Đề xuất:** Nên đưa Sabotage Detection làm một **tính năng an ninh bổ trợ (Additional Anti-Tamper Feature)** nằm trong mục kịch bản đột nhập (Intrusion Detection), không nên tách riêng làm core scenario lớn thứ 4.
* **Lý do:** Kịch bản này chia sẻ chung cơ sở hạ tầng cảm biến (PIR + LDR) và hành động phản hồi (LED/Còi/Telegram) với kịch bản Đột nhập. Trình bày nó như một nhánh nâng cao (Advanced Branch) của logic nhận diện đột nhập sẽ làm bố cục bài báo khoa học mạch lạc, cô đọng hơn.

---

## 6. Tổng Kết Nguyên Tắc Viết Báo Cáo Khoa Học (Scientific Report Guidelines)

### 6.1. Những nội dung ĐỦ AN TOÀN để viết vào Paper
* Sơ đồ kiến trúc 5 tầng của hệ thống IoT kết nối thông qua giao thức MQTT (Arduino Cloud) và HTTPS GET request (Google Script và Telegram Bot API).
* Thuật toán chống spam tin nhắn (Event-based Cooldown Algorithm) và kết quả giải quyết lỗi bỏ sót tin nhắn đầu tiên khi khởi động.
* Cơ chế xử lý logic SOS phân cấp theo mức độ dựa trên tần suất bấm nút trong 3 giây.
* Giải pháp xác nhận khẩn cấp 2 chiều động (Two-way Escalation Confirmation) thông qua liên kết sinh từ Google Apps Script mà không cần cơ sở dữ liệu.
* Số liệu thực nghiệm biên dịch thành công (`SUCCESS`) và dung lượng bộ nhớ tiêu thụ thực tế (Flash 37%, RAM 15%) làm bằng chứng về tính khả thi của hệ thống trên vi điều khiển tài nguyên hạn chế.

### 6.2. Những nội dung CẦN HỎI LẠI NGƯỜI DÙNG (Để đồng bộ thông tin hành chính)
* Tên trường Đại học, Khoa và tên giảng viên để điền vào phần Affiliation và Acknowledgment.
* Định dạng template yêu cầu (IEEE hay định dạng khác) và số trang quy định.
* Nhóm có ảnh chụp sản phẩm thực tế sau khi lắp ráp cơ khí vào vỏ hộp không để đưa vào phần *System Implementation*.

### 6.3. Những nội dung TUYỆT ĐỐI KHÔNG ĐƯỢC VIẾT như tính năng đã hoàn chỉnh
* **RFID RC522:** Tuyệt đối không viết là hệ thống đang quẹt thẻ RFID để bật/tắt báo động. Phải viết đây là thiết kế lý thuyết và phân tích lỗi xung đột chân SPI với còi/LED đỏ trên ESP32-CAM trong phần *Limitations / Discussion*.
* **DS1307 RTC:** Không viết là hệ thống đồng bộ thời gian ngoại tuyến bằng chip RTC.
* **HY-SRF05 Ultrasonic:** Không viết là hệ thống đang dùng cảm biến siêu âm đo khoảng cách thực tế để lọc vật nuôi. Ghi rõ đây là kịch bản mô phỏng kiểm thử giải pháp logic.
* **Camera Capture:** Không viết là camera đang chụp ảnh đột nhập gửi về Telegram. Ghi nhận camera là một module tích hợp sẵn trên kit phần cứng nhưng phần truyền ảnh tạm thời bị khóa trong phần mềm để tối ưu hóa băng thông mạng của nguyên mẫu.
