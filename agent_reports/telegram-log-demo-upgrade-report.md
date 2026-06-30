# Báo Cáo Nâng Cấp Hệ Thống Cảnh Báo Telegram & Logs Dashboard

Báo cáo này chi tiết các nội dung đã thay đổi, kết quả biên dịch và hướng dẫn kiểm thử sau nâng cấp.

---

## 1. Tóm Tắt Mục Tiêu Sửa
* **Nâng cấp nội dung tin nhắn Telegram:** Chuyển tin nhắn cảnh báo sang tiếng Việt đời thường, trực quan và thể hiện rõ mức độ nguy hiểm thay vì định dạng debug kỹ thuật.
* **Chuẩn hóa logs dashboard:** Đồng bộ hóa các biến trạng thái trên Arduino IoT Cloud (`alarm_status`, `last_event`, `last_event_type`, `threat_level`, `event_counter`) để dashboard hiển thị rõ ràng thông tin trạng thái bằng tiếng Việt tự nhiên.
* **Nâng cấp kịch bản Demo (`demo_scenario`):** Xử lý mượt mà các tình huống demo (đột nhập, vật nuôi, phá hoại, báo cháy, SOS), đồng thời tự động đưa `demo_scenario = 0` sau khi kích hoạt xong nhằm tránh lặp đi lặp lại.
* **Cơ chế chống spam thông minh & Sửa lỗi boot cooldown:** 
  * Bổ sung cooldown per-event-type (60 giây cho mỗi loại cảnh báo) để tránh bị khóa API, nhưng vẫn cho phép gửi cảnh báo tức thì khi đổi sang loại sự kiện khác.
  * **Sửa lỗi bỏ sót cảnh báo đầu tiên sau khi khởi động:** Điều chỉnh logic cooldown kiểm tra thêm điều kiện `lastXAlertMs > 0`. Nhờ đó, tin nhắn cảnh báo đầu tiên ngay sau khi thiết bị khởi chạy sẽ KHÔNG bị chặn (kể cả khi chạy trong 60 giây đầu tiên).
* **Vá dữ liệu cảm biến Demo:**
  * Đồng bộ hóa biến trạng thái cảm biến trước khi gọi hàm trigger demo:
    * Kịch bản 6 (phá hoại): Gán `pir_detected = true` và `ldr_value = 100` để hiển thị đúng tình huống cảm biến bị che khuất và có chuyển động.
    * Kịch bản 1 (đột nhập): Gán `pir_detected = true` và `ldr_value = 900` để hiển thị đúng tình huống có chuyển động bất thường ngoài sáng.
* **Ẩn thông tin nhạy cảm trên Serial Monitor:**
  * Loại bỏ câu lệnh in ra toàn bộ Google Script Request URL chứa Token bí mật. Thay thế bằng log thông báo chung: `[GMAIL] Sending alert request to Google Apps Script...`.
* **Tuyệt đối không đụng camera:** Giữ nguyên các chức năng camera và không thêm thư viện camera nào khác.

---

## 2. Danh Sách File Đã Sửa
* **Đường dẫn file:** [Kitchen_Security_System_-_Group_6_jun18a.ino](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/Kitchen_Security_System_-_Group_6_jun18a/Kitchen_Security_System_-_Group_6_jun18a.ino)
  * **Sửa phần nào:**
    * **Section 4 (Biến nội bộ):** Thêm biến `TYPE_ALERT_COOLDOWN_MS`, các mốc thời gian gửi cảnh báo cuối cùng cho từng sự kiện (`lastSosChildAlertMs`, `lastSosAdultAlertMs`, `lastFireAlertMs`, `lastSabotageAlertMs`, `lastIntrusionAlertMs`), và biến cờ `localDemoReset`.
    * **Section 8 (`sendGoogleAlert`):** Ẩn in full Google Script URL ra Serial.
    * **Section 10 (`sendAlertNotification`):** Sửa lỗi cooldown cảnh báo đầu tiên bằng cách so sánh `lastXAlertMs > 0` trước khi chặn. Cập nhật log tiếng Việt thông báo kết quả gửi thành công/thất bại sau khi tiến hành gửi.
    * **Section 12 (Triggers):** Viết lại các hàm `triggerSos`, `triggerFireAlert`, `triggerSabotageAlert`, `triggerIntrusionDemo` để thiết lập đúng các câu log tiếng Việt trạng thái ban đầu dạng `"Đang gửi cảnh báo..."` và định dạng tin nhắn Telegram phong phú.
    * **Section 12 (Thêm mới):** Tạo hàm `triggerPetIgnoredDemo` phục vụ kịch bản demo vật nuôi.
    * **Section 13 (`resetSystem`):** Đưa thêm biến `pet_detected = false` và reset tất cả mốc thời gian cooldown về `0` để hỗ trợ test lại lập tức.
    * **Section 18 (`onDemoScenarioChange` & Callbacks):** Thiết lập `pir_detected` và `ldr_value` trước khi kích hoạt kịch bản đột nhập và phá hoại. Chuẩn hóa toàn bộ các chuỗi log tiếng Anh kỹ thuật (như bật/tắt còi, nhấn SOS, kích hoạt demo, chụp ảnh bị tắt) sang tiếng Việt đời thường. Slider `demo_scenario` tự reset về `0` mà không gây lặp đệ quy.
  * **Vì sao sửa:** Để đáp ứng toàn bộ các yêu cầu nâng cấp trải nghiệm người dùng, chuẩn hóa dữ liệu hiển thị trên cloud dashboard, chống spam theo loại sự kiện và đảm bảo demo chạy mượt mà không lặp.

---

## 3. Danh Sách File Đã Tạo Mới
* [agent/AGENTS.md](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/agent/AGENTS.md): Tài liệu quy tắc làm việc dự án cho Agent.
* [agent_reports/backups/Kitchen_Security_System_-_Group_6_jun18a.ino.bak](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/agent_reports/backups/Kitchen_Security_System_-_Group_6_jun18a.ino.bak): Bản sao lưu của file `.ino` gốc trước khi sửa.
* [agent_reports/telegram-log-demo-upgrade-report.md](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/agent_reports/telegram-log-demo-upgrade-report.md): File báo cáo hiện tại.

---

## 4. Danh Sách Biến Cloud Đã Dùng
*(Chỉ sử dụng các biến đã khai báo trong [thingProperties.h](file:///d:/Semester%204/IOT102/iot-kitchen-security-system/Firmware/Kitchen_Security_System_-_Group_6_jun18a/thingProperties.h))*
* `alarm_status` (String)
* `email_event_type` (String)
* `email_sent_status` (String)
* `last_event` (String)
* `last_event_type` (String)
* `sos_message` (String)
* `kitchen_temperature` (float)
* `flame_value` (int)
* `ldr_value` (int)
* `demo_scenario` (int)
* `event_counter` (int)
* `sos_level` (int)
* `threat_level` (int)
* `alarm_enabled` (bool)
* `buzzer_on` (bool)
* `demo_mode` (bool)
* `fire_alert` (bool)
* `flame_detected` (bool)
* `intrusion_alert` (bool)
* `led_red_on` (bool)
* `pet_detected` (bool)
* `pir_detected` (bool)
* `reset_alarm` (bool)
* `sabotage_alert` (bool)
* `send_email_request` (bool)
* `sos_adult` (bool)
* `sos_child` (bool)
* `system_armed` (bool)

---

## 5. Danh Sách Biến Cloud Không Dùng
*(Các biến này có trong thiết kế lý thuyết nhưng KHÔNG có trong `thingProperties.h` nên được bỏ qua hoàn toàn trong code để tránh lỗi compile)*
* `cooldown_active`
* `last_alert_time`
* `ldr_delta`
* `light_abnormal`
* `ldr_covered`
* `object_near`
* `intrusion_score`
* `device_tampered`
* `current_time`
* `current_hour`
* `night_mode`

---

## 6. Nội Dung Thông Báo Telegram Mẫu Sau Khi Sửa

### SOS Trẻ em:
```text
🆘 CẢNH BÁO SOS TRẺ EM
Trẻ em vừa kích hoạt SOS từ điện thoại.
Khu vực: Bếp
Mức nguy hiểm: 4/4
Hệ thống đã bật còi và đèn đỏ tại chỗ.
Gmail cảnh báo đã được gửi cho người lớn.
Vui lòng kiểm tra ngay.
```

### SOS Người lớn:
```text
🆘 SOS NGƯỜI LỚN
Người lớn vừa kích hoạt cảnh báo khẩn cấp.
Khu vực: Bếp
Hệ thống đã bật còi và đèn đỏ.
Vui lòng kiểm tra tình hình xung quanh thiết bị.
```

### Báo cháy:
```text
🔥 NGUY HIỂM! PHÁT HIỆN DẤU HIỆU CHÁY
Hệ thống phát hiện dấu hiệu ngọn lửa tại khu vực bếp.
Giá trị flame: 200
Nhiệt độ bếp: 65.0°C
Còi và đèn đỏ đã được kích hoạt.
Vui lòng kiểm tra khu vực bếp ngay lập tức.
```

### Phá hoại:
```text
🛠️ CẢNH BÁO PHÁ HOẠI THIẾT BỊ
Thiết bị phát hiện dấu hiệu bị che/can thiệp.
Khu vực: Bếp
PIR phát hiện chuyển động: Có
Giá trị LDR: 100
Hệ thống đã bật còi và đèn đỏ.
Vui lòng kiểm tra thiết bị ngay.
```

### Đột nhập:
```text
🚨 CẢNH BÁO ĐỘT NHẬP
Hệ thống phát hiện chuyển động bất thường gần khu vực bếp.
PIR: Có
Giá trị LDR: 900
Mức nguy hiểm: 3/4
Còi và đèn đỏ đã được kích hoạt.
Vui lòng kiểm tra ngay.
```

---

## 7. Nội Dung Log Dashboard Mẫu

### Khi xảy ra SOS Trẻ em (Trạng thái gửi):
* `alarm_status` = `"SOS_CHILD_ALERT"`
* `last_event_type` = `"SOS_CHILD"`
* `last_event` = `"Trẻ em vừa kích hoạt SOS. Hệ thống đã bật còi và đèn đỏ. Đang gửi cảnh báo cho người lớn."`
* `threat_level` = `4`
* `email_sent_status` = `"EMAIL_SENDING"`

### Khi gửi thành công Telegram và Gmail (Sau gửi):
* `last_event_type` = `"EMAIL_SENT"`
* `last_event` = `"Thông báo đã được gửi đến người nhà qua Gmail và Telegram."`
* `email_sent_status` = `"EMAIL_SENT"`

### Khi xảy ra Báo cháy:
* `alarm_status` = `"FIRE_ALERT"`
* `last_event_type` = `"FIRE"`
* `last_event` = `"Nguy hiểm! Hệ thống phát hiện dấu hiệu cháy tại khu vực bếp. Đang gửi cảnh báo cho người lớn."`
* `threat_level` = `4`
* `email_sent_status` = `"EMAIL_SENDING"` (Sau khi gửi thành công sẽ ghi log và chuyển sang `"EMAIL_SENT"`)

### Khi xảy ra Phá hoại:
* `alarm_status` = `"SABOTAGE_ALERT"`
* `last_event_type` = `"SABOTAGE"`
* `last_event` = `"Cảnh báo phá hoại! Thiết bị phát hiện dấu hiệu bị che hoặc can thiệp. Đang gửi cảnh báo cho người lớn."`
* `threat_level` = `3`

### Khi xảy ra Đột nhập:
* `alarm_status` = `"INTRUSION_ALERT"`
* `last_event_type` = `"INTRUSION"`
* `last_event` = `"Cảnh báo đột nhập! Hệ thống phát hiện chuyển động bất thường. Đang gửi cảnh báo cho người lớn."`
* `threat_level` = `3`

### Khi xảy ra Vật nuôi đi qua (Demo scenario 2):
* `alarm_status` = `"PET_IGNORED"`
* `last_event_type` = `"PET_IGNORED"`
* `last_event` = `"Hệ thống phát hiện chuyển động nhưng xác định là vật nuôi. Bỏ qua cảnh báo."`
* `threat_level` = `1`
* `email_sent_status` = `"EMAIL_IDLE"` (không gửi mail)

### Khi bật/tắt báo động trên Dashboard:
* `last_event_type` = `"ALARM_ENABLED"`
* `last_event` = `"Hệ thống bảo vệ đã được bật từ dashboard."` hoặc `"Hệ thống bảo vệ đã được tắt từ dashboard."`

### Khi nhấn SOS trên Cloud (nhưng chưa hết thời gian đếm số lần bấm):
* `alarm_status` = `"SOS_INPUT_WAITING"`
* `sos_message` = `"Hệ thống đã nhận tín hiệu SOS trẻ em. Đang chờ xác nhận mức độ."` (hoặc SOS người lớn tương ứng)

### Khi Reset hệ thống:
* `alarm_status` = `"ARMED"` hoặc `"SAFE"` (tùy theo trạng thái kích hoạt)
* `last_event_type` = `"SYSTEM_RESET"`
* `last_event` = `"Người dùng đã reset cảnh báo. Hệ thống đã tắt còi và đèn đỏ."`
* `threat_level` = `0`
* `email_sent_status` = `"EMAIL_IDLE"`

---

## 8. Kiểm Tra Chống Spam
* **Cơ chế hoạt động:** Sử dụng biến nội bộ lưu trữ thời điểm gửi thành công cuối cùng theo từng loại cảnh báo riêng biệt (sử dụng hàm `millis()`).
* **Thời gian chặn:** Đặt ngưỡng `TYPE_ALERT_COOLDOWN_MS = 60000` (60 giây). Trong khoảng thời gian này, nếu cùng một sự kiện xảy ra liên tục (ví dụ cháy vẫn tiếp diễn), tin nhắn thứ hai trở đi sẽ bị bỏ qua để tránh spam API của Google Script và Telegram.
* **Đổi loại sự kiện:** Nếu hệ thống đang cooldown cảnh báo `fire` nhưng đột nhiên xảy ra `sos_child`, tin nhắn `sos_child` vẫn được gửi đi ngay lập tức không bị cản trở nhờ việc phân tách cooldown theo loại sự kiện.
* **Sửa lỗi bỏ sót cảnh báo đầu tiên:** Do mốc khởi tạo các biến cooldown bằng `0`, nếu thời điểm hiện tại `now` nhỏ hơn 60000ms kể từ lúc khởi động mạch, phép trừ sẽ cho kết quả nhỏ hơn 60 giây và chặn mất tin nhắn. Bằng cách thêm điều kiện `lastXAlertMs > 0`, hệ thống nhận dạng sự kiện đầu tiên luôn được gửi ngay lập tức sau khi mạch khởi động.

---

## 9. Những Gì Cố Tình Không Sửa
* **Không sửa `thingProperties.h`:** Đảm bảo tính tương thích với cấu hình thiết bị trên Arduino Cloud.
* **Không sửa camera:** Không can thiệp hay cài thêm thư viện để tránh gây bất ổn định cho kit ESP32-CAM.
* **Không sửa GPIO / phần cứng:** Giữ nguyên các chân cắm đã kiểm tra pass trước đó.
* **Không sửa secrets:** Không can thiệp hay thay đổi file `arduino_secrets.h`.
* **Không đổi Google Apps Script:** Giữ nguyên cấu hình đầu nhận email của Apps Script.

---

## 10. Hướng Dẫn Test Sau Khi Upload

Hãy thực hiện kiểm thử theo checklist sau trên Arduino IoT Cloud Dashboard:

1. **Biên dịch & nạp code:** Đảm bảo nạp code thành công xuống mạch ESP32-CAM.
2. **Kiểm tra trạng thái khởi động:** Quan sát Dashboard, các giá trị ban đầu phải hiển thị bình thường.
3. **Bật chế độ Demo:**
   * Gạt switch `demo_mode` thành `true`.
4. **Test kịch bản Báo cháy:**
   * Chuyển `demo_scenario` sang `7`.
   * **Kết quả mong muốn:**
     * Còi hú và đèn LED đỏ nhấp nháy trên mạch.
     * `alarm_status` đổi thành `"FIRE_ALERT"`.
     * `last_event_type` đổi thành `"FIRE"`.
     * `threat_level` đổi thành `4`.
     * Nhận được tin nhắn Telegram cảnh báo cháy tiếng Việt có kèm giá trị flame và nhiệt độ.
     * Slider `demo_scenario` trên Dashboard tự động nhảy ngược về `0` (nhưng báo động vẫn duy trì).
5. **Test Reset hệ thống:**
   * Nhấp chọn nút `reset_alarm = true` hoặc gửi reset.
   * **Kết quả mong muốn:**
     * Còi ngừng hú, đèn LED tắt.
     * `alarm_status` quay về `"ARMED"` (hoặc `"SAFE"`).
     * `last_event` đổi thành log reset hệ thống tiếng Việt.
     * `threat_level` về `0`.
6. **Test kịch bản Phá hoại:**
   * Chuyển `demo_scenario` sang `6`.
   * **Kết quả mong muốn:**
     * Còi hú, đèn LED sáng.
     * Dashboard hiển thị trạng thái `SABOTAGE_ALERT`, `threat_level` = `3`.
     * `pir_detected` đổi thành `true` và `ldr_value` đổi thành `100`.
     * Nhận được tin nhắn Telegram cảnh báo phá hoại thiết bị có kèm LDR value.
     * Slider `demo_scenario` tự động về `0`.
7. **Test kịch bản SOS Trẻ em:**
   * Chuyển `demo_scenario` sang `8`.
   * **Kết quả mong muốn:**
     * Còi hú, đèn LED sáng.
     * Dashboard hiển thị `SOS_CHILD_ALERT`, `threat_level` = `4`.
     * Nhận được tin nhắn Telegram cảnh báo SOS trẻ em.
     * Slider `demo_scenario` tự động về `0`.
8. **Test kịch bản Vật nuôi (Demo scenario 2):**
   * Chuyển `demo_scenario` sang `2`.
   * **Kết quả mong muốn:**
     * Không kích hoạt còi và LED.
     * Dashboard cập nhật trạng thái `PET_IGNORED`, `threat_level` = `1`.
     * Không có email hay Telegram được gửi đi.
     * Slider `demo_scenario` tự động về `0`.
9. **Test kịch bản Đột nhập:**
   * Chuyển `demo_scenario` sang `1`.
   * **Kết quả mong muốn:**
     * Còi hú, đèn LED sáng.
     * Dashboard hiển thị `INTRUSION_ALERT`, `threat_level` = `3`.
     * `pir_detected` đổi thành `true` và `ldr_value` đổi thành `900`.
     * Nhận được tin nhắn Telegram cảnh báo đột nhập.
     * Slider `demo_scenario` tự động về `0`.
10. **Test chống spam:**
    * Kích hoạt liên tiếp kịch bản báo cháy `7` nhiều lần trong 10 giây.
    * **Kết quả mong muốn:** Tin nhắn Telegram và Email chỉ gửi duy nhất 1 lần đầu tiên.
    * Kích hoạt kịch bản báo cháy `7`, sau đó ngay lập tức chuyển sang phá hoại `6`.
    * **Kết quả mong muốn:** Nhận được cả 2 tin nhắn của cả 2 sự kiện khác nhau mà không bị block.

---

## 11. Kết Quả Biên Dịch (Compile Status)
* **Trạng thái:** **THÀNH CÔNG (SUCCESS)**
* **Công cụ sử dụng:** `arduino-cli.exe` (Local AppData Programs)
* **Board sử dụng:** AI Thinker ESP32-CAM (`esp32:esp32:esp32cam`)
* **Thông số tài nguyên:**
  * **Bộ nhớ chương trình (Flash):** Sử dụng `1178194` bytes (chiếm `37%` dung lượng, tối đa `3145728` bytes).
  * **Bộ nhớ RAM động (SRAM):** Sử dụng `51504` bytes (chiếm `15%` dung lượng, tối đa `327680` bytes), còn trống `276176` bytes.

---

## 12. Cảnh Báo Secrets (Security Notice)
> [!WARNING]
> Phát hiện file `arduino_secrets.h` chứa thông tin cấu hình thực tế bao gồm tên WiFi (`SECRET_SSID`), mật khẩu WiFi (`SECRET_OPTIONAL_PASS`), Device Key của Cloud (`SECRET_DEVICE_KEY`), Telegram Bot Token (`SECRET_TELEGRAM_BOT_TOKEN`), Chat ID nhận cảnh báo (`SECRET_TELEGRAM_CHAT_ID`), và URL của Google Script (`SECRET_GOOGLE_SCRIPT_URL`).
> 
> Nhằm bảo mật tối đa, các giá trị nhạy cảm này đã được bỏ qua và **không hiển thị trong báo cáo này**. Vui lòng giữ file `arduino_secrets.h` an toàn và tuyệt đối tránh commit lên các kho mã nguồn công khai (Public Repository).
