# Đặc tả yêu cầu phần mềm (SRS): Hệ thống chống trộm IoT

> **Phiên bản tài liệu:** 0.8.0
> **Trạng thái:** Checkpoint triển khai ổn định kết nối và cấu hình dịch vụ theo Dashboard; thay đổi này chưa được nạp/test tích hợp trên thiết bị
> **Ngày tạo:** 2026-06-26
> **Sửa đổi gần nhất:** 2026-07-20
> **Mục đích:** Làm nguồn yêu cầu chính cho agent, chatbot, lập trình viên và người rà soát. Nếu có mâu thuẫn với tài liệu cũ, tài liệu này được ưu tiên.

---

## Tóm tắt tài liệu

Tài liệu này đặc tả yêu cầu phần mềm cho hệ thống chống trộm IoT dùng Arduino Cloud, board `Freenove ESP32-S3 WROOM + Camera OV3660`, cảm biến chuyển động/khoảng cách/ánh sáng, còi, LED, camera, Telegram và Google Apps Script.

Bản 0.7.0 là checkpoint khóa phạm vi cuối trước demo. Phiên bản này chính thức loại BLE trusted phone và quét WiFi/MAC khỏi phạm vi bắt buộc. Từ v0.8.0, các property và logic cũ liên quan phải được xóa thay vì chỉ giữ để tương thích.

Bản 0.7.1 giữ chuỗi xử lý phá hoại hai giai đoạn và ghi nhận cấu hình đã test thực tế. Bản 0.8.0 chốt đợt triển khai tiếp theo: làm sạch hoàn toàn property/logic legacy, cho phép bật/tắt từng dịch vụ từ Dashboard, giảm tải đồng bộ Cloud và tăng khả năng phục hồi khi dùng hotspot điện thoại.

```text
DS-04: Phát hiện kẻ xấu đang che hoặc can thiệp thiết bị
→ theo dõi tình trạng thiết bị bằng heartbeat và health check đơn giản
→ DS-03: Thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại
→ yêu cầu Parent/Admin xác nhận
→ Google Apps Script gửi báo cáo đến contact cơ quan chức năng mô phỏng
```

Hệ thống không được khẳng định chắc chắn nguyên nhân mất liên lạc hoặc module nào bị hỏng nếu không có đủ bằng chứng. Nội dung phải dùng cách diễn đạt trung tính như “mất liên lạc sau hành vi phá hoại”, “có nguy cơ bị vô hiệu hóa” hoặc “mất một phần khả năng giám sát”. Chuẩn nội dung tiếng Việt có dấu của v0.6.9 tiếp tục được giữ nguyên.

### Cấu hình triển khai đã kiểm thử

Các giá trị trong bảng này là nguồn ưu tiên khi các phần mô tả cũ bên dưới còn dùng ngưỡng hoặc mã trạng thái kế hoạch.

| Hạng mục | Giá trị đang chạy và đã test |
|---|---|
| Board/camera | Freenove ESP32-S3 WROOM với camera OV3660 |
| DS-04 anti-sabotage | `ldr_covered = true` **và** khoảng cách siêu âm `<= 15 cm` liên tục 3 giây |
| Phản hồi DS-04 | Còi, LED đỏ, Telegram chữ, một ảnh Telegram và event `sabotage_alert` đến Apps Script |
| Heartbeat | ESP32 gửi mỗi 10 giây khi `google_script_enabled` và `heartbeat_enabled` cùng bật; Apps Script lưu giờ server của heartbeat cuối |
| Timeout mất kết nối | Apps Script đánh dấu heartbeat quá hạn sau 40 giây; trigger quét mỗi 1 phút nên email thực tế có thể đến khoảng 40–100 giây sau heartbeat cuối |
| Đồng bộ Cloud | Cảm biến vẫn đọc mỗi 500 ms; chỉ publish định kỳ mỗi 3 giây hoặc ngay khi có thay đổi an ninh quan trọng |
| Kết nối | WiFi reconnect backoff 2 → 5 → 10 → 30 giây; HTTPS có timeout cứng và không chạy đồng thời nhiều request nặng |
| DS-05 | `manual_capture_photo` là nút cạnh lên; firmware chốt lệnh vào cờ nội bộ rồi tự trả biến Cloud về `false` |
| `photo_status` | Dùng câu tiếng Việt như `Đang chụp ảnh`, `Đã chụp và gửi ảnh` hoặc thông báo lỗi cụ thể, không dùng các mã `CAPTURED`/`FAILED` làm giao diện chính |
| Địa chỉ escalation demo | Trường Đại học FPT Campus TP.HCM, Lô E2a-7, Đường D1, Khu Công nghệ cao, Phường Tăng Nhơn Phú, TP. Hồ Chí Minh; chỉ dùng trong email contact mô phỏng sau xác nhận |

## Mục lục

- 0. Kiểm soát tài liệu
- 1. Giới thiệu
- 2. Mô tả tổng quan
- 3. Nguyên tắc thiết kế hệ thống
- 4. Yêu cầu giao diện bên ngoài
- 5. Yêu cầu biến Cloud
- 6. Yêu cầu trạng thái hệ thống
- 7. Quy tắc nghiệp vụ
- 8. Yêu cầu chức năng
- 9. Yêu cầu phi chức năng
- 10. Ràng buộc
- 11. Giả định và phụ thuộc
- 12. Ngoài phạm vi
- 13. Vấn đề cần phân rã tiếp
- 14. Phụ lục A — Checkpoint quyết định hiện tại

---

## 0. Kiểm soát tài liệu

### 0.1 Thông tin tài liệu

| Mục | Giá trị |
|---|---|
| Tên dự án | Hệ Thống Chống Trộm IoT |
| Tên tiếng Anh | IoT Based Anti-Theft System |
| Tên tài liệu | SRS.md |
| Loại tài liệu | Software Requirements Specification |
| Phiên bản tài liệu | 0.8.0 |
| Phiên bản nguồn | 0.6.2 |
| Ngày tạo | 2026-06-26 |
| Sửa đổi gần nhất | 2026-07-20 |
| Người phụ trách | Group 6 / Ngô Gia Long |
| Use Case chính | Giám sát chống trộm thông minh cho khu vực phòng bếp |
| Khu vực giám sát hiện tại | Phòng bếp |
| Hướng mở rộng | Nhiều module theo khu vực như bếp, phòng ngủ, cửa chính, phòng khách |
| Cách viết | Theo cấu trúc kiểu IEEE, tối ưu để agent và người bảo trì đọc |
| Ngôn ngữ | Tiếng Việt; giữ nguyên mã biến, enum, ID và thuật ngữ kỹ thuật bằng tiếng Anh khi cần |
| Trạng thái hiện tại | Checkpoint 0.8.0; làm sạch legacy, cấu hình dịch vụ, heartbeat liên tục và độ ổn định kết nối trước khi triển khai |

### 0.2 Lịch sử phiên bản

| Phiên bản | Ngày tạo | Sửa đổi gần nhất | Người phụ trách | Tóm tắt thay đổi |
|---|---|---|---|---|
| 0.1.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Tạo bản SRS checkpoint đầu tiên sau khi làm sạch phạm vi và phân rã cụm Security Mode Control. Loại bỏ báo cháy và RFID-RC522 khỏi phạm vi chính. Thêm quy tắc tự tắt chống trộm khi phát hiện thiết bị đáng tin và cấu hình thời gian tự bật lại bảo vệ. |
| 0.2.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Cập nhật checkpoint sau khi phân rã thêm: điều khiển chống trộm theo lịch, phát hiện đột nhập bằng `intrusion_score`, cấu hình độ nhạy bằng `sensitivity_level`, bật/tắt phát hiện WiFi/MAC lạ, WiFi/MAC lạ chỉ là pre-warning, giữ điểm nghi ngờ 5 giây, kích hoạt cảnh báo đột nhập đầy đủ, gửi thông báo khi có đột nhập, tự chụp 1 ảnh cho mỗi lần cảnh báo đột nhập mới và cho phép chụp thủ công thêm. |
| 0.3.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Cập nhật checkpoint sau khi phân rã `threat_level`, lọc báo động giả/vật nuôi, Telegram notification và chống spam WiFi/MAC lạ. Đổi nhóm biến `email_*` sang `notification_*`, chọn Telegram là kênh mặc định, bổ sung rule WiFi/MAC lạ lặp lại cùng MAC 3 lần trong 60 giây và cooldown 60 giây. |
| 0.4.0 | 2026-06-29 | 2026-06-29 | Group 6 / Ngô Gia Long | Cập nhật checkpoint demo scope: chốt SOS luôn hoạt động độc lập, SOS bật còi/LED, xác nhận escalation qua email Google Apps Script, gửi email đến demo authority contact sau xác nhận, Anti-Sabotage dùng LDR bị che và vật quá gần cảm biến, Event Logging chỉ lưu sự kiện gần nhất, Cooldown notification đơn giản và Demo Mode không bắt buộc triển khai phức tạp. |
| 0.5.0 | 2026-06-29 | 2026-06-29 | Group 6 / Ngô Gia Long | Hoàn tất phân rã yêu cầu trước khi code: chốt trạng thái `notification_sent_status`, trạng thái `emergency_escalation_status`, luồng Google Apps Script cho SOS confirmation/escalation, dashboard tối thiểu theo vai trò, xử lý camera thất bại đơn giản và bộ test case demo cuối. |
| 0.6.0 | 2026-06-29 | 2026-06-29 | Group 6 / Ngô Gia Long | Cập nhật sau rà soát mâu thuẫn: chốt ưu tiên `alarm_status`, reset tổng, tách Telegram notification và SOS email escalation, polling Google Apps Script, timeout SOS 60 giây, cấu hình authority contact trong Apps Script, tự reset notification request, trạng thái notification gần nhất, tách chụp ảnh thủ công/tự động, thêm LED xanh, rule LED đỏ nháy nội bộ, cooldown hiển thị, anti-sabotage 5 giây và pipeline tính `intrusion_score`. |
| 0.6.1 | 2026-06-26 | 2026-07-01 | Group 6 / Ngô Gia Long | Chuẩn hóa cấu trúc tài liệu, cách trình bày, heading, thuật ngữ và câu chữ; không thay đổi phạm vi chức năng, biến Cloud, quy tắc nghiệp vụ hoặc test case demo so với checkpoint 0.6.0. |
| 0.6.2 | 2026-06-26 | 2026-07-01 | Group 6 / Ngô Gia Long | Cập nhật nền tảng phần cứng chính từ ESP32-CAM cũ sang `ESP32-S3 CAM OV2640 N16R8`; bổ sung ghi chú về camera OV2640, bộ nhớ N16R8, ràng buộc dùng pinout/board profile đúng theo board mới; không thay đổi logic nghiệp vụ hoặc phạm vi chức năng chống trộm. |
| 0.6.3 | 2026-06-26 | 2026-07-03 | Group 6 / Ngô Gia Long | Cập nhật phần cứng chính từ `ESP32-S3 CAM OV2640 N16R8` sang `Freenove ESP32-S3 WROOM + Camera OV3660`; thay camera từ OV2640 sang OV3660; loại bỏ giả định bộ nhớ N16R8 khỏi yêu cầu hiện tại; không thay đổi logic nghiệp vụ, biến Cloud hoặc phạm vi chức năng chống trộm. |
| 0.6.4 | 2026-06-26 | 2026-07-03 | Group 6 / Ngô Gia Long | Làm rõ MVP demo hoàn chỉnh, cập nhật quy tắc kiểm soát WiFi/MAC lạ bằng `unknown_wifi_detection_enabled`, sửa lỗi tên event `photo_capture_failed`, thống nhất cảm biến siêu âm HY-SRF05 / HC-SR04-compatible và cho phép controlled demo input cho `known_device_present` nếu môi trường demo nhiễu. |
| 0.6.5 | 2026-06-26 | 2026-07-03 | Group 6 / Ngô Gia Long | Bổ sung quét Bluetooth Low Energy (BLE) bằng ESP32-S3 để nhận diện thiết bị/điện thoại đáng tin của chủ nhà; cập nhật `known_device_present` có thể lấy từ BLE trusted device; mở rộng SOS escalation qua Google Apps Script để gửi địa chỉ nhà và ghi chú bổ sung của Parent/Admin đến contact cơ quan chức năng/công an được cấu hình hợp lệ sau khi xác nhận; bổ sung ràng buộc pháp lý/an toàn và trạng thái cấu hình địa chỉ. |
| 0.6.6 | 2026-06-26 | 2026-07-03 | Group 6 / Ngô Gia Long | Cập nhật DS-03/FR-04 từ intrusion score đồng trọng số sang weighted intrusion score: PIR và HY-SRF05/object-near cộng +2, LDR/light abnormal và night mode cộng +1; cập nhật ngưỡng `sensitivity_level` thành 5/4/3; làm rõ LDR và night mode là tín hiệu bối cảnh, không nên tự tạo alarm khi thiếu xác nhận vật lý mạnh. |
| 0.6.7 | 2026-06-26 | 2026-07-03 | Group 6 / Ngô Gia Long | Chốt DS-06/FR-10 theo logic SOS alarm latch: SOS luôn bật còi/LED đỏ trong lúc chờ xác nhận, sau khi Parent/Admin ACK và sau khi gửi authority escalation; ACK không phải reset; chỉ `reset_alarm` từ Parent/Admin mới tắt còi/LED và kết thúc trạng thái SOS active. |
| 0.6.8 | 2026-06-26 | 2026-07-06 | Group 6 / Ngô Gia Long | Chốt cách triển khai FR-16 bằng app BLE advertiser bên thứ ba trên điện thoại: dùng `Complete Local Name` trong `Scan Response data`, ESP32-S3 dùng active scan, whitelist tên `Mobile_Phone_Number1..4`, không dùng MAC address/pairing làm định danh chính; cập nhật acceptance test BLE. |
| 0.6.9 | 2026-06-26 | 2026-07-10 | Group 6 / Ngô Gia Long | Chốt FR-18 và bộ quy tắc BR-88..BR-95 về nội dung hiển thị đa kênh: tiếng Việt có dấu, giọng trung tính, giải thích trạng thái/nguyên nhân/hành động/bước tiếp theo; áp dụng cho Serial Monitor/CLI, Arduino Cloud, Telegram và Gmail/Google Apps Script; giữ nguyên mã kỹ thuật nội bộ và không thay đổi logic nghiệp vụ. |
| 0.7.0 | 2026-06-26 | 2026-07-11 | Group 6 / Ngô Gia Long | Khóa phạm vi demo cuối: loại BLE trusted phone, auto re-arm theo thiết bị đáng tin và quét WiFi/MAC khỏi phạm vi; bỏ đóng góp WiFi lạ khỏi `intrusion_score`; bổ sung heartbeat ESP32-S3 → Google Apps Script, trạng thái `CRITICAL_SECURITY_COMPROMISE`, theo dõi mất liên lạc/suy giảm sau `sabotage_alert`, email xác nhận và authority escalation riêng cho sự cố phá hoại nghiêm trọng; cập nhật 6 kịch bản demo chính và dashboard. |
| 0.8.0 | 2026-06-26 | 2026-07-20 | Group 6 / Ngô Gia Long | Chốt triển khai ổn định khi dùng hotspot: xóa property/callback/logic BLE, trusted-device, WiFi/MAC scanning và re-arm legacy; thêm 7 công tắc dịch vụ Dashboard; heartbeat liên tục 10 giây, timeout 40 giây, email offline một lần và email khôi phục; giữ critical escalation sau sabotage có xác nhận; giảm tần suất publish cảm biến, WiFi reconnect backoff và timeout HTTPS. |

### 0.3 Quy tắc dành cho agent và người bảo trì

Phần này dành cho AI agent, chatbot, lập trình viên và người bảo trì khi đọc hoặc cập nhật tài liệu.

1. `SRS.md` là nguồn yêu cầu chính ở checkpoint hiện tại.
2. Khi tài liệu demo, dashboard hoặc code cũ mâu thuẫn với SRS v0.8.0, phải ưu tiên SRS v0.8.0.
3. Khi cập nhật SRS, phải giữ bảng lịch sử phiên bản và ghi rõ thay đổi phạm vi.
4. `alarm_enabled` chỉ điều khiển chức năng phát hiện đột nhập; không được dùng để tắt SOS hoặc anti-sabotage.
5. `system_armed` là trạng thái kết quả do hệ thống tự tính; dashboard và lịch không được ép trực tiếp biến này.
6. Lịch tự động chỉ thay đổi `alarm_enabled`.
7. Cảnh báo đột nhập, SOS, sabotage và critical compromise không được tự tắt khi tín hiệu đầu vào biến mất; chỉ `reset_alarm` từ Parent/Admin mới kết thúc cảnh báo còn active khi board vẫn liên lạc được.
8. Weighted intrusion score phải dùng PIR `+2`, object-near `+2`, light abnormal `+1`, night mode `+1`; không còn thành phần WiFi/MAC lạ.
9. BLE trusted phone, `known_device_present`, auto re-arm theo điện thoại và quét WiFi/MAC đã bị loại khỏi phạm vi v0.8.0. Không được tự triển khai lại nếu chưa có phiên bản SRS mới.
10. Các property, widget Dashboard, callback và logic BLE/WiFi/re-arm legacy phải bị xóa hoàn toàn; không giữ tương thích bằng biến chết.
11. DS-04 là phát hiện hành vi phá hoại đang diễn ra; DS-03 là trạng thái nghiêm trọng tiếp theo khi thiết bị mất liên lạc hoặc suy giảm sau DS-04.
12. Không được kích hoạt `CRITICAL_SECURITY_COMPROMISE` chỉ vì mất mạng bình thường. Phải có `sabotage_alert` trước đó và có bằng chứng mất heartbeat hoặc lỗi nghiêm trọng kéo dài.
13. Một lần đọc camera/cảm biến lỗi không đủ để kết luận suy giảm nghiêm trọng; phải dùng số lần lỗi liên tiếp hoặc timeout đã cấu hình.
14. Nếu ESP32-S3 mất điện hoàn toàn, ESP32 không thể gửi email. Google Apps Script phải phát hiện việc này bằng heartbeat quá hạn.
15. Mất heartbeat không chứng minh chắc chắn kẻ trộm cắt điện; nội dung phải nêu các khả năng như mất nguồn, mất mạng, board treo hoặc bị vô hiệu hóa.
16. SOS escalation và critical-compromise escalation là hai event khác nhau, nhưng đều phải có xác nhận Parent/Admin trước khi gửi đến contact mô phỏng.
17. Không được giả mạo critical compromise thành `sos_alert`.
18. Contact cơ quan chức năng trong demo phải là contact mô phỏng; không tự động gửi đến địa chỉ công an/cơ quan thật khi chưa có phê duyệt hợp pháp.
19. ACK hoặc xác nhận email không phải reset cảnh báo cục bộ.
20. Telegram là kênh thông báo nhanh; Google Apps Script là kênh xác nhận và escalation.
21. Camera lỗi không được làm mất còi, LED, log hoặc cảnh báo chữ nếu các chức năng đó còn hoạt động.
22. `reset_alarm` là reset tổng các cảnh báo active tại board nhưng không xóa `last_event`, `last_event_type` hoặc `event_counter`.
23. Nội dung người dùng trên Serial Monitor/CLI, Arduino Cloud, Telegram và Gmail/Google Apps Script phải dùng tiếng Việt có dấu, trung tính và không khẳng định quá mức bằng chứng.
24. Mã nội bộ như `INTRUSION_ALERT`, `SABOTAGE_ALERT`, `CRITICAL_SECURITY_COMPROMISE`, `SENT`, `FAILED` vẫn được giữ cho code và debug nhưng không được hiển thị một mình cho người dùng.
25. Các tài liệu dashboard cũ chỉ là tài liệu tham khảo bố cục; dashboard v0.8.0 phải bỏ toàn bộ widget BLE/WiFi/re-arm, thêm widget heartbeat/critical compromise và nhóm công tắc dịch vụ.
26. Demo Mode chỉ được dùng để mô phỏng mất liên lạc hoặc suy giảm mà không phá hỏng phần cứng thật; phải nói rõ đây là controlled demo input.
27. Không được thêm phần cứng mới vào phạm vi bắt buộc nếu không cập nhật SRS.
28. Board chính vẫn là `Freenove ESP32-S3 WROOM + Camera OV3660`; không thay pin map đã chốt.
29. Các yêu cầu lịch sử đã bị loại vẫn được giữ ID để truy vết nhưng phải có nhãn `Loại khỏi phạm vi`.
30. Mọi thay đổi code phải được đối chiếu với 6 kịch bản demo chính và acceptance tests của FR-15, FR-19, FR-20 và FR-21.

### 0.4 Quy ước thuật ngữ và trình bày

- Tên biến Cloud, enum, ID yêu cầu và tên kỹ thuật được giữ nguyên bằng tiếng Anh để tránh sai lệch khi triển khai.
- Các thuật ngữ chuyên ngành được ưu tiên giải thích ngay trong bảng thuật ngữ hoặc trong ngữ cảnh sử dụng.
- Các quy tắc nghiệp vụ dùng tiền tố `BR`; yêu cầu chức năng dùng tiền tố `FR`; yêu cầu phi chức năng dùng tiền tố `NFR`; ràng buộc dùng tiền tố `CON`.
- Nội dung yêu cầu phải đủ rõ để triển khai và kiểm thử, nhưng không thay thế cho mã nguồn chi tiết.
- Những phần chưa được phân rã đầy đủ phải được ghi rõ là `Chờ phân rã` hoặc đưa vào mục vấn đề cần phân rã tiếp.

---

## 1. Giới Thiệu

### 1.1 Mục đích

Tài liệu này mô tả các yêu cầu phần mềm cho hệ thống chống trộm IoT. Hệ thống được thiết kế để giám sát khu vực phòng bếp bằng cảm biến, Arduino Cloud Dashboard, cơ chế cảnh báo, còi/LED tại chỗ và ghi nhận sự kiện.

Mục đích của SRS là tạo ra một tài liệu chuyên nghiệp, dễ bảo trì và dễ đọc cho agent. Đây không phải bản chuyển đổi đơn thuần từ tài liệu cũ. Đây là bản đặc tả yêu cầu đã được thiết kế lại theo các quyết định hiện tại.

### 1.2 Phạm vi

Hệ thống tập trung vào bảo vệ an ninh cho một khu vực giám sát là phòng bếp.

Phạm vi v0.8.0 gồm:

- bật/tắt chống trộm và tự động hóa theo RTC;
- phát hiện đột nhập bằng PIR, HY-SRF05 và LDR;
- weighted intrusion score;
- còi, LED đỏ và LED xanh;
- camera OV3660, chụp ảnh tự động và thủ công;
- Telegram notification;
- chống phá hoại bằng dấu hiệu che LDR hoặc vật áp sát bất thường;
- heartbeat ESP32-S3 đến Google Apps Script;
- phát hiện mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại;
- SOS Child/Adult;
- xác nhận email và authority escalation đến contact mô phỏng;
- dashboard Child, Parent và Admin;
- event log đơn giản.

Phiên bản này không triển khai nhận diện người nhà bằng BLE và không quét WiFi/MAC lạ.

Tài liệu vẫn dùng khái niệm `monitored zone` — khu vực giám sát — để có thể mở rộng trong tương lai, nhưng phiên bản hiện tại chỉ có phòng bếp.

### 1.3 Góc nhìn sản phẩm

Sản phẩm là một hệ thống an ninh IoT nhúng, bao gồm:

- module vi điều khiển,
- cảm biến chuyển động/khoảng cách/ánh sáng,
- biến Arduino Cloud,
- dashboard theo vai trò người dùng,
- đầu ra cảnh báo như còi và LED,
- cơ chế gửi thông báo,
- ghi log sự kiện,
- định hướng kiến trúc mở rộng nhiều khu vực.

### 1.4 Thuật ngữ

| Thuật ngữ | Ý nghĩa |
|---|---|
| SRS | Software Requirements Specification, tài liệu đặc tả yêu cầu phần mềm |
| IoT | Internet of Things, hệ thống thiết bị kết nối Internet |
| Freenove ESP32-S3 WROOM | Board điều khiển chính của hệ thống |
| OV3660 | Cảm biến camera dùng để chụp ảnh |
| Monitored Zone | Khu vực đang được hệ thống giám sát; hiện tại là phòng bếp |
| Dashboard | Giao diện điều khiển và giám sát trên Arduino Cloud |
| Cloud Variable | Biến đồng bộ giữa board và Arduino Cloud |
| Weighted Intrusion Score | Điểm đột nhập có trọng số từ nhiều tín hiệu vật lý |
| Anti-sabotage | Cơ chế phát hiện hành vi che hoặc can thiệp thiết bị |
| Heartbeat | Tín hiệu định kỳ từ ESP32-S3 đến Google Apps Script để báo thiết bị vẫn hoạt động |
| Heartbeat Timeout | Khoảng thời gian không nhận heartbeat trước khi xem thiết bị đã mất liên lạc |
| Health Check | Kiểm tra đơn giản xem một chức năng như camera hoặc siêu âm còn phản hồi hay không |
| System Degraded | Hệ thống vẫn hoạt động nhưng đã mất một phần khả năng giám sát hoặc cảnh báo |
| Critical Security Compromise | Trạng thái an ninh nghiêm trọng khi thiết bị mất liên lạc hoặc suy giảm ngay sau cảnh báo phá hoại |
| Event Correlation | Đối chiếu nhiều sự kiện theo thời gian; ví dụ sabotage xảy ra trước rồi heartbeat mất sau đó |
| Authority Escalation | Gửi báo cáo đã được Parent/Admin xác nhận đến contact cơ quan chức năng hoặc contact mô phỏng |
| ACK | Xác nhận đã nhận hoặc cho phép gửi escalation; ACK không phải reset |
| Controlled Demo Input | Đầu vào demo có kiểm soát để mô phỏng tình huống khó hoặc nguy hiểm mà không phá phần cứng thật |
| Telegram Bot | Kênh gửi cảnh báo nhanh và ảnh |
| Google Apps Script | Web App nhận heartbeat, lưu event, gửi email xác nhận và authority escalation |
| `intrusion_score` | Điểm nghi ngờ đột nhập |
| `threat_level` | Mức nguy hiểm tổng thể, từ 0 đến 4 |
| `alarm_enabled` | Công tắc bật/tắt chức năng phát hiện đột nhập |
| `system_armed` | Trạng thái chống trộm thực tế do hệ thống tự tính |
| `sabotage_alert` | Cảnh báo phát hiện hành vi can thiệp hoặc che thiết bị |
| `critical_security_compromise` | Cờ xác nhận hệ thống mất liên lạc hoặc suy giảm nghiêm trọng sau sabotage |
| Cooldown | Thời gian chờ để tránh gửi cảnh báo lặp |
| Agent | AI/chatbot/dev tool đọc tài liệu để hỗ trợ code hoặc kiểm thử |
| User-facing Message | Nội dung người dùng nhìn thấy trên Dashboard, Telegram, Gmail hoặc Serial Monitor |
| Internal Status Code | Mã kỹ thuật dùng trong code và kiểm thử |
| Message Mapping | Ánh xạ mã kỹ thuật sang câu tiếng Việt dễ hiểu |
| CLI / Serial Monitor | Kênh văn bản để theo dõi và chẩn đoán thiết bị |

## 2. Mô Tả Tổng Quan

### 2.1 Các chức năng chính của sản phẩm

| ID | Chức năng | Trạng thái v0.8.0 |
|---|---|---|
| FR-01 | Điều khiển chế độ chống trộm | Bắt buộc |
| FR-02 | Xử lý reset báo động | Bắt buộc |
| FR-03 | Điều khiển chống trộm theo lịch RTC | Bắt buộc |
| FR-04 | Phát hiện đột nhập đa cảm biến | Bắt buộc |
| FR-05 | Phát hiện WiFi/MAC lạ | Loại khỏi phạm vi v0.7.0 |
| FR-06 | Gửi Telegram / notification | Bắt buộc |
| FR-07 | Tự chụp ảnh khi có đột nhập | Bắt buộc |
| FR-08 | Lọc báo động giả / vật nhỏ | Giữ ở mức đơn giản |
| FR-09 | Phát hiện hành vi phá hoại thiết bị | Bắt buộc — DS-04 |
| FR-10 | SOS qua Dashboard và Google Apps Script | Bắt buộc — DS-06 |
| FR-11 | Ghi log sự kiện gần nhất | Bắt buộc |
| FR-12 | Chống spam cảnh báo / cooldown | Bắt buộc |
| FR-13 | Chế độ demo có kiểm soát | Bắt buộc ở mức tối thiểu |
| FR-14 | Dashboard Child, Parent và Admin | Bắt buộc |
| FR-15 | Demo Acceptance Test | Bắt buộc |
| FR-16 | Nhận diện thiết bị đáng tin bằng BLE | Loại khỏi phạm vi v0.7.0 |
| FR-17 | SOS escalation có địa chỉ và ghi chú | Bắt buộc |
| FR-18 | Nội dung đa kênh bằng tiếng Việt | Bắt buộc |
| FR-19 | Gửi và theo dõi heartbeat | Bắt buộc — DS-03 |
| FR-20 | Phát hiện critical security compromise sau sabotage | Bắt buộc — DS-03 |
| FR-21 | Xác nhận và escalation cho sự cố phá hoại nghiêm trọng | Bắt buộc — DS-03 |

### 2.2 Nhóm người dùng

| Vai trò | Mô tả | Quyền chính |
|---|---|---|
| Trẻ em | Người dùng chỉ cần thao tác SOS đơn giản | Bấm SOS, xem trạng thái cơ bản |
| Người lớn / Phụ huynh | Người dùng chính trong nhà | Bật/tắt chống trộm, reset báo động, bấm SOS, cấu hình thời gian tự bật lại, xem cảnh báo |
| Admin / Người demo | Leader, tester hoặc người thuyết trình | Quan sát toàn bộ, điều khiển demo, xem biến debug, xem countdown |
| Hệ thống / Board | Thiết bị nhúng chạy logic cảm biến | Đọc cảm biến, cập nhật biến, kích hoạt cảnh báo, ghi log |
| Dịch vụ thông báo bên ngoài | Telegram Bot hoặc dịch vụ notification | Nhận loại sự kiện và gửi thông báo |
| Google Apps Script | Web App xử lý email confirmation và SOS authority escalation | Gửi email xác nhận, nhận ACK, ghép địa chỉ nhà/ghi chú bổ sung và gửi email escalation |
| Contact cơ quan chức năng/công an | Email/contact được cấu hình hợp lệ trong Apps Script hoặc contact mô phỏng khi demo | Nhận email escalation sau khi Parent/Admin xác nhận |

### 2.3 Môi trường vận hành

Hệ thống chạy trên `Freenove ESP32-S3 WROOM + Camera OV3660`, Arduino Cloud, Telegram Bot và Google Apps Script.

| Thành phần | Vai trò |
|---|---|
| ESP32-S3 WROOM + OV3660 | Điều khiển chính, xử lý cảm biến, camera và kết nối mạng |
| PIR | Phát hiện chuyển động |
| HY-SRF05 / HC-SR04-compatible | Đo khoảng cách và phát hiện vật áp sát |
| LDR | Phát hiện thay đổi ánh sáng và dấu hiệu bị che |
| DS1307 RTC | Cung cấp thời gian cho lịch và log |
| Buzzer | Cảnh báo âm thanh tại chỗ |
| LED đỏ | Cảnh báo hình ảnh khi có sự cố |
| LED xanh | Báo hệ thống bình thường |
| Arduino Cloud | Đồng bộ biến và dashboard |
| Telegram Bot | Gửi cảnh báo nhanh và ảnh |
| Google Apps Script | Nhận heartbeat, lưu event, gửi email xác nhận và escalation |
| Contact mô phỏng | Nhận báo cáo sau khi Parent/Admin xác nhận |

WiFi được dùng để kết nối Cloud, Telegram và Apps Script. WiFi không được dùng để quét hoặc nhận diện thiết bị xung quanh trong phạm vi v0.7.0.

### 2.4 Nội dung đã loại khỏi phạm vi hiện tại

- BLE trusted phone và active BLE scan;
- whitelist `Mobile_Phone_Number1..4`;
- `known_device_present` làm nguồn tự tắt chống trộm;
- auto re-arm dựa trên điện thoại rời khỏi khu vực;
- quét WiFi/MAC lạ;
- unknown WiFi pre-warning;
- MAC whitelist và RSSI BLE;
- Flame Sensor, báo cháy và RFID-RC522;
- gửi trực tiếp đến cơ quan chức năng thật khi chưa có phê duyệt hợp pháp;
- streaming video liên tục;
- nhận diện khuôn mặt;
- xác định chắc chắn module nào hỏng nếu không có cảm biến phản hồi riêng.

Các biến BLE/WiFi/re-arm cũ phải bị xóa khỏi code và Thing, không được dùng trong logic nghiệp vụ hoặc dashboard demo.

### 2.5 Phạm vi MVP demo hoàn chỉnh bắt buộc

Phiên bản demo phải chạy được 6 kịch bản:

| Thứ tự trình diễn | Mã | Kịch bản |
|---:|---|---|
| 1 | DS-01 | Tự động bật chống trộm theo lịch RTC |
| 2 | DS-02 | Phát hiện đột nhập ban đêm bằng nhiều cảm biến, camera và Telegram |
| 3 | DS-04 | Phát hiện kẻ xấu đang che hoặc can thiệp thiết bị |
| 4 | DS-03 | Thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại, sau đó xin xác nhận escalation |
| 5 | DS-05 | Phụ huynh chụp ảnh kiểm tra khu vực từ xa |
| 6 | DS-06 | Trẻ em bấm SOS và phụ huynh xác nhận escalation |

DS-04 phải được trình diễn trước DS-03 vì DS-03 là giai đoạn tiếp theo của cùng chuỗi tấn công.

## 3. Nguyên Tắc Thiết Kế Hệ Thống

### 3.1 Phiên bản hiện tại một khu vực, tương lai nhiều khu vực

Phiên bản hiện tại xử lý phòng bếp là khu vực giám sát duy nhất.

Tuy nhiên, yêu cầu nên dùng cách diễn đạt:

```text
khu vực giám sát
```

thay vì hardcode mọi luật vào riêng phòng bếp. Điều này giúp SRS dễ mở rộng sau này.

### 3.2 Chế độ chống trộm khác với bật/tắt toàn hệ thống

Biến `alarm_enabled` không được hiểu là tắt toàn bộ thiết bị.

`alarm_enabled` chỉ điều khiển chức năng phát hiện đột nhập.

Ngay cả khi `alarm_enabled = false`, hệ thống vẫn tiếp tục hỗ trợ:

- xử lý SOS,
- chống phá hoại thiết bị,
- cập nhật trạng thái dashboard,
- đồng bộ Cloud,
- ghi log sự kiện,
- reset báo động,
- chụp ảnh thủ công nếu chức năng này được bật.

### 3.3 `system_armed` là trạng thái kết quả

`system_armed` là trạng thái chống trộm thực tế do hệ thống tính toán. Người dùng, lịch tự động và dashboard không được ép trực tiếp `system_armed`.

Đầu vào chính là `alarm_enabled`, lịch RTC và trạng thái cảnh báo active. BLE/WiFi/re-arm không tham gia phép tính `system_armed`.

### 3.4 Không phụ thuộc vào nhận diện điện thoại

Phiên bản v0.7.0 không dùng BLE hoặc WiFi/MAC để nhận diện người nhà. Khi `alarm_enabled = true` và không có cảnh báo active, `system_armed` phải được tính trực tiếp là `true`.

Việc bật/tắt bảo vệ do Parent/Admin hoặc lịch RTC điều khiển.

### 3.5 Heartbeat và phát hiện mất liên lạc

ESP32-S3 phải gửi heartbeat định kỳ đến Google Apps Script. Heartbeat chỉ dùng để chứng minh thiết bị còn liên lạc, không dùng để phát hiện người nhà.

Mất heartbeat bình thường chỉ tạo trạng thái mất kết nối. Chỉ khi mất heartbeat xảy ra sau `sabotage_alert` trong cửa sổ theo dõi thì hệ thống mới được nâng thành `CRITICAL_SECURITY_COMPROMISE`.

### 3.6 Authority escalation phải có xác nhận và cấu hình hợp lệ

Google Apps Script hỗ trợ hai loại escalation:

1. SOS do Child/Adult kích hoạt.
2. Critical security compromise sau sabotage.

Cả hai loại đều phải:

- gửi yêu cầu xác nhận đến Parent/Admin;
- cho phép ghi chú bổ sung;
- chỉ gửi đến contact mô phỏng sau khi có xác nhận;
- không tự reset còi hoặc cảnh báo cục bộ;
- dùng địa chỉ và contact được cấu hình bảo mật.

### 3.7 Nguyên tắc nội dung hiển thị đa kênh

Hệ thống có bốn nhóm kênh văn bản chính:

1. Serial Monitor/CLI dùng để quan sát và chẩn đoán thiết bị.
2. Arduino Cloud Dashboard dùng để xem trạng thái và thao tác điều khiển.
3. Telegram dùng để nhận cảnh báo từ xa.
4. Gmail và trang Web App của Google Apps Script dùng cho xác nhận SOS, critical compromise và authority escalation.

Các kênh trên phải tuân theo một chuẩn nội dung thống nhất:

- dùng tiếng Việt có dấu theo mã hóa UTF-8,
- diễn đạt trung tính, không dùng giọng quá cảm tính hoặc phóng đại,
- giải thích rõ hệ thống đang làm gì thay vì chỉ hiển thị mã trạng thái,
- không khẳng định chắc chắn có trộm hoặc phá hoại khi hệ thống mới chỉ phát hiện tín hiệu nghi ngờ,
- nêu hành động hệ thống đã thực hiện và thao tác tiếp theo nếu người dùng cần xử lý,
- giữ mã kỹ thuật nội bộ để debug nhưng không dùng mã đó làm toàn bộ nội dung người dùng,
- cùng một sự kiện phải có cùng ý nghĩa trên mọi kênh.

Việc chuẩn hóa nội dung chỉ thay đổi lớp trình bày và thông báo. Nó không được thay đổi logic cảm biến, `intrusion_score`, `threat_level`, trạng thái cảnh báo, cooldown, lịch, heartbeat, SOS hoặc reset.

---

## 4. Yêu Cầu Giao Diện Bên Ngoài

### 4.1 Yêu cầu giao diện người dùng

Hệ thống cần có ba loại dashboard.

| Dashboard | Người dùng | Mục đích chính | Biến chính |
|---|---|---|---|
| Child SOS Dashboard | Trẻ em | Gửi SOS đơn giản và xem trạng thái cơ bản | `sos_child`, `alarm_status`, `last_event` |
| Parent Control Dashboard | Người lớn | Điều khiển chống trộm, lịch, reset, SOS, chụp ảnh và xem cảnh báo nghiêm trọng | `alarm_enabled`, `schedule_enabled`, `sos_adult`, `reset_alarm`, `alarm_status`, `critical_security_compromise`, `device_connection_status`, `manual_capture_photo`, `photo_status`, `last_event` |
| Admin Demo Dashboard | Admin / Người demo | Quan sát cảm biến, heartbeat, critical compromise, debug và chọn kịch bản | cảm biến, `intrusion_score`, `sabotage_alert`, `last_heartbeat_time`, `compromise_reason`, `critical_escalation_status`, `demo_mode`, `demo_scenario` |

Chi tiết bố cục dashboard có thể nằm ở tài liệu thiết kế dashboard riêng. SRS chỉ định nghĩa yêu cầu ở mức vai trò, widget chính và biến liên quan.

### 4.1.1 Yêu cầu nội dung hiển thị cho người dùng

Mỗi thông báo có ý nghĩa phải cung cấp các thành phần phù hợp với ngữ cảnh:

| Thành phần | Yêu cầu |
|---|---|
| Tên sự kiện/trạng thái | Viết bằng tiếng Việt có dấu và dùng tên nhất quán giữa các kênh |
| Trạng thái hiện tại | Cho biết hệ thống đang bật, tắt, cảnh báo, chờ xác nhận, đang gửi, thành công hay thất bại |
| Nguyên nhân/tín hiệu | Nêu tín hiệu liên quan như chuyển động, vật thể ở gần, ánh sáng bất thường hoặc SOS; không suy diễn quá mức |
| Hành động hệ thống | Nêu rõ còi/LED/camera/Telegram/email đã được kích hoạt hay chưa |
| Bước tiếp theo | Khi cần, hướng dẫn người dùng bấm `reset_alarm`, kiểm tra kết nối, cấu hình địa chỉ hoặc xác nhận SOS |
| Thời gian và khu vực | Hiển thị khi sự kiện là cảnh báo, SOS, email escalation hoặc log quan trọng |
| Mã kỹ thuật | Chỉ dùng bổ sung cho debug; không được đứng một mình trong nội dung người dùng |

Ví dụ cách diễn đạt phù hợp:

```text
Phát hiện dấu hiệu đột nhập trong khu vực bếp. Hệ thống đã bật còi, đèn đỏ và yêu cầu camera chụp ảnh. Cảnh báo sẽ duy trì cho đến khi người dùng đặt lại cảnh báo trên Arduino Cloud.
```

Ví dụ không phù hợp khi hiển thị cho người dùng:

```text
INTRUSION_ALERT
```

### 4.1.2 Phạm vi từng kênh

| Kênh | Yêu cầu nội dung |
|---|---|
| Serial Monitor/CLI | Có thể giữ nhãn kỹ thuật, giá trị cảm biến và mã event để debug, nhưng mỗi sự kiện quan trọng phải có câu giải thích tiếng Việt rõ ràng |
| Arduino Cloud | Các biến String và tên widget người dùng nhìn thấy phải dùng nội dung tiếng Việt dễ hiểu; mã enum nội bộ phải được ánh xạ sang câu hiển thị |
| Telegram | Thông báo cảnh báo phải nêu sự kiện, thời gian, khu vực, nguyên nhân chính, hành động đã thực hiện và bước xử lý tiếp theo nếu có |
| Gmail/Google Apps Script | Email, trang xác nhận, trang kết quả và thông báo lỗi phải dùng tiếng Việt có dấu; tên trường kỹ thuật chỉ xuất hiện ở phần chẩn đoán nếu cần |

### 4.2 Yêu cầu giao tiếp phần cứng

| Phần cứng | Yêu cầu |
|---|---|
| PIR sensor | Hệ thống phải đọc trạng thái chuyển động từ PIR. |
| HY-SRF05 ultrasonic sensor | Hệ thống phải đọc khoảng cách để hỗ trợ phát hiện vật thể và lọc báo động giả. |
| LDR sensor | Hệ thống phải đọc mức ánh sáng và độ thay đổi ánh sáng để phát hiện ánh sáng bất thường hoặc che cảm biến. |
| DS1307 RTC | Hệ thống phải dùng thời gian RTC cho log sự kiện và logic theo thời gian nếu cần. |
| Buzzer | Hệ thống phải bật còi cho các cảnh báo đang hoạt động theo luật sự kiện. |
| LED đỏ | Hệ thống phải bật LED đỏ cho các cảnh báo đang hoạt động theo luật sự kiện; khi `led_red_on = true`, LED đỏ nháy theo logic nội bộ. |
| LED xanh | Hệ thống phải bật LED xanh khi hệ thống chạy bình thường và không có cảnh báo active. |
| Camera OV3660 trên `Freenove ESP32-S3 WROOM + Camera OV3660` | Hệ thống phải hỗ trợ chụp ảnh tự động cho đột nhập và chụp thủ công từ dashboard. |

### 4.3 Yêu cầu giao tiếp phần mềm

| Giao tiếp | Yêu cầu |
|---|---|
| Arduino Cloud | Đồng bộ biến điều khiển, cảm biến, cảnh báo và dashboard |
| Telegram Bot API | Gửi cảnh báo chữ và ảnh khi có sự kiện an ninh |
| Google Apps Script Web App | Nhận heartbeat, sabotage event, SOS event, critical compromise event; cung cấp trang xác nhận và status polling |
| Script Properties | Lưu địa chỉ nhà, contact mô phỏng, token và trạng thái event nhạy cảm |

Apps Script phải phân biệt tối thiểu các event:

```text
sos_alert
sabotage_alert
critical_security_compromise
heartbeat
```

### 4.4 Yêu cầu giao tiếp truyền thông

- ESP32-S3 dùng WiFi để kết nối Arduino Cloud, Telegram và Google Apps Script.
- ESP32-S3 gửi heartbeat mỗi 10 giây khi dịch vụ Google Apps Script và heartbeat được bật; chu kỳ này chỉ thay đổi sau khi kiểm thử lại luồng offline và DS-03.
- Google Apps Script xem heartbeat quá hạn khi không nhận tín hiệu trong 40 giây.
- Heartbeat request phải có tối thiểu `deviceId`, `time`, `status` và token nếu bật xác thực.
- `sabotage_alert` phải được gửi một lần cho mỗi sự kiện mới, không gửi lặp ở mỗi vòng lặp.
- Khi ESP32-S3 còn mạng và tự phát hiện nhiều chức năng lỗi, board có thể chủ động gửi `critical_security_compromise`.
- Khi board mất mạng hoặc mất nguồn, Apps Script phải dùng heartbeat timeout để phát hiện mất liên lạc.
- Thất bại mạng không được chặn việc đọc cảm biến, còi và LED tại chỗ.

## 5. Yêu Cầu Biến Cloud

### 5.1 Mô hình biến Cloud

Hệ thống dùng mô hình một Thing cho phiên bản hiện tại:

```text
1 Device
→ 1 Thing
→ Nhiều Dashboard
```

Tên Thing đề xuất:

```text
IoT_Anti_Theft_System
```

### 5.2 Quy tắc Permission

| Permission | Ý nghĩa |
|---|---|
| Read & Write | Dashboard đọc được biến và gửi lệnh/cấu hình xuống thiết bị |
| Read Only | Dashboard chỉ xem; board là nguồn cập nhật chính |

### 5.3 Quy tắc Update Policy

| Update Policy | Ý nghĩa |
|---|---|
| On Change | Chỉ gửi cập nhật khi giá trị thay đổi |
| Periodic | Gửi cập nhật theo chu kỳ đã cấu hình |

### 5.4 Danh sách biến Cloud hiện tại

> Danh sách này là phạm vi v0.8.0. Chỉ các biến trong bảng này được phép tồn tại trong Thing/Dashboard hiện hành.

| Nhóm | Biến | Kiểu | Quyền | Dashboard | Mục đích |
|---|---|---|---|---|---|
| Điều khiển | `alarm_enabled` | bool | Read & Write | Parent, Admin | Bật/tắt phát hiện đột nhập |
| Điều khiển | `system_armed` | bool | Read Only | Parent, Admin | Trạng thái bảo vệ thực tế |
| Điều khiển | `reset_alarm` | bool | Read & Write | Parent, Admin | Reset cảnh báo active tại board |
| Trạng thái | `alarm_status` | String | Read Only | Child, Parent, Admin | Trạng thái tổng đã ánh xạ tiếng Việt |
| Lịch | `schedule_enabled` | bool | Read & Write | Parent, Admin | Bật/tắt lịch RTC |
| Lịch | `auto_arm_hour` | int | Read & Write | Parent, Admin | Giờ tự bật |
| Lịch | `auto_arm_minute` | int | Read & Write | Parent, Admin | Phút tự bật |
| Lịch | `auto_disarm_hour` | int | Read & Write | Parent, Admin | Giờ tự tắt |
| Lịch | `auto_disarm_minute` | int | Read & Write | Parent, Admin | Phút tự tắt |
| Cấu hình | `sensitivity_level` | int | Read & Write | Parent, Admin | Mức nhạy 1–3 |
| Dịch vụ | `telegram_enabled` | bool | Read & Write | Parent, Admin | Bật/tắt toàn bộ Telegram |
| Dịch vụ | `telegram_photo_enabled` | bool | Read & Write | Parent, Admin | Cho phép gửi ảnh qua Telegram khi Telegram và camera cùng bật |
| Dịch vụ | `google_script_enabled` | bool | Read & Write | Parent, Admin | Bật/tắt event, email flow và heartbeat qua Google Apps Script |
| Dịch vụ | `heartbeat_enabled` | bool | Read & Write | Parent, Admin | Bật/tắt gửi heartbeat; khi tắt, Apps Script phải coi monitoring là paused |
| Dịch vụ | `camera_enabled` | bool | Read & Write | Parent, Admin | Cho phép camera chụp ảnh |
| Dịch vụ | `gemini_enabled` | bool | Read & Write | Admin | Cho phép phân tích ảnh Gemini; mặc định tắt |
| Dịch vụ | `auto_photo_on_alert` | bool | Read & Write | Parent, Admin | Tự chụp khi có intrusion, sabotage hoặc SOS |
| SOS | `sos_child` | bool | Read & Write | Child, Admin | Nút SOS trẻ em |
| SOS | `sos_adult` | bool | Read & Write | Parent, Admin | Nút SOS người lớn |
| Escalation | `emergency_confirmation_requested` | bool | Read Only | Parent, Admin | Đã gửi yêu cầu xác nhận |
| Escalation | `emergency_confirmed` | bool | Read Only | Parent, Admin | Đã có xác nhận |
| Escalation | `emergency_escalation_status` | String | Read Only | Parent, Admin | Trạng thái escalation gần nhất |
| Escalation | `emergency_authority_message_status` | String | Read Only | Parent, Admin | Trạng thái gửi contact mô phỏng |
| Escalation | `home_address_configured` | bool | Read Only | Parent, Admin | Địa chỉ đã cấu hình hợp lệ |
| Escalation | `sos_authority_note` | String | Read & Write | Parent, Admin | Ghi chú cho SOS |
| Cảm biến | `pir_detected` | bool | Read Only | Admin | PIR phát hiện chuyển động |
| Cảm biến | `ultrasonic_distance` | float | Read Only | Admin | Khoảng cách siêu âm |
| Cảm biến | `object_near` | bool | Read Only | Admin | Có vật ở gần |
| Cảm biến | `pet_detected` | bool | Read Only | Admin | Nghi vật nhỏ/báo nhầm |
| Cảm biến | `ldr_value` | int | Read Only | Admin | Giá trị LDR |
| Cảm biến | `ldr_delta` | int | Read Only | Admin | Độ thay đổi ánh sáng |
| Cảm biến | `light_abnormal` | bool | Read Only | Admin | Ánh sáng bất thường |
| Cảm biến | `ldr_covered` | bool | Read Only | Admin | LDR có dấu hiệu bị che |
| Đột nhập | `intrusion_alert` | bool | Read Only | Parent, Admin | Cảnh báo đột nhập |
| Đột nhập | `intrusion_score` | int | Read Only | Admin | Điểm đột nhập có trọng số |
| Đột nhập | `threat_level` | int | Read Only | Parent, Admin | Mức nguy hiểm 0–4 |
| Sabotage | `sabotage_alert` | bool | Read Only | Parent, Admin | Phát hiện hành vi phá hoại |
| Sabotage | `device_tampered` | bool | Read Only | Admin | Dấu hiệu can thiệp vật lý |
| Critical | `critical_security_compromise` | bool | Read Only | Parent, Admin | Mất liên lạc hoặc suy giảm nghiêm trọng sau sabotage |
| Critical | `system_degraded` | bool | Read Only | Parent, Admin | Hệ thống mất một phần khả năng |
| Critical | `compromise_reason` | String | Read Only | Parent, Admin | Lý do trung tính: heartbeat timeout, camera failure, multi-module failure... |
| Critical | `critical_event_id` | String | Read Only | Admin | ID sự kiện critical gần nhất |
| Critical | `critical_confirmation_requested` | bool | Read Only | Parent, Admin | Đã gửi email xin xác nhận |
| Critical | `critical_confirmed` | bool | Read Only | Parent, Admin | Parent/Admin đã xác nhận |
| Critical | `critical_escalation_status` | String | Read Only | Parent, Admin | `IDLE`, `WAITING_CONFIRMATION`, `CONFIRMED`, `SENT`, `FAILED` |
| Critical | `critical_authority_note` | String | Read & Write | Parent, Admin | Ghi chú bổ sung cho sự cố phá hoại nghiêm trọng |
| Heartbeat | `heartbeat_send_status` | String | Read Only | Admin | Trạng thái lần gửi heartbeat gần nhất |
| Heartbeat | `last_heartbeat_time` | String | Read Only | Parent, Admin | Thời điểm heartbeat gần nhất mà board biết đã gửi |
| Heartbeat | `device_connection_status` | String | Read Only | Parent, Admin | Trạng thái online/offline/suspected offline; có thể cũ nếu board mất điện |
| Thời gian | `current_time` | String | Read Only | Parent, Admin | Thời gian RTC hiện tại |
| Thời gian | `current_hour` | int | Read Only | Admin | Giờ hiện tại |
| Thời gian | `night_mode` | bool | Read Only | Admin | Chế độ ban đêm |
| Camera | `manual_capture_photo` | bool | Read & Write | Parent, Admin | Chụp ảnh thủ công |
| Camera | `auto_capture_photo_request` | bool | Read Only | Admin | Đang yêu cầu chụp tự động |
| Camera | `photo_status` | String | Read Only | Parent, Admin | Trạng thái chụp/gửi ảnh |
| Đầu ra | `buzzer_on` | bool | Read Only | Parent, Admin | Trạng thái lệnh còi |
| Đầu ra | `led_red_on` | bool | Read Only | Parent, Admin | LED đỏ cảnh báo |
| Đầu ra | `led_green_on` | bool | Read Only | Parent, Admin | LED xanh bình thường |
| Notification | `send_notification_request` | bool | Read Only | Admin | Yêu cầu gửi thông báo |
| Notification | `notification_event_type` | String | Read Only | Admin | Loại event thông báo |
| Notification | `notification_sent_status` | String | Read Only | Parent, Admin | Trạng thái gửi thông báo |
| Notification | `notification_channel` | String | Read Only | Admin | Kênh thông báo |
| Log | `last_event` | String | Read Only | Child, Parent, Admin | Mô tả sự kiện gần nhất |
| Log | `last_event_type` | String | Read Only | Admin | Loại sự kiện gần nhất |
| Log | `event_counter` | int | Read Only | Admin | Bộ đếm sự kiện |
| Cooldown | `cooldown_active` | bool | Read Only | Admin | Đang trong cooldown |
| Cooldown | `last_alert_time` | String | Read Only | Admin | Thời điểm cảnh báo gần nhất |
| Demo | `demo_mode` | bool | Read & Write | Admin | Bật controlled demo input |
| Demo | `demo_scenario` | int | Read & Write | Admin | Chọn kịch bản demo |

### 5.5 Biến Cloud đã loại bỏ

Các biến dưới đây không thuộc phạm vi nghiệp vụ v0.8.0. Chúng phải được xóa khỏi Arduino IoT Cloud Thing, Dashboard, `thingProperties.h`, callback và firmware; bảng chỉ giữ tên để truy vết lịch sử.

| Biến | Trạng thái |
|---|---|
| `known_device_present` | Xóa; trusted-device bị loại |
| `trusted_ble_detection_enabled` | Xóa; BLE bị loại |
| `trusted_ble_present` | Xóa |
| `trusted_device_source` | Xóa |
| `trusted_ble_rssi` | Xóa |
| `trusted_ble_last_seen_seconds` | Xóa |
| `rearm_delay_seconds` | Xóa; auto re-arm theo điện thoại bị loại |
| `rearm_countdown_remaining` | Xóa |
| `unknown_wifi_detection_enabled` | Xóa; WiFi scanning bị loại |
| `unknown_wifi_count` | Xóa |
| `unknown_wifi_alert` | Xóa |
| `last_unknown_mac` | Xóa |
| `flame_value`, `fire_alert` | Ngoài phạm vi báo cháy |
| `rfid_uid` | RFID bị loại |
| `capture_photo` | Thay bằng `manual_capture_photo` và `auto_capture_photo_request` |
| nhóm `email_*` cũ | Thay bằng nhóm escalation/notification hiện tại |

## 6. Yêu Cầu Trạng Thái Hệ Thống

### 6.1 Giá trị hợp lệ của `alarm_status`

| Mã nội bộ | Ý nghĩa người dùng | Ưu tiên |
|---|---|---:|
| `SOS_ALERT` | Đang có SOS khẩn cấp | 1 |
| `CRITICAL_SECURITY_COMPROMISE` | Thiết bị mất liên lạc hoặc suy giảm nghiêm trọng sau phá hoại | 2 |
| `SABOTAGE_ALERT` | Phát hiện hành vi phá hoại/can thiệp | 3 |
| `INTRUSION_ALERT` | Đã xác nhận đột nhập | 4 |
| `SYSTEM_ERROR` | Có lỗi hệ thống nhưng chưa đủ điều kiện critical | 5 |
| `ARMED` | Hệ thống đang bảo vệ | 6 |
| `DISARMED` | Chống trộm đang tắt | 7 |
| `RESET_COMPLETE` | Vừa reset thành công, sau đó tính lại trạng thái | 8 |

Khi nhiều trạng thái cùng tồn tại, hệ thống phải hiển thị trạng thái có ưu tiên cao nhất. `CRITICAL_SECURITY_COMPROMISE` không xóa `sabotage_alert`; nó là mức nâng cao hơn của cùng chuỗi sự kiện.

## 7. Quy Tắc Nghiệp Vụ

### BR-01. Quy tắc thiết bị đáng tin — Loại khỏi phạm vi v0.7.0

BLE/WiFi trusted-device detection không còn thuộc phạm vi. `system_armed` không được phụ thuộc `known_device_present`.

### BR-02. Quy tắc auto re-arm theo thiết bị đáng tin — Loại khỏi phạm vi v0.7.0

Không còn countdown tự bật lại theo sự hiện diện điện thoại. Parent/Admin hoặc lịch RTC chịu trách nhiệm bật/tắt `alarm_enabled`.

### BR-03. Quy tắc thiết bị đáng tin quay lại — Loại khỏi phạm vi v0.7.0

Không áp dụng trong phạm vi hiện tại.

### BR-03A. Quy tắc quét BLE — Loại khỏi phạm vi v0.7.0

Không triển khai active BLE scan, advertiser whitelist hoặc nhận diện trusted phone.

### BR-03B. Quy tắc an toàn BLE — Lưu lịch sử

Quy tắc này không còn active trong v0.7.0. Nếu BLE được đưa lại ở phiên bản sau, phải cập nhật SRS mới.

### BR-04. Quy tắc reset báo động

Khi Parent/Admin kích hoạt `reset_alarm` và board vẫn online, hệ thống phải:

1. dừng còi và LED đỏ;
2. xóa các cờ cảnh báo local đang active;
3. giữ `last_event`, `last_event_type` và `event_counter`;
4. tính lại `system_armed` chỉ theo `alarm_enabled` và điều kiện lỗi an toàn hiện tại;
5. cập nhật `alarm_status`;
6. tự đặt `reset_alarm = false`.

Nếu board đã offline, Apps Script không được tuyên bố reset thiết bị thành công. Xác nhận email không thay thế reset local.

### BR-05. Quy tắc re-arm delay — Loại khỏi phạm vi v0.7.0

`rearm_delay_seconds` và `rearm_countdown_remaining` là biến legacy, không dùng trong nghiệp vụ hiện tại.

### BR-06. Quy tắc phạm vi của `alarm_enabled`

Biến `alarm_enabled` chỉ điều khiển chức năng phát hiện đột nhập của khu vực giám sát.

Khi `alarm_enabled = false`, hệ thống phải:

1. đặt `system_armed = false`,
2. không tạo cảnh báo đột nhập,
3. không bật còi do lý do đột nhập,
4. không gửi notification/Telegram do lý do đột nhập,
5. vẫn tiếp tục hỗ trợ:
   - SOS,
   - chống phá hoại,
   - reset,
   - cập nhật dashboard,
   - đồng bộ Cloud,
   - ghi log,
   - chụp ảnh thủ công nếu được hỗ trợ.

### BR-07. Quy tắc độ nhạy phát hiện đột nhập

Hệ thống phải cho phép người lớn và Admin cấu hình mức nhạy bằng biến `sensitivity_level`.

| `sensitivity_level` | Ý nghĩa | Ngưỡng nội bộ |
|---:|---|---:|
| 1 | Ít nhạy | `intrusion_score >= 5` |
| 2 | Bình thường | `intrusion_score >= 4` |
| 3 | Nhạy cao | `intrusion_score >= 3` |

Nếu `sensitivity_level` nằm ngoài khoảng hợp lệ, hệ thống phải dùng giá trị mặc định là `2`.

Người dùng không được chỉnh trực tiếp `intrusion_threshold`. Ngưỡng điểm là logic nội bộ do hệ thống tự map từ `sensitivity_level`.

### BR-08. Quy tắc WiFi/MAC lạ — Loại khỏi phạm vi v0.7.0

Không quét WiFi/MAC và không dùng MAC lạ làm tín hiệu cảnh báo hoặc tính điểm.

### BR-09. Quy tắc tính `intrusion_score`

```text
intrusion_score =
    (pir_detected ? 2 : 0)
  + (object_near ? 2 : 0)
  + (light_abnormal ? 1 : 0)
  + (night_mode ? 1 : 0)
```

Không có thành phần BLE hoặc WiFi/MAC. PIR và object-near là bằng chứng vật lý mạnh; ánh sáng và ban đêm là tín hiệu bối cảnh.

### BR-10. Quy tắc WiFi/MAC pre-warning — Loại khỏi phạm vi v0.7.0

Không áp dụng.

### BR-11. Quy tắc giữ điểm nghi ngờ

Khi một tín hiệu đáng nghi vừa biến mất, hệ thống phải giữ điểm nghi ngờ của tín hiệu đó trong 5 giây.

Sau 5 giây, nếu tín hiệu không xuất hiện lại, hệ thống phải loại điểm đó khỏi `intrusion_score`.

Thời gian giữ điểm này là cố định trong phiên bản hiện tại và không cần Cloud Variable riêng.

### BR-12. Quy tắc kích hoạt cảnh báo đột nhập

Khi `system_armed = true` và `intrusion_score` đạt hoặc vượt ngưỡng tương ứng với `sensitivity_level`, hệ thống phải kích hoạt cảnh báo đột nhập đầy đủ.

Hệ thống phải cập nhật:

- `intrusion_alert = true`,
- `alarm_status = INTRUSION_ALERT`,
- `buzzer_on = true`,
- `led_red_on = true`,
- `last_event = "Phát hiện đột nhập."`,
- `last_event_type = "intrusion_alert"`.

Cảnh báo đột nhập phải duy trì cho đến khi người lớn hoặc Admin kích hoạt `reset_alarm`.

### BR-13. Quy tắc cập nhật điểm trong lúc cảnh báo đột nhập đang hoạt động

Khi `intrusion_alert = true`, hệ thống vẫn tiếp tục đọc các tín hiệu cảm biến và cập nhật `intrusion_score` theo trạng thái hiện tại.

Tuy nhiên, cảnh báo đột nhập không được tự tắt chỉ vì `intrusion_score` giảm xuống dưới ngưỡng.

Cảnh báo đột nhập chỉ được tắt khi người lớn hoặc Admin kích hoạt `reset_alarm`.

### BR-14. Quy tắc gửi notification khi có đột nhập

Khi hệ thống kích hoạt `intrusion_alert`, hệ thống phải tạo yêu cầu gửi notification ra ngoài.

Khi `intrusion_alert = true`, hệ thống phải cập nhật:

- `send_notification_request = true`,
- `notification_event_type = "intrusion_alert"`,
- `notification_channel = "telegram"` nếu không có cấu hình khác,
- `notification_sent_status` theo trạng thái gửi hiện tại.

Việc gửi notification phải tuân theo rule cooldown để tránh spam cảnh báo lặp lại.

### BR-15. Quy tắc tự chụp ảnh khi có đột nhập

Khi một `intrusion_alert` mới được kích hoạt, hệ thống phải tự động kích hoạt chụp ảnh bằng camera OV3660 trên `Freenove ESP32-S3 WROOM + Camera OV3660`.

Hệ thống phải cập nhật:

- `auto_capture_photo_request = true` khi bắt đầu yêu cầu chụp tự động,
- `photo_status = "Đang chụp ảnh"` khi đang xử lý ảnh,
- `photo_status = "Đã chụp và gửi ảnh"` nếu chụp/gửi thành công,
- thông báo lỗi tiếng Việt cụ thể nếu chụp thất bại,
- `auto_capture_photo_request = false` sau khi xử lý xong.

Việc chụp ảnh tự động phải tránh lặp lại liên tục khi cùng một cảnh báo đột nhập vẫn đang hoạt động.

### BR-16. Quy tắc số lượng ảnh tự động khi có đột nhập

Khi một lần `intrusion_alert` mới được kích hoạt, hệ thống phải tự động chụp 1 ảnh bằng camera OV3660 trên `Freenove ESP32-S3 WROOM + Camera OV3660`.

Sau khi ảnh tự động đầu tiên đã được xử lý, hệ thống không được tự động chụp thêm ảnh trong cùng một lần `intrusion_alert`.

Trong lúc `intrusion_alert` vẫn đang hoạt động, Parent/Admin vẫn được phép kích hoạt `manual_capture_photo` thủ công để yêu cầu chụp thêm ảnh.

Sau khi `reset_alarm` được xử lý và một `intrusion_alert` mới xảy ra trong tương lai, hệ thống được phép tự động chụp 1 ảnh mới cho lần cảnh báo mới đó.

### BR-17. Quy tắc cờ nội bộ kiểm soát chụp ảnh tự động

Hệ thống được phép dùng trạng thái nội bộ trong code để kiểm soát việc tự động chụp ảnh trong mỗi lần `intrusion_alert`.

Trạng thái nội bộ này không cần là Cloud Variable.

Ví dụ tên biến nội bộ:

```cpp
bool auto_photo_captured_for_current_alert;
```

Mục đích của trạng thái nội bộ là đảm bảo:

1. mỗi lần `intrusion_alert` mới chỉ tự động chụp 1 ảnh,
2. không tự chụp lặp lại liên tục trong cùng một cảnh báo,
3. sau khi `reset_alarm` được xử lý, lần `intrusion_alert` tiếp theo được phép tự động chụp 1 ảnh mới.

### BR-18. Quy tắc lịch tự động điều khiển `alarm_enabled`

Lịch RTC chỉ được thay đổi `alarm_enabled`, không thay đổi trực tiếp các cờ cảnh báo.

Sau khi lịch thay đổi `alarm_enabled`:

```text
alarm_enabled = true  -> system_armed = true nếu không có cảnh báo/lỗi chặn
alarm_enabled = false -> system_armed = false
```

Không kiểm tra `known_device_present`, không chạy re-arm countdown.

### BR-19. Quy tắc lịch tự động hằng ngày

Hệ thống phải cho phép người lớn/Admin cấu hình một thời điểm tự bật chống trộm và một thời điểm tự tắt chống trộm mỗi ngày.

Nếu `schedule_enabled = true`:

```text
đến auto_arm_hour:auto_arm_minute
    hệ thống đặt alarm_enabled = true

đến auto_disarm_hour:auto_disarm_minute
    hệ thống đặt alarm_enabled = false
```

Nếu `schedule_enabled = false`, hệ thống không tự thay đổi `alarm_enabled` theo lịch.

Giá trị mặc định đề xuất:

| Biến | Default |
|---|---:|
| `schedule_enabled` | `false` |
| `auto_arm_hour` | `23` |
| `auto_arm_minute` | `0` |
| `auto_disarm_hour` | `6` |
| `auto_disarm_minute` | `0` |

### BR-20. Quy tắc lịch ghi đè thao tác thủ công

Nếu `schedule_enabled = true`, lịch tự động được quyền thay đổi `alarm_enabled` khi đến thời điểm đã cấu hình.

Thao tác thủ công của người dùng không vô hiệu hóa lịch.

Nếu người dùng không muốn lịch tự động thay đổi `alarm_enabled`, người dùng phải đặt `schedule_enabled = false`.

### BR-21. Quy tắc giờ tự bật và tự tắt bị trùng nhau

Nếu `schedule_enabled = true` nhưng thời điểm tự bật và thời điểm tự tắt bị trùng nhau, hệ thống không được tự thay đổi `alarm_enabled` tại thời điểm đó.

Hệ thống phải ghi nhận lỗi cấu hình lịch vào:

```text
last_event = "Lỗi cấu hình lịch: giờ tự bật và giờ tự tắt bị trùng nhau."
last_event_type = "schedule_config_error"
```

Hệ thống có thể đặt `alarm_status = ERROR` nếu lỗi cấu hình làm hệ thống không thể xác định hành động an toàn.

Hệ thống không được tự động tắt `schedule_enabled`, trừ khi người dùng/Admin chủ động tắt.

### BR-22. Quy tắc giá trị giờ/phút không hợp lệ

Nếu `schedule_enabled = true` nhưng một trong các giá trị giờ/phút của lịch không hợp lệ, hệ thống không được tự thay đổi `alarm_enabled` theo lịch.

Giá trị hợp lệ:

| Biến | Khoảng hợp lệ |
|---|---|
| `auto_arm_hour` | 0–23 |
| `auto_arm_minute` | 0–59 |
| `auto_disarm_hour` | 0–23 |
| `auto_disarm_minute` | 0–59 |

Nếu có giá trị không hợp lệ, hệ thống phải:

1. bỏ qua lịch tự động,
2. giữ nguyên `alarm_enabled` hiện tại,
3. không đổi trực tiếp `system_armed`,
4. ghi lỗi cấu hình vào `last_event`,
5. ghi `last_event_type = "schedule_config_error"`.

### BR-23. Quy tắc ghi log khi lịch chạy

Khi lịch tự động thay đổi `alarm_enabled`, hệ thống phải ghi log sự kiện.

Nếu lịch tự động bật chống trộm:

```text
alarm_enabled = true
last_event = "Lịch tự động đã bật chế độ chống trộm."
last_event_type = "schedule_auto_arm"
```

Nếu lịch tự động tắt chống trộm:

```text
alarm_enabled = false
last_event = "Lịch tự động đã tắt chế độ chống trộm."
last_event_type = "schedule_auto_disarm"
```

Sau khi `alarm_enabled` thay đổi, hệ thống phải tính lại `system_armed` theo các rule hiện có.

### BR-24. Quy tắc nguồn thời gian cho lịch tự động

Hệ thống phải dùng thời gian từ DS1307 RTC làm nguồn thời gian chính để kiểm tra lịch tự động bật/tắt chống trộm.

Các biến `current_time` và `current_hour` phải phản ánh thời gian hiện tại từ RTC.

Nếu RTC chưa được cấu hình đúng giờ hoặc trả về thời gian không hợp lệ, hệ thống không được chạy lịch tự động và phải ghi log lỗi thời gian:

```text
last_event = "Lỗi lịch tự động: thời gian RTC không hợp lệ."
last_event_type = "schedule_time_error"
```

Hệ thống không được phụ thuộc vào Internet/Cloud time để chạy lịch trong phiên bản hiện tại.

### BR-25. Quy tắc lịch và thiết bị đáng tin — Thay thế trong v0.7.0

Khi lịch đến giờ tự bật, hệ thống đặt `alarm_enabled = true` và `system_armed = true` nếu không có cảnh báo active hoặc lỗi bắt buộc ngăn khởi động. Không kiểm tra điện thoại đáng tin.

### BR-26. Quy tắc lưu sự kiện lịch

Hệ thống không cần tạo biến Cloud riêng để lưu hành động lịch gần nhất.

Khi lịch tự động chạy, hệ thống vẫn phải cập nhật:

- `last_event`,
- `last_event_type`.

Sau này, khi chức năng Event Logging được phân rã, các sự kiện lịch như `schedule_auto_arm` và `schedule_auto_disarm` phải được ghi vào lịch sử sự kiện dài hạn.

### BR-27. Quy tắc phạm vi của `threat_level`

`threat_level` là mức nguy hiểm tổng thể, không phải bản sao của `intrusion_score`.

Nguồn nguy cơ gồm:

- tín hiệu cảm biến và điểm đột nhập;
- intrusion đã xác nhận;
- sabotage;
- critical security compromise;
- SOS;
- lỗi hệ thống cần chú ý.

### BR-28. Quy tắc thang đo `threat_level`

`threat_level` phải là số nguyên từ 0 đến 4.

| Mức | Ý nghĩa |
|---:|---|
| 0 | Bình thường |
| 1 | Cần chú ý |
| 2 | Nghi ngờ trung bình |
| 3 | Nguy hiểm cao |
| 4 | Khẩn cấp |

### BR-29. Quy tắc tổng hợp `threat_level`

Khi nhiều loại nguy cơ xảy ra cùng lúc, hệ thống phải tính `threat_level` bằng mức nguy hiểm cao nhất trong các nguy cơ hiện tại.

Hệ thống không cộng dồn điểm nguy cơ để tạo `threat_level`.

Mỗi nhóm nguy cơ được ánh xạ sang một mức riêng. Sau đó hệ thống lấy mức lớn nhất làm `threat_level` tổng thể.

### BR-30. Quy tắc mapping `threat_level`

| Nguồn nguy cơ | Điều kiện | Mức |
|---|---|---:|
| Bình thường | Không có tín hiệu đáng nghi | 0 |
| Tín hiệu nhẹ | Một tín hiệu vật lý đơn lẻ hoặc pet/false positive | 1 |
| Gần ngưỡng | `intrusion_score = threshold - 1` | 2 |
| Đột nhập | `intrusion_alert = true` | 3 |
| Sabotage | `sabotage_alert = true` | 4 |
| Critical compromise | `critical_security_compromise = true` | 4 |
| SOS | SOS active | 4 |

### BR-31. Quy tắc gần ngưỡng của `threat_level`

Nếu `intrusion_alert = false` và `intrusion_score = threshold - 1`, hệ thống phải đặt mức nguy cơ đột nhập tương ứng là `threat_level = 2`.

Trong đó `threshold` là ngưỡng nội bộ được suy ra từ `sensitivity_level`.

### BR-32. Quy tắc giảm điểm do `pet_detected`

Khi `pet_detected = true`, hệ thống phải giảm mức nghi ngờ đột nhập thay vì chặn hoàn toàn cảnh báo.

`pet_detected` không được tự động đặt `intrusion_alert = false`.

`pet_detected` chỉ được dùng để giảm `intrusion_score` hoặc loại bỏ một phần điểm đến từ tín hiệu chuyển động/khoảng cách yếu.

### BR-33. Quy tắc mức giảm điểm do `pet_detected`

Khi `pet_detected = true`, hệ thống phải giảm `intrusion_score` cuối cùng đi 1 điểm.

Mức giảm này là cố định trong phiên bản hiện tại.

`intrusion_score` sau khi giảm không được nhỏ hơn 0.

### BR-34. Quy tắc điều kiện nhận diện `pet_detected`

Hệ thống chỉ được đặt `pet_detected = true` khi tín hiệu hiện tại giống thú nuôi/vật nhỏ/báo nhầm và không đi kèm dấu hiệu nguy hiểm rõ ràng.

Điều kiện định hướng:

1. Có tín hiệu vật thể hoặc chuyển động nhẹ/ngắn.
2. Tín hiệu không kéo dài.
3. Không có `light_abnormal`.
4. Chưa có `intrusion_alert`.
5. Tổng tín hiệu nguy hiểm chưa đủ mạnh.

### BR-35. Quy tắc thời lượng tín hiệu ngắn cho `pet_detected`

Trong phạm vi lọc báo động giả, một tín hiệu được xem là ngắn nếu tín hiệu đó xuất hiện và biến mất trong thời gian dưới 5 giây.

Nếu tín hiệu chuyển động/vật thể tồn tại từ 5 giây trở lên, hệ thống không được xem tín hiệu đó là tín hiệu ngắn để tự đặt `pet_detected = true`.

### BR-36. Quy tắc ý nghĩa của `pet_detected`

`pet_detected = true` không có nghĩa là hệ thống xác nhận chắc chắn có thú nuôi.

`pet_detected = true` chỉ có nghĩa là hệ thống nghi tín hiệu hiện tại giống thú nuôi, vật nhỏ hoặc báo động giả hơn là người đột nhập.

Hệ thống không được dùng `pet_detected` như bằng chứng tuyệt đối để bỏ qua nguy cơ an ninh.

### BR-37. Quy tắc nguồn tín hiệu cho `pet_detected`

Hệ thống chỉ được đặt `pet_detected = true` khi cả hai tín hiệu sau cùng xuất hiện ngắn:

- `object_near = true`,
- `pir_detected = true`.

Đồng thời phải thỏa các điều kiện an toàn:

- `light_abnormal = false`,
- `intrusion_alert = false`.

### BR-38. Quy tắc `pet_detected` khi gần ngưỡng

Nếu `intrusion_score = threshold - 1` và các điều kiện nhận diện pet/báo nhầm vẫn hợp lệ, hệ thống vẫn được phép đặt `pet_detected = true`.

Khi `pet_detected = true`, hệ thống giảm `intrusion_score` cuối cùng đi 1 điểm.

Tuy nhiên, `pet_detected` không được dùng để tắt một cảnh báo đột nhập đã được kích hoạt.

### BR-39. Quy tắc `threat_level` khi chỉ là pet/báo nhầm

Nếu `pet_detected = true` và không có nguy cơ khác nghiêm trọng hơn, hệ thống phải giới hạn `threat_level` tối đa ở mức 1.

Điều kiện áp dụng:

- `pet_detected = true`,
- `intrusion_alert = false`,
- `light_abnormal = false`,
- `sabotage_alert = false`,
- không có SOS đang hoạt động.

Khi đó: `threat_level = 1`.

### BR-40. Quy tắc notification cho pet/báo nhầm

Khi `pet_detected = true` nhưng chưa có `intrusion_alert`, hệ thống không được gửi Telegram alert khẩn cấp.

Hệ thống chỉ cần cập nhật:

- `last_event`,
- `last_event_type`,
- `threat_level`.

Nội dung đề xuất:

```text
last_event = "Phát hiện tín hiệu giống vật nhỏ hoặc báo nhầm."
last_event_type = "pet_or_false_positive"
threat_level = 1
```

### BR-41. Quy tắc đặt tên biến notification

Hệ thống phải dùng nhóm biến `notification_*` để đại diện cho yêu cầu gửi thông báo ra ngoài.

Các biến `email_*` không còn là tên ưu tiên trong SRS vì hệ thống có thể gửi thông báo qua nhiều kênh như Telegram Bot hoặc Email.

Nhóm biến notification phải hỗ trợ tối thiểu:

- yêu cầu gửi thông báo,
- loại sự kiện cần gửi,
- trạng thái gửi,
- kênh thông báo.

### BR-42. Quy tắc kênh notification mặc định

Kênh thông báo mặc định của hệ thống là Telegram Bot.

Khi hệ thống cần gửi thông báo ra ngoài, nếu không có cấu hình khác, hệ thống phải dùng `notification_channel = "telegram"`.

Email không phải kênh notification mặc định cho cảnh báo nhanh. Tuy nhiên, Gmail/Google Apps Script là kênh bắt buộc cho xác nhận và escalation của SOS và critical compromise.

### BR-43. Quy tắc event gửi Telegram

Telegram được dùng cho:

- `intrusion_alert`;
- `sabotage_alert`;
- `critical_security_compromise` khi board còn mạng;
- `sos_alert`;
- `photo_capture_failed`;
- lỗi lịch quan trọng.

Không gửi Telegram cho heartbeat, trạng thái bình thường, pet/false positive hoặc thao tác reset thành công.

### BR-43A. Quy tắc cấu hình dịch vụ từ Dashboard

Mỗi dịch vụ từ xa phải kiểm tra công tắc tương ứng trước khi tạo request. Giá trị mặc định sau khi nạp firmware là: `telegram_enabled = true` nếu có token, `heartbeat_enabled = true`, còn `telegram_photo_enabled`, `camera_enabled`, `gemini_enabled` và `auto_photo_on_alert` là `false`.

- `telegram_photo_enabled` chỉ có hiệu lực khi `telegram_enabled` và `camera_enabled` cùng bật.
- `gemini_enabled` chỉ có hiệu lực khi `camera_enabled` bật.
- `google_script_enabled = false` phải dừng gửi event và heartbeat sang Apps Script; trạng thái Dashboard phải ghi rõ monitoring đang tạm dừng để không suy diễn board offline.
- Tắt một dịch vụ không được làm dừng cảm biến, còi, LED, `ArduinoCloud.update()` hoặc logic cảnh báo cục bộ.

### BR-44. Quy tắc chống spam Telegram cho WiFi/MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-45. Quy tắc ngưỡng Telegram cho WiFi/MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-46. Quy tắc cùng một MAC lạ lặp lại

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-47. Quy tắc cooldown Telegram cho cùng một MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-48. Quy tắc timer nội bộ cho cooldown WiFi/MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-49. Quy tắc reset trạng thái khi tắt phát hiện WiFi/MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-50. Quy tắc phiên mới khi bật lại phát hiện WiFi/MAC lạ

Loại khỏi phạm vi v0.7.0. Giữ ID chỉ để truy vết lịch sử.

### BR-51. Quy tắc SOS alarm latch cho demo

SOS luôn hoạt động độc lập với `alarm_enabled` và `system_armed`.

Khi `sos_child` hoặc `sos_adult` được kích hoạt, hệ thống phải:

- đặt `alarm_status = SOS_ALERT`,
- đặt `threat_level = 4`,
- bật `buzzer_on = true`,
- đặt `led_green_on = false`,
- bật `led_red_on = true`,
- cập nhật `last_event`,
- cập nhật `last_event_type = "sos_alert"`,
- gửi notification cho Parent/Admin,
- yêu cầu Google Apps Script gửi email xác nhận emergency escalation.

`alarm_enabled = false` hoặc `system_armed = false` không được làm vô hiệu hóa SOS.

Sau khi SOS active, còi và LED đỏ phải duy trì trong toàn bộ các giai đoạn sau:

- đang chờ Parent/Admin xác nhận escalation,
- Parent/Admin đã ACK/xác nhận escalation,
- Google Apps Script đã gửi authority escalation thành công,
- Google Apps Script gửi escalation thất bại,
- hết thời gian chờ xác nhận nhưng chưa có ai ACK.

Parent/Admin dùng `reset_alarm` để tắt còi/LED và kết thúc cảnh báo SOS. Hệ thống không được tự tắt còi/LED chỉ vì đã gửi Telegram, đã gửi email xác nhận, đã nhận ACK hoặc đã gửi email escalation.

### BR-52. Quy tắc tự reset nút SOS

`sos_child` và `sos_adult` chỉ là nút bấm tạo sự kiện SOS, không phải trạng thái SOS đang hoạt động lâu dài.

Khi hệ thống phát hiện `sos_child = true` hoặc `sos_adult = true`, hệ thống phải xử lý sự kiện SOS rồi tự đặt lại biến tương ứng về `false`.

Trạng thái SOS đang hoạt động phải được thể hiện bằng:

- `alarm_status = SOS_ALERT`,
- `threat_level = 4`,
- `buzzer_on = true`,
- `led_green_on = false`,
- `led_red_on = true`.

Nếu cả `sos_child` và `sos_adult` cùng được kích hoạt, hệ thống được phép xử lý như hai sự kiện SOS riêng biệt, nhưng trạng thái cảnh báo tổng thể vẫn chỉ cần là `SOS_ALERT`.

### BR-53. Quy tắc xác nhận escalation SOS qua email

Sau khi có SOS từ `sos_child` hoặc `sos_adult`, hệ thống phải yêu cầu Google Apps Script gửi email xác nhận đến Parent/Admin.

Emergency escalation chỉ được thực hiện khi có ít nhất một Parent/Admin xác nhận qua email.

Việc xác nhận có thể được thực hiện bằng cách bấm link xác nhận trong email do Google Apps Script tạo.

Nếu chưa có xác nhận, hệ thống không được gửi escalation đến contact cơ quan chức năng/công an hoặc contact mô phỏng.

Sau khi có xác nhận, Google Apps Script được phép gửi email escalation đến contact cơ quan chức năng/công an được cấu hình hợp lệ. Trong demo, contact này có thể là email mô phỏng của thành viên nhóm và email phải ghi rõ đây là bản demo nếu có nguy cơ bị hiểu nhầm là cảnh báo thật.

### BR-53A. Quy tắc gửi địa chỉ nhà trong SOS authority escalation

Địa chỉ nhà/khu vực cần hỗ trợ khẩn cấp phải được cấu hình trong Google Apps Script hoặc cấu hình triển khai bảo mật, không tạo Cloud Variable dạng `home_address` để tránh lộ thông tin nhạy cảm trên dashboard.

Khi Parent/Admin xác nhận escalation, Google Apps Script phải ghép địa chỉ nhà vào email escalation gửi đến contact cơ quan chức năng/công an hoặc contact mô phỏng.

Email escalation tối thiểu phải có:

- tiêu đề thể hiện đây là SOS khẩn cấp hoặc demo SOS khẩn cấp,
- địa chỉ nhà/khu vực cần hỗ trợ,
- thời điểm phát sinh SOS,
- nguồn SOS: `sos_child` hoặc `sos_adult`,
- trạng thái hệ thống gần nhất,
- ghi chú bổ sung của Parent/Admin nếu có,
- thông tin liên hệ lại của Parent/Admin hoặc nhóm demo nếu được cấu hình.

### BR-53B. Quy tắc ghi chú bổ sung của Parent/Admin cho cơ quan tiếp nhận

Parent/Admin được phép nhập `sos_authority_note` trước hoặc trong lúc xác nhận SOS escalation.

`sos_authority_note` dùng để bổ sung tình hình hiện tại hoặc nhắc nhở cho cơ quan tiếp nhận, ví dụ:

```text
Có trẻ em ở trong nhà.
Nghi có người lạ trong khu vực bếp.
Vui lòng gọi lại số liên hệ trước khi đến.
Cổng chính ở phía bên phải.
```

Google Apps Script phải đưa nội dung ghi chú này vào email escalation nếu ghi chú không rỗng.

Hệ thống phải giới hạn độ dài ghi chú ở mức phù hợp với bộ nhớ và email, ví dụ tối đa 500 ký tự trong bản demo.

### BR-53C. Quy tắc giới hạn trách nhiệm phản hồi khẩn cấp

Hệ thống phải cố gắng gửi thông tin SOS đã xác nhận nhanh và đầy đủ nhất có thể để hỗ trợ cơ quan tiếp nhận phản hồi sớm.

Tuy nhiên, hệ thống không được cam kết hoặc bảo đảm rằng công an/cơ quan chức năng sẽ đến trong một thời gian cụ thể. Thời gian phản hồi phụ thuộc vào mạng, email, contact cấu hình, quy trình tiếp nhận và điều kiện thực tế bên ngoài hệ thống.

### BR-53D. Quy tắc ACK không tắt cảnh báo SOS

ACK trong luồng SOS chỉ có nghĩa là Parent/Admin đã nhận cảnh báo và xác nhận bước escalation hoặc xác nhận đang xử lý.

ACK không được hiểu là sự kiện đã an toàn, không được hiểu là reset và không được tự động tắt cảnh báo tại chỗ.

Khi `emergency_confirmed = true`, `emergency_escalation_status = CONFIRMED` hoặc `emergency_authority_message_status = SENT`, hệ thống vẫn phải giữ:

- `alarm_status = SOS_ALERT`,
- `threat_level = 4`,
- `buzzer_on = true`,
- `led_red_on = true`,
- `led_green_on = false`.

Cảnh báo SOS chỉ được kết thúc khi Parent/Admin kích hoạt `reset_alarm`. Khi đó hệ thống xử lý theo BR-65.

### BR-54. Quy tắc chống phá hoại trong demo

Anti-sabotage luôn hoạt động, không phụ thuộc `alarm_enabled` hoặc `system_armed`.

Kích hoạt khi một trong các dấu hiệu kéo dài đủ thời gian:

- `ldr_covered = true`;
- `object_near = true` trong khoảng thời gian bất thường;
- controlled demo input hợp lệ.

Khi kích hoạt:

```text
device_tampered = true
sabotage_alert = true
alarm_status = SABOTAGE_ALERT
threat_level = 4
bật còi và LED đỏ
gửi Telegram
bắt đầu cửa sổ theo dõi heartbeat/health check cho DS-03
```

DS-04 chỉ xác nhận hành vi can thiệp; chưa được kết luận thiết bị đã hỏng.

### BR-55. Quy tắc ghi log sự kiện đơn giản

Hệ thống chỉ cần ghi nhận sự kiện quan trọng gần nhất trong phiên bản demo.

Khi một event quan trọng xảy ra, hệ thống phải:

- cập nhật `last_event`,
- cập nhật `last_event_type`,
- tăng `event_counter` thêm 1.

Hệ thống không cần lưu danh sách log nhiều dòng hoặc database log dài hạn trong phiên bản hiện tại.

Các event được xem là quan trọng gồm:

- `intrusion_alert`,
- `sos_alert`,
- `sabotage_alert`,
- `pet_or_false_positive`,
- `schedule_config_error`,
- `schedule_time_error`,
- `photo_capture_failed`,
- `emergency_escalation_sent`,
- `emergency_confirmation_timeout`.

Các cập nhật cảm biến thường xuyên như `pir_detected`, `ldr_value`, `ultrasonic_distance` hoặc dashboard refresh không được tính là event quan trọng nếu chưa tạo cảnh báo hoặc trạng thái đáng chú ý.

### BR-56. Quy tắc cooldown notification đơn giản

Trong phiên bản demo, hệ thống không gửi notification lặp liên tục cho cùng một cảnh báo đang active.

Với các cảnh báo chính như:

- `intrusion_alert`,
- `sos_alert`,
- `sabotage_alert`,

hệ thống chỉ gửi notification một lần cho mỗi lần cảnh báo mới.

Nếu cảnh báo vẫn đang active, hệ thống không gửi lại notification liên tục.

Sau khi Parent/Admin kích hoạt `reset_alarm`, nếu cùng loại cảnh báo xảy ra lại trong tương lai, hệ thống được phép gửi notification mới.

### BR-57. Quy tắc phạm vi Demo Mode

Trong phiên bản hiện tại, hệ thống không yêu cầu triển khai Demo Mode phức tạp.

Biến `demo_mode` và `demo_scenario` nếu có chỉ dùng để hỗ trợ kiểm thử thủ công hoặc trình diễn đơn giản.

Hệ thống không được phụ thuộc vào `demo_mode` để chạy các chức năng chính.

Các chức năng chính như:

- `intrusion_alert`,
- `sos_alert`,
- `sabotage_alert`,
- notification,
- `reset_alarm`,

phải hoạt động được bằng logic thật hoặc thao tác dashboard/cảm biến thật, không bắt buộc phải thông qua `demo_mode`.

### BR-58. Quy tắc trạng thái notification đơn giản

Hệ thống dùng `notification_sent_status` để biểu diễn trạng thái gửi notification.

Giá trị hợp lệ:

- `IDLE`: chưa có yêu cầu gửi,
- `PENDING`: đang chờ gửi hoặc vừa tạo request,
- `SENT`: đã gửi thành công,
- `FAILED`: gửi thất bại.

Khi chưa có yêu cầu gửi, trạng thái là `IDLE`.

Khi hệ thống tạo yêu cầu gửi, trạng thái là `PENDING`.

Nếu gửi thành công, trạng thái là `SENT`.

Nếu gửi thất bại, trạng thái là `FAILED`.

### BR-59. Quy tắc trạng thái emergency escalation

Hệ thống dùng `emergency_escalation_status` để theo dõi luồng xác nhận và gửi escalation sau SOS.

Giá trị hợp lệ:

- `IDLE`: chưa có SOS escalation,
- `WAITING_CONFIRMATION`: đã gửi email hỏi Parent/Admin xác nhận,
- `CONFIRMED`: đã có ít nhất một Parent/Admin xác nhận,
- `SENT`: đã gửi email escalation đến contact cơ quan chức năng/công an hoặc contact mô phỏng được cấu hình trong Google Apps Script,
- `FAILED`: gửi escalation thất bại,
- `NO_CONFIRMATION_TIMEOUT`: hết thời gian chờ nhưng không ai xác nhận.

Các giá trị `CONFIRMED`, `SENT`, `FAILED` và `NO_CONFIRMATION_TIMEOUT` chỉ thể hiện trạng thái của luồng escalation. Các giá trị này không được tự động kết thúc `SOS_ALERT`, không được tự tắt `buzzer_on` và không được tự tắt `led_red_on`.

### BR-60. Quy tắc luồng khẩn cấp qua Google Apps Script

Google Apps Script phải xử lý hai luồng độc lập:

1. `sos_alert`.
2. `critical_security_compromise`.

Mỗi event phải có `eventId`, loại event, thời gian, khu vực, trạng thái, ghi chú và trạng thái xác nhận riêng. Không được dùng chung một event record cho SOS và critical compromise.

Apps Script cũng phải nhận `heartbeat` và `sabotage_alert` để phát hiện việc mất liên lạc sau phá hoại.

### BR-61. Quy tắc phạm vi dashboard tối thiểu

Phiên bản demo dùng ba dashboard tối thiểu:

- Child Dashboard,
- Parent Dashboard,
- Admin Dashboard.

Child Dashboard chỉ chứa SOS và trạng thái cơ bản.

Parent Dashboard chứa điều khiển chính và trạng thái cảnh báo.

Admin Dashboard chứa biến debug, cảm biến và biến kiểm thử.

### BR-62. Quy tắc xử lý lỗi camera đơn giản

Trong phiên bản demo, hệ thống không cần retry phức tạp khi chụp ảnh thất bại.

Khi camera đang xử lý ảnh, `photo_status` hiển thị:

```text
photo_status = "Đang chụp ảnh"
```

Nếu chụp thành công:

```text
photo_status = "Đã chụp và gửi ảnh"
```

Nếu chụp thất bại:

```text
photo_status = "Chụp ảnh thất bại"
last_event_type = "photo_capture_failed"
notification_event_type = "photo_capture_failed"
```

### BR-63. Quy tắc kiểm thử chấp nhận demo

Bộ test bắt buộc gồm:

- DS-01 lịch RTC tự bật bảo vệ;
- DS-02 đột nhập đa cảm biến, camera và Telegram;
- DS-04 phát hiện sabotage;
- DS-03 sabotage rồi mất heartbeat/suy giảm, gửi email xác nhận và escalation;
- DS-05 chụp ảnh thủ công;
- DS-06 SOS Child và escalation;
- mất heartbeat không có sabotage thì không escalation;
- heartbeat còn đều sau sabotage thì chỉ giữ `SABOTAGE_ALERT`;
- xác nhận lặp không gửi email trùng;
- camera lỗi vẫn giữ các kênh cảnh báo còn lại.

### BR-64. Quy tắc ưu tiên `alarm_status`

```text
SOS_ALERT
> CRITICAL_SECURITY_COMPROMISE
> SABOTAGE_ALERT
> INTRUSION_ALERT
> SYSTEM_ERROR
> ARMED
> DISARMED
```

Trạng thái ưu tiên cao hơn chỉ chi phối phần hiển thị tổng; các cờ sự kiện bên dưới vẫn được giữ cho đến reset hoặc khi Apps Script đóng event theo đúng quy tắc.

### BR-65. Quy tắc reset tổng cảnh báo

Reset local phải xóa intrusion, sabotage, SOS và critical flags tại board khi board còn online; giữ log và event counter. Apps Script event có thể vẫn giữ lịch sử và trạng thái đã gửi. Nếu board offline, không được giả lập reset local thành công.

### BR-66. Quy tắc tách Telegram notification và email escalation

- Telegram dùng cho cảnh báo nhanh: intrusion, sabotage, SOS, camera failure và critical compromise khi board còn mạng.
- Google Apps Script dùng cho heartbeat, xác nhận SOS và xác nhận critical compromise.
- Email escalation chỉ gửi sau xác nhận Parent/Admin.
- Nếu board đã mất liên lạc, Apps Script phải tự gửi email xác nhận dựa trên heartbeat timeout; không chờ ESP32 gửi thêm request.

### BR-67. Quy tắc polling trạng thái từ Google Apps Script

Khi board còn liên lạc, firmware có thể polling theo `eventId` để cập nhật trạng thái SOS hoặc critical compromise.

Nếu board mất liên lạc, email và Apps Script là nguồn trạng thái chính; dashboard Arduino Cloud có thể giữ dữ liệu cuối cùng và phải ghi rõ có thể đã cũ.

### BR-68. Quy tắc timeout xác nhận

Timeout xác nhận không được tự gửi contact mô phỏng. Trạng thái chuyển thành `NO_CONFIRMATION_TIMEOUT` hoặc tương đương và chờ thao tác mới của Parent/Admin.

Heartbeat timeout và confirmation timeout là hai khái niệm khác nhau, không được dùng chung timer.

### BR-69. Quy tắc cấu hình authority contact và địa chỉ nhà

Địa chỉ nhà, family recipients, contact mô phỏng và token phải nằm trong Google Apps Script Properties hoặc cấu hình triển khai bảo mật. Cùng cấu hình này được dùng cho SOS và critical-compromise escalation nhưng nội dung email phải phân biệt loại event.

### BR-70. Quy tắc tự reset yêu cầu notification

`send_notification_request` là command flag, tức biến dạng yêu cầu xử lý một lần.

Khi hệ thống cần gửi notification, hệ thống phải đặt:

- `send_notification_request = true`
- `notification_sent_status = PENDING`

Sau khi quá trình gửi notification kết thúc, dù thành công hay thất bại, hệ thống phải đặt:

- `notification_sent_status = SENT` nếu gửi thành công,
- `notification_sent_status = FAILED` nếu gửi thất bại,
- `send_notification_request = false`.

`send_notification_request` không được giữ `true` liên tục trong suốt thời gian cảnh báo active.

### BR-71. Quy tắc trạng thái notification gần nhất

Trong phiên bản demo, hệ thống chỉ theo dõi trạng thái gửi notification gần nhất.

`notification_event_type` biểu diễn loại event gần nhất cần gửi notification. `notification_sent_status` biểu diễn trạng thái gửi của `notification_event_type` hiện tại.

Nếu một event notification mới xuất hiện trong lúc event cũ vừa gửi xong hoặc đang chờ gửi, event mới được quyền ghi đè:

- `notification_event_type`,
- `notification_sent_status`,
- `send_notification_request`.

Hệ thống không cần lưu trạng thái gửi riêng cho từng loại event trong phiên bản demo.

### BR-72. Quy tắc tách chụp ảnh thủ công và tự động

Hệ thống phải tách rõ hai loại yêu cầu chụp ảnh:

1. Chụp thủ công:
   - dùng biến `manual_capture_photo`,
   - Parent/Admin bấm từ Dashboard,
   - Permission: Read & Write,
   - firmware chốt lệnh vào cờ nội bộ ngay khi button chuyển `true`, tự đặt `manual_capture_photo = false` và xử lý đúng một lần chụp.
2. Chụp tự động:
   - dùng biến `auto_capture_photo_request`,
   - do hệ thống tự tạo khi có `intrusion_alert` mới,
   - Permission: Read Only,
   - mỗi lần `intrusion_alert` mới chỉ được tự động chụp 1 ảnh.

Biến cũ `capture_photo` không còn dùng trong checkpoint hiện tại.

### BR-73. Quy tắc trạng thái chụp ảnh đơn giản

Hệ thống dùng `photo_status` để hiển thị trạng thái xử lý camera/chụp ảnh.

`photo_status` dùng câu tiếng Việt cho người dùng, gồm các trạng thái điển hình: `Camera sẵn sàng`, `Đã nhận yêu cầu chụp ảnh thủ công`, `Đang chụp ảnh`, `Đã chụp và gửi ảnh` và thông báo lỗi camera cụ thể. Trạng thái có thể cho biết nguồn chụp qua `last_event` (`MANUAL_DASHBOARD`, `AUTO_INTRUSION_ALERT`, `AUTO_SABOTAGE_ALERT`).

### BR-74. Quy tắc hiển thị cooldown notification

Trong phiên bản demo, hệ thống vẫn dùng timer nội bộ để chống gửi notification lặp liên tục.

Tuy nhiên, hệ thống phải giữ hai Cloud Variable để Admin Dashboard quan sát trạng thái cooldown:

- `cooldown_active`,
- `last_alert_time`.

Ý nghĩa:

- `cooldown_active = true` khi hệ thống đang chặn gửi notification lặp cho cùng một cảnh báo đang active,
- `cooldown_active = false` khi hệ thống sẵn sàng gửi notification cho cảnh báo mới,
- `last_alert_time` là thời điểm gần nhất hệ thống gửi notification thành công hoặc tạo yêu cầu gửi notification.

Khi một cảnh báo chính như `intrusion_alert`, `sos_alert`, hoặc `sabotage_alert` được gửi notification lần đầu, hệ thống phải cập nhật `last_alert_time` và đặt `cooldown_active = true`. Khi Parent/Admin kích hoạt `reset_alarm`, hệ thống phải đặt `cooldown_active = false`.

### BR-75. Quy tắc trạng thái trung gian `device_tampered`

`device_tampered` là trạng thái trung gian cho biết thiết bị có dấu hiệu bị can thiệp vật lý.

`device_tampered = true` khi hệ thống phát hiện một trong các dấu hiệu sau:

- `ldr_covered = true`,
- `object_near = true` liên tục đủ lâu theo ngưỡng anti-sabotage,
- cảm biến có dấu hiệu bị che, bị áp sát hoặc bị vô hiệu hóa trong phạm vi demo.

`sabotage_alert = true` chỉ được kích hoạt khi dấu hiệu can thiệp đủ nghiêm trọng theo Anti-Sabotage Rule. `device_tampered = true` không nhất thiết luôn đồng nghĩa với `sabotage_alert = true`.

### BR-76. Quy tắc anti-sabotage kết hợp

Anti-sabotage không kích hoạt chỉ vì một tín hiệu đơn lẻ. Điều kiện phải đồng thời thỏa:

- `ldr_covered = true`;
- khoảng cách siêu âm `<= 15 cm` (`objectTooClose` nội bộ);
- duy trì liên tục ít nhất 3 giây.

Khi đủ điều kiện, hệ thống đặt `sabotage_alert = true`, `threat_level = 4`, bật còi/LED đỏ, gửi Telegram chữ và một ảnh Telegram, gửi event `sabotage_alert` đến Apps Script và bắt đầu theo dõi heartbeat. Nếu một trong hai tín hiệu biến mất trước 3 giây thì không kích hoạt `sabotage_alert`.

### BR-77. Quy tắc pipeline tính `intrusion_score`

1. Đọc PIR, siêu âm, LDR và thời gian RTC.
2. Tính điểm thô: PIR +2, object-near +2, light abnormal +1, night mode +1.
3. Áp dụng pet/false-positive adjustment nếu hợp lệ.
4. Giữ điểm tạm thời theo BR-11.
5. So sánh với ngưỡng `sensitivity_level`.
6. Không dùng BLE hoặc WiFi/MAC trong pipeline.

### BR-78. Quy tắc `night_mode` trong `intrusion_score`

Trong phiên bản demo, `night_mode` được xem là yếu tố làm tăng mức nghi ngờ đột nhập.

Khi `night_mode = true`, hệ thống cộng trực tiếp 1 điểm vào `raw_intrusion_score`. Điểm từ `night_mode` phải được tính cùng các tín hiệu khác trước khi áp dụng rule giảm điểm do `pet_detected`.

Ở `sensitivity_level = 3`, threshold = 3. Vì `pir_detected` là tín hiệu mạnh +2 và `night_mode` là tín hiệu bối cảnh +1, hệ thống có thể kích hoạt `intrusion_alert` khi có `night_mode = true` và một tín hiệu vật lý mạnh như `pir_detected = true` hoặc `object_near = true`.

### BR-79. Quy tắc `event_counter` cho sự kiện lịch

Trong phiên bản demo, `event_counter` chỉ đếm các sự kiện quan trọng hoặc sự kiện lỗi.

Các event lịch tự động chạy bình thường không làm tăng `event_counter`, gồm:

- `schedule_auto_arm`,
- `schedule_auto_disarm`.

Các event lỗi lịch phải làm tăng `event_counter`, gồm:

- `schedule_config_error`,
- `schedule_time_error`.

### BR-80. Quy tắc phạm vi phần cứng bắt buộc

Trong phiên bản hiện tại, tất cả phần cứng được liệt kê trong SRS là bắt buộc cho demo.

Các phần cứng bắt buộc gồm:

- `Freenove ESP32-S3 WROOM + Camera OV3660`,
- PIR sensor,
- HY-SRF05 ultrasonic sensor,
- LDR sensor,
- DS1307 RTC,
- Buzzer,
- LED đỏ,
- LED xanh,
- Arduino Cloud.

Hệ thống không được giả lập HC-SR04 hoặc DS1307 RTC nếu chưa có xác nhận riêng từ chủ dự án.

### BR-81. Quy tắc LED báo bình thường và báo động

Hệ thống phải có hai LED trạng thái:

- LED xanh: báo hệ thống đang chạy bình thường.
- LED đỏ: báo hệ thống đang có cảnh báo.

Khi hệ thống không có cảnh báo active:

- `led_green_on = true`,
- `led_red_on = false`,
- `buzzer_on = false`.

Khi có cảnh báo active như `intrusion_alert`, `sos_alert`, hoặc `sabotage_alert`, hệ thống phải:

- `led_green_on = false`,
- `led_red_on = true`,
- LED đỏ nháy theo chu kỳ cảnh báo,
- buzzer bật theo rule cảnh báo tương ứng.

LED xanh không dùng để phân biệt `ARMED` hay `DISARMED`. LED xanh chỉ biểu thị hệ thống đang hoạt động bình thường và không có cảnh báo active.

### BR-82. Quy tắc nháy LED đỏ nội bộ

Trong phiên bản demo, hệ thống không cần tạo Cloud Variable riêng để biểu diễn trạng thái nháy của LED đỏ.

Quy ước:

- `led_red_on = false`: LED đỏ tắt.
- `led_red_on = true`: LED đỏ đang ở chế độ cảnh báo và sẽ nháy theo logic nội bộ trong code.

Việc bật/tắt LED đỏ theo chu kỳ nháy phải được xử lý bằng timer nội bộ trong chương trình.

### BR-83. Quy tắc hiển thị yêu cầu chụp ảnh tự động

`auto_capture_photo_request` là Cloud Variable Read Only cho Admin Dashboard quan sát.

Khi một `intrusion_alert` hoặc `sabotage_alert` mới được kích hoạt, nếu hệ thống chưa tự chụp ảnh cho cảnh báo hiện tại, hệ thống phải:

- đặt `auto_capture_photo_request = true`,
- đặt `photo_status` thành thông báo đang chụp bằng tiếng Việt,
- xử lý chụp ảnh bằng camera OV3660 trên `Freenove ESP32-S3 WROOM + Camera OV3660`.

Sau khi xử lý xong, hệ thống phải:

- đặt `auto_capture_photo_request = false`,
- đặt `photo_status = "Đã chụp và gửi ảnh"` nếu thành công,
- đặt thông báo lỗi cụ thể nếu thất bại,
- đặt cờ nội bộ `auto_photo_captured_for_current_alert = true`.

### BR-84. Quy tắc notification khi camera lỗi

Khi hệ thống xử lý yêu cầu chụp ảnh nhưng camera chụp thất bại, hệ thống phải:

1. đặt `photo_status = "FAILED"`,
2. cập nhật `last_event = "Chụp ảnh thất bại."`,
3. cập nhật `last_event_type = "photo_capture_failed"`,
4. tăng `event_counter` thêm 1,
5. đặt `notification_event_type = "photo_capture_failed"`,
6. đặt `notification_channel = "telegram"`,
7. đặt `send_notification_request = true`,
8. gửi Telegram/notification cho Parent/Admin.

Quy tắc này áp dụng cho cả chụp ảnh tự động do `intrusion_alert` và chụp ảnh thủ công do Parent/Admin bấm `manual_capture_photo`.

### BR-85. Quy tắc không cooldown lỗi camera

Trong phiên bản demo, lỗi chụp ảnh không dùng cooldown riêng. Mỗi lần hệ thống xử lý yêu cầu chụp ảnh và kết quả thất bại, hệ thống phải xem đó là một event lỗi riêng và được phép gửi Telegram/notification riêng.

### BR-86. Quy tắc LDR trong anti-sabotage

`ldr_covered` là điều kiện bắt buộc nhưng không đủ một mình để tạo sabotage. Quy tắc đầy đủ dùng điều kiện kết hợp và thời lượng 3 giây tại BR-76.

### BR-87. Quy tắc hiển thị `device_tampered`

`device_tampered` phản ánh snapshot cảm biến hiện tại để Admin quan sát/debug; không phải trạng thái latch riêng. `sabotage_alert` mới là trạng thái cảnh báo được giữ cho đến khi `reset_alarm`.


### BR-88. Quy tắc giữ mã kỹ thuật và tách nội dung người dùng

Tên biến, event ID, enum và mã trạng thái kỹ thuật phải được giữ ổn định để code, polling và test không bị sai. Khi hiển thị cho người dùng, hệ thống phải ánh xạ mã đó sang câu tiếng Việt có dấu.

Ví dụ:

```text
Mã nội bộ: INTRUSION_ALERT
Nội dung người dùng: Phát hiện dấu hiệu đột nhập trong khu vực bếp. Hệ thống đang bật cảnh báo tại chỗ.
```

### BR-89. Quy tắc cấu trúc thông báo có ý nghĩa

Một thông báo cảnh báo, SOS, lỗi hoặc thao tác thành công phải nêu tối thiểu:

1. sự kiện hoặc trạng thái hiện tại,
2. nguyên nhân hoặc tín hiệu chính nếu có,
3. hành động hệ thống đã thực hiện,
4. thao tác tiếp theo của người dùng nếu cần.

Đối với cảnh báo quan trọng gửi từ xa, thông báo phải bổ sung thời gian và khu vực khi dữ liệu này khả dụng.

### BR-90. Quy tắc diễn đạt trung tính và không khẳng định quá mức

Khi hệ thống chỉ dựa trên cảm biến và điểm nghi ngờ, nội dung phải dùng cách diễn đạt như:

- `Phát hiện dấu hiệu đột nhập`,
- `Phát hiện chuyển động bất thường`,
- `Thiết bị có dấu hiệu bị che hoặc can thiệp`.

Không được khẳng định `Có trộm`, `Kẻ trộm đã vào nhà` hoặc nội dung tương đương nếu hệ thống không có bằng chứng xác nhận chắc chắn.

### BR-91. Quy tắc nội dung Serial Monitor/CLI

Serial Monitor/CLI được phép hiển thị thêm mã event, giá trị cảm biến, điểm và tên hàm phục vụ debug. Tuy nhiên, sự kiện chính phải có câu tiếng Việt giải thích.

Định dạng khuyến nghị:

```text
[CẢM BIẾN] PIR phát hiện chuyển động.
[PHÂN TÍCH] Điểm nghi ngờ hiện tại: 4/4.
[CẢNH BÁO] Phát hiện dấu hiệu đột nhập trong khu vực bếp.
[HÀNH ĐỘNG] Đã bật còi, đèn đỏ và yêu cầu camera chụp ảnh.
```

### BR-92. Quy tắc nội dung Arduino Cloud

Các biến String dùng cho người dùng như `alarm_status`, `last_event`, `photo_status`, `notification_sent_status`, `sos_message`, `emergency_escalation_status` và `emergency_authority_message_status` phải có nội dung tiếng Việt dễ hiểu hoặc có lớp ánh xạ hiển thị tương đương.

Dashboard không được chỉ hiển thị mã như `FAILED` hoặc `WAITING_CONFIRMATION` mà không giải thích thao tác hoặc ý nghĩa.

### BR-93. Quy tắc nội dung Telegram

Mỗi thông báo Telegram quan trọng phải bao gồm, khi dữ liệu khả dụng:

- tiêu đề sự kiện tiếng Việt,
- thời gian,
- khu vực giám sát,
- tín hiệu/nguyên nhân chính,
- mức nguy hiểm hoặc điểm nghi ngờ khi hữu ích,
- hành động hệ thống đã thực hiện,
- hướng dẫn reset hoặc kiểm tra nếu người dùng cần thao tác.

Thông báo gửi kèm ảnh phải nói rõ ảnh được chụp tự động do cảnh báo hay do yêu cầu thủ công.

### BR-94. Quy tắc nội dung Gmail và Google Apps Script

Email xác nhận SOS, email escalation, email cập nhật kết quả, trang xác nhận và trang lỗi phải dùng tiếng Việt có dấu và giải thích đầy đủ trạng thái.

Các trường kỹ thuật như `emergency_escalation_status`, `eventId` hoặc `home_address_configured` có thể được giữ trong phần chi tiết kỹ thuật, nhưng phần chính phải hiển thị bằng nhãn tiếng Việt, ví dụ:

```text
Trạng thái xử lý SOS: Đang chờ người lớn xác nhận.
Địa chỉ hỗ trợ khẩn cấp: Chưa được cấu hình.
```

### BR-95. Quy tắc nhất quán và không thay đổi nghiệp vụ

Một event phải giữ cùng ý nghĩa trên mọi kênh. Việc rút gọn hoặc mở rộng câu theo từng kênh không được làm thay đổi:

- loại sự kiện,
- mức nguy hiểm,
- trạng thái gửi,
- trạng thái xác nhận,
- hành động hệ thống đã thực hiện,
- yêu cầu reset.

Chuẩn hóa nội dung không được thay đổi thuật toán hoặc luồng nghiệp vụ hiện tại.

---

### BR-96. Quy tắc gửi heartbeat

ESP32-S3 gửi heartbeat đến Google Apps Script mỗi 10 giây khi `google_script_enabled` và `heartbeat_enabled` cùng bật. Gửi heartbeat thất bại không được chặn vòng lặp cảm biến quá lâu.

### BR-97. Quy tắc heartbeat timeout

Apps Script xem heartbeat quá hạn khi thời điểm server nhận cuối cùng cũ hơn 40 giây. Trigger kiểm tra chạy mỗi phút, nên thời gian gửi email thực tế có thể nằm trong khoảng 40–100 giây sau heartbeat cuối. Một request lỗi đơn lẻ không đủ để kết luận offline.

### BR-98. Quy tắc tương quan sabotage và mất liên lạc

`CRITICAL_SECURITY_COMPROMISE` chỉ được tạo khi:

```text
sabotage_alert đã xảy ra trước
AND
heartbeat quá hạn hoặc health check nghiêm trọng thất bại kéo dài
AND
hai sự kiện nằm trong cửa sổ theo dõi cấu hình
```

### BR-99. Quy tắc mất heartbeat không có sabotage

Nếu heartbeat quá hạn nhưng không có sabotage active/gần đây, Apps Script phải đặt `DEVICE_OFFLINE` hoặc `CONNECTION_LOST` và gửi một email mất kết nối duy nhất cho Parent/Admin. Khi heartbeat trở lại, Apps Script phải ghi `DEVICE_ONLINE` và chỉ gửi một email khôi phục duy nhất. Không gửi authority escalation trong cả hai trường hợp.

### BR-100. Quy tắc health check cục bộ

Camera hoặc siêu âm chỉ được xem là lỗi kéo dài sau nhiều lần thất bại liên tiếp. Giá trị mặc định cho demo là 3 lần. Một lỗi đơn lẻ chỉ cập nhật trạng thái kỹ thuật.

### BR-101. Quy tắc suy giảm có kiểm soát

Khi một chức năng lỗi, các chức năng còn lại phải tiếp tục nếu có thể. Ví dụ camera lỗi không được làm tắt còi, LED, log hoặc Telegram chữ.

### BR-102. Quy tắc nội dung critical compromise

Thông báo phải nêu:

- trước đó có hành vi phá hoại;
- thiết bị vừa mất liên lạc hoặc mất một phần khả năng;
- nguyên nhân chưa được xác định chắc chắn;
- các khả năng có thể gồm mất nguồn, mất mạng, board treo hoặc bị vô hiệu hóa;
- Parent/Admin cần xác nhận trước khi gửi báo cáo.

### BR-103. Quy tắc chống gửi escalation trùng

Mỗi `critical_event_id` chỉ được gửi một email authority escalation thành công. Nhiều người bấm xác nhận hoặc tải lại trang không được tạo email trùng.

### BR-104. Quy tắc reset và thiết bị offline

Nếu board còn online, `reset_alarm` có thể xóa cảnh báo local. Nếu board đã offline hoặc mất điện, Apps Script không được tuyên bố đã reset thiết bị; chỉ được đóng/ghi nhận event phía Cloud sau thao tác phù hợp.

### BR-105. Quy tắc controlled demo input cho DS-03

Admin được phép mô phỏng heartbeat timeout hoặc multi-module failure mà không phá phần cứng thật. Demo phải hiển thị rõ đây là mô phỏng đầu vào lỗi, trong khi luồng xử lý, email và xác nhận là thật.

## 8. Yêu Cầu Chức Năng

### FR-01. Điều khiển chế độ chống trộm

#### Mục đích

Cho phép Parent/Admin bật hoặc tắt chức năng phát hiện đột nhập.

#### Luồng chính

1. Parent/Admin thay đổi `alarm_enabled`.
2. Nếu `alarm_enabled = false`:
   - `system_armed = false`;
   - PIR/LDR/siêu âm không được tạo intrusion alert;
   - SOS và anti-sabotage vẫn hoạt động.
3. Nếu `alarm_enabled = true` và không có cảnh báo active:
   - `system_armed = true`;
   - hệ thống bắt đầu tính `intrusion_score`.
4. Không dùng BLE, WiFi/MAC hoặc `known_device_present` để thay đổi trạng thái.

#### Acceptance Scenario

Bật `alarm_enabled` làm hệ thống chuyển sang bảo vệ; tắt biến này ngăn cảnh báo đột nhập nhưng vẫn cho phép sabotage và SOS.

### FR-02. Xử lý reset báo động

#### Mục đích

Cho phép Parent/Admin kết thúc cảnh báo local sau khi đã kiểm tra tình hình.

#### Luồng chính

1. Parent/Admin đặt `reset_alarm = true`.
2. Nếu board online, firmware tắt còi/LED đỏ và xóa các cờ cảnh báo local.
3. Giữ log và bộ đếm.
4. Tính `system_armed = alarm_enabled` nếu không có lỗi an toàn chặn.
5. Tự đặt `reset_alarm = false`.
6. Nếu board offline, dashboard/email không được báo reset thiết bị thành công.

#### Acceptance Scenario

Reset khi board online phải tắt đầu ra và giữ log. ACK email không được tự gọi reset.

### FR-03. Điều khiển chế độ chống trộm theo lịch

#### Mục đích

Tự bật/tắt `alarm_enabled` theo RTC.

#### Luồng chính

1. Parent/Admin cấu hình giờ/phút và bật `schedule_enabled`.
2. Đến giờ tự bật, đặt `alarm_enabled = true` và tính `system_armed`.
3. Đến giờ tự tắt, đặt `alarm_enabled = false`, `system_armed = false`.
4. Lịch không reset SOS, sabotage, intrusion hoặc critical compromise đang active.
5. Không phụ thuộc điện thoại đáng tin.

#### Acceptance Scenario

RTC đến giờ tự bật phải bật bảo vệ; đến giờ tự tắt phải tắt phát hiện đột nhập, nhưng SOS và anti-sabotage vẫn hoạt động.

### FR-04. Phát hiện đột nhập đa cảm biến

#### Mục đích

Phát hiện đột nhập bằng weighted intrusion score từ tín hiệu vật lý.

#### Đầu vào

- `pir_detected` +2;
- `object_near` +2;
- `light_abnormal` +1;
- `night_mode` +1.

#### Luồng chính

1. Chỉ tính điểm khi `system_armed = true`.
2. Tính score theo trọng số.
3. Áp dụng ngưỡng 5/4/3 cho sensitivity 1/2/3.
4. Khi đạt ngưỡng, latch `intrusion_alert`, bật còi/LED, gửi Telegram và chụp một ảnh.
5. Điểm giảm không tự tắt cảnh báo.
6. Không dùng BLE hoặc WiFi/MAC.

#### Acceptance Scenario

PIR + object-near đạt 4 điểm và phải kích hoạt ở sensitivity 2; WiFi/MAC không ảnh hưởng kết quả.

### FR-05. Phát hiện thiết bị WiFi/MAC lạ — Loại khỏi phạm vi v0.7.0

Không triển khai quét WiFi/MAC, unknown-device pre-warning hoặc cộng điểm từ MAC lạ. ID FR-05 được giữ để truy vết lịch sử.

### FR-06. Gửi Telegram / notification khi có sự kiện an ninh

#### Mục đích

Gửi cảnh báo nhanh cho intrusion, sabotage, critical compromise, SOS, camera failure và lỗi lịch quan trọng.

#### Luồng chính

1. Firmware tạo `notification_event_type`.
2. Đặt trạng thái `PENDING`.
3. Gửi Telegram chữ hoặc ảnh phù hợp.
4. Cập nhật `SENT` hoặc `FAILED`.
5. Nếu camera lỗi trong intrusion, gửi cảnh báo chữ thay thế.
6. Không gửi heartbeat qua Telegram.

#### Ràng buộc

Mỗi event chỉ gửi một lần trong cooldown; nội dung tiếng Việt, không khẳng định quá mức.

#### Acceptance Scenario

Critical compromise khi board còn mạng phải gửi Telegram chữ; nếu board đã offline, Apps Script gửi email xác nhận dựa trên heartbeat timeout.

### FR-07. Tự chụp ảnh khi có đột nhập

#### Mục đích

Hệ thống phải tự động chụp ảnh bằng camera OV3660 trên `Freenove ESP32-S3 WROOM + Camera OV3660` khi có cảnh báo đột nhập để ghi nhận bằng chứng.

#### Luồng chính

1. Một lần `intrusion_alert` mới được kích hoạt.
2. Hệ thống kiểm tra cờ nội bộ `auto_photo_captured_for_current_alert`.
3. Nếu cờ nội bộ là false, hệ thống đặt `auto_capture_photo_request = true` và tự chụp 1 ảnh.
4. Hệ thống cập nhật `photo_status = "Đang chụp ảnh"` trong lúc xử lý.
5. Nếu chụp thành công, hệ thống đặt `photo_status = "Đã chụp và gửi ảnh"`.
6. Nếu chụp thất bại, hệ thống đặt thông báo lỗi tiếng Việt, cập nhật `last_event_type = "photo_capture_failed"` và gửi notification theo Camera Thất bạiure Notification Rule.
7. Hệ thống đặt `auto_capture_photo_request = false` và đặt cờ nội bộ đã chụp ảnh tự động cho lần cảnh báo hiện tại.
8. Trong cùng một lần `intrusion_alert`, hệ thống không tự chụp thêm.
9. Parent/Admin vẫn có thể đặt `manual_capture_photo = true` để chụp thủ công thêm.
10. Khi `reset_alarm` được xử lý, hệ thống reset cờ nội bộ để lần cảnh báo tiếp theo có thể tự chụp 1 ảnh mới.

#### Acceptance Scenario

Khi một `intrusion_alert` mới xảy ra, hệ thống phải tự chụp đúng 1 ảnh. Trong cùng cảnh báo đó, hệ thống không tự chụp thêm ảnh, nhưng Parent/Admin vẫn được phép chụp thủ công thêm.

---

### FR-08. Lọc báo động giả / vật nhỏ

#### Mục đích

Giảm báo động do tín hiệu ngắn mà không làm mất cảnh báo thật.

#### Luồng chính

1. Theo dõi PIR và object-near xuất hiện ngắn dưới 5 giây.
2. Nếu không có light abnormal và chưa có intrusion alert, có thể đặt `pet_detected = true`.
3. Giảm score tối đa 1 điểm, không nhỏ hơn 0.
4. Đặt `threat_level = 1`, ghi log và không gửi Telegram.
5. Không được dùng pet logic để tắt cảnh báo đã latch hoặc hạ nguy cơ khi tín hiệu kéo dài.

#### Acceptance Scenario

Tín hiệu PIR/object ngắn, không có ánh sáng bất thường, phải có thể được ghi nhận là false positive mà không gửi Telegram.

### FR-09. Phát hiện phá hoại thiết bị — DS-04

#### Mục đích

Phát hiện kẻ xấu đang che hoặc can thiệp thiết bị, trước khi kết luận thiết bị đã mất chức năng.

#### Luồng chính

1. Hệ thống theo dõi `ldr_covered` cùng vật thể rất gần (`<= 15 cm`) trong 3 giây.
2. Khi điều kiện sabotage đủ thời gian:
   - đặt `device_tampered = true`;
   - đặt `sabotage_alert = true`;
   - đặt `alarm_status = SABOTAGE_ALERT`;
   - đặt `threat_level = 4`;
   - bật còi và LED đỏ;
   - gửi Telegram chữ và một ảnh Telegram;
   - gửi event `sabotage_alert` đến Apps Script;
   - bắt đầu cửa sổ theo dõi heartbeat và health check.
3. Không tự kết luận module nào đã hỏng.

#### Acceptance Scenario

Khi LDR bị che **và** vật áp sát đủ thời gian, hệ thống báo sabotage ngay cả khi `alarm_enabled = false`, sau đó vẫn tiếp tục theo dõi để có thể nâng lên DS-03.

### FR-10. SOS khẩn cấp qua Dashboard và Google Apps Script Authority Escalation

**Trạng thái:** Đã phân rã bản demo mở rộng

#### Mục đích

Hệ thống phải cho phép Child hoặc Parent/Admin kích hoạt SOS khẩn cấp qua dashboard. Khi SOS được kích hoạt, hệ thống phải bật cảnh báo tại chỗ và giữ cảnh báo đó cho đến khi Parent/Admin chủ động reset. Sau khi Parent/Admin xác nhận, Google Apps Script phải gửi email escalation kèm địa chỉ nhà và ghi chú bổ sung đến contact cơ quan chức năng/công an hoặc contact mô phỏng được cấu hình hợp lệ.

#### Luồng chính

1. Người dùng bấm `sos_child` hoặc `sos_adult`.
2. Hệ thống kích hoạt:
   - `alarm_status = SOS_ALERT`,
   - `threat_level = 4`,
   - `buzzer_on = true`,
   - `led_green_on = false`,
   - `led_red_on = true`,
   - `last_event_type = "sos_alert"`.
3. Hệ thống gửi notification cho Parent/Admin.
4. Hệ thống yêu cầu Google Apps Script gửi email xác nhận emergency escalation cho Parent/Admin.
5. Hệ thống tự reset biến `sos_child` hoặc `sos_adult` về `false` sau khi đã nhận sự kiện.
6. Parent/Admin có thể nhập `sos_authority_note` để bổ sung tình hình hiện tại hoặc nhắc nhở cho cơ quan tiếp nhận.
7. Board polling Google Apps Script mỗi 5 giây để kiểm tra trạng thái xác nhận.
8. Trong lúc chờ xác nhận, hệ thống vẫn giữ `SOS_ALERT`, `buzzer_on = true`, `led_red_on = true` và `led_green_on = false`.
9. Nếu ít nhất một Parent/Admin xác nhận qua email trong vòng 60 giây, Google Apps Script gửi email escalation đến contact được cấu hình hợp lệ.
10. Email escalation phải chứa địa chỉ nhà đã cấu hình, nguồn SOS, thời điểm SOS, trạng thái hệ thống và `sos_authority_note` nếu có.
11. Sau khi ACK/xác nhận hoặc sau khi email escalation được gửi thành công/thất bại, hệ thống vẫn giữ còi/LED đỏ và `SOS_ALERT`.
12. Trong demo, contact này là email phụ của thành viên nhóm và đóng vai trò cơ quan chức năng mô phỏng; trong triển khai thật, contact chỉ được cấu hình khi hợp lệ.
13. Parent/Admin dùng `reset_alarm` để tắt còi/LED và kết thúc cảnh báo SOS.

#### Ràng buộc

Hệ thống chỉ gửi authority escalation sau khi Parent/Admin xác nhận. ACK/xác nhận escalation không được tự tắt cảnh báo tại chỗ. Hệ thống không đảm bảo thời gian phản hồi hoặc thời gian công an/cơ quan chức năng đến hiện trường.

#### Acceptance Scenario

Khi `sos_child = true`, hệ thống phải bật còi/LED, đặt `alarm_status = SOS_ALERT`, gửi notification cho Parent/Admin, yêu cầu Google Apps Script gửi email xác nhận escalation và sau xác nhận phải gửi email escalation có địa chỉ nhà/ghi chú đến contact đã cấu hình. Còi/LED đỏ và `SOS_ALERT` phải tiếp tục duy trì sau ACK và sau khi gửi escalation, chỉ tắt khi Parent/Admin kích hoạt `reset_alarm`.

### FR-11. Ghi log sự kiện

Hệ thống lưu `last_event`, `last_event_type` và tăng `event_counter` cho:

- intrusion;
- sabotage;
- critical compromise;
- SOS;
- photo failure;
- lịch lỗi;
- heartbeat lost/restored;
- escalation sent/failed;
- reset.

Bản demo không yêu cầu database lịch sử dài hạn.

### FR-12. Chống spam cảnh báo / cooldown

Mỗi event intrusion, sabotage, critical compromise, SOS hoặc camera failure chỉ gửi notification một lần cho một event ID trong thời gian cooldown. Heartbeat không được gửi qua Telegram. Apps Script phải chống gửi email xác nhận và authority escalation trùng.

### FR-13. Chế độ demo

**Trạng thái:** Demo Mode chỉ hỗ trợ mức cần thiết; không làm giảm phạm vi chức năng chính

#### Mục đích

Hệ thống có thể có `demo_mode` và `demo_scenario` để hỗ trợ kiểm thử hoặc trình diễn, nhưng các chức năng chính không được phụ thuộc vào demo mode.

#### Luồng chính

1. Nếu `demo_mode = false`, hệ thống chạy theo logic thật từ cảm biến và dashboard.
2. Nếu `demo_mode = true`, hệ thống có thể cho phép Admin hỗ trợ kiểm thử thủ công hoặc chọn kịch bản demo đơn giản.
3. Hệ thống không bắt buộc mô phỏng toàn bộ cảm biến. `demo_scenario` chỉ nên dùng để mô phỏng heartbeat timeout hoặc suy giảm mà không phá phần cứng thật.

#### Cách demo khuyến nghị

| Tình huống demo | Cách làm |
|---|---|
| Đột nhập | Di chuyển trước PIR và/hoặc đặt vật gần HC-SR04 |
| Pet/báo nhầm | Tạo chuyển động ngắn dưới 5 giây |
| Sabotage | Che LDR và đặt vật trong phạm vi <= 15 cm liên tục 3 giây |
| SOS | Bấm `sos_child` hoặc `sos_adult` |
| Reset | Bấm `reset_alarm` |
| Emergency escalation | Dùng Google Apps Script gửi email xác nhận và email demo authority |

#### Acceptance Scenario

Khi `demo_mode = false`, các chức năng chính như intrusion, SOS, sabotage, notification và reset vẫn phải hoạt động bình thường.

### FR-14. Hỗ trợ Arduino Cloud Dashboard tối thiểu

#### Child Dashboard

- nút SOS trẻ em;
- trạng thái hệ thống đơn giản;
- sự kiện gần nhất.

#### Parent Dashboard

- bật/tắt chống trộm;
- reset cảnh báo;
- lịch RTC;
- intrusion, sabotage, critical compromise và SOS;
- trạng thái thiết bị online/mất liên lạc;
- chụp ảnh thủ công;
- công tắc Telegram, Google Apps Script, heartbeat, camera và tự chụp khi cảnh báo;
- trạng thái xác nhận/escalation;
- sự kiện gần nhất.

#### Admin Dashboard

- toàn bộ cảm biến và điểm đột nhập;
- trạng thái camera, notification và output;
- heartbeat gần nhất;
- `critical_security_compromise`, `system_degraded`, `compromise_reason`;
- event ID và trạng thái email;
- `demo_mode`, `demo_scenario`.
- công tắc Gemini và trạng thái retry/kết nối kỹ thuật.

Dashboard v0.8.0 không chứa BLE, WiFi scanning, trusted phone hoặc re-arm countdown.

### FR-15. Demo Acceptance Test

#### Sáu kịch bản chính

1. DS-01 — RTC tự bật chống trộm.
2. DS-02 — Đột nhập ban đêm bằng PIR + siêu âm + LDR, camera và Telegram.
3. DS-04 — Phát hiện hành vi che/can thiệp thiết bị.
4. DS-03 — Sau DS-04, thiết bị mất heartbeat hoặc suy giảm; Apps Script gửi email xác nhận; Parent/Admin xác nhận; contact mô phỏng nhận báo cáo.
5. DS-05 — Chụp ảnh thủ công từ Parent Dashboard.
6. DS-06 — Child SOS, email xác nhận và escalation.

#### Test âm bắt buộc

- mất heartbeat không có sabotage: không escalation;
- sabotage nhưng heartbeat vẫn còn: không nâng critical;
- một lỗi camera: chưa kết luận critical;
- xác nhận lặp: không gửi trùng;
- ACK email: không tự reset cảnh báo local.

### FR-16. Nhận diện thiết bị đáng tin bằng BLE scanning — Loại khỏi phạm vi v0.7.0

Không triển khai BLE advertiser, active scan, whitelist tên điện thoại, RSSI hoặc auto-disarm/re-arm. ID được giữ để truy vết lịch sử.

### FR-17. SOS authority escalation có địa chỉ nhà và ghi chú bổ sung

**Trạng thái:** Đã phân rã ở mức Google Apps Script

#### Mục đích

Hệ thống phải hỗ trợ gửi thông tin SOS đã xác nhận đến contact cơ quan chức năng/công an hoặc contact mô phỏng bằng Google Apps Script, trong đó có địa chỉ nhà và ghi chú bổ sung của Parent/Admin.

#### Actor chính

- Child
- Parent/Admin
- Google Apps Script
- Contact cơ quan chức năng/công an hoặc contact mô phỏng

#### Biến liên quan

| Biến | Ý nghĩa |
|---|---|
| `emergency_confirmation_requested` | Đã yêu cầu xác nhận SOS escalation |
| `emergency_confirmed` | Parent/Admin đã xác nhận escalation |
| `emergency_escalation_status` | Trạng thái xác nhận/gửi escalation |
| `home_address_configured` | Apps Script đã cấu hình địa chỉ nhà hợp lệ |
| `sos_authority_note` | Ghi chú bổ sung của Parent/Admin |
| `emergency_authority_message_status` | Trạng thái chuẩn bị/gửi email authority escalation |

#### Luồng chính

1. SOS được kích hoạt từ `sos_child` hoặc `sos_adult`.
2. Hệ thống yêu cầu Google Apps Script gửi email xác nhận cho Parent/Admin.
3. Parent/Admin xem tình huống và có thể nhập `sos_authority_note`.
4. Parent/Admin bấm xác nhận escalation trong email.
5. Google Apps Script kiểm tra `home_address_configured` và contact escalation.
6. Nếu hợp lệ, Apps Script gửi email escalation có địa chỉ nhà, thời điểm SOS, nguồn SOS, trạng thái hệ thống và ghi chú bổ sung.
7. Hệ thống cập nhật `emergency_escalation_status` và `emergency_authority_message_status`.

#### Ràng buộc

Trong demo, contact nhận escalation phải là contact mô phỏng để tránh gửi cảnh báo thật ngoài ý muốn. Trong triển khai thật, chỉ dùng contact cơ quan chức năng/công an hợp lệ và có sự đồng ý của chủ hệ thống.

#### Acceptance Scenario

Khi `sos_child = true`, Parent/Admin xác nhận escalation và `home_address_configured = true`, Google Apps Script phải gửi email authority escalation chứa địa chỉ nhà và `sos_authority_note` nếu có; trạng thái phải chuyển sang `SENT` nếu gửi thành công hoặc `FAILED`/`NOT_CONFIGURED` nếu không gửi được.


### FR-18. Chuẩn hóa nội dung hiển thị đa kênh bằng tiếng Việt có dấu

**Trạng thái:** Đã phân rã ở mức yêu cầu và acceptance test

#### Mục đích

Hệ thống phải chuẩn hóa toàn bộ nội dung mà người dùng hoặc người kiểm thử nhìn thấy trên Serial Monitor/CLI, Arduino Cloud, Telegram và Gmail/Google Apps Script để người dùng hiểu chính xác hệ thống đang làm gì.

#### Phạm vi

| Kênh | Nội dung thuộc phạm vi |
|---|---|
| Serial Monitor/CLI | Log khởi động, cảm biến, phân tích điểm, thay đổi trạng thái, cảnh báo, camera, BLE, Cloud, Telegram, SOS và lỗi |
| Arduino Cloud | Tên widget và giá trị String như `alarm_status`, `last_event`, `photo_status`, `notification_sent_status`, `sos_message`, trạng thái escalation |
| Telegram | Tin nhắn đột nhập, SOS, sabotage, unknown WiFi, lỗi lịch, lỗi camera, ảnh tự động và ảnh thủ công |
| Gmail/Google Apps Script | Email xác nhận SOS, email escalation, email cập nhật, trang xác nhận, trang thành công, trang lỗi và nội dung trạng thái trả về phục vụ chẩn đoán |

#### Luồng yêu cầu

1. Hệ thống xác định event hoặc trạng thái nội bộ.
2. Hệ thống giữ nguyên mã kỹ thuật cần thiết cho logic và test.
3. Hệ thống ánh xạ mã kỹ thuật sang nội dung tiếng Việt có dấu.
4. Nội dung được tạo theo cấu trúc BR-89 và quy tắc riêng của từng kênh.
5. Hệ thống gửi hoặc hiển thị nội dung mà không thay đổi logic nghiệp vụ của event.

#### Bảng ánh xạ tối thiểu

| Mã nội bộ | Nội dung người dùng tối thiểu |
|---|---|
| `ARMED` | Hệ thống chống trộm đang được bật và đang giám sát khu vực bếp. |
| `DISARMED` | Chức năng chống trộm hiện đang tắt; SOS và chống phá hoại vẫn tiếp tục hoạt động. |
| `DISARMED_BY_TRUSTED_DEVICE` | Đã phát hiện thiết bị đáng tin ở gần nên chức năng chống trộm tạm thời chưa được kích hoạt. |
| `INTRUSION_ALERT` | Phát hiện dấu hiệu đột nhập trong khu vực bếp; cảnh báo tại chỗ đang hoạt động. |
| `SABOTAGE_ALERT` | Phát hiện thiết bị có dấu hiệu bị che hoặc can thiệp; cảnh báo tại chỗ đang hoạt động. |
| `SOS_ALERT` | Yêu cầu hỗ trợ khẩn cấp đang hoạt động và sẽ duy trì cho đến khi người dùng đặt lại cảnh báo. |
| `Đang chụp ảnh` | Camera đang lấy khung hình. |
| `Đã chụp và gửi ảnh` | Camera đã chụp và Telegram đã nhận ảnh. |
| Thông báo lỗi camera | Thao tác không thành công; thông báo phải nêu thao tác nào thất bại và gợi ý kiểm tra phù hợp. |
| `WAITING_CONFIRMATION` | Đã gửi yêu cầu SOS và đang chờ người lớn xác nhận. |
| `NOT_CONFIGURED` | Chức năng chưa được cấu hình đầy đủ; thông báo phải nêu cấu hình còn thiếu. |

Bảng trên là nội dung tối thiểu. Thông báo thực tế được phép bổ sung thời gian, nguyên nhân, điểm, khu vực và hành động đã thực hiện.

#### Acceptance Scenario

1. Khi có `INTRUSION_ALERT`, cả Serial, Arduino Cloud và Telegram phải diễn đạt đây là dấu hiệu đột nhập, nêu hành động đã thực hiện và không khẳng định chắc chắn có trộm.
2. Khi `photo_status = FAILED`, người dùng phải biết camera/chụp ảnh thất bại và cần kiểm tra gì; không chỉ nhìn thấy từ `FAILED`.
3. Khi SOS đang `WAITING_CONFIRMATION`, Arduino Cloud, email và trang Apps Script phải cùng thể hiện rằng hệ thống đang chờ xác nhận và còi/LED chưa được reset.
4. Nội dung tiếng Việt phải giữ nguyên dấu khi hiển thị hoặc gửi qua các kênh mục tiêu.
5. Sau khi chuẩn hóa nội dung, toàn bộ test logic cũ vẫn phải cho kết quả như trước.

### FR-19. Gửi và theo dõi heartbeat

#### Mục đích

Cho phép Google Apps Script biết ESP32-S3 còn liên lạc hay không.

#### Luồng chính

1. ESP32-S3 gửi heartbeat mỗi 10 giây khi hai công tắc Google Apps Script và heartbeat đều bật.
2. Apps Script lưu `lastHeartbeatAt`, `deviceId` và trạng thái gần nhất.
3. Nếu heartbeat còn trong giới hạn, trạng thái là online.
4. Nếu heartbeat cuối quá 40 giây sau sabotage, trạng thái được nâng lên critical; do trigger chạy mỗi phút, email đến trong khoảng 40–100 giây.
5. Mất liên lạc không có sabotage gửi một email offline đến Parent/Admin, không escalation; heartbeat trở lại gửi một email khôi phục.

#### Acceptance Scenario

Dừng heartbeat khi không có sabotage phải gửi tối đa một email offline đến Parent/Admin, không được gửi email escalation đến contact mô phỏng.

### FR-20. Phát hiện `CRITICAL_SECURITY_COMPROMISE` sau sabotage

#### Mục đích

Xác định hệ thống đã mất liên lạc hoặc suy giảm nghiêm trọng ngay sau hành vi phá hoại.

#### Tiền điều kiện

- có `sabotage_alert` active hoặc sabotage event xảy ra trong cửa sổ theo dõi;
- Apps Script hoặc firmware có bằng chứng heartbeat timeout hoặc health check lỗi kéo dài.

#### Luồng chính

1. DS-04 kích hoạt sabotage.
2. Hệ thống bắt đầu cửa sổ theo dõi.
3. Nếu heartbeat quá hạn hoặc nhiều health check thất bại liên tiếp:
   - tạo `critical_event_id`;
   - đặt `critical_security_compromise = true` nếu board còn online;
   - đặt `system_degraded = true` khi phù hợp;
   - đặt `alarm_status = CRITICAL_SECURITY_COMPROMISE`;
   - ghi `compromise_reason` trung tính;
   - gửi email xin xác nhận.
4. Nếu heartbeat vẫn đều và module còn hoạt động, chỉ giữ `SABOTAGE_ALERT`.

#### Acceptance Scenario

Sabotage rồi ngắt heartbeat đủ timeout phải tạo critical event. Ngắt heartbeat mà không có sabotage không được tạo critical event.

### FR-21. Critical compromise confirmation và authority escalation

#### Mục đích

Cho phép Parent/Admin xác nhận gửi báo cáo sự cố phá hoại nghiêm trọng đến contact mô phỏng.

#### Luồng chính

1. Apps Script gửi email xác nhận chứa:
   - thời điểm sabotage;
   - thời điểm mất heartbeat/suy giảm;
   - khu vực;
   - trạng thái hiện tại;
   - địa chỉ nhà;
   - các khả năng nguyên nhân;
   - ô ghi chú.
2. Parent/Admin bấm xác nhận.
3. Apps Script chống xác nhận trùng.
4. Apps Script gửi một email đến contact mô phỏng.
5. Trạng thái chuyển `WAITING_CONFIRMATION → CONFIRMED → SENT` hoặc `FAILED`.
6. Nếu board còn online, firmware có thể polling để cập nhật dashboard.
7. Nếu board offline, Apps Script/email là nguồn trạng thái chính.

#### Ràng buộc

- Không tự gửi khi chưa xác nhận.
- Không khẳng định chắc chắn kẻ trộm cắt điện.
- Không dùng event SOS thay cho critical compromise.
- Xác nhận không đồng nghĩa reset thiết bị.

#### Acceptance Scenario

Một critical event chỉ tạo tối đa một email authority escalation thành công dù nhiều người bấm xác nhận.
## 9. Yêu Cầu Phi Chức Năng

### NFR-01. Độ tin cậy

Hệ thống nên tránh cảnh báo không cần thiết khi thiết bị đáng tin đang xuất hiện trong khu vực giám sát.

### NFR-02. Khả năng bảo trì

ID yêu cầu, tên biến và giá trị trạng thái phải nhất quán giữa SRS, code, dashboard và test.

### NFR-03. Dễ đọc cho agent

Tài liệu phải đánh dấu rõ chức năng đã phân rã và chức năng đang chờ phân rã để AI agent không tự triển khai logic chưa được duyệt.

### NFR-04. Khả năng cấu hình

Thời gian tự bật lại bảo vệ phải được cấu hình qua Cloud Variable, không hardcode.

**Hardcode** nghĩa là viết cố định giá trị trong code, khiến sau này khó chỉnh.

### NFR-05. An toàn khi reset

Reset báo động không được xóa log sự kiện hoặc bộ đếm sự kiện.

### NFR-06. Nhất quán phạm vi

Báo cháy và RFID không thuộc phạm vi hiện tại nếu chưa được duyệt lại.

### NFR-07. Đơn giản cho dashboard trẻ em

Trẻ em chỉ được truy cập SOS và thông tin trạng thái đơn giản. Các điều khiển nâng cao chỉ dành cho người lớn/Admin.

### NFR-08. Tránh spam cảnh báo

Thông báo/email và chụp ảnh tự động phải có cơ chế tránh lặp liên tục trong cùng một cảnh báo hoặc trong thời gian cooldown.

### NFR-09. Khả năng demo

Hệ thống phải hỗ trợ controlled demo input để mô phỏng mất heartbeat hoặc nhiều chức năng lỗi mà không phá phần cứng thật. Controlled input không được thay thế các cảm biến thật trong DS-01, DS-02, DS-04, DS-05 và DS-06.


### NFR-10. Khả năng hiểu nội dung

Người dùng phải có thể hiểu trạng thái chính của hệ thống mà không cần biết tên biến, enum hoặc kiến trúc firmware.

### NFR-11. Hỗ trợ tiếng Việt và mã hóa

Nội dung người dùng phải dùng tiếng Việt có dấu và được xử lý bằng UTF-8 hoặc cơ chế tương thích của từng kênh. Hệ thống không được làm mất dấu, lỗi ký tự hoặc thay thế chữ tiếng Việt bằng chuỗi khó đọc.

### NFR-12. Nhất quán nội dung đa kênh

Cùng một event phải có cùng ý nghĩa cốt lõi trên Serial Monitor/CLI, Arduino Cloud, Telegram và Gmail/Google Apps Script. Sự khác nhau về độ dài hoặc bố cục không được dẫn đến mâu thuẫn trạng thái.

### NFR-13. Tính trung thực của thông báo

Thông báo không được tuyên bố một hành động đã thành công khi hệ thống chưa nhận được kết quả thành công. Ví dụ, không được ghi `Đã gửi ảnh` nếu Telegram upload thất bại hoặc chưa hoàn tất.

### NFR-14. Khả năng suy giảm có kiểm soát

Lỗi camera, Telegram hoặc một cảm biến không được làm firmware dừng toàn bộ nếu các chức năng cốt lõi khác vẫn có thể tiếp tục.

### NFR-15. Độ tin cậy heartbeat

Heartbeat timeout phải có khoảng đệm đủ lớn so với chu kỳ gửi để tránh báo offline do một request lỗi. Cấu hình v0.8.0 là heartbeat 10 giây và timeout 40 giây; Apps Script phân biệt offline thông thường với critical compromise sau sabotage.

### NFR-16. Ổn định kết nối và đồng bộ Cloud

- Firmware phải đọc cảm biến nội bộ mỗi 500 ms để không làm chậm cảnh báo cục bộ.
- Firmware chỉ publish snapshot cảm biến lên Cloud mỗi 3 giây; thay đổi Boolean, event an ninh, trạng thái cảnh báo và lỗi quan trọng phải publish ngay.
- Khi WiFi mất kết nối, firmware phải retry theo backoff 2 → 5 → 10 → 30 giây và quay lại 2 giây sau khi kết nối thành công.
- Mỗi request HTTPS phải có timeout cứng: 5 giây cho Telegram chữ/Apps Script, 10 giây cho upload ảnh, 8 giây cho Gemini. Khi hết hạn, client phải được đóng, lỗi được ghi nhận và request chỉ được retry theo backoff.
- Không được chạy đồng thời hai request nặng (upload ảnh, Gemini hoặc Apps Script). Trong lúc chờ I/O, firmware vẫn phải duy trì cơ hội gọi `ArduinoCloud.update()` và xử lý cảm biến.

### NFR-17. Khả năng truy vết event

Mỗi sabotage, SOS và critical compromise phải có event ID để chống xử lý trùng và đối chiếu firmware với Apps Script.

---

## 10. Ràng Buộc

### CON-01. Ràng buộc biến Arduino Cloud

Tên biến Cloud phải khớp chính xác với SRS này, trừ khi có thay đổi được duyệt và cập nhật phiên bản.

### CON-02. Ràng buộc biến legacy re-arm

`rearm_delay_seconds` và `rearm_countdown_remaining` không thuộc nghiệp vụ v0.8.0. Chúng phải bị xóa khỏi Thing, Dashboard, firmware và không được tác động `system_armed`.

### CON-03. Ràng buộc module đã loại bỏ

Hệ thống hiện tại không yêu cầu Flame Sensor, cảm biến nhiệt cho báo cháy hoặc RFID-RC522.

### CON-04. Ràng buộc khu vực giám sát

Phiên bản hiện tại có thể chỉ hỗ trợ một khu vực, nhưng yêu cầu phải tương thích với hướng mở rộng nhiều khu vực sau này.

### CON-05. Ràng buộc độ nhạy

`sensitivity_level` chỉ được nhận giá trị 1, 2 hoặc 3. Nếu không hợp lệ, hệ thống dùng mặc định 2.

### CON-06. Ràng buộc lịch tự động

Giờ phải nằm trong khoảng 0–23 và phút phải nằm trong khoảng 0–59. Nếu sai, hệ thống bỏ qua lịch và ghi lỗi.

### CON-07. Ràng buộc nguồn thời gian

Lịch tự động dùng DS1307 RTC làm nguồn thời gian chính trong phiên bản hiện tại, không phụ thuộc Internet/Cloud time.

### CON-08. Ràng buộc BLE/WiFi scanning

Không triển khai BLE trusted phone hoặc quét WiFi/MAC trong v0.7.0. WiFi chỉ dùng để kết nối dịch vụ.

### CON-09. Ràng buộc chụp ảnh tự động

Mỗi lần `intrusion_alert` mới chỉ được tự động chụp 1 ảnh. Việc chụp thêm trong cùng cảnh báo chỉ xảy ra khi Parent/Admin yêu cầu thủ công.

### CON-10. Ràng buộc board chính

Checkpoint hiện tại dùng `Freenove ESP32-S3 WROOM + Camera OV3660` làm board chính. Khi triển khai code, sơ đồ chân, cấu hình Arduino IDE hoặc cấu hình upload, nhóm phát triển phải dùng đúng board profile, pinout và cấu hình camera tương ứng với module mới. Không được dùng mặc định cấu hình của ESP32-CAM/AI Thinker cũ hoặc cấu hình của board ESP32-S3 CAM OV2640 trước đó.


### CON-11. Ràng buộc mã trạng thái nội bộ

Không đổi tên biến, enum, event ID hoặc giao thức response chỉ để dịch giao diện, trừ khi thay đổi đó được rà soát đầy đủ và không phá vỡ code, polling hoặc test. Ưu tiên dùng lớp ánh xạ nội dung hiển thị.

### CON-12. Ràng buộc thông tin nhạy cảm

Nội dung Telegram, Dashboard và Serial Monitor không được làm lộ token, mật khẩu WiFi, API key hoặc dữ liệu cấu hình nhạy cảm. Địa chỉ nhà chỉ xuất hiện trong luồng SOS hoặc critical-compromise escalation đã được Parent/Admin xác nhận.

### CON-13. Ràng buộc thay đổi phạm vi

Checkpoint 0.8.0 khóa phạm vi 6 kịch bản. Không được tự thêm lại BLE/WiFi scanning, nhận diện khuôn mặt, phần cứng mới hoặc escalation tự động không xác nhận.

### CON-14. Ràng buộc heartbeat

Chu kỳ heartbeat demo là 10 giây. Timeout Apps Script là 40 giây và trigger kiểm tra mỗi 1 phút; phải kiểm thử lại cả offline thông thường lẫn DS-03 sau khi thay đổi các giá trị này.

### CON-15. Ràng buộc tính trung thực critical compromise

Hệ thống chỉ được báo “mất liên lạc/suy giảm sau phá hoại”, không được khẳng định module cụ thể hỏng hoặc kẻ trộm cắt điện nếu không có bằng chứng phần cứng riêng.

## 11. Giả Định Và Phụ Thuộc

### 11.1 Giả định

1. Phòng bếp là khu vực giám sát duy nhất.
2. Parent/Admin có thể truy cập Arduino Cloud và email.
3. Child chỉ dùng nút SOS và xem trạng thái đơn giản.
4. DS1307 RTC đã được đặt giờ đúng.
5. WiFi hoạt động cho Cloud, Telegram và Apps Script, nhưng logic báo động cục bộ vẫn chạy khi mất mạng.
6. Apps Script có trigger định kỳ hoặc cơ chế kiểm tra heartbeat timeout.
7. Contact cơ quan chức năng trong demo là contact mô phỏng.
8. Địa chỉ nhà và token được cấu hình bảo mật.
9. Controlled demo input được phép dùng cho phần mất heartbeat/suy giảm, không phá phần cứng thật.
10. Board chính là `Freenove ESP32-S3 WROOM + Camera OV3660`.

### 11.2 Phụ thuộc

| Phụ thuộc | Vai trò |
|---|---|
| Arduino Cloud | Đồng bộ biến và dashboard |
| WiFi | Kết nối Cloud, Telegram và Apps Script |
| Telegram Bot | Cảnh báo nhanh và ảnh |
| Google Apps Script | Heartbeat, event store, email xác nhận, polling và escalation |
| Apps Script Trigger | Kiểm tra heartbeat timeout khi board không còn gửi request |
| Cảm biến PIR/HY-SRF05/LDR | Phát hiện đột nhập và sabotage |
| DS1307 RTC | Lịch và thời gian event |
| Camera OV3660 | Chụp ảnh tự động/thủ công |
| Family email recipients | Xác nhận SOS và critical compromise |
| Contact mô phỏng | Nhận báo cáo sau xác nhận |

## 12. Ngoài Phạm Vi

1. BLE trusted phone, BLE advertiser và active BLE scan.
2. Quét WiFi/MAC lạ hoặc MAC whitelist.
3. Auto-disarm/re-arm theo điện thoại.
4. Báo cháy, Flame Sensor và cảm biến nhiệt.
5. RFID-RC522.
6. Nhận diện khuôn mặt hoặc AI vision.
7. Tự xác định chắc chắn module vật lý đã hỏng nếu không có feedback sensor riêng.
8. Pin dự phòng hoặc phần cứng phát hiện mất điện riêng.
9. Gửi tự động đến cơ quan chức năng thật khi chưa có phê duyệt và xác nhận.
10. Điều khiển khóa cửa.
11. Streaming video liên tục.
12. Database lịch sử dài hạn.
13. Nhiều khu vực giám sát.
14. Mobile app riêng.
15. Retry queue lưu bền qua reboot hoặc database lịch sử dài hạn khi mạng phục hồi.

## 13. Vấn Đề Cần Phân Rã Tiếp

Các yêu cầu nghiệp vụ đã đủ để bắt đầu triển khai. Không mở rộng thêm phạm vi trước deadline.

Các việc triển khai còn lại:

| Ưu tiên | Việc | Kết quả cần đạt |
|---:|---|---|
| 1 | Làm sạch Arduino Cloud | Xóa property/widget BLE, WiFi/MAC và re-arm legacy; tạo 7 công tắc dịch vụ |
| 2 | Cập nhật firmware | Service gates, publish cảm biến 3 giây, reconnect backoff, timeout HTTPS và serialization request |
| 3 | Cập nhật Google Apps Script | Heartbeat 10 giây/timeout 40 giây, email offline/recovered một lần, critical sau sabotage và chống gửi trùng |
| 4 | Cập nhật dashboard | Thêm nhóm Services và trạng thái kết nối; bỏ toàn bộ widget legacy |
| 5 | Chuẩn hóa thông báo tiếng Việt | Đồng nhất Serial, Cloud, Telegram và Gmail |
| 6 | Chạy acceptance tests | Chạy đủ 6 kịch bản, offline thường, recovery và test âm |

Chi tiết API heartbeat và event payload có thể được tách sang tài liệu kỹ thuật riêng, nhưng phải tuân theo FR-19 đến FR-21.

## 14. Phụ Lục A — Checkpoint Quyết Định Hiện Tại

### A.1 Các quyết định đã chốt

| Chủ đề | Quyết định v0.8.0 |
|---|---|
| Board | Freenove ESP32-S3 WROOM + OV3660 |
| Khu vực | Phòng bếp |
| BLE trusted phone | Loại khỏi phạm vi |
| Quét WiFi/MAC | Loại khỏi phạm vi |
| `alarm_enabled` | Chỉ điều khiển phát hiện đột nhập |
| `system_armed` | `alarm_enabled` và trạng thái cảnh báo quyết định; không phụ thuộc điện thoại |
| Intrusion score | PIR +2, object-near +2, light abnormal +1, night mode +1 |
| Sabotage | DS-04, phát hiện hành vi che/can thiệp |
| Critical compromise | DS-03, xảy ra sau sabotage khi mất heartbeat hoặc suy giảm nghiêm trọng |
| Heartbeat | Board gửi 10 giây/lần; timeout Apps Script là 40 giây, trigger kiểm tra mỗi 1 phút |
| Dịch vụ | Dashboard điều khiển độc lập Telegram, ảnh Telegram, Apps Script, heartbeat, camera, Gemini và auto photo |
| Kết nối | Cloud publish 3 giây/lần, WiFi backoff 2→5→10→30 giây, HTTPS timeout và một request nặng tại một thời điểm |
| SOS | Luôn hoạt động; ACK không reset |
| Critical escalation | Có xác nhận Parent/Admin; event riêng, không giả thành SOS |
| Contact | Contact mô phỏng trong demo |
| Thông báo | Tiếng Việt có dấu, trung tính, không kết luận quá mức |
| Demo order | DS-01 → DS-02 → DS-04 → DS-03 → DS-05 → DS-06 |

### A.2 Logic trạng thái lõi

```text
Nếu SOS active:
    alarm_status = SOS_ALERT

Ngược lại nếu critical_security_compromise:
    alarm_status = CRITICAL_SECURITY_COMPROMISE

Ngược lại nếu sabotage_alert:
    alarm_status = SABOTAGE_ALERT

Ngược lại nếu intrusion_alert:
    alarm_status = INTRUSION_ALERT

Ngược lại nếu alarm_enabled:
    system_armed = true
    alarm_status = ARMED

Ngược lại:
    system_armed = false
    alarm_status = DISARMED
```

### A.3 Logic lịch RTC

```text
schedule_enabled = false:
    không đổi alarm_enabled

đến auto_arm:
    alarm_enabled = true
    system_armed = true nếu không có cảnh báo active

đến auto_disarm:
    alarm_enabled = false
    system_armed = false

lịch không thay đổi trực tiếp trạng thái SOS, sabotage hoặc critical compromise
```

### A.4 Logic đột nhập

```text
intrusion_score = PIR*2 + OBJECT_NEAR*2 + LIGHT_ABNORMAL*1 + NIGHT_MODE*1

sensitivity_level:
    1 -> threshold 5
    2 -> threshold 4
    3 -> threshold 3

đạt ngưỡng:
    latch intrusion_alert
    bật còi/LED
    chụp 1 ảnh
    gửi Telegram
```

### A.5 Logic DS-04 → DS-03

```text
DS-04:
    phát hiện LDR bị che và vật áp sát <= 15 cm liên tục 3 giây
    latch sabotage_alert
    gửi Telegram
    gửi sabotage event đến Apps Script
    bắt đầu theo dõi heartbeat/health check

DS-03:
    sabotage đã xảy ra
    AND heartbeat timeout hoặc lỗi nghiêm trọng kéo dài
    => tạo CRITICAL_SECURITY_COMPROMISE
    => email xin Parent/Admin xác nhận
    => xác nhận rồi gửi contact mô phỏng
```

### A.6 Logic heartbeat phía Cloud

```text
ESP32 -> heartbeat mỗi 10 giây
Apps Script lưu lastHeartbeatAt

nếu quá timeout và không có sabotage:
    DEVICE_OFFLINE
    không escalation

nếu quá timeout sau sabotage:
    CRITICAL_SECURITY_COMPROMISE
    gửi email xác nhận một lần
```

### A.7 Logic escalation

```text
SOS và CRITICAL là hai event type riêng
mỗi event có eventId riêng
Parent/Admin xác nhận
Apps Script chống xử lý trùng
gửi đến contact mô phỏng
ACK không reset còi/LED tại board
```

### A.8 Sáu kịch bản demo

```text
DS-01: Tự bật chống trộm theo lịch RTC
DS-02: Đột nhập ban đêm đa cảm biến + camera + Telegram
DS-04: Phát hiện đang che/can thiệp thiết bị
DS-03: Mất liên lạc/suy giảm sau phá hoại + escalation
DS-05: Chụp ảnh thủ công từ xa
DS-06: Child SOS + Parent confirmation + escalation
```

### A.9 Nội dung legacy

Các yêu cầu BLE trusted phone, unknown WiFi/MAC và auto re-arm trong v0.6.x chỉ còn giá trị lịch sử. Không dùng chúng để đánh giá code v0.8.0.

### A.10 Chuẩn thông báo critical compromise

```text
CẢNH BÁO AN NINH NGHIÊM TRỌNG

Trước đó hệ thống đã phát hiện hành vi che hoặc can thiệp thiết bị.
Ngay sau sự kiện, thiết bị đã mất liên lạc hoặc mất một phần khả năng giám sát.

Nguyên nhân chưa được xác định chắc chắn. Thiết bị có thể đã mất nguồn,
mất kết nối, bị treo hoặc bị vô hiệu hóa.

Vui lòng kiểm tra tình hình và xác nhận có gửi báo cáo khẩn cấp
đến contact tiếp nhận mô phỏng hay không.
```

---

## Kết thúc bản SRS checkpoint 0.7.0
