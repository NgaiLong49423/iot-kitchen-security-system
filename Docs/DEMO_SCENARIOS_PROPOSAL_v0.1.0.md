# DEMO.md

# Kịch bản trình diễn chính thức — Hệ thống chống trộm IoT

> **Tên tài liệu:** DEMO.md  
> **Phiên bản:** v1.0.0  
> **Ngày cập nhật:** 2026-07-11  
> **Dự án:** Kitchen Security System — Group 6  
> **Board chính:** Freenove ESP32-S3 WROOM + Camera OV3660  
> **Trạng thái:** Đã chốt phạm vi demo cuối cùng  
> **Mục tiêu:** Chuẩn hóa 6 kịch bản trình diễn chính, loại bỏ BLE trusted phone và quét WiFi/MAC để tập trung hoàn thiện các luồng có giá trị trình diễn cao và khả thi trong thời gian còn lại.

---

## 1. Phạm vi chính thức

### 1.1 Chức năng giữ lại

Phiên bản demo cuối cùng giữ các nhóm chức năng sau:

- bật/tắt chống trộm;
- tự bật/tắt chống trộm theo lịch RTC;
- phát hiện đột nhập bằng PIR, cảm biến siêu âm và LDR;
- weighted intrusion score — điểm đột nhập có trọng số;
- còi, LED đỏ và LED xanh;
- camera OV3660;
- chụp ảnh tự động khi có đột nhập;
- chụp ảnh thủ công từ dashboard;
- gửi cảnh báo và ảnh qua Telegram;
- cảnh báo phá hoại thiết bị;
- theo dõi tình trạng thiết bị sau khi bị phá hoại;
- heartbeat — tín hiệu định kỳ cho biết thiết bị vẫn đang hoạt động;
- SOS dành cho trẻ em và người lớn;
- email xác nhận khẩn cấp qua Google Apps Script;
- gửi escalation đến contact mô phỏng sau khi Parent/Admin xác nhận;
- event log — thông tin sự kiện gần nhất;
- reset tổng từ Parent/Admin;
- dashboard theo vai trò Child, Parent và Admin;
- nội dung hiển thị bằng tiếng Việt có dấu.

### 1.2 Chức năng loại khỏi phạm vi

Các nội dung sau không triển khai trong phiên bản demo cuối:

- BLE trusted phone;
- active BLE scan;
- whitelist `Mobile_Phone_Number1..4`;
- tự nhận diện người nhà bằng điện thoại;
- tự tắt chống trộm khi điện thoại người nhà xuất hiện;
- auto re-arm dựa trên sự hiện diện của điện thoại;
- quét WiFi/MAC xung quanh;
- cảnh báo thiết bị WiFi/MAC lạ;
- MAC whitelist;
- RSSI BLE;
- các kịch bản phụ thuộc BLE hoặc quét WiFi.

Các biến cũ liên quan đến BLE hoặc WiFi có thể vẫn được giữ trong code để tránh làm hỏng cấu hình Arduino Cloud, nhưng không được dùng làm chức năng chính hoặc đưa lên dashboard demo.

---

## 2. Nguyên tắc trình diễn

Demo phải ưu tiên:

- cảm biến thật;
- còi và LED thật;
- camera thật;
- Telegram thật;
- dashboard thật;
- Google Apps Script thật;
- reset thật;
- log và trạng thái thật.

Chỉ những tình huống có thể gây hỏng phần cứng hoặc không thể tạo an toàn trong lớp học mới được dùng controlled demo input — đầu vào demo có kiểm soát.

Không được tuyên bố hệ thống làm được chức năng chưa triển khai.

---

## 3. Danh sách 6 kịch bản chính

| Mã | Kịch bản | Giá trị nổi bật |
|---|---|---|
| DS-01 | Tự động bật chống trộm theo lịch RTC | Tự động hóa |
| DS-02 | Phát hiện đột nhập ban đêm bằng nhiều cảm biến | Chống trộm đa cảm biến |
| DS-03 | Thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại | Khả năng chịu lỗi và escalation |
| DS-04 | Phát hiện kẻ xấu đang che hoặc can thiệp thiết bị | Anti-sabotage |
| DS-05 | Phụ huynh kiểm tra khu vực từ xa bằng camera | Giám sát từ xa |
| DS-06 | Trẻ em bấm SOS và phụ huynh xác nhận khẩn cấp | Xử lý tình huống khẩn cấp |

> **Lưu ý về thứ tự:** DS-03 là hậu quả tiếp theo của DS-04. Vì vậy thứ tự trình diễn thực tế là:
>
> `DS-01 → DS-02 → DS-04 → DS-03 → DS-05 → DS-06`

---

# DS-01 — Tự động bật chống trộm theo lịch RTC

## 1. Bối cảnh

Gia đình muốn hệ thống tự bật bảo vệ vào buổi tối mà không cần thao tác thủ công mỗi ngày.

Parent/Admin đặt thời gian tự bật gần thời điểm trình diễn. RTC cung cấp thời gian cục bộ cho thiết bị.

## 2. Chức năng liên quan

- `schedule_enabled`
- `auto_arm_hour`
- `auto_arm_minute`
- `alarm_enabled`
- `system_armed`
- `alarm_status`
- `current_time`
- `last_event`
- `last_event_type`

## 3. Luồng demo

1. Parent/Admin bật `schedule_enabled`.
2. Đặt giờ tự bật gần thời điểm hiện tại.
3. Hệ thống tiếp tục hiển thị thời gian RTC.
4. Khi RTC đến đúng giờ:
   - `alarm_enabled = true`;
   - hệ thống tính lại `system_armed`;
   - `system_armed = true` nếu không có điều kiện ngăn cản;
   - trạng thái hiển thị chuyển sang đang bảo vệ.
5. LED xanh thể hiện hệ thống đang hoạt động bình thường.
6. Event log ghi nhận hệ thống đã tự bật chống trộm theo lịch.

## 4. Kết quả mong đợi

```text
Chế độ chống trộm đã được tự động bật theo lịch.
Hệ thống hiện đang bảo vệ khu vực bếp.
```

## 5. Điểm trình bày

- RTC là nguồn thời gian chính.
- Lịch chỉ thay đổi `alarm_enabled`.
- `system_armed` là trạng thái do hệ thống tự tính, không bị dashboard hoặc lịch ép trực tiếp.

---

# DS-02 — Phát hiện đột nhập ban đêm bằng nhiều cảm biến

## 1. Bối cảnh

Sau khi hệ thống tự bật bảo vệ, một người lạ đi vào khu vực bếp và dùng đèn pin để quan sát.

Hệ thống không phụ thuộc vào một cảm biến duy nhất mà kết hợp nhiều tín hiệu vật lý.

## 2. Chức năng liên quan

- `pir_detected`
- `object_near`
- `ldr_value`
- `ldr_delta`
- `light_abnormal`
- `night_mode`
- `intrusion_score`
- `intrusion_alert`
- `threat_level`
- `buzzer_on`
- `led_red_on`
- `led_green_on`
- `photo_status`
- Telegram notification
- `last_event`
- `last_event_type`
- `reset_alarm`

## 3. Weighted intrusion score

```text
PIR phát hiện chuyển động       +2
Vật thể ở gần                  +2
Ánh sáng thay đổi bất thường   +1
Ban đêm                        +1
```

## 4. Luồng demo

1. Hệ thống đang ở trạng thái bảo vệ.
2. Người demo đi ngang qua PIR.
3. Người demo tiến gần cảm biến siêu âm.
4. Người demo chiếu đèn pin vào khu vực LDR.
5. Hệ thống cập nhật các tín hiệu và tính `intrusion_score`.
6. Khi đủ ngưỡng:
   - `intrusion_alert = true`;
   - còi bật;
   - LED đỏ bật hoặc nháy;
   - LED xanh tắt;
   - camera tự chụp ảnh;
   - Telegram gửi cảnh báo và ảnh;
   - event log được cập nhật.
7. Người demo rời khỏi cảm biến.
8. Cảnh báo vẫn được giữ, không tự tắt khi điểm nghi ngờ giảm.
9. Parent/Admin bấm `reset_alarm`.

## 5. Kết quả mong đợi

```text
CẢNH BÁO ĐỘT NHẬP

Hệ thống phát hiện chuyển động, vật thể ở gần và thay đổi ánh sáng bất thường trong thời gian bảo vệ.

Còi và đèn cảnh báo đã được kích hoạt.
Camera đã chụp ảnh và gửi đến phụ huynh.
Vui lòng kiểm tra khu vực và reset cảnh báo sau khi an toàn.
```

## 6. Điểm trình bày

- Sensor fusion — kết hợp nhiều cảm biến.
- Không tự tắt cảnh báo khi kẻ trộm rời khỏi vùng quét.
- Camera và Telegram tạo bằng chứng từ xa.
- Reset là thao tác có chủ đích của Parent/Admin.

---

# DS-04 — Phát hiện kẻ xấu đang che hoặc can thiệp thiết bị

## 1. Bối cảnh

Kẻ xấu nhận ra thiết bị chống trộm và cố che cảm biến hoặc áp vật sát thiết bị để vô hiệu hóa hệ thống trước khi đột nhập.

Đây là giai đoạn hệ thống phát hiện hành vi phá hoại đang diễn ra.

## 2. Chức năng liên quan

- `ldr_covered`
- `object_near`
- `device_tampered`
- `sabotage_alert`
- `alarm_status`
- `threat_level`
- `buzzer_on`
- `led_red_on`
- Telegram notification
- `last_event`
- `last_event_type`
- `reset_alarm`

## 3. Quy tắc quan trọng

Anti-sabotage — chống phá hoại — hoạt động độc lập với:

- `alarm_enabled`;
- `system_armed`.

Ngay cả khi chống trộm đang tắt, thiết bị vẫn phải phát hiện hành vi che hoặc can thiệp.

## 4. Luồng demo

1. Có thể đặt `alarm_enabled = false` để chứng minh anti-sabotage hoạt động độc lập.
2. Người demo che LDR hoặc đặt vật quá gần cảm biến trong thời gian đủ dài.
3. Hệ thống xác nhận hành vi bất thường:
   - `device_tampered = true`;
   - `sabotage_alert = true`;
   - `alarm_status = SABOTAGE_ALERT`;
   - `threat_level` chuyển sang mức nghiêm trọng.
4. Hệ thống:
   - bật còi;
   - bật hoặc nháy LED đỏ;
   - gửi Telegram cảnh báo cho gia đình;
   - ghi nhận thời điểm và loại sự kiện;
   - bắt đầu theo dõi tình trạng hoạt động của thiết bị.
5. Không reset ngay vì DS-03 sẽ tiếp tục từ trạng thái này.

## 5. Nội dung cảnh báo đề xuất

```text
CẢNH BÁO PHÁ HOẠI THIẾT BỊ

Hệ thống phát hiện cảm biến đang bị che hoặc có vật áp sát trong thời gian bất thường.

Thiết bị vẫn đang hoạt động nhưng có nguy cơ bị can thiệp.
Hệ thống đã kích hoạt cảnh báo và đang tiếp tục theo dõi tình trạng thiết bị.

Vui lòng kiểm tra khu vực ngay.
```

## 6. Điểm trình bày

DS-04 trả lời câu hỏi:

> Hệ thống có phát hiện được khi kẻ xấu đang cố vô hiệu hóa thiết bị hay không?

DS-04 chưa kết luận thiết bị đã hỏng. Nó chỉ xác nhận hành vi can thiệp nghiêm trọng đang xảy ra.

---

# DS-03 — Thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại

## 1. Bối cảnh

DS-03 là giai đoạn tiếp theo của DS-04.

Sau khi hệ thống đã phát hiện hành vi che hoặc can thiệp, thiết bị tiếp tục gặp một trong các dấu hiệu nghiêm trọng:

- mất heartbeat;
- mất kết nối Cloud;
- camera không còn phản hồi;
- cảm biến siêu âm liên tục trả về lỗi;
- nhiều chức năng đồng thời không còn hoạt động;
- hệ thống mất một phần khả năng giám sát.

Hệ thống không khẳng định tuyệt đối rằng kẻ trộm đã phá hỏng một module cụ thể. Hệ thống chỉ kết luận trung tính:

> Ngay sau hành vi phá hoại, thiết bị đã mất liên lạc hoặc mất một phần khả năng bảo vệ. Có nguy cơ thiết bị đã bị vô hiệu hóa.

## 2. Tên trạng thái đề xuất

```text
CRITICAL_SECURITY_COMPROMISE
```

Ý nghĩa:

```text
Hệ thống an ninh đang ở tình trạng nghiêm trọng,
một phần khả năng giám sát có thể đã bị vô hiệu hóa.
```

## 3. Heartbeat

Heartbeat là tín hiệu định kỳ cho biết board vẫn đang hoạt động.

Ví dụ:

```text
Mỗi 10 giây:
ESP32-S3 gửi DEVICE_ALIVE đến Google Apps Script.
```

Google Apps Script lưu:

- thời điểm heartbeat gần nhất;
- trạng thái phá hoại gần nhất;
- thời điểm bắt đầu mất liên lạc.

## 4. Điều kiện kích hoạt

Không kích hoạt DS-03 chỉ vì một lần đọc lỗi.

Điều kiện đề xuất:

```text
SABOTAGE_ALERT đang active
+
mất heartbeat hoặc lỗi hệ thống nghiêm trọng
+
lỗi kéo dài quá thời gian cho phép
=
CRITICAL_SECURITY_COMPROMISE
```

Ví dụ:

```text
Đã phát hiện phá hoại
+
không nhận heartbeat trong 30–60 giây
=
thiết bị có thể đã mất nguồn, mất mạng hoặc bị vô hiệu hóa
```

Hoặc:

```text
Đã phát hiện phá hoại
+
camera thất bại nhiều lần liên tiếp
+
cảm biến siêu âm không phản hồi
=
khả năng giám sát đã suy giảm nghiêm trọng
```

## 5. Luồng demo

### Giai đoạn A — Tiếp nối DS-04

1. `sabotage_alert` vẫn đang active.
2. Hệ thống tiếp tục gửi heartbeat và kiểm tra tình trạng các chức năng.
3. Người demo dùng controlled demo input để mô phỏng:
   - mất heartbeat;
   - mất kết nối thiết bị;
   - hoặc nhiều module không phản hồi.

Không phá camera hoặc phần cứng thật trong lớp học.

### Giai đoạn B — Xác nhận suy giảm nghiêm trọng

4. Apps Script hoặc firmware phát hiện điều kiện nghiêm trọng.
5. Hệ thống chuyển sang:

```text
CRITICAL_SECURITY_COMPROMISE
```

6. Dashboard và Telegram hiển thị:

```text
CẢNH BÁO AN NINH NGHIÊM TRỌNG

Trước đó, hệ thống đã phát hiện hành vi che hoặc can thiệp thiết bị.

Ngay sau cảnh báo, thiết bị đã mất liên lạc hoặc mất một phần khả năng giám sát.

Các nguyên nhân có thể gồm:
- Thiết bị bị mất nguồn
- Kết nối mạng bị ngắt
- Một hoặc nhiều thành phần không còn phản hồi
- Thiết bị có thể đã bị vô hiệu hóa sau hành vi phá hoại

Hệ thống không thể tiếp tục xác nhận đầy đủ tình trạng tại khu vực.
```

### Giai đoạn C — Xin phép gửi escalation

7. Google Apps Script gửi email khẩn cấp cho Parent/Admin.
8. Email trình bày:
   - khu vực xảy ra sự cố;
   - thời điểm phát hiện phá hoại;
   - thời điểm mất liên lạc;
   - trạng thái thiết bị;
   - các khả năng có thể xảy ra;
   - địa chỉ nhà;
   - ô nhập ghi chú bổ sung.
9. Parent/Admin bấm:

```text
XÁC NHẬN GỬI BÁO CÁO KHẨN CẤP
```

10. Apps Script gửi báo cáo đến contact cơ quan chức năng mô phỏng.
11. Hệ thống ghi nhận:
   - đã yêu cầu xác nhận;
   - đã được xác nhận;
   - đã gửi escalation;
   - hoặc gửi thất bại.

## 6. Nội dung email xác nhận đề xuất

```text
CẢNH BÁO AN NINH NGHIÊM TRỌNG

Trước đó, hệ thống đã phát hiện hành vi phá hoại tại khu vực bếp.

Ngay sau sự kiện, thiết bị đã mất liên lạc hoặc mất một phần khả năng giám sát.

Hệ thống nghi ngờ thiết bị có thể đã bị mất nguồn, mất kết nối hoặc bị vô hiệu hóa.

Vui lòng kiểm tra tình hình và xác nhận có gửi báo cáo khẩn cấp đến contact tiếp nhận hay không.
```

## 7. Nội dung báo cáo gửi contact mô phỏng

```text
BÁO CÁO SỰ CỐ AN NINH NGHIÊM TRỌNG

Khu vực: Phòng bếp
Thời điểm phát hiện phá hoại: ...
Thời điểm mất liên lạc hoặc suy giảm chức năng: ...

Diễn biến:
1. Hệ thống phát hiện hành vi che hoặc can thiệp thiết bị.
2. Cảnh báo phá hoại đã được kích hoạt.
3. Ngay sau đó, thiết bị mất liên lạc hoặc nhiều chức năng không còn phản hồi.
4. Gia đình đã xác nhận gửi báo cáo khẩn cấp.

Địa chỉ: ...
Ghi chú của gia đình: ...
```

## 8. Giới hạn phải nói đúng

Nếu ESP32 bị mất điện hoàn toàn, chính ESP32 không thể gửi Telegram hoặc email.

Trong trường hợp đó, Google Apps Script phát hiện mất heartbeat và gửi email từ phía Cloud.

Mất heartbeat không chứng minh chắc chắn rằng kẻ trộm đã cắt điện. Nó cũng có thể do:

- mất WiFi;
- router bị tắt;
- board bị treo;
- dây nguồn bị rút;
- thiết bị bị phá hỏng.

Do đó, nội dung phải dùng các cụm:

```text
nghi ngờ
có nguy cơ
có thể đã bị vô hiệu hóa
mất liên lạc sau hành vi phá hoại
```

Không được khẳng định tuyệt đối:

```text
Kẻ trộm đã cắt điện thiết bị.
```

## 9. Điểm trình bày

DS-03 trả lời câu hỏi:

> Nếu kẻ xấu phá thiết bị thành công hoặc làm thiết bị mất liên lạc thì hệ thống còn làm được gì?

Giá trị nổi bật:

- theo dõi heartbeat từ Cloud;
- phát hiện mất liên lạc sau hành vi phá hoại;
- không phụ thuộc hoàn toàn vào ESP32 còn hoạt động;
- xin phép gia đình trước khi gửi escalation;
- không tự ý gửi đến contact tiếp nhận;
- báo cáo trung tính và không kết luận quá mức bằng chứng.

---

# DS-05 — Phụ huynh kiểm tra khu vực từ xa bằng camera

## 1. Bối cảnh

Phụ huynh đang ở ngoài nhà và muốn kiểm tra nhanh khu vực bếp mà không cần tạo cảnh báo đột nhập.

## 2. Chức năng liên quan

- `manual_capture_photo`
- `photo_status`
- camera OV3660
- Telegram
- `last_event`
- `last_event_type`

## 3. Luồng demo

1. Mở Parent Dashboard.
2. Bấm nút chụp ảnh thủ công.
3. Hệ thống cập nhật:

```text
Đang chụp ảnh
```

4. Camera chụp ảnh.
5. Ảnh được gửi qua Telegram.
6. Trạng thái cập nhật:

```text
Đã chụp và gửi ảnh thành công.
```

7. Nếu chụp thất bại:
   - hệ thống báo camera không khả dụng;
   - không làm treo các chức năng còn lại;
   - ghi nhận lỗi vào event log.

## 4. Điểm trình bày

- Không cần chờ cảnh báo mới kiểm tra được nhà.
- Phụ huynh có thể chủ động quan sát từ xa.
- Giống tính năng của sản phẩm an ninh thương mại.

---

# DS-06 — Trẻ em bấm SOS và phụ huynh xác nhận khẩn cấp

## 1. Bối cảnh

Trẻ em ở nhà gặp tình huống nguy hiểm và cần một thao tác đơn giản để gọi trợ giúp.

## 2. Chức năng liên quan

- `sos_child`
- `sos_adult`
- `sos_message`
- `emergency_confirmation_requested`
- `emergency_confirmed`
- `emergency_escalation_status`
- `emergency_authority_message_status`
- `sos_authority_note`
- `home_address_configured`
- `buzzer_on`
- `led_red_on`
- Telegram
- Google Apps Script
- `reset_alarm`

## 3. Luồng demo

1. Mở Child Dashboard.
2. Trẻ em bấm nút SOS.
3. Hệ thống:
   - chuyển sang `SOS_ALERT`;
   - bật còi;
   - bật hoặc nháy LED đỏ;
   - gửi Telegram cho gia đình;
   - gửi yêu cầu xác nhận qua Google Apps Script;
   - giữ cảnh báo tại chỗ.
4. Parent/Admin mở email.
5. Parent/Admin đọc:
   - nguồn SOS;
   - khu vực;
   - thời gian;
   - địa chỉ;
   - trạng thái hiện tại.
6. Parent/Admin nhập ghi chú bổ sung.
7. Parent/Admin xác nhận gửi escalation.
8. Apps Script gửi báo cáo đến contact mô phỏng.
9. Email xác nhận không tắt còi và LED.
10. Parent/Admin bấm `reset_alarm` sau khi đã xử lý tình huống.

## 4. Quy tắc quan trọng

```text
ACK không phải reset.
```

Việc xác nhận email chỉ cho phép gửi escalation. Còi, LED đỏ và trạng thái SOS phải duy trì cho đến khi Parent/Admin chủ động reset.

## 5. Nội dung cảnh báo đề xuất

```text
SOS KHẨN CẤP

Trẻ em đã kích hoạt nút SOS tại khu vực bếp.

Còi và đèn cảnh báo đang hoạt động.
Yêu cầu xác nhận đã được gửi đến phụ huynh.

Vui lòng kiểm tra tình hình ngay.
Xác nhận email không tự tắt báo động.
```

## 6. Điểm trình bày

- Dashboard đơn giản theo đúng vai trò trẻ em.
- Telegram thông báo nhanh.
- Email cho phép Parent/Admin xác nhận.
- Có địa chỉ và ghi chú bổ sung.
- Có quy trình escalation nhưng không tự gửi mà chưa có sự đồng ý.

---

## 4. Thứ tự trình diễn chính thức

```text
1. DS-01 — Tự bật chống trộm theo lịch RTC
2. DS-02 — Phát hiện đột nhập ban đêm
3. DS-04 — Phát hiện hành vi phá hoại
4. DS-03 — Mất liên lạc hoặc suy giảm sau phá hoại
5. DS-05 — Chụp ảnh kiểm tra từ xa
6. DS-06 — SOS trẻ em và escalation khẩn cấp
```

Lý do DS-04 được trình diễn trước DS-03:

```text
DS-04 = phát hiện kẻ xấu đang phá thiết bị
DS-03 = hậu quả nghiêm trọng nếu thiết bị mất liên lạc hoặc suy giảm sau đó
```

---

## 5. Các thay đổi cần triển khai để DS-03 hoạt động thật

### 5.1 Firmware ESP32-S3

Cần bổ sung:

- gửi heartbeat định kỳ đến Google Apps Script;
- trạng thái `critical_security_compromise`;
- ghi nhận thời điểm sabotage bắt đầu;
- kiểm tra lỗi camera hoặc cảm biến lặp lại;
- phân biệt lỗi một lần và lỗi kéo dài;
- gửi sự kiện critical damage/compromise đến Apps Script nếu ESP32 vẫn còn kết nối;
- thông báo tiếng Việt rõ ràng;
- giữ các chức năng còn hoạt động khi một chức năng bị lỗi.

### 5.2 Google Apps Script

Cần bổ sung:

- endpoint nhận heartbeat;
- lưu heartbeat gần nhất;
- lưu trạng thái sabotage;
- kiểm tra timeout heartbeat;
- event mới, ví dụ:

```text
critical_security_compromise
```

- email xác nhận dành riêng cho sự cố phá hoại nghiêm trọng;
- form nhập ghi chú;
- gửi authority escalation sau khi Parent/Admin xác nhận;
- không trộn event này với `sos_alert`;
- thông báo bằng tiếng Việt có dấu;
- chống gửi email lặp cho cùng một sự kiện.

### 5.3 Dashboard

Admin Dashboard nên hiển thị:

- trạng thái heartbeat;
- thời điểm thiết bị hoạt động gần nhất;
- cảnh báo phá hoại;
- trạng thái suy giảm nghiêm trọng;
- trạng thái gửi email xác nhận;
- trạng thái escalation;
- event gần nhất.

Parent Dashboard nên hiển thị:

- cảnh báo nghiêm trọng;
- tình trạng liên lạc thiết bị;
- trạng thái xác nhận;
- trạng thái gửi báo cáo;
- nút reset khi thiết bị vẫn còn kết nối.

---

## 6. Những nội dung không được nói quá khi demo

Không được nói:

- hệ thống biết chắc kẻ trộm đã cắt điện;
- hệ thống biết chính xác module nào bị phá nếu không có kiểm tra riêng;
- hệ thống vẫn gửi được từ ESP32 khi ESP32 đã mất điện;
- hệ thống tự động gửi email đến cơ quan chức năng mà không cần xác nhận;
- hệ thống đã triển khai BLE hoặc quét WiFi.

Nên nói:

- thiết bị mất liên lạc ngay sau hành vi phá hoại;
- hệ thống nghi ngờ thiết bị đã bị mất nguồn, mất mạng hoặc bị vô hiệu hóa;
- Cloud phát hiện mất heartbeat;
- gia đình được yêu cầu xác nhận trước khi gửi báo cáo;
- contact tiếp nhận hiện là contact mô phỏng phục vụ demo.

---

## 7. Kịch bản dự phòng

### 7.1 Camera lỗi nhưng cảnh báo vẫn hoạt động

- camera không chụp được;
- còi và LED vẫn hoạt động;
- Telegram gửi cảnh báo văn bản;
- event log ghi nhận camera không khả dụng.

### 7.2 Internet mất nhưng báo động cục bộ vẫn hoạt động

- PIR, siêu âm và LDR vẫn được xử lý;
- còi và LED vẫn hoạt động;
- Cloud và Telegram tạm thời không cập nhật;
- không tuyên bố có retry nếu chưa triển khai.

### 7.3 Cảnh báo không tự tắt khi tín hiệu cảm biến biến mất

- kẻ trộm rời khỏi vùng cảm biến;
- cảnh báo vẫn được giữ;
- chỉ Parent/Admin mới reset được.

---

## 8. Kết luận

Bộ 6 kịch bản cuối cùng thể hiện 6 giá trị sản phẩm khác nhau:

```text
DS-01: Tự động hóa
DS-02: Phát hiện đột nhập đa cảm biến
DS-04: Phát hiện hành vi phá hoại
DS-03: Phát hiện mất liên lạc và escalation sau phá hoại
DS-05: Giám sát hình ảnh từ xa
DS-06: Xử lý SOS khẩn cấp
```

Chuỗi DS-04 → DS-03 là điểm nhấn kỹ thuật của bản demo:

```text
Phát hiện đang bị phá
→ Theo dõi tình trạng thiết bị
→ Thiết bị mất liên lạc hoặc suy giảm
→ Cloud phát hiện tình trạng nghiêm trọng
→ Gia đình xác nhận
→ Gửi báo cáo khẩn cấp đến contact mô phỏng
```

Đây là hướng trình diễn mạnh, có câu chuyện rõ ràng và không phụ thuộc vào BLE hoặc quét WiFi.
