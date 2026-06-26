# Đặc Tả Yêu Cầu Phần Mềm (SRS)

# Hệ Thống Chống Trộm IoT

> **Trạng thái tài liệu:** Bản checkpoint cập nhật 0.3.0  
> **Mục đích:** Đây là tài liệu nguồn hiện tại cho agent, chatbot, lập trình viên và người review. Các tài liệu cũ chỉ dùng để tham khảo lịch sử. Nếu có mâu thuẫn, tài liệu `SRS.md` này được ưu tiên.

---

## Lịch Sử Phiên Bản

| Phiên bản | Ngày tạo | Sửa đổi gần nhất | Người phụ trách | Tóm tắt thay đổi |
|---|---|---|---|---|
| 0.1.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Tạo bản SRS checkpoint đầu tiên sau khi làm sạch phạm vi và phân rã cụm Security Mode Control. Loại bỏ báo cháy và RFID-RC522 khỏi phạm vi chính. Thêm quy tắc tự tắt chống trộm khi phát hiện thiết bị đáng tin và cấu hình thời gian tự bật lại bảo vệ. |
| 0.2.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Cập nhật checkpoint sau khi phân rã thêm: điều khiển chống trộm theo lịch, phát hiện đột nhập bằng `intrusion_score`, cấu hình độ nhạy bằng `sensitivity_level`, bật/tắt phát hiện WiFi/MAC lạ, WiFi/MAC lạ chỉ là pre-warning, giữ điểm nghi ngờ 5 giây, kích hoạt cảnh báo đột nhập đầy đủ, gửi thông báo khi có đột nhập, tự chụp 1 ảnh cho mỗi lần cảnh báo đột nhập mới và cho phép chụp thủ công thêm. |
| 0.3.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Cập nhật checkpoint sau khi phân rã `threat_level`, lọc báo động giả/vật nuôi, Telegram notification và chống spam WiFi/MAC lạ. Đổi nhóm biến `email_*` sang `notification_*`, chọn Telegram là kênh mặc định, bổ sung rule WiFi/MAC lạ lặp lại cùng MAC 3 lần trong 60 giây và cooldown 60 giây. |

---

## Thông Tin Kiểm Soát Tài Liệu

| Mục | Giá trị |
|---|---|
| Tên dự án | Hệ Thống Chống Trộm IoT |
| Tên tiếng Anh | IoT Based Anti-Theft System |
| Use Case chính | Giám sát chống trộm thông minh cho khu vực phòng bếp |
| Khu vực giám sát hiện tại | Phòng bếp |
| Hướng mở rộng | Nhiều module theo khu vực như bếp, phòng ngủ, cửa chính, phòng khách |
| Loại tài liệu | SRS.md |
| Cách viết | Theo cấu trúc kiểu IEEE, tối ưu để agent và người bảo trì đọc |
| Ngôn ngữ | Tiếng Việt, giữ nguyên mã biến/ID kỹ thuật bằng tiếng Anh |
| Trạng thái hiện tại | Bản checkpoint 0.3.0; đã phân rã thêm Threat Level, False Positive / Pet Detection, Telegram Notification và Unknown WiFi Anti-Spam ở mức yêu cầu chính |

---

## Quy Tắc Cho Agent Khi Đọc Tài Liệu

Phần này dành riêng cho AI agent, chatbot và người bảo trì sau này.

1. `SRS.md` là nguồn yêu cầu chính ở checkpoint hiện tại.
2. Không được sao chép máy móc tài liệu cũ vào SRS.
3. Khi cập nhật SRS, phải giữ và cập nhật bảng lịch sử phiên bản.
4. Mỗi lần sửa đổi cần cập nhật:
   - số phiên bản,
   - ngày sửa đổi gần nhất,
   - tóm tắt thay đổi.
5. Nếu một chức năng chưa được phân rã đầy đủ, phải đánh dấu `Chờ phân rã`.
6. Không được tự ý thêm lại module đã loại bỏ nếu chưa có xác nhận rõ ràng từ chủ dự án.
7. Không được hiểu `alarm_enabled` là công tắc tắt/bật toàn bộ hệ thống. Biến này chỉ điều khiển chức năng chống trộm.
8. Không được làm `SOS` hoặc `anti-sabotage` phụ thuộc vào `system_armed`.
9. Không được để lịch tự động thay đổi trực tiếp `system_armed`. Lịch chỉ được thay đổi `alarm_enabled`.
10. Không được để WiFi/MAC lạ tự kích hoạt báo động đột nhập độc lập.
11. Không được tự động tắt cảnh báo đột nhập chỉ vì `intrusion_score` giảm xuống dưới ngưỡng. Cảnh báo chỉ tắt bằng `reset_alarm`.
12. Không được hiểu `pet_detected` là xác nhận chắc chắn có thú nuôi; đây chỉ là nghi ngờ vật nhỏ/báo nhầm.
13. Không được dùng nhóm biến `email_*` làm tên ưu tiên trong SRS mới; phải dùng nhóm `notification_*`.
14. Không được gửi Telegram cho mọi sự kiện; Telegram chỉ dùng cho cảnh báo quan trọng và pre-warning đã được chốt.
15. Phiên bản hiện tại thiết kế cho một khu vực giám sát: phòng bếp.
16. Nội dung nên đủ rõ để triển khai, nhưng không biến thành mã nguồn thô.

---

# 1. Giới Thiệu

## 1.1 Mục đích

Tài liệu này mô tả các yêu cầu phần mềm cho hệ thống chống trộm IoT. Hệ thống được thiết kế để giám sát khu vực phòng bếp bằng cảm biến, Arduino Cloud Dashboard, cơ chế cảnh báo, còi/LED tại chỗ và ghi nhận sự kiện.

Mục đích của SRS là tạo ra một tài liệu chuyên nghiệp, dễ bảo trì và dễ đọc cho agent. Đây không phải bản chuyển đổi đơn thuần từ tài liệu cũ. Đây là bản đặc tả yêu cầu đã được thiết kế lại theo các quyết định hiện tại.

## 1.2 Phạm vi

Hệ thống tập trung vào chức năng chống trộm và giám sát an ninh.

Phiên bản hiện tại bao gồm một khu vực giám sát:

```text
Khu vực phòng bếp
```

Tuy nhiên, tài liệu cố ý dùng khái niệm:

```text
monitored zone
```

nghĩa là **khu vực giám sát**, để sau này có thể mở rộng sang nhiều module:

```text
Module bếp
Module phòng ngủ
Module cửa chính
Module phòng khách
```

## 1.3 Góc nhìn sản phẩm

Sản phẩm là một hệ thống an ninh IoT nhúng, bao gồm:

- module vi điều khiển,
- cảm biến chuyển động/khoảng cách/ánh sáng,
- biến Arduino Cloud,
- dashboard theo vai trò người dùng,
- đầu ra cảnh báo như còi và LED,
- cơ chế gửi thông báo,
- ghi log sự kiện,
- định hướng kiến trúc mở rộng nhiều khu vực.

## 1.4 Thuật ngữ

| Thuật ngữ | Ý nghĩa |
|---|---|
| SRS | Software Requirements Specification, tài liệu đặc tả yêu cầu phần mềm |
| IoT | Internet of Things, hệ thống thiết bị kết nối Internet |
| Monitored Zone | Khu vực đang được hệ thống giám sát |
| Dashboard | Giao diện điều khiển/giám sát trên Arduino Cloud |
| Cloud Variable | Biến đồng bộ giữa board và Arduino Cloud |
| Whitelist | Danh sách thiết bị được xem là đáng tin |
| Known Device | Thiết bị đáng tin, ví dụ điện thoại người nhà |
| Unknown WiFi/MAC | Thiết bị WiFi/MAC lạ không nằm trong danh sách đáng tin |
| Pre-warning | Cảnh báo sớm, chưa phải báo động chính thức |
| Notification | Thông báo bên ngoài, có thể gửi qua Telegram Bot hoặc kênh khác |
| Telegram Bot | Kênh thông báo mặc định của hệ thống hiện tại |
| `intrusion_score` | Điểm nghi ngờ đột nhập dùng để xác nhận có báo động đột nhập hay không |
| `threat_level` | Mức nguy hiểm tổng thể hiển thị cho dashboard và ưu tiên cảnh báo |
| `pet_detected` | Hệ thống nghi tín hiệu giống thú nuôi/vật nhỏ/báo nhầm, không phải xác nhận chắc chắn là thú nuôi |
| `alarm_enabled` | Công tắc bật/tắt chức năng chống trộm |
| `system_armed` | Trạng thái chống trộm thực tế của khu vực |
| `known_device_present` | Có thiết bị đáng tin được phát hiện gần khu vực |
| Re-arm | Tự bật lại chống trộm sau khi người nhà rời khỏi khu vực |
| Anti-sabotage | Chống phá hoại/can thiệp thiết bị |
| Cooldown | Thời gian chờ để tránh gửi cảnh báo liên tục |
| Agent | AI/chatbot/dev tool đọc tài liệu để hỗ trợ code, sửa hoặc kiểm thử |

---

# 2. Mô Tả Tổng Quan

## 2.1 Các chức năng chính của sản phẩm

Hệ thống cần hỗ trợ các nhóm chức năng cấp cao sau.

| ID | Chức năng | Trạng thái thiết kế hiện tại |
|---|---|---|
| FR-01 | Điều khiển chế độ chống trộm | Đã phân rã |
| FR-02 | Xử lý reset báo động | Đã phân rã |
| FR-03 | Điều khiển chế độ chống trộm theo lịch | Đã phân rã |
| FR-04 | Phát hiện đột nhập đa cảm biến | Đã phân rã |
| FR-05 | Phát hiện thiết bị WiFi/MAC lạ dạng pre-warning | Đã phân rã ở mức rule chính |
| FR-06 | Gửi Telegram / notification khi có sự kiện an ninh | Đã phân rã ở mức rule chính |
| FR-07 | Tự chụp ảnh khi có đột nhập | Đã phân rã ở mức rule chính |
| FR-08 | Lọc báo động giả / vật nuôi | Đã phân rã ở mức rule chính |
| FR-09 | Phát hiện phá hoại thiết bị | Chờ phân rã |
| FR-10 | SOS khẩn cấp qua Dashboard | Chờ phân rã |
| FR-11 | Ghi log sự kiện dài hạn | Chờ phân rã |
| FR-12 | Chống spam cảnh báo / cooldown | Đã phân rã một phần cho Unknown WiFi/Telegram |
| FR-13 | Chế độ demo | Chờ phân rã |
| FR-14 | Hỗ trợ Arduino Cloud Dashboard chi tiết | Chờ phân rã |

## 2.2 Nhóm người dùng

| Vai trò | Mô tả | Quyền chính |
|---|---|---|
| Trẻ em | Người dùng chỉ cần thao tác SOS đơn giản | Bấm SOS, xem trạng thái cơ bản |
| Người lớn / Phụ huynh | Người dùng chính trong nhà | Bật/tắt chống trộm, reset báo động, bấm SOS, cấu hình thời gian tự bật lại, xem cảnh báo |
| Admin / Người demo | Leader, tester hoặc người thuyết trình | Quan sát toàn bộ, điều khiển demo, xem biến debug, xem countdown |
| Hệ thống / Board | Thiết bị nhúng chạy logic cảm biến | Đọc cảm biến, cập nhật biến, kích hoạt cảnh báo, ghi log |
| Dịch vụ thông báo bên ngoài | Telegram Bot hoặc dịch vụ notification | Nhận loại sự kiện và gửi thông báo |

## 2.3 Môi trường vận hành

Hệ thống dự kiến chạy trên thiết bị vi điều khiển IoT có kết nối Arduino Cloud. Phiên bản hiện tại giả định chỉ có một khu vực giám sát vật lý.

Các thành phần dự kiến gồm:

| Thành phần | Vai trò |
|---|---|
| ESP32-CAM hoặc board tương đương | Bộ điều khiển chính và camera |
| PIR sensor | Phát hiện chuyển động |
| HC-SR04 ultrasonic sensor | Đo khoảng cách vật thể |
| LDR light sensor | Phát hiện ánh sáng bất thường hoặc hành vi che cảm biến |
| DS1307 RTC | Cung cấp thời gian cho log và logic theo thời gian |
| Buzzer | Cảnh báo âm thanh tại chỗ |
| LED đỏ | Cảnh báo hình ảnh tại chỗ |
| Arduino Cloud | Đồng bộ biến và dashboard |
| Google Apps Script hoặc dịch vụ tương đương | Gửi notification/Telegram |

## 2.4 Nội dung đã loại khỏi phạm vi hiện tại

| Nội dung bị loại | Lý do |
|---|---|
| Báo cháy | Không đúng trọng tâm chống trộm |
| Flame Sensor | Gắn với chức năng báo cháy đã loại bỏ |
| Đo nhiệt độ bếp | Gắn với chức năng báo cháy đã loại bỏ |
| RFID-RC522 | Không đủ giá trị trong phạm vi hiện tại, trùng vai trò với điều khiển qua dashboard/điện thoại |
| Trạng thái truy cập RFID | Bị loại cùng RFID-RC522 |

Agent không được tự thêm lại các nội dung này nếu chưa có yêu cầu rõ ràng.

---

# 3. Nguyên Tắc Thiết Kế Hệ Thống

## 3.1 Phiên bản hiện tại một khu vực, tương lai nhiều khu vực

Phiên bản hiện tại xử lý phòng bếp là khu vực giám sát duy nhất.

Tuy nhiên, yêu cầu nên dùng cách diễn đạt:

```text
khu vực giám sát
```

thay vì hardcode mọi luật vào riêng phòng bếp. Điều này giúp SRS dễ mở rộng sau này.

## 3.2 Chế độ chống trộm khác với bật/tắt toàn hệ thống

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

## 3.3 `system_armed` là trạng thái kết quả

`system_armed` là trạng thái chống trộm thực tế do hệ thống tính toán. Người dùng, lịch tự động và dashboard không được ép trực tiếp `system_armed`.

Các đầu vào như `alarm_enabled`, `known_device_present`, `rearm_delay_seconds` và lịch tự động sẽ làm hệ thống tính ra `system_armed`.

## 3.4 Sự hiện diện của thiết bị đáng tin

Hệ thống dùng phát hiện thiết bị đáng tin để quyết định chức năng chống trộm của khu vực có nên hoạt động hay không.

Nếu thiết bị đáng tin xuất hiện, hệ thống không báo đột nhập cho khu vực đó.

Chức năng này tồn tại để giảm cảnh báo thừa khi người nhà ở nhà. Nó không nhằm mục đích chính là tăng độ chính xác phát hiện trộm.

## 3.5 WiFi/MAC lạ chỉ là tín hiệu cảnh báo sớm

WiFi/MAC lạ không được tự kích hoạt báo động đột nhập. Nó chỉ được xem là pre-warning và chỉ được góp điểm vào `intrusion_score` khi có ít nhất một tín hiệu cảm biến vật lý đáng nghi đi kèm.

---

# 4. Yêu Cầu Giao Diện Bên Ngoài

## 4.1 Yêu cầu giao diện người dùng

Hệ thống cần có ba loại dashboard.

| Dashboard | Người dùng | Mục đích chính | Biến chính |
|---|---|---|---|
| Child SOS Dashboard | Trẻ em | Gửi SOS đơn giản và xem trạng thái cơ bản | `sos_child`, `alarm_status`, `last_event` |
| Parent Control Dashboard | Người lớn | Điều khiển chống trộm, reset, SOS, cấu hình re-arm, xem cảnh báo quan trọng | `alarm_enabled`, `sos_adult`, `reset_alarm`, `alarm_status`, `last_event`, `threat_level`, `notification_sent_status`, `capture_photo`, `photo_status`, `rearm_delay_seconds` |
| Admin Demo Dashboard | Admin / Người demo | Quan sát toàn bộ, kiểm thử, debug, chọn kịch bản demo | toàn bộ biến cần thiết, gồm `rearm_countdown_remaining`, `demo_mode`, `demo_scenario`, biến cảm biến, WiFi, log |

Chi tiết bố cục dashboard có thể nằm ở tài liệu thiết kế dashboard riêng. SRS chỉ định nghĩa yêu cầu ở mức vai trò, widget chính và biến liên quan.

## 4.2 Yêu cầu giao tiếp phần cứng

| Phần cứng | Yêu cầu |
|---|---|
| PIR sensor | Hệ thống phải đọc trạng thái chuyển động từ PIR. |
| HC-SR04 ultrasonic sensor | Hệ thống phải đọc khoảng cách để hỗ trợ phát hiện vật thể và lọc báo động giả. |
| LDR sensor | Hệ thống phải đọc mức ánh sáng và độ thay đổi ánh sáng để phát hiện ánh sáng bất thường hoặc che cảm biến. |
| DS1307 RTC | Hệ thống phải dùng thời gian RTC cho log sự kiện và logic theo thời gian nếu cần. |
| Buzzer | Hệ thống phải bật còi cho các cảnh báo đang hoạt động theo luật sự kiện. |
| LED đỏ | Hệ thống phải bật LED đỏ cho các cảnh báo đang hoạt động theo luật sự kiện. |
| ESP32-CAM camera | Hệ thống phải hỗ trợ chụp ảnh cho sự kiện an ninh được chọn hoặc yêu cầu thủ công. |

## 4.3 Yêu cầu giao tiếp phần mềm

| Dịch vụ bên ngoài | Yêu cầu |
|---|---|
| Arduino Cloud | Hệ thống phải đồng bộ biến Cloud giữa thiết bị và dashboard. |
| Dịch vụ notification/Telegram | Hệ thống phải gửi thông báo cho các loại sự kiện được cấu hình. |
| Ứng dụng Dashboard | Hệ thống phải cho phép tương tác theo vai trò qua widget dashboard. |

## 4.4 Yêu cầu giao tiếp truyền thông

Hệ thống dùng WiFi cho:

- đồng bộ Arduino Cloud,
- điều khiển dashboard,
- gửi thông báo Cloud,
- quét thiết bị đáng tin/thiết bị lạ nếu chức năng này được triển khai.

Hệ thống cần xử lý việc WiFi mất ổn định tạm thời mà không xóa trạng thái sự kiện cục bộ.

---

# 5. Yêu Cầu Biến Cloud

## 5.1 Mô hình biến Cloud

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

## 5.2 Quy tắc Permission

| Permission | Ý nghĩa |
|---|---|
| Read & Write | Dashboard đọc được biến và gửi lệnh/cấu hình xuống thiết bị |
| Read Only | Dashboard chỉ xem; board là nguồn cập nhật chính |

## 5.3 Quy tắc Update Policy

| Update Policy | Ý nghĩa |
|---|---|
| On Change | Chỉ gửi cập nhật khi giá trị thay đổi |
| Periodic | Gửi cập nhật theo chu kỳ đã cấu hình |

## 5.4 Danh sách biến Cloud hiện tại

> Ghi chú: Danh sách này phản ánh checkpoint hiện tại sau khi bỏ báo cháy, bỏ RFID-RC522 và thêm biến cấu hình re-arm.

| Nhóm | Biến | Kiểu dữ liệu | Permission | Update Policy | Dashboard | Mục đích |
|---|---|---|---|---|---|---|
| Điều khiển bảo vệ | `alarm_enabled` | bool | Read & Write | On Change | Parent, Admin | Mong muốn bật/tắt chức năng chống trộm từ người dùng |
| Điều khiển bảo vệ | `system_armed` | bool | Read Only | On Change | Parent, Admin | Trạng thái chống trộm thực tế của khu vực |
| Điều khiển bảo vệ | `reset_alarm` | bool | Read & Write | On Change | Parent, Admin | Yêu cầu dừng và xóa cảnh báo hiện tại |
| Điều khiển bảo vệ | `alarm_status` | String | Read Only | On Change | Child, Parent, Admin | Trạng thái hệ thống dạng chữ |
| Điều khiển bảo vệ | `rearm_delay_seconds` | int | Read & Write | On Change | Parent, Admin | Thời gian chờ trước khi tự bật lại chống trộm |
| Điều khiển bảo vệ | `rearm_countdown_remaining` | int | Read Only | On Change | Admin | Số giây còn lại trước khi tự bật lại chống trộm |
| Cấu hình lịch bảo vệ | `schedule_enabled` | bool | Read & Write | On Change | Parent, Admin | Bật/tắt chức năng tự động bật/tắt chống trộm theo lịch |
| Cấu hình lịch bảo vệ | `auto_arm_hour` | int | Read & Write | On Change | Parent, Admin | Giờ tự bật chống trộm mỗi ngày |
| Cấu hình lịch bảo vệ | `auto_arm_minute` | int | Read & Write | On Change | Parent, Admin | Phút tự bật chống trộm mỗi ngày |
| Cấu hình lịch bảo vệ | `auto_disarm_hour` | int | Read & Write | On Change | Parent, Admin | Giờ tự tắt chống trộm mỗi ngày |
| Cấu hình lịch bảo vệ | `auto_disarm_minute` | int | Read & Write | On Change | Parent, Admin | Phút tự tắt chống trộm mỗi ngày |
| Cấu hình phát hiện | `sensitivity_level` | int | Read & Write | On Change | Parent, Admin | Mức nhạy phát hiện đột nhập, giá trị hợp lệ 1–3 |
| SOS | `sos_child` | bool | Read & Write | On Change | Child, Admin | Nút SOS của trẻ em |
| SOS | `sos_adult` | bool | Read & Write | On Change | Parent, Admin | Nút SOS của người lớn |
| SOS | `sos_level` | int | Read Only | On Change | Admin | Mức độ nghiêm trọng của SOS |
| SOS | `sos_message` | String | Read Only | On Change | Parent, Admin | Nội dung SOS do hệ thống tạo |
| Chuyển động / Khoảng cách | `pir_detected` | bool | Read Only | On Change | Admin | Trạng thái phát hiện chuyển động từ PIR |
| Chuyển động / Khoảng cách | `ultrasonic_distance` | float | Read Only | Periodic | Admin | Khoảng cách đo từ cảm biến siêu âm |
| Chuyển động / Khoảng cách | `object_near` | bool | Read Only | On Change | Admin | Có vật thể trong vùng theo dõi hay không |
| Chuyển động / Khoảng cách | `pet_detected` | bool | Read Only | On Change | Admin | Hệ thống nghi tín hiệu giống thú nuôi/vật nhỏ/báo nhầm |
| Ánh sáng / LDR | `ldr_value` | int | Read Only | Periodic | Admin | Giá trị ánh sáng thô từ LDR |
| Ánh sáng / LDR | `ldr_delta` | int | Read Only | Periodic | Admin | Độ thay đổi ánh sáng |
| Ánh sáng / LDR | `light_abnormal` | bool | Read Only | On Change | Admin | Ánh sáng thay đổi bất thường |
| Ánh sáng / LDR | `ldr_covered` | bool | Read Only | On Change | Admin | LDR có dấu hiệu bị che |
| Đột nhập | `intrusion_alert` | bool | Read Only | On Change | Parent, Admin | Cảnh báo đột nhập đã được xác nhận |
| Đột nhập | `intrusion_score` | int | Read Only | On Change | Admin | Điểm nghi ngờ đột nhập |
| Đột nhập | `threat_level` | int | Read Only | On Change | Parent, Admin | Mức nguy hiểm tổng thể |
| Chống phá hoại | `sabotage_alert` | bool | Read Only | On Change | Parent, Admin | Cảnh báo phá hoại/can thiệp thiết bị |
| Chống phá hoại | `device_tampered` | bool | Read Only | On Change | Admin | Thiết bị có dấu hiệu bị can thiệp vật lý |
| Thời gian | `current_time` | String | Read Only | Periodic | Parent, Admin | Thời gian hiện tại |
| Thời gian | `current_hour` | int | Read Only | Periodic | Admin | Giờ hiện tại dạng số |
| Thời gian | `night_mode` | bool | Read Only | On Change | Admin | Hệ thống đang ở chế độ ban đêm hay không |
| WiFi đáng tin / WiFi lạ | `known_device_present` | bool | Read Only | On Change | Parent, Admin | Có thiết bị đáng tin gần khu vực hay không |
| WiFi đáng tin / WiFi lạ | `unknown_wifi_detection_enabled` | bool | Read & Write | On Change | Parent, Admin | Bật/tắt phát hiện thiết bị WiFi/MAC lạ |
| WiFi đáng tin / WiFi lạ | `unknown_wifi_count` | int | Read Only | Periodic | Admin | Số lượng/lần phát hiện thiết bị WiFi/MAC lạ |
| WiFi đáng tin / WiFi lạ | `unknown_wifi_alert` | bool | Read Only | On Change | Parent, Admin | Tín hiệu pre-warning khi có WiFi/MAC lạ đáng chú ý |
| WiFi đáng tin / WiFi lạ | `last_unknown_mac` | String | Read Only | On Change | Admin | MAC lạ gần nhất phát hiện được |
| Camera | `capture_photo` | bool | Read & Write | On Change | Parent, Admin | Yêu cầu chụp ảnh thủ công hoặc theo sự kiện |
| Camera | `photo_status` | String | Read Only | On Change | Parent, Admin | Trạng thái thao tác chụp ảnh |
| Đầu ra cảnh báo | `buzzer_on` | bool | Read Only | On Change | Parent, Admin | Trạng thái còi |
| Đầu ra cảnh báo | `led_red_on` | bool | Read Only | On Change | Parent, Admin | Trạng thái LED đỏ |
| Thông báo | `send_notification_request` | bool | Read Only | On Change | Admin | Hệ thống yêu cầu gửi notification ra ngoài |
| Thông báo | `notification_event_type` | String | Read Only | On Change | Admin | Loại sự kiện dùng cho nội dung notification |
| Thông báo | `notification_sent_status` | String | Read Only | On Change | Parent, Admin | Trạng thái gửi notification qua Telegram |
| Thông báo | `notification_channel` | String | Read Only | On Change | Admin | Kênh notification hiện tại, mặc định là `telegram` |
| Log sự kiện | `last_event` | String | Read Only | On Change | Child, Parent, Admin | Mô tả sự kiện gần nhất |
| Log sự kiện | `last_event_type` | String | Read Only | On Change | Admin | Loại sự kiện gần nhất |
| Log sự kiện | `event_counter` | int | Read Only | On Change | Admin | Tổng số sự kiện có ý nghĩa |
| Chống spam | `cooldown_active` | bool | Read Only | On Change | Admin | Hệ thống đang trong thời gian cooldown hay không |
| Chống spam | `last_alert_time` | String | Read Only | On Change | Admin | Thời điểm gửi cảnh báo gần nhất |
| Demo | `demo_mode` | bool | Read & Write | On Change | Admin | Bật/tắt hành vi demo/kiểm thử |
| Demo | `demo_scenario` | int | Read & Write | On Change | Admin | Chọn kịch bản demo |

## 5.5 Biến Cloud đã loại bỏ

Các biến sau không được tạo trong phạm vi SRS hiện tại.

| Biến bị loại | Lý do |
|---|---|
| `flame_value` | Đã bỏ chức năng báo cháy |
| `flame_detected` | Đã bỏ chức năng báo cháy |
| `kitchen_temperature` | Đã bỏ chức năng báo cháy |
| `fire_alert` | Đã bỏ chức năng báo cháy |
| `rfid_uid` | Đã bỏ RFID-RC522 |
| `rfid_authorized` | Đã bỏ RFID-RC522 |
| `access_status` | Đã bỏ RFID-RC522 |
| `last_schedule_action` | Không thêm biến riêng; sự kiện lịch dùng `last_event`, `last_event_type` và sau này ghi vào Event Logging dài hạn |
| `auto_photo_captured` | Không thêm Cloud Variable; trạng thái tự chụp ảnh dùng biến nội bộ trong code |
| `send_email_request` | Đổi sang `send_notification_request` vì hệ thống dùng Telegram Bot làm kênh mặc định |
| `email_event_type` | Đổi sang `notification_event_type` |
| `email_sent_status` | Đổi sang `notification_sent_status` |

---

# 6. Yêu Cầu Trạng Thái Hệ Thống

## 6.1 Giá trị hợp lệ của `alarm_status`

Hệ thống chỉ được dùng các giá trị sau cho `alarm_status`.

| Trạng thái | Ý nghĩa |
|---|---|
| `DISARMED` | Chống trộm tắt vì `alarm_enabled = false` |
| `DISARMED_BY_TRUSTED_DEVICE` | Chống trộm tạm tắt vì phát hiện thiết bị đáng tin |
| `ARMED` | Chống trộm đang hoạt động cho khu vực giám sát |
| `INTRUSION_ALERT` | Đang có cảnh báo đột nhập |
| `SOS_ALERT` | Đang có cảnh báo SOS |
| `SABOTAGE_ALERT` | Đang có cảnh báo phá hoại/can thiệp thiết bị |
| `ALARM_RESET` | Cảnh báo hiện tại vừa được reset |
| `ERROR` | Lỗi cảm biến, WiFi, Cloud hoặc xử lý hệ thống |

Trạng thái `REARM_PENDING` không được dùng trong phiên bản hiện tại. Countdown tự bật lại bảo vệ được theo dõi bằng biến `rearm_countdown_remaining`.

---

# 7. Quy Tắc Nghiệp Vụ

## BR-01. Quy tắc tự tắt chống trộm khi có thiết bị đáng tin

Nếu `alarm_enabled = true` và có ít nhất một thiết bị đáng tin được phát hiện gần khu vực giám sát, hệ thống phải đặt `system_armed = false` cho chức năng phát hiện đột nhập.

Nếu `alarm_enabled = true` và không phát hiện thiết bị đáng tin gần khu vực trong đủ thời gian re-arm đã cấu hình, hệ thống phải đặt `system_armed = true`.

Quy tắc tự tắt này chỉ ảnh hưởng đến phát hiện đột nhập. Nó không được tắt SOS, chống phá hoại, reset, cập nhật dashboard, đồng bộ Cloud hoặc ghi log.

## BR-02. Quy tắc tự bật lại khi mất thiết bị đáng tin

Nếu `alarm_enabled = true` và không còn phát hiện thiết bị đáng tin trong khu vực giám sát, hệ thống phải bắt đầu đếm ngược theo giá trị `rearm_delay_seconds`.

Hệ thống phải hiển thị `rearm_countdown_remaining` dưới dạng biến Cloud chỉ đọc cho Admin Dashboard.

Nếu thiết bị đáng tin quay lại trước khi countdown kết thúc, hệ thống phải:

1. hủy countdown,
2. đặt `rearm_countdown_remaining = 0`,
3. giữ `system_armed = false`,
4. giữ `alarm_status = DISARMED_BY_TRUSTED_DEVICE`.

Nếu không có thiết bị đáng tin nào được phát hiện cho đến khi countdown kết thúc, hệ thống phải:

1. đặt `system_armed = true`,
2. đặt `alarm_status = ARMED`,
3. đặt `rearm_countdown_remaining = 0`.

## BR-03. Quy tắc thiết bị đáng tin quay lại khi báo động đang xảy ra

Nếu thiết bị đáng tin được phát hiện trong lúc cảnh báo đột nhập đang hoạt động, hệ thống phải ngừng tạo thêm cảnh báo đột nhập mới cho khu vực, nhưng không được tự động xóa cảnh báo hiện tại.

Cảnh báo đột nhập hiện tại, còi, LED, log sự kiện và trạng thái cảnh báo phải được giữ cho đến khi người lớn hoặc Admin kích hoạt `reset_alarm` từ dashboard hợp lệ.

## BR-04. Quy tắc reset báo động

Khi người lớn hoặc Admin kích hoạt `reset_alarm`, hệ thống phải:

1. dừng đầu ra cảnh báo hiện tại,
2. xóa trạng thái cảnh báo đang hoạt động,
3. giữ log sự kiện,
4. giữ `last_event`,
5. giữ `last_event_type`,
6. giữ `event_counter`,
7. tính lại `system_armed` dựa trên:
   - `alarm_enabled`,
   - `known_device_present`,
   - quy tắc tự bật lại khi mất thiết bị đáng tin,
8. cập nhật `alarm_status` theo trạng thái mới,
9. tự động đặt `reset_alarm = false` sau khi xử lý xong.

Reset không có nghĩa là tắt bảo vệ. Reset có nghĩa là xác nhận và xóa cảnh báo hiện tại.

## BR-05. Quy tắc cấu hình thời gian tự bật lại bảo vệ

Hệ thống phải cho phép người lớn và Admin cấu hình `rearm_delay_seconds` trong khoảng từ 5 đến 600 giây.

Nếu giá trị nhập vào nhỏ hơn 5, hệ thống phải dùng 5 giây.

Nếu giá trị nhập vào lớn hơn 600, hệ thống phải dùng 600 giây.

Trẻ em không được xem hoặc chỉnh cấu hình này.

## BR-06. Quy tắc phạm vi của `alarm_enabled`

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

## BR-07. Quy tắc độ nhạy phát hiện đột nhập

Hệ thống phải cho phép người lớn và Admin cấu hình mức nhạy bằng biến `sensitivity_level`.

| `sensitivity_level` | Ý nghĩa | Ngưỡng nội bộ |
|---:|---|---:|
| 1 | Ít nhạy | `intrusion_score >= 4` |
| 2 | Bình thường | `intrusion_score >= 3` |
| 3 | Nhạy cao | `intrusion_score >= 2` |

Nếu `sensitivity_level` nằm ngoài khoảng hợp lệ, hệ thống phải dùng giá trị mặc định là `2`.

Người dùng không được chỉnh trực tiếp `intrusion_threshold`. Ngưỡng điểm là logic nội bộ do hệ thống tự map từ `sensitivity_level`.

## BR-08. Quy tắc bật/tắt phát hiện WiFi/MAC lạ

Hệ thống phải có biến `unknown_wifi_detection_enabled` để bật/tắt chức năng phát hiện thiết bị WiFi/MAC lạ.

Nếu `unknown_wifi_detection_enabled = false`:

1. hệ thống không kích hoạt `unknown_wifi_alert`,
2. hệ thống không dùng `unknown_wifi_alert` để cộng `intrusion_score`,
3. hệ thống có thể bỏ qua `unknown_wifi_count` hoặc chỉ ghi debug cho Admin nếu cần.

Nếu `unknown_wifi_detection_enabled = true`:

1. hệ thống được phép phát hiện thiết bị WiFi/MAC lạ,
2. hệ thống được phép cập nhật `unknown_wifi_count`,
3. hệ thống được phép cập nhật `last_unknown_mac`,
4. hệ thống được phép kích hoạt `unknown_wifi_alert`,
5. hệ thống được phép dùng WiFi/MAC lạ như tín hiệu hỗ trợ đánh giá nguy cơ theo BR-10.

## BR-09. Quy tắc tính `intrusion_score`

Hệ thống chỉ được tính `intrusion_score` cho chức năng chống trộm khi `system_armed = true`.

Các tín hiệu cộng điểm:

| Điều kiện | Điểm |
|---|---:|
| `pir_detected = true` | +1 |
| `object_near = true` | +1 |
| `light_abnormal = true` | +1 |
| `night_mode = true` | +1 |
| `unknown_wifi_detection_enabled = true` và `unknown_wifi_alert = true` và có ít nhất một tín hiệu vật lý đáng nghi | +1 |

Tín hiệu vật lý đáng nghi gồm:

```text
pir_detected = true
object_near = true
light_abnormal = true
```

Nếu `known_device_present = true`, hệ thống sẽ làm `system_armed = false` theo BR-01, do đó không xử lý đột nhập cho khu vực đó.

## BR-10. Quy tắc WiFi/MAC lạ là pre-warning

Hệ thống phải xem phát hiện WiFi/MAC lạ là tín hiệu cảnh báo sớm, không phải báo động đột nhập độc lập.

Khi `unknown_wifi_detection_enabled = true` và có hoạt động WiFi/MAC lạ, hệ thống có thể cập nhật:

- `unknown_wifi_count`,
- `last_unknown_mac`,
- `unknown_wifi_alert`,
- `last_event`,
- `threat_level`.

Hệ thống không được kích hoạt `intrusion_alert` chỉ dựa trên `unknown_wifi_alert`.

`unknown_wifi_alert` chỉ được góp điểm vào `intrusion_score` nếu đồng thời có ít nhất một tín hiệu cảm biến vật lý đáng nghi.

## BR-11. Quy tắc giữ điểm nghi ngờ

Khi một tín hiệu đáng nghi vừa biến mất, hệ thống phải giữ điểm nghi ngờ của tín hiệu đó trong 5 giây.

Sau 5 giây, nếu tín hiệu không xuất hiện lại, hệ thống phải loại điểm đó khỏi `intrusion_score`.

Thời gian giữ điểm này là cố định trong phiên bản hiện tại và không cần Cloud Variable riêng.

## BR-12. Quy tắc kích hoạt cảnh báo đột nhập

Khi `system_armed = true` và `intrusion_score` đạt hoặc vượt ngưỡng tương ứng với `sensitivity_level`, hệ thống phải kích hoạt cảnh báo đột nhập đầy đủ.

Hệ thống phải cập nhật:

- `intrusion_alert = true`,
- `alarm_status = INTRUSION_ALERT`,
- `buzzer_on = true`,
- `led_red_on = true`,
- `last_event = "Phát hiện đột nhập."`,
- `last_event_type = "intrusion_alert"`.

Cảnh báo đột nhập phải duy trì cho đến khi người lớn hoặc Admin kích hoạt `reset_alarm`.

## BR-13. Quy tắc cập nhật điểm trong lúc cảnh báo đột nhập đang hoạt động

Khi `intrusion_alert = true`, hệ thống vẫn tiếp tục đọc các tín hiệu cảm biến và cập nhật `intrusion_score` theo trạng thái hiện tại.

Tuy nhiên, cảnh báo đột nhập không được tự tắt chỉ vì `intrusion_score` giảm xuống dưới ngưỡng.

Cảnh báo đột nhập chỉ được tắt khi người lớn hoặc Admin kích hoạt `reset_alarm`.

## BR-14. Quy tắc gửi notification khi có đột nhập

Khi hệ thống kích hoạt `intrusion_alert`, hệ thống phải tạo yêu cầu gửi notification ra ngoài.

Khi `intrusion_alert = true`, hệ thống phải cập nhật:

- `send_notification_request = true`,
- `notification_event_type = "intrusion_alert"`,
- `notification_channel = "telegram"` nếu không có cấu hình khác,
- `notification_sent_status` theo trạng thái gửi hiện tại.

Việc gửi notification phải tuân theo rule cooldown để tránh spam cảnh báo lặp lại.

## BR-15. Quy tắc tự chụp ảnh khi có đột nhập

Khi `intrusion_alert = true`, hệ thống phải tự động kích hoạt chụp ảnh bằng ESP32-CAM.

Hệ thống phải cập nhật:

- `capture_photo = true` hoặc tạo yêu cầu chụp ảnh nội bộ,
- `photo_status = "CAPTURING"` khi đang xử lý ảnh,
- `photo_status = "CAPTURED"` nếu chụp thành công,
- `photo_status = "FAILED"` nếu chụp thất bại.

Việc chụp ảnh tự động phải tránh lặp lại liên tục khi cùng một cảnh báo đột nhập vẫn đang hoạt động.

## BR-16. Quy tắc số lượng ảnh tự động khi có đột nhập

Khi một lần `intrusion_alert` mới được kích hoạt, hệ thống phải tự động chụp 1 ảnh bằng ESP32-CAM.

Sau khi ảnh tự động đầu tiên đã được xử lý, hệ thống không được tự động chụp thêm ảnh trong cùng một lần `intrusion_alert`.

Trong lúc `intrusion_alert` vẫn đang hoạt động, Parent/Admin vẫn được phép kích hoạt `capture_photo` thủ công để yêu cầu chụp thêm ảnh.

Sau khi `reset_alarm` được xử lý và một `intrusion_alert` mới xảy ra trong tương lai, hệ thống được phép tự động chụp 1 ảnh mới cho lần cảnh báo mới đó.

## BR-17. Quy tắc cờ nội bộ kiểm soát chụp ảnh tự động

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

## BR-18. Quy tắc lịch tự động điều khiển `alarm_enabled`

Hệ thống phải cho phép người lớn/Admin cấu hình lịch tự động bật/tắt chức năng chống trộm.

Lịch tự động chỉ được thay đổi `alarm_enabled`.

Lịch tự động không được thay đổi trực tiếp `system_armed`.

Sau khi `alarm_enabled` được thay đổi bởi lịch, hệ thống phải tính lại `system_armed` dựa trên:

- `alarm_enabled`,
- `known_device_present`,
- `rearm_delay_seconds`,
- trạng thái countdown hiện tại nếu có.

## BR-19. Quy tắc lịch tự động hằng ngày

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

## BR-20. Quy tắc lịch ghi đè thao tác thủ công

Nếu `schedule_enabled = true`, lịch tự động được quyền thay đổi `alarm_enabled` khi đến thời điểm đã cấu hình.

Thao tác thủ công của người dùng không vô hiệu hóa lịch.

Nếu người dùng không muốn lịch tự động thay đổi `alarm_enabled`, người dùng phải đặt `schedule_enabled = false`.

## BR-21. Quy tắc giờ tự bật và tự tắt bị trùng nhau

Nếu `schedule_enabled = true` nhưng thời điểm tự bật và thời điểm tự tắt bị trùng nhau, hệ thống không được tự thay đổi `alarm_enabled` tại thời điểm đó.

Hệ thống phải ghi nhận lỗi cấu hình lịch vào:

```text
last_event = "Lỗi cấu hình lịch: giờ tự bật và giờ tự tắt bị trùng nhau."
last_event_type = "schedule_config_error"
```

Hệ thống có thể đặt `alarm_status = ERROR` nếu lỗi cấu hình làm hệ thống không thể xác định hành động an toàn.

Hệ thống không được tự động tắt `schedule_enabled`, trừ khi người dùng/Admin chủ động tắt.

## BR-22. Quy tắc giá trị giờ/phút không hợp lệ

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

## BR-23. Quy tắc ghi log khi lịch chạy

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

## BR-24. Quy tắc nguồn thời gian cho lịch tự động

Hệ thống phải dùng thời gian từ DS1307 RTC làm nguồn thời gian chính để kiểm tra lịch tự động bật/tắt chống trộm.

Các biến `current_time` và `current_hour` phải phản ánh thời gian hiện tại từ RTC.

Nếu RTC chưa được cấu hình đúng giờ hoặc trả về thời gian không hợp lệ, hệ thống không được chạy lịch tự động và phải ghi log lỗi thời gian:

```text
last_event = "Lỗi lịch tự động: thời gian RTC không hợp lệ."
last_event_type = "schedule_time_error"
```

Hệ thống không được phụ thuộc vào Internet/Cloud time để chạy lịch trong phiên bản hiện tại.

## BR-25. Quy tắc lịch tự bật khi có thiết bị đáng tin

Nếu lịch tự động bật chống trộm và `known_device_present = true`, hệ thống vẫn phải đặt:

```text
alarm_enabled = true
```

Tuy nhiên, hệ thống không được bật `system_armed` ngay.

Hệ thống phải đặt:

```text
system_armed = false
alarm_status = DISARMED_BY_TRUSTED_DEVICE
```

Khi thiết bị đáng tin rời khỏi khu vực và không quay lại trong `rearm_delay_seconds`, hệ thống mới được đặt:

```text
system_armed = true
alarm_status = ARMED
```

## BR-26. Quy tắc lưu sự kiện lịch

Hệ thống không cần tạo biến Cloud riêng để lưu hành động lịch gần nhất.

Khi lịch tự động chạy, hệ thống vẫn phải cập nhật:

- `last_event`,
- `last_event_type`.

Sau này, khi chức năng Event Logging được phân rã, các sự kiện lịch như `schedule_auto_arm` và `schedule_auto_disarm` phải được ghi vào lịch sử sự kiện dài hạn.


## BR-27. Quy tắc phạm vi của `threat_level`

`threat_level` không được hiểu là bản sao của `intrusion_score`.

`threat_level` là mức nguy hiểm tổng thể của hệ thống, được tính từ nhiều nguồn nguy cơ như:

- WiFi/MAC lạ,
- điểm nghi ngờ đột nhập,
- cảnh báo đột nhập đã xác nhận,
- phá hoại thiết bị,
- SOS.

`threat_level` dùng chủ yếu cho dashboard, ưu tiên cảnh báo, notification và event logging.

## BR-28. Quy tắc thang đo `threat_level`

`threat_level` phải là số nguyên từ 0 đến 4.

| Mức | Ý nghĩa |
|---:|---|
| 0 | Bình thường |
| 1 | Cần chú ý |
| 2 | Nghi ngờ trung bình |
| 3 | Nguy hiểm cao |
| 4 | Khẩn cấp |

## BR-29. Quy tắc tổng hợp `threat_level`

Khi nhiều loại nguy cơ xảy ra cùng lúc, hệ thống phải tính `threat_level` bằng mức nguy hiểm cao nhất trong các nguy cơ hiện tại.

Hệ thống không cộng dồn điểm nguy cơ để tạo `threat_level`.

Mỗi nhóm nguy cơ được ánh xạ sang một mức riêng. Sau đó hệ thống lấy mức lớn nhất làm `threat_level` tổng thể.

## BR-30. Quy tắc mapping `threat_level`

Hệ thống phải ánh xạ từng nhóm nguy cơ sang `threat_level` theo thang 0–4.

| Nguồn nguy cơ | Điều kiện | `threat_level` |
|---|---|---:|
| Bình thường | Không có tín hiệu đáng nghi | 0 |
| WiFi/MAC lạ đơn độc | `unknown_wifi_alert = true`, chưa có cảm biến vật lý đáng nghi | 1 |
| Một tín hiệu vật lý nhẹ | Chỉ có một trong các tín hiệu: `pir_detected`, `object_near`, `light_abnormal` | 1 |
| Nghi ngờ đột nhập trung bình | `intrusion_score = threshold - 1` và chưa có `intrusion_alert` | 2 |
| WiFi/MAC lạ + tín hiệu vật lý | `unknown_wifi_alert = true` và có tín hiệu vật lý đáng nghi | 2 |
| Đột nhập đã xác nhận | `intrusion_alert = true` | 3 |
| Phá hoại thiết bị nhẹ | Ví dụ che LDR tạm thời, cảm biến bất thường nhưng chưa nghiêm trọng | 3 |
| SOS | `sos_child = true` hoặc `sos_adult = true` | 4 |
| Phá hoại thiết bị nghiêm trọng | Ví dụ thiết bị bị tháo, cảm biến bị vô hiệu hóa rõ ràng | 4 |

Phần sabotage nhẹ/nghiêm trọng là mapping định hướng. Tiêu chí chi tiết sẽ được chốt trong cụm Anti-Sabotage.

## BR-31. Quy tắc gần ngưỡng của `threat_level`

Nếu `intrusion_alert = false` và `intrusion_score = threshold - 1`, hệ thống phải đặt mức nguy cơ đột nhập tương ứng là `threat_level = 2`.

Trong đó `threshold` là ngưỡng nội bộ được suy ra từ `sensitivity_level`.

## BR-32. Quy tắc giảm điểm do `pet_detected`

Khi `pet_detected = true`, hệ thống phải giảm mức nghi ngờ đột nhập thay vì chặn hoàn toàn cảnh báo.

`pet_detected` không được tự động đặt `intrusion_alert = false`.

`pet_detected` chỉ được dùng để giảm `intrusion_score` hoặc loại bỏ một phần điểm đến từ tín hiệu chuyển động/khoảng cách yếu.

## BR-33. Quy tắc mức giảm điểm do `pet_detected`

Khi `pet_detected = true`, hệ thống phải giảm `intrusion_score` cuối cùng đi 1 điểm.

Mức giảm này là cố định trong phiên bản hiện tại.

`intrusion_score` sau khi giảm không được nhỏ hơn 0.

## BR-34. Quy tắc điều kiện nhận diện `pet_detected`

Hệ thống chỉ được đặt `pet_detected = true` khi tín hiệu hiện tại giống thú nuôi/vật nhỏ/báo nhầm và không đi kèm dấu hiệu nguy hiểm rõ ràng.

Điều kiện định hướng:

1. Có tín hiệu vật thể hoặc chuyển động nhẹ/ngắn.
2. Tín hiệu không kéo dài.
3. Không có `light_abnormal`.
4. Không có `unknown_wifi_alert`.
5. Chưa có `intrusion_alert`.
6. Tổng tín hiệu nguy hiểm chưa đủ mạnh.

## BR-35. Quy tắc thời lượng tín hiệu ngắn cho `pet_detected`

Trong phạm vi lọc báo động giả, một tín hiệu được xem là ngắn nếu tín hiệu đó xuất hiện và biến mất trong thời gian dưới 5 giây.

Nếu tín hiệu chuyển động/vật thể tồn tại từ 5 giây trở lên, hệ thống không được xem tín hiệu đó là tín hiệu ngắn để tự đặt `pet_detected = true`.

## BR-36. Quy tắc ý nghĩa của `pet_detected`

`pet_detected = true` không có nghĩa là hệ thống xác nhận chắc chắn có thú nuôi.

`pet_detected = true` chỉ có nghĩa là hệ thống nghi tín hiệu hiện tại giống thú nuôi, vật nhỏ hoặc báo động giả hơn là người đột nhập.

Hệ thống không được dùng `pet_detected` như bằng chứng tuyệt đối để bỏ qua nguy cơ an ninh.

## BR-37. Quy tắc nguồn tín hiệu cho `pet_detected`

Hệ thống chỉ được đặt `pet_detected = true` khi cả hai tín hiệu sau cùng xuất hiện ngắn:

- `object_near = true`,
- `pir_detected = true`.

Đồng thời phải thỏa các điều kiện an toàn:

- `light_abnormal = false`,
- `unknown_wifi_alert = false`,
- `intrusion_alert = false`.

## BR-38. Quy tắc `pet_detected` khi gần ngưỡng

Nếu `intrusion_score = threshold - 1` và các điều kiện nhận diện pet/báo nhầm vẫn hợp lệ, hệ thống vẫn được phép đặt `pet_detected = true`.

Khi `pet_detected = true`, hệ thống giảm `intrusion_score` cuối cùng đi 1 điểm.

Tuy nhiên, `pet_detected` không được dùng để tắt một cảnh báo đột nhập đã được kích hoạt.

## BR-39. Quy tắc `threat_level` khi chỉ là pet/báo nhầm

Nếu `pet_detected = true` và không có nguy cơ khác nghiêm trọng hơn, hệ thống phải giới hạn `threat_level` tối đa ở mức 1.

Điều kiện áp dụng:

- `pet_detected = true`,
- `intrusion_alert = false`,
- `light_abnormal = false`,
- `unknown_wifi_alert = false`,
- `sabotage_alert = false`,
- không có SOS đang hoạt động.

Khi đó: `threat_level = 1`.

## BR-40. Quy tắc notification cho pet/báo nhầm

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

## BR-41. Quy tắc đặt tên biến notification

Hệ thống phải dùng nhóm biến `notification_*` để đại diện cho yêu cầu gửi thông báo ra ngoài.

Các biến `email_*` không còn là tên ưu tiên trong SRS vì hệ thống có thể gửi thông báo qua nhiều kênh như Telegram Bot hoặc Email.

Nhóm biến notification phải hỗ trợ tối thiểu:

- yêu cầu gửi thông báo,
- loại sự kiện cần gửi,
- trạng thái gửi,
- kênh thông báo.

## BR-42. Quy tắc kênh notification mặc định

Kênh thông báo mặc định của hệ thống là Telegram Bot.

Khi hệ thống cần gửi thông báo ra ngoài, nếu không có cấu hình khác, hệ thống phải dùng `notification_channel = "telegram"`.

Email không phải kênh mặc định trong phiên bản hiện tại. Email có thể được xem là hướng mở rộng hoặc tùy chọn sau này.

## BR-43. Quy tắc event gửi Telegram

Telegram Bot phải được dùng để gửi các thông báo an ninh quan trọng và một số cảnh báo sớm có ý nghĩa.

Hệ thống phải gửi Telegram cho các event sau:

- `intrusion_alert`,
- `sos_alert`,
- `sabotage_alert`,
- `unknown_wifi_alert`,
- `schedule_config_error`,
- `schedule_time_error`,
- `photo_capture_failed`.

Hệ thống không gửi Telegram cho các event nhẹ hoặc event vận hành bình thường như:

- `pet_or_false_positive`,
- `schedule_auto_arm`,
- `schedule_auto_disarm`,
- `alarm_reset` thành công,
- cập nhật dashboard thông thường.

## BR-44. Quy tắc chống spam Telegram cho WiFi/MAC lạ

Hệ thống không được gửi Telegram ngay chỉ vì phát hiện một lần WiFi/MAC lạ.

Hệ thống chỉ được gửi Telegram cho `unknown_wifi_alert` khi thiết bị WiFi/MAC lạ được phát hiện nhiều lần trong một khoảng thời gian ngắn.

## BR-45. Quy tắc ngưỡng Telegram cho WiFi/MAC lạ

Nếu `unknown_wifi_detection_enabled = true`, hệ thống chỉ được gửi Telegram cho sự kiện `unknown_wifi_alert` khi phát hiện WiFi/MAC lạ ít nhất 3 lần trong vòng 60 giây.

Nếu số lần phát hiện WiFi/MAC lạ nhỏ hơn 3 trong 60 giây, hệ thống chỉ cập nhật dashboard/log và không gửi Telegram.

## BR-46. Quy tắc cùng một MAC lạ lặp lại

Hệ thống chỉ được gửi Telegram cho sự kiện `unknown_wifi_alert` khi cùng một MAC lạ xuất hiện ít nhất 3 lần trong vòng 60 giây.

Nếu trong 60 giây có nhiều MAC lạ khác nhau nhưng không có MAC nào lặp lại đủ 3 lần, hệ thống chỉ cập nhật dashboard/log và không gửi Telegram.

## BR-47. Quy tắc cooldown Telegram cho cùng một MAC lạ

Sau khi hệ thống đã gửi Telegram cho một MAC lạ, hệ thống không được gửi lại Telegram cho cùng MAC đó trong vòng 60 giây.

Cooldown này áp dụng theo từng MAC lạ.

Nếu cùng MAC lạ tiếp tục xuất hiện trong thời gian cooldown, hệ thống chỉ cập nhật dashboard/log, không gửi Telegram mới.

Sau khi hết 60 giây, nếu cùng MAC lạ lại thỏa điều kiện xuất hiện ít nhất 3 lần trong 60 giây, hệ thống được phép gửi Telegram lại.

## BR-48. Quy tắc timer nội bộ cho cooldown WiFi/MAC lạ

Cooldown gửi Telegram cho WiFi/MAC lạ phải được xử lý bằng timer nội bộ trong code.

Hệ thống không cần tạo Cloud Variable riêng để hiển thị trạng thái cooldown WiFi/MAC lạ.

Admin có thể theo dõi kết quả qua:

- `last_event`,
- `last_event_type`,
- `last_unknown_mac`,
- `unknown_wifi_count`,
- `notification_sent_status`.

## BR-49. Quy tắc reset trạng thái khi tắt phát hiện WiFi/MAC lạ

Khi `unknown_wifi_detection_enabled` được đặt thành `false`, hệ thống phải dừng chức năng phát hiện WiFi/MAC lạ.

Hệ thống phải cập nhật:

- `unknown_wifi_alert = false`,
- `unknown_wifi_count = 0`.

Hệ thống được giữ lại:

- `last_unknown_mac`.

`last_unknown_mac` khi đó chỉ được hiểu là MAC lạ gần nhất từng được ghi nhận trước khi chức năng bị tắt, không phải cảnh báo đang hoạt động.

## BR-50. Quy tắc phiên mới khi bật lại phát hiện WiFi/MAC lạ

Khi `unknown_wifi_detection_enabled` được chuyển từ `false` sang `true`, hệ thống phải bắt đầu một phiên theo dõi WiFi/MAC lạ mới.

Hệ thống phải:

- đặt `unknown_wifi_alert = false`,
- đặt `unknown_wifi_count = 0`,
- reset bộ đếm nội bộ theo từng MAC,
- reset timer cửa sổ 60 giây,
- reset cooldown nội bộ nếu phù hợp.

`last_unknown_mac` có thể được giữ lại để Admin tham khảo, nhưng không được dùng để tính tiếp điều kiện gửi Telegram trong phiên mới.

---

# 8. Yêu Cầu Chức Năng

## FR-01. Điều khiển chế độ chống trộm

### Mục đích

Hệ thống phải cho phép người lớn và Admin bật/tắt chức năng phát hiện đột nhập cho khu vực giám sát.

### Actor chính

- Người lớn
- Admin
- Hệ thống / Board

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `alarm_enabled` | Parent/Admin Dashboard |
| `known_device_present` | Board phát hiện thiết bị đáng tin |
| `rearm_delay_seconds` | Parent/Admin Dashboard |
| `reset_alarm` | Parent/Admin Dashboard |

### Tiền điều kiện

1. Thiết bị đã được cấp nguồn.
2. Biến Arduino Cloud đã được khởi tạo.
3. Khu vực giám sát đã được cấu hình.
4. Người lớn/Admin có dashboard để điều khiển thủ công.

### Luồng chính

1. Người lớn/Admin đặt `alarm_enabled = true`.
2. Hệ thống kiểm tra `known_device_present`.
3. Nếu `known_device_present = true`, hệ thống đặt:
   - `system_armed = false`,
   - `alarm_status = DISARMED_BY_TRUSTED_DEVICE`.
4. Nếu `known_device_present = false`, hệ thống bắt đầu hoặc tiếp tục countdown re-arm theo `rearm_delay_seconds`.
5. Nếu countdown kết thúc và vẫn không phát hiện thiết bị đáng tin, hệ thống đặt:
   - `system_armed = true`,
   - `alarm_status = ARMED`,
   - `rearm_countdown_remaining = 0`.
6. Nếu người lớn/Admin đặt `alarm_enabled = false`, hệ thống đặt:
   - `system_armed = false`,
   - `alarm_status = DISARMED`,
   - không tạo cảnh báo đột nhập.

### Luồng thay thế A — Thiết bị đáng tin quay lại trong lúc countdown

1. Hệ thống đang đếm thời gian tự bật lại bảo vệ.
2. Thiết bị đáng tin xuất hiện lại.
3. Hệ thống hủy countdown.
4. Hệ thống đặt:
   - `rearm_countdown_remaining = 0`,
   - `system_armed = false`,
   - `alarm_status = DISARMED_BY_TRUSTED_DEVICE`.

### Luồng thay thế B — Đang có cảnh báo hoạt động

1. Cảnh báo đột nhập đang hoạt động.
2. Thiết bị đáng tin xuất hiện.
3. Hệ thống không tạo thêm cảnh báo đột nhập mới.
4. Hệ thống không tự động xóa cảnh báo hiện tại.
5. Người lớn/Admin phải kích hoạt `reset_alarm` để xóa cảnh báo.

### Đầu ra

| Đầu ra | Mô tả |
|---|---|
| `system_armed` | Trạng thái chống trộm thực tế |
| `alarm_status` | Trạng thái dạng chữ |
| `rearm_countdown_remaining` | Số giây còn lại cho Admin theo dõi |
| `last_event` | Có thể cập nhật khi chế độ thay đổi |

### Hậu điều kiện

Hệ thống sẽ ở một trong các trạng thái:

- `DISARMED`,
- `DISARMED_BY_TRUSTED_DEVICE`,
- `ARMED`,
- trạng thái cảnh báo nếu đang có báo động.

### Acceptance Scenario

**Acceptance Scenario** nghĩa là kịch bản chấp nhận, dùng để biết chức năng có đạt yêu cầu hay chưa.

Khi `alarm_enabled = true` và `known_device_present = true`, nếu board phát hiện thiết bị đáng tin gần khu vực giám sát, thì hệ thống phải đặt `system_armed = false` và không tạo cảnh báo đột nhập.

Khi `alarm_enabled = true` và không có thiết bị đáng tin nào được phát hiện liên tục cho đến khi `rearm_delay_seconds` kết thúc, hệ thống phải đặt `system_armed = true` và `alarm_status = ARMED`.

---

## FR-02. Xử lý reset báo động

### Mục đích

Hệ thống phải cho phép người lớn/Admin xóa cảnh báo hiện tại mà không xóa lịch sử sự kiện.

### Actor chính

- Người lớn
- Admin
- Hệ thống / Board

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `reset_alarm` | Parent/Admin Dashboard |
| Trạng thái cảnh báo hiện tại | Board |
| `alarm_enabled` | Cloud variable |
| `known_device_present` | Board detection |

### Tiền điều kiện

1. Hệ thống có thể đang có hoặc không có cảnh báo.
2. Parent/Admin Dashboard có thể truy cập được.
3. `reset_alarm` chỉ cho phép người lớn/Admin ghi.

### Luồng chính

1. Người lớn/Admin đặt `reset_alarm = true`.
2. Hệ thống dừng đầu ra cảnh báo:
   - `buzzer_on = false`,
   - `led_red_on = false`.
3. Hệ thống xóa trạng thái cảnh báo đang hoạt động nếu phù hợp:
   - `intrusion_alert = false`,
   - `sabotage_alert = false` chỉ khi tình huống phá hoại đã được xác nhận xử lý hoặc không còn tồn tại.
4. Hệ thống giữ nguyên:
   - `last_event`,
   - `last_event_type`,
   - `event_counter`.
5. Hệ thống tính lại `system_armed`.
6. Hệ thống cập nhật `alarm_status`.
7. Hệ thống tự đặt `reset_alarm = false`.

### Đầu ra

| Đầu ra | Mô tả |
|---|---|
| `buzzer_on` | false sau khi reset |
| `led_red_on` | false sau khi reset |
| `intrusion_alert` | false sau khi reset |
| `alarm_status` | trạng thái đã tính lại |
| `reset_alarm` | tự trở về false |

### Hậu điều kiện

Cảnh báo hiện tại được xóa, nhưng log sự kiện vẫn được giữ.

### Acceptance Scenario

Khi cảnh báo đột nhập đang hoạt động, nếu người lớn/Admin kích hoạt `reset_alarm`, hệ thống phải tắt còi và LED, xóa cảnh báo đột nhập hiện tại, giữ log sự kiện, rồi tính lại `system_armed` theo điều kiện hiện tại.

---

## FR-03. Điều khiển chế độ chống trộm theo lịch

### Mục đích

Hệ thống phải cho phép người lớn/Admin cấu hình thời gian tự bật và tự tắt chức năng chống trộm mỗi ngày.

### Actor chính

- Người lớn
- Admin
- Hệ thống / Board

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `schedule_enabled` | Parent/Admin Dashboard |
| `auto_arm_hour` | Parent/Admin Dashboard |
| `auto_arm_minute` | Parent/Admin Dashboard |
| `auto_disarm_hour` | Parent/Admin Dashboard |
| `auto_disarm_minute` | Parent/Admin Dashboard |
| `current_time` / `current_hour` | DS1307 RTC |

### Tiền điều kiện

1. DS1307 RTC hoạt động và trả về thời gian hợp lệ.
2. Các giá trị giờ/phút cấu hình nằm trong khoảng hợp lệ.
3. `schedule_enabled = true`.

### Luồng chính

1. Hệ thống đọc thời gian hiện tại từ DS1307 RTC.
2. Hệ thống kiểm tra cấu hình lịch.
3. Nếu đến `auto_arm_hour:auto_arm_minute`, hệ thống đặt `alarm_enabled = true`.
4. Hệ thống ghi:
   - `last_event = "Lịch tự động đã bật chế độ chống trộm."`,
   - `last_event_type = "schedule_auto_arm"`.
5. Hệ thống tính lại `system_armed` theo `alarm_enabled`, `known_device_present` và `rearm_delay_seconds`.
6. Nếu đến `auto_disarm_hour:auto_disarm_minute`, hệ thống đặt `alarm_enabled = false`.
7. Hệ thống ghi:
   - `last_event = "Lịch tự động đã tắt chế độ chống trộm."`,
   - `last_event_type = "schedule_auto_disarm"`.
8. Hệ thống đặt `system_armed = false` và `alarm_status = DISARMED`.

### Luồng thay thế

- Nếu `schedule_enabled = false`, hệ thống không tự thay đổi `alarm_enabled` theo lịch.
- Nếu giờ tự bật và giờ tự tắt trùng nhau, hệ thống không thay đổi `alarm_enabled` và ghi lỗi cấu hình lịch.
- Nếu giờ/phút không hợp lệ, hệ thống bỏ qua lịch, giữ nguyên `alarm_enabled`, không đổi trực tiếp `system_armed` và ghi `last_event_type = "schedule_config_error"`.
- Nếu lịch bật `alarm_enabled = true` nhưng `known_device_present = true`, hệ thống vẫn giữ `system_armed = false` và đặt `alarm_status = DISARMED_BY_TRUSTED_DEVICE`.

### Acceptance Scenario

Khi `schedule_enabled = true` và RTC đến 23:00 theo cấu hình tự bật, hệ thống phải đặt `alarm_enabled = true`, ghi log `schedule_auto_arm`, rồi tính lại `system_armed`.

Khi `schedule_enabled = true` và RTC đến 06:00 theo cấu hình tự tắt, hệ thống phải đặt `alarm_enabled = false`, ghi log `schedule_auto_disarm`, và đặt `system_armed = false`.

---

## FR-04. Phát hiện đột nhập đa cảm biến

### Mục đích

Hệ thống phải xác định nguy cơ đột nhập bằng cách tính `intrusion_score` từ nhiều tín hiệu cảm biến và ngữ cảnh, thay vì dùng điều kiện cứng AND/OR.

### Actor chính

- Hệ thống / Board
- Admin
- Người lớn

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `system_armed` | Logic trạng thái hệ thống |
| `sensitivity_level` | Parent/Admin Dashboard |
| `pir_detected` | PIR sensor |
| `object_near` | HC-SR04 / logic khoảng cách |
| `light_abnormal` | LDR / logic ánh sáng |
| `night_mode` | RTC / logic thời gian |
| `unknown_wifi_detection_enabled` | Parent/Admin Dashboard |
| `unknown_wifi_alert` | Logic WiFi/MAC lạ |

### Tiền điều kiện

1. `system_armed = true`.
2. Cảm biến cần thiết đã được khởi tạo.
3. `sensitivity_level` nằm trong khoảng 1–3 hoặc được fallback về 2.
4. Nếu `known_device_present = true`, hệ thống không xử lý đột nhập cho khu vực vì `system_armed = false`.

### Luồng chính

1. Hệ thống đọc các tín hiệu cảm biến.
2. Hệ thống tính điểm:
   - `pir_detected = true` → +1,
   - `object_near = true` → +1,
   - `light_abnormal = true` → +1,
   - `night_mode = true` → +1,
   - WiFi/MAC lạ hợp lệ → +1.
3. Nếu một tín hiệu vừa biến mất, hệ thống giữ điểm tín hiệu đó thêm 5 giây.
4. Hệ thống map `sensitivity_level` sang ngưỡng nội bộ.
5. Nếu `intrusion_score` đạt hoặc vượt ngưỡng, hệ thống kích hoạt cảnh báo đột nhập đầy đủ:
   - `intrusion_alert = true`,
   - `alarm_status = INTRUSION_ALERT`,
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - gửi yêu cầu thông báo,
   - tự chụp 1 ảnh cho lần cảnh báo mới.

### Luồng thay thế

- Nếu `unknown_wifi_detection_enabled = false`, hệ thống không dùng WiFi/MAC lạ để cộng điểm.
- Nếu chỉ có `unknown_wifi_alert = true` nhưng không có `pir_detected`, `object_near` hoặc `light_abnormal`, hệ thống không được kích hoạt `intrusion_alert`.
- Nếu `intrusion_alert = true`, hệ thống vẫn tiếp tục cập nhật `intrusion_score`, nhưng không tự tắt cảnh báo khi điểm giảm.

### Acceptance Scenario

Khi `system_armed = true`, `sensitivity_level = 2`, `pir_detected = true`, `object_near = true`, `night_mode = true`, hệ thống phải tính `intrusion_score = 3` và kích hoạt `intrusion_alert = true`.

Khi `unknown_wifi_alert = true` nhưng không có tín hiệu vật lý đáng nghi, hệ thống không được kích hoạt `intrusion_alert`.

---

## FR-05. Phát hiện thiết bị WiFi/MAC lạ dạng pre-warning

### Mục đích

Hệ thống phải hỗ trợ phát hiện thiết bị WiFi/MAC lạ như một dạng cảnh báo sớm, nhưng không được xem đây là báo động đột nhập độc lập.

### Actor chính

- Hệ thống / Board
- Người lớn
- Admin
- Telegram Bot

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `unknown_wifi_detection_enabled` | Parent/Admin Dashboard |
| Thiết bị WiFi/MAC lạ | Logic WiFi scanning |
| Bộ đếm MAC nội bộ | Board |
| Timer cửa sổ 60 giây | Board |
| Cooldown từng MAC | Board |

### Luồng chính

1. Parent/Admin bật `unknown_wifi_detection_enabled = true`.
2. Hệ thống bắt đầu một phiên theo dõi WiFi/MAC lạ mới.
3. Khi phát hiện thiết bị lạ, hệ thống cập nhật `last_unknown_mac`, bộ đếm nội bộ theo từng MAC và `unknown_wifi_count`.
4. Nếu cùng một MAC lạ xuất hiện ít nhất 3 lần trong vòng 60 giây, hệ thống đặt `unknown_wifi_alert = true` và `last_event_type = "unknown_wifi_alert"`.
5. Nếu MAC đó chưa trong cooldown 60 giây, hệ thống tạo yêu cầu gửi Telegram:
   - `send_notification_request = true`,
   - `notification_event_type = "unknown_wifi_alert"`,
   - `notification_channel = "telegram"`.
6. Nếu MAC đó đang trong cooldown, hệ thống chỉ cập nhật dashboard/log và không gửi Telegram mới.

### Luồng thay thế A — Nhiều MAC khác nhau

Nếu trong 60 giây có nhiều MAC lạ khác nhau nhưng không có MAC nào lặp lại đủ 3 lần, hệ thống không gửi Telegram.

### Luồng thay thế B — Tắt phát hiện WiFi/MAC lạ

Khi `unknown_wifi_detection_enabled = false`, hệ thống phải đặt `unknown_wifi_alert = false`, đặt `unknown_wifi_count = 0` và giữ `last_unknown_mac` để Admin tham khảo.

### Luồng thay thế C — Bật lại phát hiện WiFi/MAC lạ

Khi bật lại `unknown_wifi_detection_enabled`, hệ thống phải bắt đầu phiên theo dõi mới, reset bộ đếm nội bộ và không dùng dữ liệu cũ để gửi Telegram.

### Acceptance Scenario

Nếu cùng một MAC lạ xuất hiện 3 lần trong 60 giây và chưa trong cooldown, hệ thống phải gửi Telegram `unknown_wifi_alert`.

Nếu có 3 MAC lạ khác nhau trong 60 giây nhưng không có MAC nào lặp lại đủ 3 lần, hệ thống không được gửi Telegram.

## FR-06. Gửi Telegram / notification khi có sự kiện an ninh

### Mục đích

Hệ thống phải gửi Telegram/notification cho các sự kiện an ninh quan trọng và một số pre-warning có ý nghĩa.

### Actor chính

- Hệ thống / Board
- Telegram Bot
- Người lớn
- Admin

### Biến liên quan

| Biến | Ý nghĩa |
|---|---|
| `send_notification_request` | Yêu cầu gửi thông báo |
| `notification_event_type` | Loại sự kiện cần gửi |
| `notification_sent_status` | Trạng thái gửi |
| `notification_channel` | Kênh gửi, mặc định `telegram` |

### Event được gửi Telegram

Hệ thống phải gửi Telegram cho:

- `intrusion_alert`,
- `sos_alert`,
- `sabotage_alert`,
- `unknown_wifi_alert`,
- `schedule_config_error`,
- `schedule_time_error`,
- `photo_capture_failed`.

### Event không gửi Telegram

Hệ thống không gửi Telegram cho:

- `pet_or_false_positive`,
- `schedule_auto_arm`,
- `schedule_auto_disarm`,
- `alarm_reset` thành công,
- cập nhật dashboard thông thường.

### Luồng chính khi có đột nhập

1. Hệ thống kích hoạt `intrusion_alert = true`.
2. Hệ thống đặt:
   - `send_notification_request = true`,
   - `notification_event_type = "intrusion_alert"`,
   - `notification_channel = "telegram"`.
3. Telegram Bot xử lý yêu cầu gửi.
4. Hệ thống cập nhật `notification_sent_status` theo kết quả.

### Ràng buộc

Notification phải tuân theo cooldown phù hợp để tránh spam, đặc biệt với WiFi/MAC lạ và các cảnh báo lặp lại.

### Acceptance Scenario

Khi `intrusion_alert = true`, hệ thống phải tạo yêu cầu gửi Telegram với `notification_event_type = "intrusion_alert"`.

Khi `pet_detected = true` nhưng chưa có `intrusion_alert`, hệ thống không được gửi Telegram.

## FR-07. Tự chụp ảnh khi có đột nhập

### Mục đích

Hệ thống phải tự động chụp ảnh bằng ESP32-CAM khi có cảnh báo đột nhập để ghi nhận bằng chứng.

### Luồng chính

1. Một lần `intrusion_alert` mới được kích hoạt.
2. Hệ thống kiểm tra cờ nội bộ `auto_photo_captured_for_current_alert`.
3. Nếu cờ nội bộ là false, hệ thống tự chụp 1 ảnh.
4. Hệ thống cập nhật `photo_status = "CAPTURING"` trong lúc xử lý.
5. Nếu chụp thành công, hệ thống đặt `photo_status = "CAPTURED"`.
6. Nếu chụp thất bại, hệ thống đặt `photo_status = "FAILED"` và có thể ghi `last_event_type = "photo_capture_failed"`.
7. Hệ thống đặt cờ nội bộ đã chụp ảnh tự động cho lần cảnh báo hiện tại.
8. Trong cùng một lần `intrusion_alert`, hệ thống không tự chụp thêm.
9. Parent/Admin vẫn có thể đặt `capture_photo = true` để chụp thủ công thêm.
10. Khi `reset_alarm` được xử lý, hệ thống reset cờ nội bộ để lần cảnh báo tiếp theo có thể tự chụp 1 ảnh mới.

### Acceptance Scenario

Khi một `intrusion_alert` mới xảy ra, hệ thống phải tự chụp đúng 1 ảnh. Trong cùng cảnh báo đó, hệ thống không tự chụp thêm ảnh, nhưng Parent/Admin vẫn được phép chụp thủ công thêm.

---

## FR-08. Lọc báo động giả / vật nuôi

### Mục đích

Hệ thống phải giảm khả năng báo động giả do vật nhỏ/thú nuôi/báo nhầm mà không làm mất an toàn của chức năng chống trộm.

### Ý nghĩa của `pet_detected`

`pet_detected = true` không có nghĩa là hệ thống xác nhận chắc chắn có thú nuôi.

Nó chỉ có nghĩa là hệ thống nghi tín hiệu hiện tại giống thú nuôi, vật nhỏ hoặc báo động giả hơn là người đột nhập.

### Actor chính

- Hệ thống / Board
- Admin

### Đầu vào

| Đầu vào | Nguồn |
|---|---|
| `pir_detected` | PIR sensor |
| `object_near` | HC-SR04 / logic khoảng cách |
| `light_abnormal` | LDR / logic ánh sáng |
| `unknown_wifi_alert` | Logic WiFi/MAC lạ |
| `intrusion_score` | Logic phát hiện đột nhập |
| `intrusion_alert` | Logic cảnh báo đột nhập |

### Luồng chính

1. Hệ thống theo dõi `pir_detected` và `object_near`.
2. Nếu cả `pir_detected` và `object_near` cùng xuất hiện ngắn dưới 5 giây, hệ thống tiếp tục kiểm tra điều kiện an toàn.
3. Nếu đồng thời `light_abnormal = false`, `unknown_wifi_alert = false` và `intrusion_alert = false`, hệ thống được phép đặt `pet_detected = true`.
4. Khi `pet_detected = true`, hệ thống giảm `intrusion_score` cuối cùng đi 1 điểm.
5. `intrusion_score` sau khi giảm không được nhỏ hơn 0.
6. Nếu chỉ là pet/báo nhầm và không có nguy cơ khác, hệ thống đặt `threat_level = 1`.
7. Hệ thống cập nhật:
   - `last_event = "Phát hiện tín hiệu giống vật nhỏ hoặc báo nhầm."`,
   - `last_event_type = "pet_or_false_positive"`.
8. Hệ thống không gửi Telegram cho event này.

### Luồng thay thế A — Có dấu hiệu nguy hiểm khác

Nếu có `light_abnormal = true` hoặc `unknown_wifi_alert = true`, hệ thống không được dùng pet logic để kéo `threat_level` xuống 1.

### Luồng thay thế B — Đã có cảnh báo đột nhập

Nếu `intrusion_alert = true`, `pet_detected` không được dùng để tắt hoặc hạ cảnh báo hiện tại.

### Luồng thay thế C — Gần ngưỡng báo động

Nếu `intrusion_score = threshold - 1` nhưng điều kiện pet vẫn hợp lệ, hệ thống vẫn được phép đặt `pet_detected = true` và giảm 1 điểm để hỗ trợ kịch bản pet/báo nhầm hoạt động trơn tru.

### Acceptance Scenario

Khi `pir_detected` và `object_near` cùng xuất hiện dưới 5 giây, không có `light_abnormal`, không có `unknown_wifi_alert` và chưa có `intrusion_alert`, hệ thống phải được phép đặt `pet_detected = true`, giảm `intrusion_score` 1 điểm, đặt `threat_level` tối đa 1 và không gửi Telegram.

Khi `light_abnormal = true` hoặc `unknown_wifi_alert = true`, hệ thống không được dùng `pet_detected` để hạ mức nguy hiểm xuống 1.

## FR-09. Phát hiện phá hoại thiết bị

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách hệ thống phát hiện hành vi che cảm biến, chặn cảm biến hoặc can thiệp vật lý.

Quy tắc hiện tại đã chốt:

```text
Anti-sabotage không phụ thuộc system_armed.
```

## FR-10. SOS khẩn cấp qua Dashboard

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa SOS trẻ em, SOS người lớn, mức SOS, đầu ra cảnh báo, thông báo và reset.

Quy tắc hiện tại đã chốt:

```text
SOS không phụ thuộc system_armed.
```

## FR-11. Ghi log sự kiện dài hạn

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa định dạng log, cách tăng `event_counter`, cách lưu nhiều dòng sự kiện và sự kiện nào được xem là có ý nghĩa.

Các sự kiện đã biết cần được ghi vào Event Logging dài hạn sau này:

- `schedule_auto_arm`,
- `schedule_auto_disarm`,
- `schedule_config_error`,
- `schedule_time_error`,
- `intrusion_alert`,
- `photo_capture_failed`,
- các sự kiện SOS,
- các sự kiện sabotage,
- các sự kiện WiFi/MAC lạ đáng chú ý.

## FR-12. Chống spam cảnh báo / cooldown

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa hành vi cooldown cho các cảnh báo lặp lại, đặc biệt là notification/Telegram khi có đột nhập.

## FR-13. Chế độ demo

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách `demo_mode` và `demo_scenario` ảnh hưởng tới demo/kiểm thử.

---

# 9. Yêu Cầu Phi Chức Năng

## NFR-01. Độ tin cậy

Hệ thống nên tránh cảnh báo không cần thiết khi thiết bị đáng tin đang xuất hiện trong khu vực giám sát.

## NFR-02. Khả năng bảo trì

ID yêu cầu, tên biến và giá trị trạng thái phải nhất quán giữa SRS, code, dashboard và test.

## NFR-03. Dễ đọc cho agent

Tài liệu phải đánh dấu rõ chức năng đã phân rã và chức năng đang chờ phân rã để AI agent không tự triển khai logic chưa được duyệt.

## NFR-04. Khả năng cấu hình

Thời gian tự bật lại bảo vệ phải được cấu hình qua Cloud Variable, không hardcode.

**Hardcode** nghĩa là viết cố định giá trị trong code, khiến sau này khó chỉnh.

## NFR-05. An toàn khi reset

Reset báo động không được xóa log sự kiện hoặc bộ đếm sự kiện.

## NFR-06. Nhất quán phạm vi

Báo cháy và RFID không thuộc phạm vi hiện tại nếu chưa được duyệt lại.

## NFR-07. Đơn giản cho dashboard trẻ em

Trẻ em chỉ được truy cập SOS và thông tin trạng thái đơn giản. Các điều khiển nâng cao chỉ dành cho người lớn/Admin.

## NFR-08. Tránh spam cảnh báo

Thông báo/email và chụp ảnh tự động phải có cơ chế tránh lặp liên tục trong cùng một cảnh báo hoặc trong thời gian cooldown.

## NFR-09. Khả năng demo

Hệ thống phải hỗ trợ cấu hình phù hợp môi trường demo, ví dụ tắt phát hiện WiFi/MAC lạ bằng `unknown_wifi_detection_enabled = false` để tránh nhiễu từ nhiều điện thoại trong lớp học.

---

# 10. Ràng Buộc

## CON-01. Ràng buộc biến Arduino Cloud

Tên biến Cloud phải khớp chính xác với SRS này, trừ khi có thay đổi được duyệt và cập nhật phiên bản.

## CON-02. Ràng buộc khoảng giá trị re-arm

`rearm_delay_seconds` phải bị giới hạn trong khoảng 5–600 giây.

## CON-03. Ràng buộc module đã loại bỏ

Hệ thống hiện tại không yêu cầu Flame Sensor, cảm biến nhiệt cho báo cháy hoặc RFID-RC522.

## CON-04. Ràng buộc khu vực giám sát

Phiên bản hiện tại có thể chỉ hỗ trợ một khu vực, nhưng yêu cầu phải tương thích với hướng mở rộng nhiều khu vực sau này.

## CON-05. Ràng buộc độ nhạy

`sensitivity_level` chỉ được nhận giá trị 1, 2 hoặc 3. Nếu không hợp lệ, hệ thống dùng mặc định 2.

## CON-06. Ràng buộc lịch tự động

Giờ phải nằm trong khoảng 0–23 và phút phải nằm trong khoảng 0–59. Nếu sai, hệ thống bỏ qua lịch và ghi lỗi.

## CON-07. Ràng buộc nguồn thời gian

Lịch tự động dùng DS1307 RTC làm nguồn thời gian chính trong phiên bản hiện tại, không phụ thuộc Internet/Cloud time.

## CON-08. Ràng buộc WiFi/MAC lạ

WiFi/MAC lạ không được tự kích hoạt báo động đột nhập độc lập.

## CON-09. Ràng buộc chụp ảnh tự động

Mỗi lần `intrusion_alert` mới chỉ được tự động chụp 1 ảnh. Việc chụp thêm trong cùng cảnh báo chỉ xảy ra khi Parent/Admin yêu cầu thủ công.

---

# 11. Giả Định Và Phụ Thuộc

## 11.1 Giả định

1. Phòng bếp là khu vực giám sát đầu tiên.
2. Người lớn/Admin có thể truy cập Arduino Cloud Dashboard.
3. Trẻ em không được chỉnh cấu hình hệ thống.
4. Phát hiện thiết bị đáng tin được dùng để giảm báo động thừa khi người nhà ở nhà.
5. Phát hiện thiết bị đáng tin có thể dựa trên WiFi/MAC, nhưng cách triển khai chi tiết có thể được điều chỉnh sau.
6. `known_device_present = true` nghĩa là ít nhất một thiết bị đáng tin được phát hiện gần khu vực giám sát.
7. Môi trường lớp học có thể có nhiều thiết bị WiFi/MAC lạ, nên `unknown_wifi_detection_enabled` cần có thể tắt khi demo.
8. DS1307 RTC phải được set giờ đúng để lịch tự động chạy đúng.

## 11.2 Phụ thuộc

| Phụ thuộc | Vai trò |
|---|---|
| Arduino Cloud | Đồng bộ biến và dashboard |
| WiFi | Đồng bộ Cloud, phát hiện thiết bị đáng tin/lạ, gửi thông báo |
| Cảm biến | Phát hiện đột nhập và phá hoại |
| Dịch vụ thông báo | Gửi Telegram hoặc cảnh báo bên ngoài |
| RTC module | Ghi thời gian sự kiện và hỗ trợ lịch tự động |
| ESP32-CAM | Chụp ảnh khi có cảnh báo đột nhập hoặc yêu cầu thủ công |

---

# 12. Ngoài Phạm Vi

Các nội dung sau không thuộc phạm vi SRS hiện tại:

1. Báo cháy thật.
2. Xử lý Flame Sensor.
3. Đo nhiệt độ bếp để xác nhận cháy.
4. Xác thực bằng thẻ RFID.
5. Gửi cảnh báo tự động đến công an/cứu hỏa/cơ quan khẩn cấp thật.
6. Điều khiển khóa cửa.
7. Triển khai đầy đủ nhiều khu vực giám sát.
8. Xác thực danh tính cấp production.
9. Phát triển app mobile riêng ngoài Arduino Cloud Dashboard.
10. Streaming video liên tục.
11. Chức năng lịch nhiều khung giờ trong ngày.
12. Đồng bộ thời gian RTC bằng Cloud/Internet.
13. Tự động chụp nhiều ảnh liên tiếp khi có đột nhập.

**Production** nghĩa là môi trường vận hành thật, yêu cầu độ ổn định và bảo mật cao hơn demo/đồ án.

---

# 13. Vấn Đề Cần Phân Rã Tiếp

Các câu hỏi sau vẫn đang mở và cần phân rã trước khi có bản SRS cuối.

| Vấn đề mở | Phần liên quan |
|---|---|
| Điều kiện chính xác nào kích hoạt `sabotage_alert`? | Anti-Sabotage |
| SOS trẻ em khác SOS người lớn như thế nào? | Emergency SOS |
| Định dạng log sự kiện dài hạn là gì? | Event Logging |
| Cooldown tổng quát cho intrusion/SOS/sabotage kéo dài bao lâu? | Anti-Spam |
| Demo mode mô phỏng hoặc override dữ liệu cảm biến như thế nào? | Demo Mode |
| Camera lỗi liên tục thì xử lý nâng cao ra sao? | Photo Capture |
| Notification/Telegram gửi lỗi nhiều lần thì retry hay chỉ báo FAILED? | Notification |

**Override** nghĩa là ghi đè hành vi hoặc dữ liệu thật bằng hành vi/dữ liệu khác trong một ngữ cảnh đặc biệt, ví dụ demo.

---

# 14. Phụ Lục A — Checkpoint Quyết Định Hiện Tại

## A.1 Các quyết định đã chốt

| Chủ đề | Quyết định |
|---|---|
| Kiểu SRS | Theo cấu trúc kiểu IEEE nhưng tối ưu cho agent |
| Phạm vi | Full scope nhưng phân rã và thiết kế lại dần |
| Cách xử lý demo | SRS chỉ ghi acceptance scenario ngắn; demo chi tiết để file riêng |
| Cloud Variables | Đưa bảng biến hiện tại vào SRS |
| Dashboard | Đưa vai trò dashboard và widget chính; layout chi tiết để file riêng |
| Báo cháy | Loại bỏ |
| RFID-RC522 | Loại bỏ |
| `alarm_enabled` | Chỉ điều khiển phát hiện đột nhập |
| `system_armed` | Trạng thái chống trộm thực tế của khu vực |
| `known_device_present = true` | Tự tắt phát hiện đột nhập cho khu vực |
| Logic re-arm | Có thời gian chờ cấu hình được, từ 5–600 giây |
| Hiển thị countdown | Biến Admin-only `rearm_countdown_remaining` |
| Báo động đang xảy ra + thiết bị đáng tin quay lại | Không tự xóa cảnh báo, cần reset |
| `reset_alarm` | Xóa cảnh báo hiện tại, giữ log, tính lại trạng thái |
| SOS | Không phụ thuộc `system_armed` |
| Anti-sabotage | Không phụ thuộc `system_armed` |
| Phát hiện đột nhập | Dùng `intrusion_score` |
| Độ nhạy đột nhập | Dùng `sensitivity_level`, không cho chỉnh trực tiếp threshold |
| Tín hiệu cộng điểm | PIR, object_near, light_abnormal, night_mode, WiFi/MAC lạ hợp lệ |
| WiFi/MAC lạ | Có công tắc `unknown_wifi_detection_enabled` |
| Vai trò WiFi/MAC lạ | Pre-warning, không tự báo động độc lập |
| Giữ điểm nghi ngờ | 5 giây cố định |
| Đạt ngưỡng đột nhập | Bật cảnh báo đầy đủ |
| Đang cảnh báo | Vẫn cập nhật `intrusion_score`, nhưng không tự tắt cảnh báo |
| Gửi thông báo đột nhập | Có, ngay khi `intrusion_alert = true` |
| Tự chụp ảnh đột nhập | Có, tự chụp 1 ảnh cho mỗi lần cảnh báo mới |
| Chụp ảnh thủ công thêm | Parent/Admin được phép |
| Cờ kiểm soát ảnh tự động | Dùng biến nội bộ, không đưa lên Cloud |
| Lịch tự động | Một mốc tự bật và một mốc tự tắt mỗi ngày |
| Lịch điều khiển biến | Chỉ thay đổi `alarm_enabled`, không đổi trực tiếp `system_armed` |
| Lịch ghi đè thao tác thủ công | Có, nếu `schedule_enabled = true` |
| Nguồn thời gian lịch | DS1307 RTC |
| Lưu lịch chạy gần nhất | Không thêm `last_schedule_action`, dùng `last_event` và Event Logging dài hạn sau này |
| Threat Level | Là mức nguy hiểm tổng thể, không phải bản sao của `intrusion_score` |
| Thang `threat_level` | 0–4 |
| Tổng hợp `threat_level` | Lấy mức cao nhất trong các nguy cơ đang có |
| Level 2 | Khi `intrusion_score = threshold - 1` và chưa có `intrusion_alert` |
| Pet Detection | `pet_detected` là nghi thú nuôi/vật nhỏ/báo nhầm, không phải xác nhận chắc chắn |
| Pet ảnh hưởng điểm | Giảm `intrusion_score` 1 điểm, không chặn hoàn toàn báo động |
| Điều kiện pet | Cả `object_near` và `pir_detected` cùng xuất hiện dưới 5 giây, không có nguy cơ khác |
| Pet gần ngưỡng | Vẫn được giảm điểm nếu điều kiện pet hợp lệ |
| Threat khi chỉ là pet | Tối đa level 1 |
| Pet notification | Không gửi Telegram, chỉ cập nhật dashboard/log |
| Notification variables | Đổi `email_*` sang `notification_*` |
| Kênh notification mặc định | Telegram Bot |
| Telegram gửi cho | intrusion, SOS, sabotage, unknown WiFi đủ điều kiện, lỗi lịch, lỗi chụp ảnh |
| Telegram không gửi cho | pet/báo nhầm, lịch tự bật/tắt thành công, reset thành công |
| Unknown WiFi Telegram | Cùng một MAC lạ xuất hiện 3 lần trong 60 giây |
| Unknown WiFi cooldown | 60 giây cho cùng MAC, dùng timer nội bộ |
| Tắt unknown WiFi detection | Reset alert/count, giữ `last_unknown_mac` |
| Bật lại unknown WiFi detection | Bắt đầu phiên đếm mới, không dùng dữ liệu cũ |

## A.2 Logic trạng thái lõi hiện tại

```text
Nếu alarm_enabled = false:
    system_armed = false
    alarm_status = DISARMED
    tắt phát hiện đột nhập
    SOS / anti-sabotage / log vẫn hoạt động

Nếu alarm_enabled = true và known_device_present = true:
    system_armed = false
    alarm_status = DISARMED_BY_TRUSTED_DEVICE
    tắt phát hiện đột nhập cho khu vực này

Nếu alarm_enabled = true và known_device_present = false:
    bắt đầu countdown re-arm theo rearm_delay_seconds
    cập nhật rearm_countdown_remaining cho Admin Dashboard
    nếu thiết bị đáng tin quay lại trước khi hết countdown:
        hủy countdown
        system_armed = false
        alarm_status = DISARMED_BY_TRUSTED_DEVICE
    nếu countdown kết thúc:
        system_armed = true
        alarm_status = ARMED

Nếu cảnh báo đột nhập đang hoạt động và thiết bị đáng tin xuất hiện:
    không tạo thêm cảnh báo đột nhập mới
    không tự xóa cảnh báo hiện tại
    chờ reset_alarm từ Parent/Admin

Nếu reset_alarm = true:
    tắt còi/LED
    xóa trạng thái cảnh báo hiện tại
    giữ log
    tính lại system_armed
    đặt reset_alarm về false
```


## A.3 Logic lịch tự động hiện tại

```text
Nếu schedule_enabled = false:
    không tự thay đổi alarm_enabled theo lịch

Nếu schedule_enabled = true:
    nếu RTC không hợp lệ:
        bỏ qua lịch
        ghi last_event_type = "schedule_time_error"

    nếu giờ/phút không hợp lệ:
        bỏ qua lịch
        ghi last_event_type = "schedule_config_error"

    nếu giờ bật và giờ tắt trùng nhau:
        bỏ qua lịch tại thời điểm đó
        ghi last_event_type = "schedule_config_error"

    nếu đến auto_arm_hour:auto_arm_minute:
        alarm_enabled = true
        ghi last_event_type = "schedule_auto_arm"
        tính lại system_armed theo known_device_present

    nếu đến auto_disarm_hour:auto_disarm_minute:
        alarm_enabled = false
        system_armed = false
        alarm_status = DISARMED
        ghi last_event_type = "schedule_auto_disarm"
```

## A.4 Logic phát hiện đột nhập hiện tại

```text
Chỉ tính intrusion_score khi system_armed = true

intrusion_score =
    pir_detected
  + object_near
  + light_abnormal
  + night_mode
  + unknown_wifi_factor

unknown_wifi_factor = 1 nếu:
    unknown_wifi_detection_enabled = true
    unknown_wifi_alert = true
    có ít nhất một tín hiệu vật lý đáng nghi

Nếu tín hiệu đáng nghi vừa mất:
    giữ điểm thêm 5 giây

Ngưỡng theo sensitivity_level:
    1 -> intrusion_score >= 4
    2 -> intrusion_score >= 3
    3 -> intrusion_score >= 2

Nếu intrusion_score đạt ngưỡng:
    intrusion_alert = true
    alarm_status = INTRUSION_ALERT
    buzzer_on = true
    led_red_on = true
    gửi thông báo
    tự chụp 1 ảnh nếu đây là cảnh báo mới
```


## A.5 Logic Threat Level hiện tại

```text
threat_level là mức nguy hiểm tổng thể
không phải bản sao của intrusion_score

threat_level hợp lệ: 0..4

Nếu nhiều nguy cơ xảy ra cùng lúc:
    threat_level = max(level của từng nguy cơ)

Mapping chính:
    0 = bình thường
    1 = cần chú ý
    2 = nghi ngờ trung bình
    3 = nguy hiểm cao
    4 = khẩn cấp

Nếu intrusion_score = threshold - 1 và chưa intrusion_alert:
    threat_level = 2

Nếu intrusion_alert = true:
    threat_level = 3

Nếu SOS hoặc sabotage nghiêm trọng:
    threat_level = 4
```

## A.6 Logic Pet / False Positive hiện tại

```text
pet_detected không có nghĩa chắc chắn là thú nuôi
pet_detected = nghi vật nhỏ/báo nhầm

Điều kiện:
    pir_detected xuất hiện ngắn dưới 5 giây
    object_near xuất hiện ngắn dưới 5 giây
    light_abnormal = false
    unknown_wifi_alert = false
    intrusion_alert = false

Nếu pet_detected = true:
    intrusion_score = max(0, intrusion_score - 1)

Nếu chỉ là pet/báo nhầm:
    threat_level = 1
    last_event_type = "pet_or_false_positive"
    không gửi Telegram
```

## A.7 Logic Telegram / Unknown WiFi hiện tại

```text
notification_channel mặc định = "telegram"

Telegram gửi cho:
    intrusion_alert
    sos_alert
    sabotage_alert
    unknown_wifi_alert
    schedule_config_error
    schedule_time_error
    photo_capture_failed

Telegram không gửi cho:
    pet_or_false_positive
    schedule_auto_arm
    schedule_auto_disarm
    alarm_reset thành công

Unknown WiFi gửi Telegram khi:
    unknown_wifi_detection_enabled = true
    cùng một MAC lạ xuất hiện >= 3 lần trong 60 giây
    MAC đó không đang trong cooldown 60 giây

Nếu nhiều MAC khác nhau xuất hiện nhưng không MAC nào lặp đủ 3 lần:
    chỉ dashboard/log
    không gửi Telegram

Nếu tắt unknown_wifi_detection_enabled:
    unknown_wifi_alert = false
    unknown_wifi_count = 0
    giữ last_unknown_mac để debug

Nếu bật lại unknown_wifi_detection_enabled:
    bắt đầu phiên đếm mới
    reset bộ đếm nội bộ
    không dùng dữ liệu cũ để gửi Telegram
```

---

# Kết thúc bản SRS checkpoint 0.3.0
