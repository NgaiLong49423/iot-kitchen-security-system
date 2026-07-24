# Kịch bản demo chính thức — Version 3

> **Phiên bản:** 1.0.0  
> **Ngày:** 2026-07-21  
> **Phạm vi:** Firmware `Firmware/Version3`, Google Apps Script Version 3, Arduino Cloud Dashboard Trẻ em/Người lớn.  
> **Lưu ý an toàn:** Email contact cơ quan trong toàn bộ demo là **contact mô phỏng**, không phải địa chỉ công an/cơ quan thật. Không dùng địa chỉ nhà thật hoặc người nhận thật trong video/tài liệu công khai.

![Parent Control Dashboard overview](../images/Screenshot%202026-07-24%20192117.png)

*Figure 1. Parent Control Dashboard overview, showing alarm, camera, notification, and monitoring controls.*

---

## 1. Chuẩn bị chung

### 1.1 Phần cứng và dịch vụ

- Freenove ESP32-S3 WROOM + camera OV3660.
- PIR, cảm biến siêu âm, LDR, RTC DS1307, còi, LED đỏ và LED xanh.
- WiFi có Internet; nên dùng router/hotspot ổn định cho demo chính.
- Arduino Cloud Thing của Version 3 và Dashboard Người lớn/Trẻ em.
- Telegram bot và chat ID đã cấu hình.
- Gemini API key hợp lệ nếu thực hiện Demo 7.
- Google Apps Script đã dán bản `Firmware/Version3/gg-app-script-email.js`, đã cấp quyền MailApp và deploy đúng URL trong `SECRET_GOOGLE_SCRIPT_URL`.
- Apps Script đã chạy `installHeartbeatMonitorTrigger()` để trigger kiểm tra heartbeat hoạt động theo lịch.

### 1.2 Kiểm tra trước khi quay demo

1. Nạp firmware Version 3, mở Serial Monitor và kiểm tra RTC/camera/WiFi/Arduino Cloud đã sẵn sàng.
2. Bật `telegram_enabled`, `google_script_enabled` và `heartbeat_enabled`.
3. Xác nhận `current_time`, `device_health_status`, `heartbeat_status` và `last_heartbeat_time` trên Dashboard Người lớn có giá trị mới.
4. Với demo cần ảnh, bật `camera_enabled`. Với demo AI, bật thêm `gemini_enabled` và `auto_photo_on_alert`.
5. Không dùng nút `reset_alarm` liên tục khi không có event vì mỗi lần reset vẫn gửi một request `action=resolve` tới Apps Script.

### 1.3 Quy tắc diễn đạt khi thuyết trình

- Nói **“phát hiện có người”**, không nói “phát hiện người lạ”.
- Nói **“mất liên lạc hoặc có nguy cơ bị vô hiệu hóa”**, không khẳng định chắc chắn bị cắt điện/phá hoại nếu không có bằng chứng.
- Demo authority escalation phải gọi là **email contact mô phỏng**.

---

## Demo 1 — Hoạt động bình thường và mất kết nối

### Mục tiêu

Chứng minh hệ thống gửi heartbeat liên tục và Google Apps Script gửi một email cho gia đình khi thiết bị mất kết nối bình thường.

### Cấu hình

| Biến | Giá trị |
|---|---|
| `google_script_enabled` | Bật |
| `heartbeat_enabled` | Bật |
| `alarm_enabled` | Tùy chọn |
| `sabotage_alert` | Không active |

### Thao tác

1. Cho thiết bị chạy ổn định, chờ `heartbeat_status = Đang liên lạc bình thường`.
2. Ghi lại `last_heartbeat_time` trên Dashboard.
3. Tắt hotspot/router hoặc ngắt WiFi của board; **không** kích hoạt sabotage trước đó.
4. Chờ Apps Script đánh dấu heartbeat quá hạn sau 40 giây. Vì trigger chạy theo lịch, email có thể đến muộn hơn thời điểm timeout.
5. Bật lại WiFi và chờ heartbeat tiếp theo.

### Kết quả mong đợi

- Khi mất kết nối: gia đình nhận tối đa một email thông báo hệ thống mất liên lạc, yêu cầu kiểm tra WiFi/nguồn/thiết bị.
- Không gửi authority escalation vì không có sabotage active.
- Khi WiFi trở lại: Apps Script gửi tối đa một email recovery; `last_heartbeat_time` tiếp tục cập nhật.

### Lời thuyết trình gợi ý

“Thiết bị gửi heartbeat mỗi 10 giây. Cloud chỉ kết luận offline khi quá hạn 40 giây; điều này tránh cảnh báo sai do một request lỗi đơn lẻ.”

---

## Demo 2 — Phát hiện đột nhập, Telegram và ảnh

### Mục tiêu

Chứng minh luồng phát hiện đột nhập vật lý, cảnh báo local và gửi ảnh Telegram.

### Cấu hình

| Biến | Giá trị |
|---|---|
| `alarm_enabled` | Bật |
| `camera_enabled` | Bật |
| `auto_photo_on_alert` | Bật |
| `telegram_enabled` | Bật |
| `gemini_enabled` | Tắt nếu chỉ demo Telegram/ảnh; bật nếu nối tiếp Demo 7 |

### Thao tác

1. Chờ qua 3 giây grace period sau boot.
2. Tạo đồng thời chuyển động PIR và đặt vật trong khoảng **trên 15 cm đến 50 cm** trước cảm biến siêu âm.
3. Giữ điều kiện liên tục ít nhất 2 giây.

### Kết quả mong đợi

- `intrusion_alert = true`, còi/LED đỏ bật và `alarm_status` thể hiện cảnh báo đột nhập.
- Camera tự chụp ảnh; Telegram nhận ảnh kèm caption an ninh.
- `photo_status`, `notification_sent_status` và `last_event` cập nhật.

### Lưu ý

Vật ở `<= 15 cm` thuộc luồng anti-sabotage, không phải luồng intrusion này.

---

## Demo 3 — Phát hiện phá hoại và suy giảm sau phá hoại

### Mục tiêu

Chứng minh anti-sabotage, gửi Telegram/ảnh, sau đó phát hiện mất heartbeat và tạo critical-confirmation.

### Pha A — Phá hoại đang diễn ra

#### Cấu hình

`camera_enabled`, `auto_photo_on_alert`, `telegram_enabled`, `google_script_enabled` và `heartbeat_enabled` đều bật.

#### Thao tác

1. Che LDR để giá trị đạt điều kiện covered theo cấu hình board.
2. Đồng thời đặt vật ở khoảng cách `<= 15 cm`.
3. Giữ cả hai điều kiện ít nhất 3 giây.

#### Kết quả mong đợi

- `sabotage_alert = true`, còi/LED đỏ hoạt động.
- `last_event` ghi sự kiện phá hoại; Telegram nhận ảnh/caption nếu camera hoạt động.
- Firmware gửi event `sabotage_alert` đến Apps Script và Apps Script bắt đầu theo dõi heartbeat cho event này.

### Pha B — Mất khả năng hoạt động sau phá hoại

#### Thao tác

1. Sau khi Pha A đã được Apps Script nhận, ngắt nguồn hoặc WiFi của board.
2. Chờ heartbeat quá hạn 40 giây và trigger Cloud chạy.
3. Mở email Parent/Admin, kiểm tra yêu cầu xác nhận critical.
4. Chỉ trong demo, Parent/Admin xác nhận để Apps Script gửi email đến contact authority **mô phỏng**.

#### Kết quả mong đợi

- Apps Script tạo `critical_security_compromise` sau sabotage + heartbeat timeout.
- Gia đình nhận email yêu cầu xác nhận; hệ thống không tự tuyên bố chắc chắn nguyên nhân là bị cắt điện.
- Sau xác nhận, contact mô phỏng nhận email escalation; không gửi cơ quan thật.

---

## Demo 4 — Bật/tắt chống trộm theo thời gian tùy chỉnh

![Scheduling controls on the dashboard](../images/Screenshot%202026-07-24%20192142.png)

*Figure 2. Dashboard controls for alarm sensitivity and scheduled arming/disarming.*

### Mục tiêu

Chứng minh RTC tự thay đổi `alarm_enabled` theo lịch.

### Cấu hình

1. Kiểm tra `current_time` trên Dashboard khớp giờ thực tế.
2. Bật `schedule_enabled`.
3. Đặt `auto_arm_hour:auto_arm_minute` sớm hơn 1–2 phút.
4. Đặt `auto_disarm_hour:auto_disarm_minute` sau đó 1–2 phút.

### Thao tác và kết quả

| Thời điểm | Kết quả mong đợi |
|---|---|
| Đúng giờ tự bật | `alarm_enabled = true`, `system_armed` bật, `last_event` ghi tự bật theo lịch. |
| Đúng giờ tự tắt | `alarm_enabled = false`, `system_armed` tắt, `last_event` ghi tự tắt theo lịch. |

### Lưu ý

Tắt chống trộm theo lịch không tắt SOS hoặc anti-sabotage.

---

## Demo 5 — SOS và escalation contact authority mô phỏng

### Mục tiêu

Chứng minh Người lớn có thể chủ động yêu cầu hỗ trợ khi nghi ngờ/nhìn thấy trộm, với xác nhận trước khi gửi contact mô phỏng.

### Thao tác

1. Trên Dashboard Người lớn, nhập `sos_authority_note`:

   > Hiện tại nhà tôi đang có kẻ trộm đột nhập. Tình trạng người trong nhà: có hoặc không có người. Vui lòng hỗ trợ ngay lập tức.

2. Bấm `sos_adult`.
3. Kiểm tra Telegram và email Parent/Admin.
4. Parent/Admin bấm link xác nhận trong email Apps Script.
5. Kiểm tra email gửi đến authority contact **mô phỏng**.

### Kết quả mong đợi

- Local alarm vẫn duy trì; Dashboard hiển thị trạng thái chờ xác nhận.
- Email xác nhận có thông tin thiết bị, thời điểm, khu vực, địa chỉ cấu hình và ghi chú SOS.
- Sau xác nhận, Apps Script gửi email đến contact mô phỏng kèm địa chỉ nhà đã cấu hình.
- ACK không tắt còi/LED. Chỉ `reset_alarm` mới kết thúc SOS local.

### Lưu ý pháp lý/an toàn

Không cấu hình email công an/cơ quan thật trong demo. Nếu xảy ra tình huống khẩn cấp thật, người dùng phải liên hệ số khẩn cấp địa phương trực tiếp; hệ thống demo không thay thế quy trình phản ứng khẩn cấp.

---

## Demo 6 — Chụp ảnh thủ công để quan sát từ xa

### Mục tiêu

Chứng minh Người lớn có thể yêu cầu camera gửi ảnh Telegram mà không kích hoạt báo động.

### Cấu hình

`camera_enabled = true` và `telegram_enabled = true`.

### Thao tác

1. Bấm `manual_capture_photo` trên trang Camera và ảnh.
2. Chờ camera capture và kiểm tra Telegram.

### Kết quả mong đợi

- `manual_capture_photo` tự trở về `false`.
- Telegram nhận một ảnh mới.
- `photo_status` phản ánh chụp/gửi thành công hoặc lỗi cụ thể.
- Không kích hoạt `intrusion_alert`, SOS hoặc email AI-person.

---

## Demo 7 — Gemini phát hiện có người, Gmail và ảnh bằng chứng

### Mục tiêu

Chứng minh AI phân loại có người trong ảnh cảnh báo tự động, gửi Gmail yêu cầu kiểm tra Telegram và tạo ba ảnh bằng chứng.

### Cấu hình

| Biến | Giá trị |
|---|---|
| `alarm_enabled` | Bật |
| `camera_enabled` | Bật |
| `auto_photo_on_alert` | Bật |
| `gemini_enabled` | Bật |
| `telegram_enabled` | Bật |
| `google_script_enabled` | Bật |

### Thao tác

1. Đặt một người thật trong khung hình camera.
2. Tạo điều kiện Demo 2 để hệ thống tự capture ảnh do intrusion.
3. Theo dõi Serial Monitor, Telegram, Gmail và Dashboard.

### Kết quả mong đợi

1. Gemini trả `PERSON_DETECTED`.
2. Telegram nhận ảnh cảnh báo có thông tin AI phát hiện có người.
3. Google Apps Script gửi **một** email đến gia đình với nội dung yêu cầu kiểm tra ảnh Telegram để xác minh danh tính.
4. Firmware chụp thêm đúng 3 ảnh bằng chứng, mỗi ảnh cách nhau khoảng 1 giây.
5. Ba ảnh follow-up không gọi Gemini lại và không gửi thêm email.

### Giới hạn hiện tại cần nói rõ

Firmware Version 3 chỉ gửi Gmail AI-person cho **ảnh tự động của intrusion hoặc sabotage**. Ảnh chụp thủ công vẫn có thể chạy Gemini, nhưng hiện không tạo email AI-person. Nếu yêu cầu “bất cứ trường hợp nào AI thấy người cũng gửi Gmail”, cần thay đổi firmware ở phiên bản tiếp theo.

---

## 2. Bảng tổng kết demo

| Demo | Tính năng chính | Kênh kiểm chứng |
|---|---|---|
| 1 | Heartbeat, offline/recovery | Dashboard, Gmail, Serial |
| 2 | Intrusion + auto photo | Còi/LED, Dashboard, Telegram |
| 3 | Sabotage → heartbeat timeout → critical | Còi/LED, Telegram, Gmail, Apps Script |
| 4 | Lịch RTC | Dashboard, `last_event` |
| 5 | SOS confirmation/escalation | Dashboard, Telegram, Gmail/App Script |
| 6 | Manual photo | Telegram, Dashboard |
| 7 | Gemini PERSON → Gmail + 3 ảnh | Serial, Telegram, Gmail, Dashboard |

---

## 3. Kết thúc demo

1. Dùng `reset_alarm` một lần để clear cảnh báo local còn active.
2. Khôi phục WiFi nếu đã ngắt ở Demo 1 hoặc Demo 3.
3. Xác nhận `heartbeat_status` trở về bình thường.
4. Không xóa log/email dùng làm bằng chứng demo trước khi nhóm hoàn tất đánh giá.
