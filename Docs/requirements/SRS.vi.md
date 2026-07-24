# Đặc tả yêu cầu phần mềm — Kitchen Security System

> **Mã tài liệu:** SRS-V3-1.0  
> **Phiên bản:** 1.1.0  
> **Trạng thái:** Tài liệu chính thức cho mã nguồn `Firmware/Version3`  
> **Ngày cập nhật:** 2026-07-23  
> **Ngôn ngữ:** Tiếng Việt

---

## 1. Mục đích và nguyên tắc nguồn sự thật

Tài liệu này đặc tả **hệ thống đang được hiện thực trong Version 3**, gồm firmware ESP32-S3 và Google Apps Script. Tài liệu thay thế vai trò tài liệu yêu cầu chính của `Docs/SRS.md` đối với Version 3.

Khi có khác biệt giữa tài liệu cũ và mã Version 3, ưu tiên theo thứ tự:

1. Firmware [`Kitchen_Security_System_-_Group_6_jun18a.ino`](../Firmware/Version3/Kitchen_Security_System_-_Group_6_jun18a/Kitchen_Security_System_-_Group_6_jun18a.ino);
2. Arduino Cloud properties trong [`thingProperties.h`](../Firmware/Version3/Kitchen_Security_System_-_Group_6_jun18a/thingProperties.h);
3. Backend [`gg-app-script-email.js`](../Firmware/Version3/gg-app-script-email.js);
4. Tài liệu này.

`Docs/SRS.md` là tài liệu lịch sử/checkpoint trước đó, không được dùng để suy diễn chức năng mà Version 3 chưa có.

## 2. Bối cảnh, mục tiêu và phạm vi

### 2.1 Bối cảnh và mục tiêu

Khi gia đình vắng nhà, người dùng khó phát hiện kịp thời hành vi đột nhập, can thiệp/phá hoại thiết bị an ninh hoặc tình huống khẩn cấp trong khu vực bếp. **Kitchen Security System** được xây dựng như một hệ thống an ninh chống trộm đa cảm biến cho khu vực nhà/bếp. Người dùng chủ động bật chế độ bảo vệ khi vắng nhà hoặc cấu hình lịch RTC; hệ thống không tự suy luận sự có mặt/vắng mặt của con người trong Version 3.

Mục tiêu của hệ thống là phát hiện sớm nguy cơ, cảnh báo tại chỗ, cung cấp bằng chứng ban đầu (thời gian, ảnh và trạng thái cảm biến), thông báo từ xa cho Parent/Admin và hỗ trợ họ xác nhận sự cố trước khi tạo báo cáo tới đầu mối khẩn cấp **mô phỏng** đã đăng ký.

Các mục tiêu cụ thể:

| Mục tiêu | Ý nghĩa đối với người dùng | Cách Version 3 đáp ứng |
|---|---|---|
| Phát hiện nguy cơ | Nhận biết sớm người/vật thể xâm nhập và hành vi che/can thiệp cảm biến. | PIR + siêu âm cho intrusion; LDR + siêu âm cho sabotage. |
| Cảnh báo ngay tại hiện trường | Răn đe và báo cho người ở gần mà không phụ thuộc Internet. | Buzzer, LED đỏ và LED xanh hoạt động cục bộ. |
| Cung cấp bối cảnh để quyết định | Người quản lý không chỉ nhận một cảnh báo chung chung. | Loại sự cố, thời gian RTC, trạng thái cảm biến và ảnh camera nếu khả dụng. |
| Phản hồi từ xa có kiểm soát | Người dùng vẫn theo dõi/điều khiển được khi không ở nhà, đồng thời tránh escalation nhầm. | Dashboard, Telegram, email xác nhận Parent/Admin và contact mô phỏng. |
| Nhận biết thiết bị có thể gặp sự cố | Phân biệt tình trạng bình thường với khả năng thiết bị bị mất liên lạc. | Tín hiệu hoạt động định kỳ, thông báo offline và recovery. |

### 2.2 Phạm vi Version 3

Hệ thống Version 3 giám sát một khu vực bếp gia đình. ESP32-S3 đọc cảm biến, điều khiển còi/LED, đồng bộ trạng thái với Arduino IoT Cloud, gửi thông báo Telegram, chụp ảnh, và giao tiếp Google Apps Script để gửi **tín hiệu hoạt động của thiết bị** (heartbeat ở mức giao thức) và thực hiện luồng xác nhận khẩn cấp qua email.

### 2.2.1 Trong phạm vi

- Bật/tắt phát hiện đột nhập, cấu hình độ nhạy hiển thị và lịch RTC.
- Phát hiện đột nhập bằng đồng thời PIR và vật thể ở gần.
- Phát hiện phá hoại bằng LDR bị che đồng thời vật thể áp sát cảm biến.
- SOS từ Child hoặc Parent/Admin.
- Còi, LED đỏ nhấp nháy và LED xanh thể hiện trạng thái.
- Chụp ảnh theo yêu cầu Dashboard hoặc tự chụp khi có alert, gửi Telegram.
- Nhận diện người trên ảnh bằng Gemini khi được bật, cùng các ảnh bổ sung làm bằng chứng.
- Tín hiệu hoạt động của thiết bị, thông báo mất/kết nối lại và escalation email có xác nhận Parent/Admin.

### 2.2.2 Ngoài phạm vi

- BLE trusted phone, quét Wi-Fi/MAC, RFID, báo cháy/Flame Sensor, nhận diện khuôn mặt và streaming video.
- Mobile app riêng, lưu lịch sử dài hạn, nhiều khu vực giám sát, pin dự phòng hoặc điều khiển khóa cửa.
- Gửi đến cơ quan chức năng thật. Contact escalation là **contact mô phỏng**.
- Khẳng định nguyên nhân vật lý của việc mất heartbeat (ví dụ bị cắt điện) khi không có bằng chứng riêng.

### 2.3 Tổng quan bảy nhóm chức năng Version 3

| Nhóm chức năng | Cách hoạt động | Kết quả/người dùng nhận được |
|---|---|---|
| 1. Bảo vệ chống trộm đa cảm biến | Khi `alarm_enabled`, PIR và siêu âm cùng thỏa điều kiện liên tục sẽ tạo intrusion. LDR bị che và vật áp sát sẽ tạo sabotage. | Phát hiện dựa trên nhiều tín hiệu, giảm phụ thuộc vào một cảm biến đơn lẻ. |
| 2. Cảnh báo và phản hồi tại chỗ | Khi SOS, intrusion, sabotage hoặc critical active, buzzer bật, LED đỏ nhấp nháy và LED xanh tắt. | Cảnh báo tức thời, vẫn hoạt động khi Internet lỗi. |
| 3. Camera và bằng chứng hình ảnh | Parent/Admin có thể yêu cầu chụp ảnh; khi alert, hệ thống tự chụp nếu `auto_photo_on_alert` bật. | Ảnh mới và trạng thái chụp/gửi; ảnh không thay thế cho kết luận của người dùng. |
| 4. AI phân tích ảnh | Khi được bật, Gemini chỉ phân tích `PERSON`/`NONE`. Nếu có người, hệ thống chụp thêm ba ảnh bằng chứng. | Tăng bối cảnh cho cảnh báo; không nhận diện danh tính hoặc kết luận ai là trộm. |
| 5. Thông báo và xử lý sự cố từ xa | Telegram gửi text/ảnh; Apps Script gửi email xác nhận cho Parent/Admin với SOS hoặc critical. | Người dùng nhận bối cảnh, xác nhận sự cố trước escalation contact mô phỏng. |
| 6. Giám sát trạng thái hoạt động | Thiết bị gửi tín hiệu hoạt động định kỳ; backend báo offline rồi báo recovery khi tín hiệu trở lại. | Người dùng biết thiết bị có thể mất nguồn, mất Wi-Fi hoặc lỗi kết nối; không suy đoán nguyên nhân. |
| 7. Điều khiển từ xa | Dashboard cho phép bật/tắt bảo vệ, đặt lịch, cấu hình ảnh/AI/kênh mạng, reset alarm và xem telemetry. | Người dùng quản trị hệ thống từ xa khi có Cloud kết nối. |

### 2.4 Luồng phản hồi sự cố cho người dùng

```text
Cảm biến phát hiện intrusion/sabotage hoặc người dùng gửi SOS
                         ↓
   Latch sự cố + buzzer/LED + ghi loại sự cố và thời gian RTC
                         ↓
  Auto-photo (nếu bật) → AI phân tích ảnh (nếu cấu hình hợp lệ)
                         ↓
 Telegram/email/Dashboard hiển thị ảnh và trạng thái liên quan
                         ↓
       Parent/Admin kiểm tra bối cảnh và xác nhận sự cố
                         ↓
 Backend gửi contact mô phỏng đã cấu hình; cảnh báo local chỉ tắt khi reset
```

Quy tắc an toàn của luồng này: một cảnh báo cảm biến hoặc kết quả AI không tự động khẳng định có tội phạm; người dùng là người đưa ra xác nhận. Version 3 không gửi cho cơ quan chức năng thật.

### 2.5 Định hướng phát triển chi tiết (không thuộc yêu cầu đã hiện thực)

Nhóm định hướng mở rộng từ thiết bị bảo vệ bếp thành nền tảng an ninh thông minh cho toàn ngôi nhà, trong đó người dùng có quyền kiểm soát dữ liệu nhạy cảm. Các nội dung dưới đây là mục tiêu thiết kế tương lai, không được trình bày như chức năng đã chạy trong bản demo.

#### 2.5.1 Website/app và dữ liệu do người dùng quản lý

Thay dần Dashboard, Apps Script, Telegram và dịch vụ AI bên thứ ba bằng website/app do nhóm hoặc chủ hệ thống vận hành. Nền tảng cần có tài khoản Parent/Admin, phân quyền người dùng, màn hình trạng thái thời gian thực, lịch sử cảnh báo, cấu hình lịch/chế độ bảo vệ và kho ảnh/video.

Ảnh, video, log sự kiện, cấu hình thiết bị và thông tin vị trí chỉ được truy cập bởi người dùng được ủy quyền. Hệ thống tương lai cần có xác thực đăng nhập, phân quyền theo vai trò, mã hóa khi truyền/lưu trữ, chính sách thời hạn lưu và thao tác xóa dữ liệu theo yêu cầu.

#### 2.5.2 AI cục bộ và nhận diện khuôn mặt có kiểm soát

AI có thể chạy trên thiết bị đủ năng lực hoặc máy chủ nội bộ để ảnh không phải gửi sang nền tảng AI công cộng. Hệ thống cho phép chủ nhà tự đăng ký/kích hoạt/vô hiệu hóa hồ sơ khuôn mặt của người được ủy quyền. Khi phát hiện khuôn mặt không khớp hồ sơ, hệ thống chỉ đánh dấu **người lạ cần kiểm tra** và tăng mức ưu tiên cảnh báo; không tự gán nhãn "kẻ trộm" hay tự gửi báo cáo khẩn cấp.

Việc dùng dữ liệu sinh trắc học phải có sự đồng ý rõ ràng của người liên quan, quyền thu hồi đồng ý, giới hạn ai được xem dữ liệu, mã hóa mẫu khuôn mặt và cơ chế xóa an toàn. Luôn phải có phương án fallback dựa trên cảm biến khi AI không chắc chắn, mất kết nối hoặc không hoạt động.

#### 2.5.3 Bằng chứng video theo sự kiện

Nâng cấp chụp ảnh thành quay video ngắn có gắn mã sự cố, thời gian, vị trí mô-đun và mức cảnh báo. Video nên có đoạn trước/trong/sau thời điểm trigger khi phần cứng/lưu trữ cho phép; không quay liên tục mặc định để giảm rủi ro riêng tư và dung lượng. Người dùng cấu hình thời lượng, bật/tắt quay tự động, quyền xem/tải và thời hạn lưu video.

#### 2.5.4 Hệ thống đa mô-đun toàn nhà

Mỗi mô-đun giám sát một khu vực như bếp, phòng khách, phòng ngủ hoặc cửa ra vào; mô-đun có `device_id`, tên vị trí, trạng thái kết nối và danh sách cảm biến riêng. Một trung tâm quản lý đồng bộ cấu hình, sự kiện và dữ liệu bằng chứng từ các mô-đun, sau đó hiển thị theo sơ đồ nhà/từng khu vực.

Sự kiện đồng bộ phải giữ nguồn tạo, thời gian thiết bị và thời gian máy chủ để truy vết. Khi một mô-đun offline, các mô-đun khác vẫn hoạt động độc lập; hệ thống trung tâm chỉ đánh dấu khu vực đó mất liên lạc, không kết luận toàn bộ ngôi nhà mất an toàn.

#### 2.5.5 Tích hợp đầu mối khẩn cấp an toàn

Tích hợp với đầu mối thật chỉ được xem xét khi có quy trình chính thức, danh sách đầu mối được chủ nhà chấp thuận, xác thực người xác nhận, cơ chế chống gửi trùng và nhật ký kiểm toán. Báo cáo phải tối thiểu gồm mã sự cố, loại nguy cơ, thời gian, vị trí, thông tin liên hệ chủ nhà và bằng chứng được người dùng cho phép chia sẻ. Quyết định gửi phải do Parent/Admin xác nhận, trừ một chính sách SOS đặc biệt được người dùng cấu hình rõ ràng.

## 3. Tổng quan kiến trúc

```text
PIR + LDR + HY-SRF05 + DS1307 ──> ESP32-S3 WROOM + OV3660
                                        ├─> Buzzer / LED đỏ / LED xanh
                                        ├─> Arduino IoT Cloud Dashboard
                                        ├─> Telegram (chữ và ảnh JPEG)
                                        ├─> Gemini API (tùy chọn)
                                        └─> Google Apps Script
                                                ├─> Script Properties / event store 24 giờ
                                                ├─> Email Family/Admin
                                                └─> Email contact mô phỏng sau xác nhận
```

### 3.1 Phần cứng và chân kết nối

| Thành phần | Vai trò | GPIO trong Version 3 |
|---|---|---:|
| Freenove ESP32-S3 WROOM + OV3660 | Bộ điều khiển/camera | Theo pin map camera trong firmware |
| LDR analog | Độ sáng, dấu hiệu che cảm biến | 1 |
| PIR | Chuyển động | 40 |
| HY-SRF05/HC-SR04 compatible | Khoảng cách vật thể | TRIG 38, ECHO 39 |
| DS1307 | Thời gian RTC/lịch | SDA 41, SCL 42 |
| LED đỏ | Cảnh báo | 14 |
| LED xanh | Bình thường | 21 |
| Buzzer | Cảnh báo tại chỗ | 47 |

## 4. Người dùng và quyền thao tác

| Vai trò | Khả năng |
|---|---|
| Child | Kích hoạt `sos_child`; theo dõi trạng thái đơn giản trên Dashboard. |
| Parent/Admin | Điều khiển các property Dashboard, chụp ảnh thủ công, reset alarm, nhận email xác nhận và xác nhận escalation. |
| Thiết bị | Đọc cảm biến, tạo alert, duy trì output và gọi các dịch vụ mạng. |
| Google Apps Script | Lưu event tạm thời, theo dõi heartbeat, gửi email, chống gửi escalation trùng. |

## 5. Trạng thái và quy tắc chung

### 5.1 Ưu tiên trạng thái hiển thị

`alarm_status` phải ưu tiên theo thứ tự: SOS → critical compromise → sabotage → intrusion → armed → disarmed.

| Điều kiện | `alarm_status` |
|---|---|
| `sosActive` | `SOS khẩn cấp đang hoạt động` |
| `critical_security_compromise` | `Sự cố nghiêm trọng sau phá hoại` |
| `sabotage_alert` | `Cảnh báo phá hoại thiết bị` |
| `intrusion_alert` | `Cảnh báo đột nhập` |
| `system_armed` | `Đang bảo vệ khu vực bếp` |
| Còn lại | `Đang tắt bảo vệ` |

### 5.2 Quy tắc latch và reset

- Intrusion, sabotage và SOS được latch: tín hiệu cảm biến mất đi không tự xóa alert.
- `reset_alarm = true` là lệnh duy nhất trong firmware để xóa các alert đang latch, tắt còi/LED đỏ, bật LED xanh và gọi endpoint `resolve` nếu có mạng.
- Xác nhận email escalation **không** reset còi/LED tại thiết bị.
- Sau reboot, các latch local khởi tạo `false`; hệ thống đọc lại cảm biến sau boot grace 3 giây.
- `alarm_enabled` chỉ chi phối phát hiện đột nhập. SOS và anti-sabotage vẫn hoạt động khi chế độ này tắt.

### 5.3 Output tại chỗ

Khi SOS, sabotage, intrusion hoặc critical active: buzzer bật, LED đỏ nhấp nháy mỗi 250 ms và LED xanh tắt. Khi không có alert: buzzer/LED đỏ tắt, LED xanh bật.

## 6. Yêu cầu chức năng

### FR-01 — Đọc cảm biến và thời gian

Firmware phải đọc LDR, PIR và siêu âm mỗi 500 ms. Khoảng cách là trung bình tối đa ba lần đo hợp lệ; kết quả không hợp lệ được biểu diễn bằng `-1`. RTC DS1307 cung cấp thời gian hiển thị và lịch; khi RTC không sẵn sàng, thời gian là `RTC_NOT_FOUND` và lịch không chạy.

### FR-02 — Cấu hình bảo vệ và lịch

- `system_armed` luôn phản ánh `alarm_enabled`.
- Khi `schedule_enabled`, đến đúng phút `auto_arm_hour:auto_arm_minute` firmware đặt `alarm_enabled=true`; đến `auto_disarm_hour:auto_disarm_minute` đặt `false`.
- Lịch không thay đổi SOS, sabotage hoặc critical đang active.
- Giá trị giờ/phút sai được kẹp về lần lượt 22:00 và 06:00; `sensitivity_level` sai được kẹp về 3.

### FR-03 — Tín hiệu đánh giá nguy cơ

Firmware tính telemetry `intrusion_score`:

```text
PIR +2; object_near (<= 50 cm) +2;
light_abnormal (chênh LDR >= 500) +1; night_mode (22:00–05:59) +1.
```

Ngưỡng hiển thị theo `sensitivity_level`: 1→5, 2→4, 3→3. `pet_detected` là một heuristic demo: vật gần nhưng không PIR và không biến động sáng.

**Lưu ý triển khai:** Version 3 không dùng score/ngưỡng làm điều kiện kích hoạt alert. Score chỉ phục vụ trạng thái, caption và báo cáo Serial.

### FR-04 — Phát hiện đột nhập

Một intrusion mới phải thỏa tất cả điều kiện trong 2 giây liên tục, sau boot grace:

```text
system_armed = true
AND PIR = true
AND 15 cm < distance <= 50 cm
```

Khi trigger, firmware phải latch `intrusion_alert`, ghi `last_event`, kích hoạt output local và yêu cầu chụp ảnh tự động. Nếu camera hoặc Gemini không cùng bật, firmware gửi một Telegram text fallback.

### FR-05 — Phát hiện phá hoại

Một sabotage mới phải thỏa trong 3 giây liên tục:

```text
LDR bị che AND distance <= 15 cm
```

LDR bị che mặc định là giá trị analog `>= 2000` (có thể đảo bằng hằng `LDR_COVER_WHEN_HIGH`). Khi trigger, firmware latch `sabotage_alert`, đánh dấu theo dõi sức khỏe, bật output, yêu cầu chụp ảnh, gửi Telegram fallback khi phù hợp và gửi `sabotage_alert` tới Apps Script.

### FR-06 — SOS

- `sos_child` tạo SOS nguồn `CHILD`; `sos_adult` tạo SOS nguồn `PARENT_ADULT`; hai property là nút cạnh lên và tự trả `false`.
- SOS có thể được kích hoạt qua Serial (`c`/`p`).
- SOS latch còi/LED, gửi Telegram text và tạo yêu cầu xác nhận trên Apps Script. Nếu SOS đã active, kích hoạt lại không tạo một event local mới.

### FR-07 — Camera và Telegram

- Camera OV3660 khởi tạo khi boot; có PSRAM dùng VGA, không có PSRAM dùng QVGA.
- `manual_capture_photo` đưa một yêu cầu vào hàng đợi nội bộ và tự trả `false`.
- Camera chỉ chụp khi `camera_enabled=true`, camera ready và cách lần chụp trước ít nhất 1,5 giây.
- Ảnh được gửi Telegram cho chụp manual, intrusion, sabotage và ảnh follow-up khi Telegram/camera được bật và đủ token/chat ID.
- Auto-photo chỉ thực sự chạy khi `auto_photo_on_alert=true`; alert vẫn local nếu tắt chức năng này.
- Telegram áp dụng cooldown độc lập 15 giây cho intrusion, sabotage, SOS và manual để giảm spam.

### FR-08 — AI phân tích ảnh Gemini (tùy chọn)

Khi `gemini_enabled`, `camera_enabled` và API key hợp lệ, một ảnh alert được gửi Gemini với kết quả chỉ nhận `PERSON` hoặc `NONE`.

- Nếu phát hiện người: firmware lập lịch 3 ảnh bổ sung, cách nhau 1 giây; các ảnh này không gọi Gemini lặp lại.
- Mỗi alert tự động chỉ gửi tối đa một email `ai_person_detected` tới family qua Apps Script.
- Gemini thất bại không được chặn còi, LED, camera hoặc thông báo hiện có.
- AI Version 3 chỉ phân tích sự hiện diện của người; không nhận diện khuôn mặt, danh tính chủ nhà hay kết luận một người là kẻ trộm.

### FR-09 — Tín hiệu hoạt động và trạng thái kết nối

- Khi `google_script_enabled` và `heartbeat_enabled`, firmware gửi tín hiệu hoạt động `heartbeat` mỗi 10 giây.
- Thay đổi hai công tắc này phải gửi `heartbeat_monitor_control`; heartbeat thường tạm dừng cho tới khi backend ACK. Lệnh control retry theo 2, 5, 10, 30 giây.
- Firmware tự reconnect Wi-Fi theo backoff 2, 5, 10, 30 giây và thể hiện `Đang khôi phục kết nối WiFi`.
- Apps Script coi heartbeat quá 40 giây là offline. Trigger backend chạy mỗi 1 phút, vì vậy thời điểm email thực tế có thể chậm hơn timeout.
- Offline bình thường gửi một email family và chỉ gửi email recovery khi heartbeat trở lại. Tắt monitoring bằng Dashboard không tạo offline/recovery giả.

### FR-10 — Critical compromise sau phá hoại

Apps Script theo dõi sabotage active. Nếu sau sabotage không có heartbeat hoặc heartbeat quá 40 giây, backend tạo event `critical_security_compromise`, gửi một email yêu cầu Parent/Admin xác nhận và không gửi escalation tự động.

Đây là nhận định trung tính: hệ thống chỉ báo mất liên lạc/suy giảm sau phá hoại, không kết luận bị cắt điện hay hỏng một module cụ thể.

**Giới hạn Version 3:** firmware không có polling status định kỳ tới Apps Script. Do đó critical được backend xử lý/email hóa khi thiết bị mất liên lạc, nhưng property `critical_security_compromise` trên board chỉ đổi nếu board nhận được response critical trong một lần gọi Apps Script; không thể đồng bộ trở lại cho một board đang offline.

### FR-11 — Xác nhận và escalation email

- SOS hoặc critical tạo event ID, lưu trong Script Properties tối đa 24 giờ.
- Family/Admin nhận email chứa link `confirm`; trang confirm có thể nhập ghi chú.
- `confirm_send` dùng Script Lock để tránh xử lý lặp.
- Sau xác nhận, backend chỉ gửi email contact mô phỏng nếu có `HOME_ADDRESS` và `AUTHORITY_DEMO_RECIPIENTS` hợp lệ. Kết quả là `SENT`, `FAILED` hoặc `NOT_CONFIGURED`.
- `resolve` đánh dấu SOS/sabotage event là `RESOLVED` và xóa active pointer; local reset vẫn hoàn thành nếu không gọi được backend.

## 7. Arduino IoT Cloud contract

| Property | Quyền | Ý nghĩa |
|---|---|---|
| `alarm_status`, `system_armed` | Read | Trạng thái bảo vệ tổng hợp. |
| `current_time` | Read | Thời gian RTC, cập nhật mỗi giây. |
| `last_event` | Read | Mô tả event gần nhất. |
| `intrusion_alert`, `sabotage_alert`, `critical_security_compromise` | Read | Các latch an ninh. |
| `device_health_status`, `heartbeat_status`, `last_heartbeat_time` | Read | Sức khỏe và tín hiệu hoạt động/liên lạc backend. |
| `notification_sent_status`, `photo_status` | Read | Kết quả gửi notification/chụp ảnh. |
| `emergency_escalation_status`, `emergency_authority_message_status` | Read | Trạng thái escalation đã ánh xạ tiếng Việt. |
| `home_address_configured` | Read | Backend xác nhận cấu hình địa chỉ. |
| `alarm_enabled` | Read/Write | Bật/tắt phát hiện intrusion. |
| `schedule_enabled`, `auto_arm_hour`, `auto_arm_minute`, `auto_disarm_hour`, `auto_disarm_minute` | Read/Write | Cấu hình lịch RTC. |
| `sensitivity_level` | Read/Write | Mức 1–3; hiện là telemetry/UX, không gate intrusion. |
| `telegram_enabled`, `google_script_enabled`, `heartbeat_enabled` | Read/Write | Công tắc dịch vụ mạng và tín hiệu hoạt động. |
| `camera_enabled`, `gemini_enabled`, `auto_photo_on_alert` | Read/Write | Công tắc ảnh/AI. |
| `manual_capture_photo`, `reset_alarm`, `sos_child`, `sos_adult` | Read/Write | Lệnh cạnh lên; firmware tự trả `false`. |
| `sos_authority_note` | Read/Write | Ghi chú tùy chọn gửi Apps Script với event. |

Raw telemetry như LDR, PIR, khoảng cách, score, threat level và trạng thái output là biến runtime/Serial trong Version 3, **không phải** Cloud property.

## 8. Giao tiếp Apps Script

Firmware gọi HTTPS GET với các trường chung: `event`, `source`, `device`, `location`, `status`, `threat`, `score`, `time`, `message`; có thể có `sos_authority_note`.

| Event/action | Mục đích | Phản hồi quan trọng |
|---|---|---|
| `heartbeat` | Cập nhật tín hiệu hoạt động server-side | `OK:HEARTBEAT` |
| `heartbeat_monitor_control` | Bật/tạm dừng monitor | `OK:HEARTBEAT_MONITOR_ENABLED/PAUSED` |
| `sabotage_alert` | Mở theo dõi sau phá hoại | `OK:SABOTAGE_MONITORING;eventId=...` |
| `sos_alert` | Tạo email xác nhận SOS | `OK:WAITING_CONFIRMATION;eventId=...` |
| `ai_person_detected` | Email thông tin phát hiện người | `OK:AI_PERSON_EMAIL_SENT` |
| `action=status` | Đọc event status (backend hỗ trợ) | `OK:STATUS;...` |
| `action=resolve` | Đóng event sau reset | `OK:RESOLVED` |
| `action=confirm`, `confirm_send` | Trang và lệnh Parent/Admin xác nhận | HTML result |

TLS ở firmware dùng `WiFiClientSecure.setInsecure()`. Đây là trạng thái kỹ thuật hiện tại, không phải tiêu chuẩn bảo mật lý tưởng.

## 9. Yêu cầu phi chức năng và ràng buộc

| ID | Yêu cầu đã triển khai |
|---|---|
| NFR-01 | Vòng lặp luôn gọi `ArduinoCloud.update()`; cảm biến theo chu kỳ 500 ms. |
| NFR-02 | HTTPS timeout: Apps Script 5 s, Telegram text 5 s, Telegram photo 10 s, Gemini 8 s. |
| NFR-03 | Gemini chỉ retry một lần cho HTTP 503, sau 1,5 giây. |
| NFR-04 | Không được hard-code Wi-Fi password, Device Key, Telegram token/chat ID, Apps Script URL hoặc Gemini key; dùng `arduino_secrets.h`. |
| NFR-05 | Không in toàn bộ `sos_authority_note` ra Serial. |
| NFR-06 | Nội dung cảnh báo hướng tới tiếng Việt có dấu, mô tả trung tính và nhắc người dùng kiểm tra thực tế trước khi reset. |
| NFR-07 | Backend dùng Script Properties và TTL 24 giờ, không phải kho lưu trữ audit dài hạn. |

## 10. Kịch bản nghiệm thu tối thiểu

| ID | Điều kiện/kích thích | Kết quả mong đợi |
|---|---|---|
| AT-01 | Bật schedule, RTC đến giờ arm/disarm | `alarm_enabled` đổi đúng giờ; SOS/sabotage không bị xóa. |
| AT-02 | Arm, PIR=1 và khoảng cách 15–50 cm trong ≥2 s | Intrusion latch, còi/LED, event; auto-photo tùy switch. |
| AT-03 | LDR covered và khoảng cách ≤15 cm trong ≥3 s | Sabotage latch, event backend monitoring, output local. |
| AT-04 | Bấm `sos_child` | SOS latch, Telegram text, email Family/Admin xác nhận; ACK không tắt output. |
| AT-05 | Bấm `manual_capture_photo` khi camera/Telegram bật | Một ảnh mới Telegram; property tự về `false`. |
| AT-06 | Tắt Wi-Fi >40 s, không có sabotage | Một email offline; khi heartbeat quay lại có một email recovery. |
| AT-07 | Sabotage active rồi ngừng heartbeat >40 s | Backend tạo critical và gửi email xác nhận; chưa gửi contact mô phỏng trước confirm. |
| AT-08 | Xác nhận critical/SOS nhưng thiếu địa chỉ/contact | Kết quả `NOT_CONFIGURED`; local alert vẫn giữ. |
| AT-09 | Bấm `reset_alarm` | Xóa latch local, tắt còi/LED đỏ, cố gắng `resolve` backend. |

## 11. Các quyết định/khác biệt quan trọng so với SRS cũ

- Không có BLE, Wi-Fi/MAC scanning hay auto re-arm theo thiết bị đáng tin.
- Intrusion Score vẫn được tính nhưng Version 3 dùng điều kiện PIR + khoảng cách liên tục để trigger, không dùng threshold score.
- Cảnh báo intrusion/sabotage có thể không gửi Telegram text nếu camera và Gemini cùng bật; khi đó luồng ảnh/AI là kênh cảnh báo chính.
- Critical compromise được phát hiện đáng tin cậy ở Apps Script khi board offline; firmware hiện chưa polling backend để phản ánh event critical đó lên Cloud sau khi kết nối lại.
- Chức năng chỉ được coi là đã triển khai khi mô tả ở các phần trên có bằng chứng trong Version 3; các hạng mục roadmap của `SRS.md` cũ không tự động thành yêu cầu của bản này.

---

## 12. Quy tắc nghiệp vụ chi tiết

### BR-01 — Phạm vi `alarm_enabled`

`alarm_enabled` chỉ cho phép hoặc chặn **FR-04 phát hiện đột nhập**. Giá trị này không được dùng để vô hiệu hóa:

- `sos_child` hoặc `sos_adult`;
- điều kiện sabotage;
- trạng thái critical đang tồn tại;
- lệnh `reset_alarm`;
- lịch sử heartbeat ở Apps Script.

### BR-02 — Điều kiện khoảng cách

| Điều kiện | Giá trị | Vai trò |
|---|---:|---|
| `object_near` | Khoảng cách `<= 50 cm` | Thành phần score và điều kiện intrusion. |
| `object_too_close` | Khoảng cách `<= 15 cm` | Điều kiện sabotage. |
| Vùng 15–50 cm | `>15 cm` và `<=50 cm` | Vùng hợp lệ cho intrusion đã arm. |
| Giá trị lỗi | `-1` | Không thỏa điều kiện near/too close. |

Một vật ở `<=15 cm` không tự tạo intrusion; nó chỉ là một phần của điều kiện sabotage khi LDR cũng bị che. Quy tắc này tránh cho hành vi áp sát/che thiết bị bị ghi nhận nhầm là đột nhập thông thường.

### BR-03 — Trình tự xử lý cảm biến

Trong mỗi chu kỳ sensor, firmware phải thực hiện theo thứ tự:

1. Đọc snapshot phần cứng và cập nhật telemetry runtime.
2. Kiểm tra lịch RTC.
3. Kẹp giá trị Dashboard không hợp lệ.
4. Đồng bộ `system_armed` từ `alarm_enabled`.
5. Đánh giá sabotage trước intrusion.
6. Đánh giá intrusion.
7. Cập nhật trạng thái hiển thị, threat level, notification placeholder và output vật lý.
8. Xử lý yêu cầu camera đang chờ.

### BR-04 — Score và threshold

`intrusion_score` và `sensitivity_level` phải được hiển thị/truyền theo bảng sau, dù không phải gate hiện tại của intrusion:

| Mức nhạy | Threshold telemetry | Ý nghĩa UI |
|---:|---:|---|
| 1 | 5 | Ít nhạy nhất |
| 2 | 4 | Trung bình |
| 3 | 3 | Nhạy nhất và là fallback mặc định |

`threat_level` được tính: SOS/sabotage → 4; intrusion → 3; score đạt threshold → 2; score dương → 1; còn lại → 0. Critical không có nhánh ưu tiên riêng trong hàm `updateThreatLevel`; sau khi backend đặt critical, mức hiển thị có thể được cập nhật lại theo các alert local còn tồn tại. Đây là hành vi code hiện tại cần được giữ đúng khi kiểm thử.

### BR-05 — LDR và night mode

- `light_abnormal` là chênh lệch tuyệt đối giữa hai lần LDR liên tiếp `>=500`.
- `night_mode=true` từ 22:00 đến trước 06:00 theo DS1307.
- Hai tín hiệu trên chỉ cộng score và mô tả event; không đủ một mình để kích hoạt intrusion.
- Cách diễn giải LDR bị che phụ thuộc lắp mạch: Version 3 mặc định coi `>=2000` là covered. Nếu phần cứng có chiều ngược lại, phải đổi hằng `LDR_COVER_WHEN_HIGH`, không tự ý đổi yêu cầu nghiệp vụ.

### BR-06 — Pet heuristic

`pet_detected = object_near AND NOT pir_detected AND NOT light_abnormal`. Giá trị này chỉ là gợi ý demo, không nhận diện vật nuôi bằng AI, không trừ score và không được coi là xác minh an toàn.

### BR-07 — Latch event và event counter

- Một alert mới chỉ tăng `event_counter` một lần khi latch chuyển từ false sang true.
- SOS có thể được gọi lặp nhưng chỉ tăng counter khi trước đó chưa active.
- Alert không tự clear sau khi điều kiện vật lý biến mất.
- `resetAllAlerts()` tăng counter một lần và ghi event `alarm_reset`.
- Event lịch arm/disarm cũng tăng counter.

### BR-08 — Quy tắc notification Telegram

| Sự kiện | Hành vi text | Hành vi ảnh |
|---|---|---|
| Intrusion | Chỉ gửi text fallback khi không đồng thời bật Gemini + camera | Chờ auto-photo nếu `auto_photo_on_alert`. |
| Sabotage | Tương tự intrusion | Chờ auto-photo nếu `auto_photo_on_alert`. |
| SOS | Luôn thử gửi text | Không tự chụp chỉ vì SOS. |
| Manual capture | Không gửi text riêng | Gửi ảnh nếu đủ cấu hình. |
| AI follow-up | Không gửi text/caption riêng | Gửi ảnh bằng chứng không chạy Gemini lại. |

Một lần gửi bị cooldown trả trạng thái `Đang chặn gửi lặp để tránh spam`. Cooldown không phải retry queue và bị mất khi reboot.

### BR-09 — Camera request

`manual_capture_photo` không được chụp ngay trong callback Cloud; callback chỉ đặt `manualCapturePending=true`. `processCameraRequests()` tiêu thụ cờ đó trong vòng lặp. Đây là quy tắc tránh chụp/gửi ảnh trực tiếp trong callback.

Mỗi capture phải:

1. kiểm tra `camera_enabled`, camera ready và cooldown 1,5 giây;
2. bỏ framebuffer cũ rồi lấy frame mới;
3. tùy cấu hình, phân tích Gemini trước khi trả framebuffer;
4. gửi Telegram khi reason được hỗ trợ;
5. luôn trả framebuffer cho driver sau khi dùng.

### BR-10 — Gemini và bằng chứng ảnh

Gemini chỉ nhận hai kết luận hợp lệ ở tầng firmware: `PERSON_DETECTED` hoặc `NO_PERSON`; các trạng thái còn lại là lỗi/cấu hình (`AI_NOT_CONFIGURED`, `AI_NO_WIFI`, `AI_HTTP_ERROR`, `AI_UNCLEAR`, ...).

Kết quả `PERSON_DETECTED` là thông báo bổ trợ, không thay đổi `alarm_status`, không thay thế sensor, không tự escalation và không phải nhận diện danh tính. Một email family được phép cho mỗi current automatic alert; các follow-up không được spam email.

### BR-11 — Heartbeat monitoring

- Backend mặc định monitoring khi chưa nhận lệnh pause rõ ràng.
- Pause monitoring phải lưu state `PAUSED`; bật lại không được gửi recovery nếu trước đó không offline.
- Offline bình thường chỉ gửi email khi state trước đó chưa `OFFLINE`.
- Khi heartbeat mới đến sau `OFFLINE`, backend gửi một recovery email rồi đặt state `ONLINE`.
- Nếu có active sabotage, timeout chuyển sang luồng critical thay vì gửi thêm email offline thường.

### BR-12 — Critical và xác nhận con người

Critical chỉ được tạo bởi Apps Script khi active sabotage liên quan và không có heartbeat/hết timeout. Critical không được tự tạo chỉ vì Wi-Fi mất kết nối trong trạng thái bình thường.

Luồng phải giữ tách biệt:

```text
SOS:       firmware → Apps Script → Family confirm → contact mô phỏng
Critical:  sabotage → heartbeat timeout ở Apps Script → Family confirm → contact mô phỏng
Offline:   heartbeat timeout không sabotage → Family offline email → recovery email
```

### BR-13 — Xác nhận không đồng nghĩa reset

Parent/Admin xác nhận gửi escalation có ý nghĩa cho phép gửi email contact mô phỏng. Nó không xác nhận khu vực đã an toàn và không được xóa latch tại ESP32. Người dùng chỉ được reset alarm sau khi kiểm tra thực tế.

### BR-14 — Address và note

Apps Script lấy địa chỉ từ Script Property `HOME_ADDRESS` nếu có, nếu không từ `CONFIG.HOME_ADDRESS`. Giá trị `DEMO_ADDRESS_NOT_CONFIGURED` coi là chưa cấu hình. `note` tối đa 1.000 ký tự trên confirm form; request parameters được sanitize tối đa 2.000 ký tự.

### BR-15 — Event retention và chống trùng

- Backend lưu record ở `ScriptProperties` với prefix `KSS_EVENT_` và TTL 24 giờ.
- Active SOS/sabotage pointer được dọn khi record tương ứng không còn.
- SOS callback/retry khi một SOS còn active trả event hiện hữu thay vì gửi một email xác nhận mới.
- `confirm_send` khóa `LockService` trong tối đa 10 giây và không gửi lại escalation khi record đã `emergency_confirmed`.

---

## 13. Use case chi tiết

### UC-01 — Parent/Admin bật hoặc tắt chống trộm

| Mục | Nội dung |
|---|---|
| Tác nhân | Parent/Admin |
| Tiền điều kiện | Arduino Cloud có kết nối hoặc sẽ đồng bộ lại. |
| Kích hoạt | Thay đổi `alarm_enabled`. |
| Luồng chính | Firmware lưu `last_event`; chu kỳ sensor tiếp theo đặt `system_armed` đúng bằng `alarm_enabled`. |
| Hậu điều kiện | Intrusion được cho phép/chặn; SOS và anti-sabotage không đổi. |
| Ngoại lệ | Khi đang có alert, đổi biến không reset alert đó. |

### UC-02 — Tự bật/tắt theo RTC

| Mục | Nội dung |
|---|---|
| Tác nhân | RTC DS1307 / Firmware |
| Tiền điều kiện | `schedule_enabled=true`, DS1307 hoạt động, cấu hình giờ hợp lệ. |
| Kích hoạt | Phút hiện tại khớp arm hoặc disarm schedule. |
| Luồng chính | Firmware thay `alarm_enabled`, ghi `last_event` và không lặp trong cùng phút bằng `lastScheduleTriggerKey`. |
| Hậu điều kiện | Chỉ chế độ intrusion đổi. |
| Ngoại lệ | RTC lỗi thì schedule không chạy. |

### UC-03 — Phát hiện đột nhập

| Mục | Nội dung |
|---|---|
| Tác nhân | PIR + ultrasonic / Firmware |
| Tiền điều kiện | Đã arm, qua boot grace, chưa có intrusion latch. |
| Kích hoạt | PIR và vùng khoảng cách 15–50 cm giữ 2 giây. |
| Luồng chính | Latch intrusion, cập nhật event/reason/score, bật output, đặt yêu cầu auto-photo và gửi notification theo rule. |
| Hậu điều kiện | Alert tồn tại đến reset. |
| Ngoại lệ | Camera/Telegram/Gemini lỗi không được làm mất output local. |

### UC-04 — Phát hiện che/can thiệp

| Mục | Nội dung |
|---|---|
| Tác nhân | LDR + ultrasonic / Firmware |
| Tiền điều kiện | Chưa có sabotage latch. |
| Kích hoạt | LDR covered và vật `<=15 cm` trong 3 giây. |
| Luồng chính | Latch sabotage, bật output, yêu cầu ảnh, gửi Apps Script `sabotage_alert`. |
| Hậu điều kiện | Apps Script tạo monitoring record với event ID. |
| Ngoại lệ | Nếu không có mạng, local sabotage vẫn active nhưng backend không nhận event. |

### UC-05 — Chụp ảnh từ Dashboard

| Mục | Nội dung |
|---|---|
| Tác nhân | Parent/Admin |
| Tiền điều kiện | Camera ready, `camera_enabled=true`; Telegram cấu hình nếu cần gửi ảnh. |
| Kích hoạt | `manual_capture_photo=true`. |
| Luồng chính | Callback queue request và auto-reset property; main loop chụp frame mới, có thể phân tích Gemini, gửi Telegram. |
| Hậu điều kiện | Cập nhật `photo_status`, `notification_sent_status`, `last_event`. |
| Ngoại lệ | Camera disabled/không ready/cooldown → status mô tả lỗi, không crash. |

### UC-06 — SOS và escalation mô phỏng

| Mục | Nội dung |
|---|---|
| Tác nhân | Child, Parent/Admin, Apps Script |
| Tiền điều kiện | Để email hoạt động: Wi-Fi, Apps Script URL và recipients hợp lệ. |
| Kích hoạt | `sos_child`, `sos_adult` hoặc Serial. |
| Luồng chính | Latch SOS → Telegram → Apps Script tạo record/email Family → Parent/Admin mở confirm link → backend gửi contact mô phỏng. |
| Hậu điều kiện | Email result `SENT` hoặc lỗi cấu hình; output local vẫn active. |
| Ngoại lệ | Mất mạng/config thiếu chỉ làm gửi thất bại, không tắt SOS local. |

### UC-07 — Mất liên lạc thường và recovery

| Mục | Nội dung |
|---|---|
| Tác nhân | Apps Script trigger |
| Tiền điều kiện | Monitoring enabled và trước đó đã có heartbeat. |
| Kích hoạt | Heartbeat age >40 s. |
| Luồng chính | Trigger mỗi phút gửi một email offline Family; heartbeat mới gửi một email recovery. |
| Hậu điều kiện | State backend cập nhật OFFLINE/ONLINE. |
| Ngoại lệ | Nếu sabotage active, chuyển UC-08 thay vì email offline thường. |

### UC-08 — Critical compromise sau sabotage

| Mục | Nội dung |
|---|---|
| Tác nhân | Apps Script trigger, Parent/Admin |
| Tiền điều kiện | Có active sabotage record. |
| Kích hoạt | Không có heartbeat hoặc heartbeat age >40 s. |
| Luồng chính | Tạo/đổi record thành critical → gửi email xác nhận Family → Parent/Admin xác nhận → gửi contact mô phỏng. |
| Hậu điều kiện | Có record critical và trạng thái escalation. |
| Ngoại lệ | Không khẳng định nguyên nhân vật lý; board offline không thể cập nhật property tức thời. |

### UC-09 — Reset alarm

| Mục | Nội dung |
|---|---|
| Tác nhân | Parent/Admin |
| Tiền điều kiện | Có thể có hoặc không có alert. |
| Kích hoạt | `reset_alarm=true` hoặc lệnh Serial `r`. |
| Luồng chính | Clear latch/cờ queue, reset status, tắt output, ghi event và gọi `action=resolve` nếu có mạng. |
| Hậu điều kiện | Dashboard command tự về false; local system trở về armed/disarmed ở chu kỳ sau. |
| Ngoại lệ | Nếu backend resolve lỗi, reset local vẫn không bị rollback. |

---

## 14. Hợp đồng dữ liệu và trạng thái backend

### 14.1 Cấu trúc event record logical

| Trường | Nguồn | Ý nghĩa |
|---|---|---|
| `eventId` | Apps Script hoặc request | Định danh event. |
| `eventType` | `sos_alert`, `sabotage_alert`, `critical_security_compromise` | Loại event. |
| `source` | Firmware/backend | Nguồn normalized. |
| `device`, `location` | Secrets/request | Thông tin triển khai. |
| `status`, `threat`, `score` | Firmware/request | Snapshot trạng thái nhận lúc event. |
| `rtcTime`, `serverTime` | RTC/backend | Hai nguồn thời gian độc lập. |
| `message`, `note` | Firmware/Parent | Nội dung event và ghi chú bổ sung. |
| `monitoring_status` | Backend | `MONITORING` hoặc `RESOLVED`. |
| `emergency_escalation_status` | Backend | `IDLE`, `WAITING_CONFIRMATION`, `CONFIRMED`, `SENT`, `FAILED`, `NOT_CONFIGURED`. |
| `emergency_authority_message_status` | Backend | Trạng thái gửi contact mô phỏng. |
| `confirmationSentAt`, `confirmedAt`, `escalationSentAt` | Backend | Dấu thời gian audit tối thiểu. |
| `lastHeartbeatAt`, `criticalRaisedAt` | Backend | Liên hệ sabotage/heartbeat. |

### 14.2 Mapping trạng thái người dùng

| Mã backend | Nội dung Cloud tiếng Việt |
|---|---|
| `IDLE` | Chưa có yêu cầu khẩn cấp / Chưa gửi contact mô phỏng |
| `SENDING` | Đang gửi yêu cầu xác nhận |
| `WAITING_CONFIRMATION` | Đang chờ phụ huynh/Admin xác nhận |
| `CONFIRMED` | Đã được phụ huynh/Admin xác nhận |
| `SENT` | Đã gửi email đến contact mô phỏng |
| `FAILED` | Gửi thất bại, cần kiểm tra cấu hình hoặc mạng |
| `NOT_CONFIGURED` | Chưa cấu hình đủ địa chỉ hoặc contact mô phỏng |
| `MONITORING` | Đang theo dõi sau cảnh báo phá hoại |

### 14.3 Trạng thái camera và notification

Các string dưới đây là contract UI thực tế, không phải enum đóng kín; Dashboard nên hiển thị nguyên văn và không dựa vào một mã duy nhất.

| Nhóm | Ví dụ trạng thái |
|---|---|
| Camera | `Camera sẵn sàng`, `Camera đang tắt từ Dashboard`, `Đang chụp ảnh`, `Đã chụp và gửi ảnh`, `Chụp ảnh thất bại`, `Camera chưa sẵn sàng`. |
| Telegram | `Đang gửi thông báo`, `Đã gửi thông báo`, `Gửi thông báo thất bại`, `Chưa cấu hình Telegram`, `Không có WiFi để gửi Telegram`. |
| Heartbeat | `Chưa gửi heartbeat`, `Đang gửi heartbeat`, `Đang liên lạc bình thường`, `Theo dõi heartbeat đang tạm dừng`, `Đang gửi lại lệnh theo dõi heartbeat`. |

---

## 15. Xử lý lỗi, suy giảm và an toàn

| ID | Tình huống | Hành vi bắt buộc/hiện thực |
|---|---|---|
| EH-01 | Camera init lỗi | Hệ thống tiếp tục sensor/còi/LED/text; `photo_status` nêu lỗi. |
| EH-02 | Không Wi-Fi | Output local tiếp tục; Telegram/Apps Script cập nhật status lỗi; Wi-Fi reconnect backoff. |
| EH-03 | Telegram token/chat ID thiếu | Không gửi request; cập nhật `Chưa cấu hình Telegram`. |
| EH-04 | Apps Script URL thiếu/sai | Event escalation status thành `NOT_CONFIGURED`; heartbeat nêu chưa cấu hình. |
| EH-05 | Gemini thiếu key/lỗi HTTP | Ghi kết quả AI lỗi; không chặn ảnh/alert local. |
| EH-06 | HTTPS redirect | GET client theo tối đa 4 redirect; quá giới hạn trả fail. |
| EH-07 | HTTP không phản hồi | Timeout theo từng service; client được stop. |
| EH-08 | DS1307 lỗi | `current_time=RTC_NOT_FOUND`, schedule không chạy; alert cảm biến vẫn hoạt động. |
| EH-09 | Siêu âm không trả echo | Khoảng cách -1, không coi near/too-close. |
| EH-10 | Dashboard gửi config sai | Firmware kẹp về default xác định. |
| EH-11 | Backend event hết hạn | Confirm/status hiển thị không tìm thấy/unknown event; không khôi phục record quá TTL. |
| EH-12 | Reset khi offline | Xóa latch local ngay; resolve backend chỉ best effort. |

### 15.1 Ràng buộc bảo mật

1. Secrets không được commit vào repository hoặc log ra Serial/Telegram/email.
2. Apps Script có thể yêu cầu `WEB_APP_TOKEN`; nếu token trống, endpoint cho phép request theo cấu hình hiện tại. Đây là lựa chọn deployment cần được bảo vệ bằng URL/permission Web App thích hợp.
3. Email escalation chỉ dùng recipients demo được owner chấp thuận. Không chuyển recipients này thành cơ quan thật bằng thay đổi code đơn lẻ.
4. `WiFiClientSecure.setInsecure()` nghĩa là firmware hiện không pin CA certificate. Nếu đưa ra môi trường thật, đây là rủi ro cần giải quyết bằng cơ chế xác thực TLS phù hợp.
5. Note và home address là dữ liệu nhạy cảm; chỉ dùng trong luồng escalation sau xác nhận và không in note đầy đủ ở Serial.

### 15.2 Giả định và phụ thuộc

| Phụ thuộc | Giả định |
|---|---|
| Arduino IoT Cloud | Thing, property name và Dashboard đã đồng bộ đúng với `thingProperties.h`. |
| Wi-Fi/Internet | Cần cho Cloud, Telegram, Gemini và Apps Script; không cần cho còi/LED/sensor local. |
| DS1307 | Đã đặt thời gian đúng và pin RTC hoạt động nếu cần schedule chính xác. |
| Telegram | Bot token/chat ID hợp lệ và bot được phép nhắn chat mục tiêu. |
| Gemini | API key có quyền dùng model cấu hình (`gemini-3.5-flash`) và quota còn đủ. |
| Apps Script | Web App đã deploy, có quyền MailApp và trigger `monitorHeartbeatAfterSabotage` được cài. |
| Recipients | Family và contact mô phỏng là địa chỉ hợp lệ, được sự đồng ý của chủ hệ thống. |

---

## 16. Ma trận truy vết yêu cầu đến mã nguồn

| Yêu cầu | Firmware / backend thực hiện chính |
|---|---|
| FR-01 | `readHardwareSnapshot`, `readUltrasonicDistanceCm`, `getRtcTimeString` |
| FR-02 | `updateScheduleLogic`, `clampCloudConfigValues`, callbacks schedule |
| FR-03 | `calculateRawIntrusionScore`, `getIntrusionThreshold`, `updateThreatLevel` |
| FR-04 | `updateIntrusionLogic`, `triggerIntrusionAlert` |
| FR-05 | `updateSabotageLogic`, `triggerSabotageAlert`, `handleSabotageRequest` |
| FR-06 | `triggerSosAlert`, `onSosChildChange`, `onSosAdultChange`, `handleSosRequest` |
| FR-07 | `captureSecurityPhoto`, `processCameraRequests`, `sendTelegramPhoto` |
| FR-08 | `analyzePersonWithGemini`, `handleAiPersonDetected` |
| FR-09 | `processGoogleAppsScriptHeartbeat`, `monitorHeartbeatConnection_`, `maintainWiFiConnection` |
| FR-10 | `monitorHeartbeatAfterSabotage`, `handleCriticalRequest` |
| FR-11 | `handleConfirmPage`, `handleConfirmSend`, `sendAuthorityEscalationEmail`, `handleResolve` |
| BR-08 | `isTelegramAllowed`, `notifySecurityTextEvent` |
| EH-01..12 | Status branches trong firmware và Apps Script theo từng service |

---

## 17. Quy trình kiểm thử và tiêu chí đạt

### 17.1 Chuẩn bị

1. Nạp firmware Version 3 vào Freenove ESP32-S3 WROOM + OV3660 với pin map đúng.
2. Đặt secrets trong Arduino Cloud/`arduino_secrets.h`; không đặt vào repository.
3. Deploy Apps Script Web App, cài trigger heartbeat mỗi phút và cấu hình recipient demo.
4. Xác nhận Dashboard có đúng toàn bộ property trong `thingProperties.h`.
5. Ghi lại giờ DS1307 và trạng thái Wi-Fi trước mỗi test.

### 17.2 Bằng chứng cần lưu cho mỗi test

- Thời điểm, người test, cấu hình service switch và input sensor.
- Ảnh Dashboard/Serial `last_event`, `alarm_status`, `photo_status`, `heartbeat_status`.
- Telegram message/photo nếu có.
- Email Family/Admin/contact mô phỏng nếu có.
- Event ID backend và kết quả pass/fail.

### 17.3 Test âm bắt buộc

| ID | Điều kiện | Kết quả không được xảy ra |
|---|---|---|
| NT-01 | Chỉ LDR thay đổi hoặc chỉ night mode | Không trigger intrusion. |
| NT-02 | PIR=true nhưng object quá xa/ultrasonic lỗi | Không trigger intrusion. |
| NT-03 | Vật `<=15 cm` nhưng LDR không covered | Không trigger sabotage. |
| NT-04 | Mất heartbeat không có sabotage | Không tạo critical/escalation. |
| NT-05 | Parent/Admin confirm email | Không tắt buzzer/LED nếu chưa `reset_alarm`. |
| NT-06 | Camera/Gemini/Telegram lỗi | Không tự clear intrusion, sabotage hoặc SOS. |
| NT-07 | Bật/tắt `alarm_enabled` khi SOS active | Không tắt SOS. |
| NT-08 | Lặp callback SOS đang active | Không gửi nhiều email confirmation cho cùng SOS active. |

---

## Phụ lục A — Cấu hình bí mật bắt buộc

`arduino_secrets.h` hoặc Arduino Cloud Secrets cần có: `SECRET_SSID`, `SECRET_OPTIONAL_PASS`, `SECRET_DEVICE_KEY`, `SECRET_GOOGLE_SCRIPT_URL`, `SECRET_TELEGRAM_BOT_TOKEN`, `SECRET_TELEGRAM_CHAT_ID`, `SECRET_GEMINI_API_KEY`, `SECRET_DEVICE_NAME`, `SECRET_DEVICE_LOCATION`.

Google Apps Script cần cấu hình recipient Family/Admin, recipient contact mô phỏng và `HOME_ADDRESS` (constant hoặc Script Property) trước khi escalation có thể gửi thành công.
