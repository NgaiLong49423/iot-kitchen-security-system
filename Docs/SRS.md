# Đặc Tả Yêu Cầu Phần Mềm (SRS)

# Hệ Thống Chống Trộm IoT

> **Trạng thái tài liệu:** Bản checkpoint nháp  
> **Mục đích:** Đây là tài liệu nguồn hiện tại cho agent, chatbot, lập trình viên và người review. Các tài liệu cũ chỉ dùng để tham khảo lịch sử. Nếu có mâu thuẫn, tài liệu `SRS.md` này được ưu tiên.

---

## Lịch Sử Phiên Bản

| Phiên bản | Ngày tạo | Sửa đổi gần nhất | Người phụ trách | Tóm tắt thay đổi |
|---|---|---|---|---|
| 0.1.0 | 2026-06-26 | 2026-06-26 | Group 6 / Ngô Gia Long | Tạo bản SRS checkpoint đầu tiên sau khi làm sạch phạm vi và phân rã cụm Security Mode Control. Loại bỏ báo cháy và RFID-RC522 khỏi phạm vi chính. Thêm quy tắc tự tắt chống trộm khi phát hiện thiết bị đáng tin và cấu hình thời gian tự bật lại bảo vệ. |

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
| Trạng thái hiện tại | Bản checkpoint một phần; cụm Security Mode Control đã phân rã; các chức năng khác đang chờ phân rã |

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
9. Phiên bản hiện tại thiết kế cho một khu vực giám sát: phòng bếp.
10. Nội dung nên đủ rõ để triển khai, nhưng không biến thành mã nguồn thô.

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
| FR-01 | Điều khiển chế độ chống trộm | Đã phân rã trong checkpoint này |
| FR-02 | Tự tắt chống trộm khi phát hiện thiết bị đáng tin | Đã phân rã trong checkpoint này |
| FR-03 | Cấu hình thời gian tự bật lại bảo vệ | Đã phân rã trong checkpoint này |
| FR-04 | Xử lý reset báo động | Đã phân rã trong checkpoint này |
| FR-05 | Phát hiện đột nhập đa cảm biến | Chờ phân rã |
| FR-06 | Lọc báo động giả / vật nuôi | Chờ phân rã |
| FR-07 | Phát hiện phá hoại thiết bị | Chờ phân rã |
| FR-08 | Phát hiện thiết bị WiFi lạ | Chờ phân rã |
| FR-09 | SOS khẩn cấp qua Dashboard | Chờ phân rã |
| FR-10 | Chụp ảnh khi có sự kiện an ninh | Chờ phân rã |
| FR-11 | Gửi email / thông báo Cloud | Chờ phân rã |
| FR-12 | Ghi log sự kiện | Chờ phân rã |
| FR-13 | Chống spam cảnh báo / cooldown | Chờ phân rã |
| FR-14 | Chế độ demo | Chờ phân rã |
| FR-15 | Hỗ trợ Arduino Cloud Dashboard | Chờ phân rã |

## 2.2 Nhóm người dùng

| Vai trò | Mô tả | Quyền chính |
|---|---|---|
| Trẻ em | Người dùng chỉ cần thao tác SOS đơn giản | Bấm SOS, xem trạng thái cơ bản |
| Người lớn / Phụ huynh | Người dùng chính trong nhà | Bật/tắt chống trộm, reset báo động, bấm SOS, cấu hình thời gian tự bật lại, xem cảnh báo |
| Admin / Người demo | Leader, tester hoặc người thuyết trình | Quan sát toàn bộ, điều khiển demo, xem biến debug, xem countdown |
| Hệ thống / Board | Thiết bị nhúng chạy logic cảm biến | Đọc cảm biến, cập nhật biến, kích hoạt cảnh báo, ghi log |
| Dịch vụ thông báo bên ngoài | Email hoặc dịch vụ Cloud | Nhận loại sự kiện và gửi thông báo |

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
| Google Apps Script hoặc dịch vụ tương đương | Gửi email/thông báo |

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

## 3.3 Sự hiện diện của thiết bị đáng tin

Hệ thống dùng phát hiện thiết bị đáng tin để quyết định chức năng chống trộm của khu vực có nên hoạt động hay không.

Nếu thiết bị đáng tin xuất hiện, hệ thống không báo đột nhập cho khu vực đó.

Chức năng này tồn tại để giảm cảnh báo thừa khi người nhà ở nhà. Nó không nhằm mục đích chính là tăng độ chính xác phát hiện trộm.

---

# 4. Yêu Cầu Giao Diện Bên Ngoài

## 4.1 Yêu cầu giao diện người dùng

Hệ thống cần có ba loại dashboard.

| Dashboard | Người dùng | Mục đích chính | Biến chính |
|---|---|---|---|
| Child SOS Dashboard | Trẻ em | Gửi SOS đơn giản và xem trạng thái cơ bản | `sos_child`, `alarm_status`, `last_event` |
| Parent Control Dashboard | Người lớn | Điều khiển chống trộm, reset, SOS, cấu hình re-arm, xem cảnh báo quan trọng | `alarm_enabled`, `sos_adult`, `reset_alarm`, `alarm_status`, `last_event`, `threat_level`, `email_sent_status`, `capture_photo`, `photo_status`, `rearm_delay_seconds` |
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
| Dịch vụ email/thông báo | Hệ thống phải gửi thông báo cho các loại sự kiện được cấu hình. |
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
| SOS | `sos_child` | bool | Read & Write | On Change | Child, Admin | Nút SOS của trẻ em |
| SOS | `sos_adult` | bool | Read & Write | On Change | Parent, Admin | Nút SOS của người lớn |
| SOS | `sos_level` | int | Read Only | On Change | Admin | Mức độ nghiêm trọng của SOS |
| SOS | `sos_message` | String | Read Only | On Change | Parent, Admin | Nội dung SOS do hệ thống tạo |
| Chuyển động / Khoảng cách | `pir_detected` | bool | Read Only | On Change | Admin | Trạng thái phát hiện chuyển động từ PIR |
| Chuyển động / Khoảng cách | `ultrasonic_distance` | float | Read Only | Periodic | Admin | Khoảng cách đo từ cảm biến siêu âm |
| Chuyển động / Khoảng cách | `object_near` | bool | Read Only | On Change | Admin | Có vật thể trong vùng theo dõi hay không |
| Chuyển động / Khoảng cách | `pet_detected` | bool | Read Only | On Change | Admin | Hệ thống nghi ngờ vật thể là thú nuôi hay không |
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
| WiFi đáng tin / WiFi lạ | `unknown_wifi_count` | int | Read Only | Periodic | Admin | Số lần phát hiện thiết bị WiFi lạ |
| WiFi đáng tin / WiFi lạ | `unknown_wifi_alert` | bool | Read Only | On Change | Parent, Admin | Cảnh báo thiết bị WiFi lạ đáng nghi |
| WiFi đáng tin / WiFi lạ | `last_unknown_mac` | String | Read Only | On Change | Admin | MAC lạ gần nhất phát hiện được |
| Camera | `capture_photo` | bool | Read & Write | On Change | Parent, Admin | Yêu cầu chụp ảnh thủ công hoặc theo sự kiện |
| Camera | `photo_status` | String | Read Only | On Change | Parent, Admin | Trạng thái thao tác chụp ảnh |
| Đầu ra cảnh báo | `buzzer_on` | bool | Read Only | On Change | Parent, Admin | Trạng thái còi |
| Đầu ra cảnh báo | `led_red_on` | bool | Read Only | On Change | Parent, Admin | Trạng thái LED đỏ |
| Thông báo | `send_email_request` | bool | Read Only | On Change | Admin | Hệ thống yêu cầu gửi email/thông báo |
| Thông báo | `email_event_type` | String | Read Only | On Change | Admin | Loại sự kiện dùng cho nội dung thông báo |
| Thông báo | `email_sent_status` | String | Read Only | On Change | Parent, Admin | Trạng thái gửi email/thông báo |
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
4. không gửi email/thông báo do lý do đột nhập,
5. vẫn tiếp tục hỗ trợ:
   - SOS,
   - chống phá hoại,
   - reset,
   - cập nhật dashboard,
   - đồng bộ Cloud,
   - ghi log,
   - chụp ảnh thủ công nếu được hỗ trợ.

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

## FR-03. Phát hiện đột nhập đa cảm biến

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách PIR, HC-SR04, LDR, `intrusion_score` và `threat_level` được dùng để xác nhận đột nhập.

Agent không được tự triển khai logic đột nhập cuối cùng chỉ dựa trên placeholder này.

## FR-04. Lọc báo động giả / vật nuôi

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách hệ thống giảm báo động giả do thú nuôi hoặc vật thể thấp.

Agent không được tự triển khai logic lọc vật nuôi cuối cùng chỉ dựa trên placeholder này.

## FR-05. Phát hiện phá hoại thiết bị

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách hệ thống phát hiện hành vi che cảm biến, chặn cảm biến hoặc can thiệp vật lý.

Quy tắc hiện tại đã chốt:

```text
Anti-sabotage không phụ thuộc system_armed.
```

## FR-06. Phát hiện thiết bị WiFi lạ

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa cách thiết bị WiFi lạ được đếm, đánh giá, ghi log và chuyển thành cảnh báo.

## FR-07. SOS khẩn cấp qua Dashboard

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa SOS trẻ em, SOS người lớn, mức SOS, đầu ra cảnh báo, thông báo và reset.

Quy tắc hiện tại đã chốt:

```text
SOS không phụ thuộc system_armed.
```

## FR-08. Chụp ảnh

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa hành vi chụp ảnh thủ công và chụp ảnh theo sự kiện.

## FR-09. Gửi email / thông báo Cloud

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa sự kiện nào kích hoạt email/thông báo và cách ánh xạ loại sự kiện.

Loại thông báo `fire` không được dùng trong phạm vi hiện tại.

## FR-10. Ghi log sự kiện

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa định dạng log, cách tăng `event_counter` và sự kiện nào được xem là có ý nghĩa.

## FR-11. Chống spam cảnh báo / cooldown

**Trạng thái:** Chờ phân rã

Chức năng này sẽ định nghĩa hành vi cooldown cho các cảnh báo lặp lại.

## FR-12. Chế độ demo

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

---

# 11. Giả Định Và Phụ Thuộc

## 11.1 Giả định

1. Phòng bếp là khu vực giám sát đầu tiên.
2. Người lớn/Admin có thể truy cập Arduino Cloud Dashboard.
3. Trẻ em không được chỉnh cấu hình hệ thống.
4. Phát hiện thiết bị đáng tin được dùng để giảm báo động thừa khi người nhà ở nhà.
5. Phát hiện thiết bị đáng tin có thể dựa trên WiFi/MAC, nhưng cách triển khai chi tiết có thể được điều chỉnh sau.
6. `known_device_present = true` nghĩa là ít nhất một thiết bị đáng tin được phát hiện gần khu vực giám sát.

## 11.2 Phụ thuộc

| Phụ thuộc | Vai trò |
|---|---|
| Arduino Cloud | Đồng bộ biến và dashboard |
| WiFi | Đồng bộ Cloud, phát hiện thiết bị đáng tin/lạ, gửi thông báo |
| Cảm biến | Phát hiện đột nhập và phá hoại |
| Dịch vụ thông báo | Gửi email hoặc cảnh báo bên ngoài |
| RTC module | Ghi thời gian sự kiện và hỗ trợ logic theo thời gian |

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

**Production** nghĩa là môi trường vận hành thật, yêu cầu độ ổn định và bảo mật cao hơn demo/đồ án.

---

# 13. Vấn Đề Cần Phân Rã Tiếp

Các câu hỏi sau vẫn đang mở và cần phân rã trước khi có bản SRS cuối.

| Vấn đề mở | Phần liên quan |
|---|---|
| Phát hiện đột nhập nên dùng điều kiện cứng AND/OR hay `intrusion_score`? | Multi-Sensor Intrusion Detection |
| `threat_level` nên được tính như thế nào? | Intrusion / Notification |
| `pet_detected` nên được xác định như thế nào? | False Positive Filtering |
| Điều kiện chính xác nào kích hoạt `sabotage_alert`? | Anti-Sabotage |
| Bao nhiêu lần phát hiện WiFi lạ thì kích hoạt `unknown_wifi_alert`? | Unknown WiFi Detection |
| SOS trẻ em khác SOS người lớn như thế nào? | Emergency SOS |
| Sự kiện nào kích hoạt chụp ảnh? | Photo Capture |
| Sự kiện nào kích hoạt email/thông báo? | Notification |
| Định dạng log sự kiện là gì? | Event Logging |
| Cooldown cảnh báo kéo dài bao lâu? | Anti-Spam |
| Demo mode mô phỏng hoặc override dữ liệu cảm biến như thế nào? | Demo Mode |

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

---

# Kết thúc bản SRS checkpoint
