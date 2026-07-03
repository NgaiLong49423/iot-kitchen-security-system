# Đề xuất kịch bản trình diễn dự án IoT Anti-Theft

> **Tên tài liệu:** DEMO_SCENARIOS_PROPOSAL.md  
> **Phiên bản:** v0.1.0  
> **Ngày tạo:** 2026-07-03  
> **Trạng thái:** Bản nháp để chọn/chốt kịch bản demo  
> **Dự án:** Hệ thống chống trộm IoT dùng Freenove ESP32-S3 WROOM + Camera OV3660  
> **Mục đích:** Tổng hợp các kịch bản trình diễn có khả năng ăn điểm cao trước hội đồng/khách hàng.

---

## 1. Cách hiểu về chế độ demo

Trong tài liệu này, **demo** không có nghĩa là giả lập toàn bộ hệ thống.

**Demo Mode** nên được hiểu là:

```text
Chế độ trình diễn có kiểm soát để khách hàng/hội đồng thấy thiết bị hoạt động như thế nào.
```

Demo tốt phải ưu tiên dùng:

- cảm biến thật,
- còi/LED thật,
- dashboard thật,
- Telegram/notification thật,
- camera thật,
- reset thật,
- log/trạng thái thật.

Chỉ các đầu vào khó kiểm soát trong môi trường demo mới nên dùng **controlled demo input**.

**Controlled demo input** nghĩa là **đầu vào demo có kiểm soát**, ví dụ Admin bật/tắt `known_device_present` trên dashboard thay vì phụ thuộc vào việc quét điện thoại thật trong lớp học.

---

## 2. Nguyên tắc chọn kịch bản ăn điểm

Một kịch bản demo tốt nên có ít nhất 3 yếu tố:

| Yếu tố | Ý nghĩa |
|---|---|
| Có bối cảnh thật | Người xem hiểu tình huống này có thể xảy ra ngoài đời |
| Có phản ứng vật lý | Còi, LED, cảm biến, camera tạo cảm giác thiết bị chạy thật |
| Có logic thông minh | Không chỉ đọc cảm biến đơn giản, mà có rule, score, cooldown, reset, hoặc phân quyền |
| Có dashboard/cloud | Thể hiện IoT thật, không chỉ mạch điện local |
| Có cảnh báo từ xa | Telegram/email giúp sản phẩm giống thiết bị thương mại |
| Có xử lý sau cảnh báo | Reset, log, trạng thái, không để hệ thống treo |

---

## 3. Bộ 6 kịch bản nên ưu tiên chốt

Đây là bộ 6 kịch bản tôi đề xuất làm chính.

| Mã | Kịch bản | Độ ăn điểm | Lý do nên chọn |
|---|---|---:|---|
| DS-01 | Tự bật chống trộm ban đêm và phát hiện trộm dùng đèn pin | 10/10 | Có lịch tự động, night mode, LDR, PIR, siêu âm, camera, Telegram |
| DS-02 | Người nhà về muộn ban đêm, hệ thống không báo nhầm | 9.5/10 | Rất giống sản phẩm thông minh dùng trong gia đình |
| DS-03 | Đột nhập thật với chuỗi báo động đầy đủ đa cảm biến | 10/10 | Thể hiện lõi chống trộm mạnh nhất |
| DS-04 | Trộm cố che/phá cảm biến trước khi đột nhập | 9.5/10 | Thể hiện tư duy security design, không chỉ đọc cảm biến cơ bản |
| DS-05 | Lọc báo động giả/vật nhỏ | 9/10 | Chứng minh hệ thống không hú bừa, phù hợp sản phẩm bán được |
| DS-06 | SOS trẻ em và xác nhận khẩn cấp | 9/10 | Có yếu tố gia đình, khẩn cấp, dashboard theo vai trò |

---

# DS-01 — Tự bật chống trộm ban đêm và phát hiện trộm dùng đèn pin

## 1. Tên kịch bản

```text
Night Scheduled Auto-Arm and Flashlight Intrusion Detection
```

Tên tiếng Việt:

```text
Tự bật chống trộm ban đêm và phát hiện trộm dùng đèn pin
```

## 2. Bối cảnh

Buổi tối, gia đình đi ngủ. Người dùng đã đặt lịch để hệ thống tự bật chống trộm vào một khung giờ cố định, ví dụ 23:00.

Đến đúng giờ, hệ thống tự bật chế độ chống trộm. Sau đó có người lạ đột nhập vào khu vực bếp và dùng đèn pin soi trong phòng. Hệ thống phát hiện chuyển động, vật thể gần và ánh sáng thay đổi bất thường rồi kích hoạt cảnh báo.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| Lịch tự động | `schedule_enabled`, `auto_arm_hour`, `auto_arm_minute` |
| Trạng thái bảo vệ | `alarm_enabled`, `system_armed`, `alarm_status` |
| Ban đêm | `night_mode`, `current_time`, `current_hour` |
| Cảm biến | `pir_detected`, `object_near`, `ldr_value`, `ldr_delta`, `light_abnormal` |
| Đột nhập | `intrusion_score`, `intrusion_alert`, `threat_level` |
| Cảnh báo | `buzzer_on`, `led_red_on`, `led_green_on` |
| Camera | `auto_capture_photo_request`, `photo_status` |
| Thông báo | `send_notification_request`, `notification_event_type`, `notification_sent_status`, `notification_channel` |
| Reset | `reset_alarm` |

## 4. Luồng demo

1. Admin/Parent bật `schedule_enabled = true`.
2. Đặt `auto_arm_hour` và `auto_arm_minute` gần thời điểm demo.
3. Đến đúng giờ, hệ thống tự đặt:
   - `alarm_enabled = true`,
   - `system_armed = true`,
   - `alarm_status = ARMED`.
4. Người demo đóng vai trộm:
   - đi ngang PIR,
   - đưa người/vật lại gần cảm biến siêu âm,
   - dùng đèn pin soi vào LDR/khu vực bếp.
5. Hệ thống ghi nhận:
   - `pir_detected = true`,
   - `object_near = true`,
   - `light_abnormal = true`,
   - `night_mode = true`.
6. `intrusion_score` đạt ngưỡng.
7. Hệ thống kích hoạt:
   - `intrusion_alert = true`,
   - `alarm_status = INTRUSION_ALERT`,
   - `threat_level = 3`,
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - `led_green_on = false`.
8. Camera OV3660 tự chụp ảnh:
   - `photo_status = CAPTURING`,
   - sau đó `photo_status = CAPTURED` hoặc `FAILED`.
9. Telegram gửi cảnh báo đột nhập.
10. Parent/Admin bấm `reset_alarm` để tắt cảnh báo.

## 5. Điểm ăn điểm

| Điểm | Giải thích |
|---|---|
| Lịch tự động | Chứng minh thiết bị không cần bật tay mỗi tối |
| Night mode | Có ngữ cảnh ban đêm, giống tình huống thật |
| Đèn pin | Rất trực quan, người xem hiểu ngay trộm đang soi đồ |
| Sensor fusion | Hợp nhất nhiều cảm biến để giảm báo nhầm |
| Camera + Telegram | Có bằng chứng và cảnh báo từ xa |
| Reset | Có quy trình xử lý sau cảnh báo |

**Sensor fusion** nghĩa là **hợp nhất nhiều cảm biến để ra quyết định**.

## 6. Rủi ro demo

| Rủi ro | Cách giảm |
|---|---|
| RTC chưa đúng giờ | Đặt giờ demo sát thời điểm hiện tại hoặc dùng mode demo có kiểm soát |
| LDR không bắt rõ đèn pin | Test trước khoảng cách/độ sáng đèn pin |
| PIR delay | Cho PIR warm-up trước khi demo |
| Camera chụp chậm | Giảm độ phân giải ảnh khi demo |

---

# DS-02 — Người nhà về muộn ban đêm, hệ thống không báo nhầm

## 1. Tên kịch bản

```text
Trusted Device Late-Night Return and Auto Re-arm
```

Tên tiếng Việt:

```text
Người nhà về muộn ban đêm, hệ thống không báo nhầm
```

## 2. Bối cảnh

Buổi tối, hệ thống đã tự bật chống trộm. Một người nhà về muộn và đi qua khu vực bếp. Vì thiết bị đáng tin được nhận diện, hệ thống tạm tắt bảo vệ khu vực để không báo nhầm.

Sau khi người nhà rời đi, hệ thống tự đếm ngược và bật lại chống trộm.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| Thiết bị đáng tin | `known_device_present` |
| Bảo vệ | `alarm_enabled`, `system_armed`, `alarm_status` |
| Tự bật lại | `rearm_delay_seconds`, `rearm_countdown_remaining` |
| Cảm biến | `pir_detected`, `object_near` |
| Reset/log | `last_event`, `last_event_type`, `event_counter` |

## 4. Luồng demo

1. Hệ thống đang ở trạng thái:
   - `alarm_enabled = true`,
   - `system_armed = true`,
   - `alarm_status = ARMED`.
2. Admin bật `known_device_present = true`.
3. Hệ thống chuyển:
   - `system_armed = false`,
   - `alarm_status = DISARMED_BY_TRUSTED_DEVICE`.
4. Người demo đi qua PIR/siêu âm.
5. Hệ thống không kích hoạt `intrusion_alert`.
6. Admin tắt `known_device_present = false`.
7. Hệ thống bắt đầu đếm:
   - `rearm_countdown_remaining`.
8. Hết countdown, hệ thống chuyển:
   - `system_armed = true`,
   - `alarm_status = ARMED`.
9. Sau khi armed lại, nếu có tín hiệu đột nhập thật thì hệ thống báo động.

## 5. Điểm ăn điểm

| Điểm | Giải thích |
|---|---|
| Không báo nhầm người nhà | Rất đúng nhu cầu khách hàng |
| Tự re-arm | Cho thấy hệ thống thông minh và tự động |
| Dashboard rõ ràng | Người xem thấy trạng thái thay đổi trực tiếp |
| Có logic trạng thái | Không chỉ bật/tắt còi đơn giản |

## 6. Ghi chú

Nếu phát hiện thiết bị đáng tin thật chưa ổn định, có thể dùng `known_device_present` từ Admin Dashboard như **controlled demo input**.

---

# DS-03 — Đột nhập thật với chuỗi báo động đầy đủ đa cảm biến

## 1. Tên kịch bản

```text
Multi-Sensor Intrusion Full Alarm Chain
```

Tên tiếng Việt:

```text
Đột nhập thật với chuỗi báo động đầy đủ đa cảm biến
```

## 2. Bối cảnh

Nhà đang ở trạng thái bảo vệ. Người lạ đột nhập vào khu vực bếp. Hệ thống dùng nhiều tín hiệu cùng lúc để xác nhận đột nhập, sau đó bật còi/LED, gửi Telegram, chụp ảnh và ghi log.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| Cảm biến | `pir_detected`, `object_near`, `light_abnormal` |
| Điểm nghi ngờ | `intrusion_score`, `sensitivity_level` |
| Cảnh báo | `intrusion_alert`, `alarm_status`, `threat_level` |
| Đầu ra | `buzzer_on`, `led_red_on`, `led_green_on` |
| Camera | `auto_capture_photo_request`, `photo_status` |
| Telegram | `send_notification_request`, `notification_event_type`, `notification_sent_status` |
| Log | `last_event`, `last_event_type`, `event_counter` |
| Reset | `reset_alarm` |

## 4. Luồng demo

1. Bật `alarm_enabled = true`.
2. Đảm bảo `system_armed = true`.
3. Người demo tạo tín hiệu đột nhập:
   - đi qua PIR,
   - tiến gần siêu âm,
   - tạo ánh sáng bất thường hoặc che cảm biến theo tình huống.
4. Hệ thống tăng `intrusion_score`.
5. Khi đủ ngưỡng:
   - `intrusion_alert = true`,
   - `alarm_status = INTRUSION_ALERT`,
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - `led_green_on = false`.
6. Telegram gửi cảnh báo.
7. Camera chụp ảnh tự động.
8. Event log cập nhật.
9. Parent/Admin bấm `reset_alarm`.

## 5. Điểm ăn điểm

Đây là kịch bản lõi của sản phẩm. Nó chứng minh hệ thống thật sự chống trộm, không chỉ có dashboard.

---

# DS-04 — Trộm cố che/phá cảm biến trước khi đột nhập

## 1. Tên kịch bản

```text
Anti-Sabotage Before Intrusion
```

Tên tiếng Việt:

```text
Trộm cố che/phá cảm biến trước khi đột nhập
```

## 2. Bối cảnh

Kẻ xấu phát hiện có thiết bị chống trộm nên cố tình che cảm biến ánh sáng hoặc đặt vật sát cảm biến để vô hiệu hóa hệ thống. Hệ thống nhận ra hành vi phá hoại và báo động.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| LDR | `ldr_covered` |
| Khoảng cách | `object_near` |
| Sabotage | `sabotage_alert`, `device_tampered` |
| Cảnh báo | `alarm_status`, `threat_level` |
| Đầu ra | `buzzer_on`, `led_red_on`, `led_green_on` |
| Telegram | `notification_event_type`, `notification_sent_status` |
| Reset | `reset_alarm` |

## 4. Luồng demo

1. Nói rõ: Anti-Sabotage hoạt động độc lập với `alarm_enabled` và `system_armed`.
2. Che LDR liên tục khoảng 5 giây hoặc đưa vật sát cảm biến siêu âm 5 giây.
3. Hệ thống chuyển:
   - `sabotage_alert = true`,
   - `alarm_status = SABOTAGE_ALERT`,
   - `threat_level = 4`,
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - `led_green_on = false`.
4. Telegram gửi cảnh báo sabotage.
5. Bấm `reset_alarm`.

## 5. Điểm ăn điểm

| Điểm | Giải thích |
|---|---|
| Có tư duy an ninh thật | Tính đến việc kẻ xấu cố phá thiết bị |
| Độc lập với chống trộm | Dù tắt chống trộm, sabotage vẫn chạy |
| Rất trực quan | Che cảm biến là người xem hiểu ngay |

**Anti-sabotage** nghĩa là **chống phá hoại/can thiệp thiết bị**.

---

# DS-05 — Lọc báo động giả/vật nhỏ

## 1. Tên kịch bản

```text
False Positive / Pet-Like Motion Filtering
```

Tên tiếng Việt:

```text
Lọc báo động giả hoặc chuyển động giống vật nhỏ
```

## 2. Bối cảnh

Có chuyển động ngắn hoặc vật nhỏ đi ngang khu vực bếp. Hệ thống không báo động ngay mà đánh giá tín hiệu giống vật nhỏ/báo nhầm.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| Cảm biến | `pir_detected`, `object_near`, `light_abnormal` |
| Lọc báo nhầm | `pet_detected` |
| Điểm | `intrusion_score`, `sensitivity_level` |
| Mức nguy hiểm | `threat_level` |
| Log | `last_event`, `last_event_type` |
| Telegram | Không gửi Telegram khẩn cấp nếu chưa có `intrusion_alert` |

## 4. Luồng demo

1. Bật `alarm_enabled = true`.
2. Đảm bảo `system_armed = true`.
3. Tạo tín hiệu ngắn dưới 5 giây:
   - vật nhỏ đi ngang,
   - hoặc chuyển động rất ngắn.
4. Hệ thống đặt:
   - `pet_detected = true`,
   - `threat_level = 1`,
   - `last_event_type = pet_or_false_positive`.
5. Hệ thống không bật còi.
6. Hệ thống không gửi Telegram khẩn cấp.
7. Sau đó tạo tín hiệu mạnh hơn để chứng minh hệ thống vẫn báo đột nhập thật khi đủ bằng chứng.

## 5. Điểm ăn điểm

Kịch bản này rất quan trọng vì sản phẩm chống trộm thương mại phải tránh báo động giả.

**False positive** nghĩa là **báo động giả**, tức báo nguy hiểm dù thực tế không nguy hiểm.

---

# DS-06 — SOS trẻ em và xác nhận khẩn cấp

## 1. Tên kịch bản

```text
Child SOS and Emergency Escalation
```

Tên tiếng Việt:

```text
SOS trẻ em và xác nhận khẩn cấp
```

## 2. Bối cảnh

Trẻ em ở nhà gặp tình huống nguy hiểm và bấm SOS trên dashboard đơn giản. Hệ thống bật cảnh báo tại chỗ, gửi notification cho Parent/Admin và yêu cầu xác nhận escalation.

## 3. Biến/chức năng liên quan

| Nhóm | Biến/chức năng |
|---|---|
| SOS | `sos_child`, `sos_adult`, `sos_message` |
| Escalation | `emergency_confirmation_requested`, `emergency_confirmed`, `emergency_escalation_status` |
| Cảnh báo | `alarm_status`, `threat_level` |
| Đầu ra | `buzzer_on`, `led_red_on`, `led_green_on` |
| Notification | `notification_event_type`, `notification_sent_status` |
| Reset | `reset_alarm` |

## 4. Luồng demo

1. Mở Child SOS Dashboard.
2. Bấm `sos_child = true`.
3. Hệ thống chuyển:
   - `alarm_status = SOS_ALERT`,
   - `threat_level = 4`,
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - `led_green_on = false`,
   - `last_event_type = sos_alert`.
4. Parent/Admin nhận notification.
5. Google Apps Script gửi email xác nhận.
6. Parent/Admin xác nhận.
7. `emergency_escalation_status` chuyển qua các trạng thái:
   - `WAITING_CONFIRMATION`,
   - `CONFIRMED`,
   - `SENT`.
8. Parent/Admin bấm `reset_alarm`.

## 5. Điểm ăn điểm

| Điểm | Giải thích |
|---|---|
| Có dashboard theo vai trò | Trẻ em chỉ cần một nút SOS đơn giản |
| Có thông báo từ xa | Parent/Admin nhận cảnh báo |
| Có escalation | Tình huống khẩn cấp có quy trình xử lý |
| Có yếu tố gia đình | Dễ thuyết phục khách hàng |

**Escalation** nghĩa là **leo thang xử lý khẩn cấp**.

---

# DS-07 — Phụ huynh kiểm tra bếp từ xa bằng camera

## 1. Tên kịch bản

```text
Remote Manual Camera Check
```

Tên tiếng Việt:

```text
Phụ huynh kiểm tra bếp từ xa bằng camera
```

## 2. Bối cảnh

Phụ huynh đang ở ngoài nhà. Dashboard báo có tín hiệu nghi ngờ nhẹ hoặc phụ huynh muốn kiểm tra bếp. Người dùng bấm chụp ảnh thủ công từ dashboard.

## 3. Luồng demo

1. Mở Parent Dashboard.
2. Bấm `manual_capture_photo = true`.
3. Hệ thống chuyển:
   - `photo_status = CAPTURING`.
4. Camera OV3660 chụp ảnh.
5. Hệ thống chuyển:
   - `photo_status = CAPTURED`.
6. Nếu thất bại:
   - `photo_status = FAILED`,
   - `last_event_type = photo_capture_failed`.

## 4. Điểm ăn điểm

Kịch bản này làm sản phẩm giống thiết bị thương mại hơn vì người dùng có thể kiểm tra nhà từ xa.

## 5. Vị trí khuyến nghị

Không nên để DS-07 là kịch bản chính độc lập nếu thời gian trình bày ngắn. Nên gộp vào DS-01 hoặc DS-03 như một bước phụ.

---

# DS-08 — Mất Internet nhưng cảnh báo tại chỗ vẫn hoạt động

## 1. Tên kịch bản

```text
Offline Local Alarm Resilience
```

Tên tiếng Việt:

```text
Mất Internet nhưng cảnh báo tại chỗ vẫn hoạt động
```

## 2. Bối cảnh

WiFi/Cloud bị mất tạm thời. Trong lúc đó, trộm vẫn đột nhập. Hệ thống không gửi Telegram được ngay nhưng còi và LED vẫn hoạt động tại chỗ.

## 3. Luồng demo

1. Cho hệ thống đang armed.
2. Ngắt WiFi hoặc giả lập trạng thái notification fail.
3. Tạo tín hiệu đột nhập.
4. Hệ thống vẫn bật:
   - `buzzer_on = true`,
   - `led_red_on = true`,
   - `alarm_status = INTRUSION_ALERT`.
5. Notification có thể:
   - `notification_sent_status = FAILED`,
   - hoặc `PENDING`.
6. Khi mạng ổn định lại, hệ thống cập nhật Cloud/notification nếu có triển khai retry.

## 4. Điểm ăn điểm

Đây là kịch bản rất chuyên nghiệp vì sản phẩm thật không nên mất mạng là vô dụng.

## 5. Rủi ro

Không nên chọn làm kịch bản chính nếu chưa code retry WiFi ổn định, vì ngắt/mở WiFi khi demo dễ rủi ro.

---

# DS-09 — WiFi/MAC lạ là cảnh báo sớm, không báo bừa

## 1. Tên kịch bản

```text
Unknown WiFi/MAC Pre-Warning
```

Tên tiếng Việt:

```text
WiFi/MAC lạ là cảnh báo sớm, không báo bừa
```

## 2. Bối cảnh

Có thiết bị WiFi/MAC lạ gần khu vực bếp. Hệ thống không báo động ngay, chỉ ghi nhận cảnh báo sớm. Khi MAC lạ đi kèm cảm biến vật lý đáng nghi, hệ thống mới tăng mức nguy cơ.

## 3. Luồng demo

Pha 1 — chỉ có MAC lạ:

1. Bật `unknown_wifi_detection_enabled = true`.
2. Tạo hoặc mô phỏng `unknown_wifi_alert = true`.
3. Dashboard hiện:
   - `unknown_wifi_count`,
   - `last_unknown_mac`,
   - `threat_level = 1`.
4. Hệ thống không bật còi.
5. Hệ thống không đặt `intrusion_alert = true`.

Pha 2 — MAC lạ + cảm biến vật lý:

1. Tạo thêm `pir_detected = true` hoặc `object_near = true`.
2. `intrusion_score` tăng.
3. Nếu đủ ngưỡng, hệ thống kích hoạt intrusion alert.

## 4. Điểm ăn điểm

Kịch bản này chứng minh hệ thống không báo bừa chỉ vì xung quanh có thiết bị lạ.

## 5. Rủi ro

Nhiều điện thoại hiện nay có **MAC randomization**, nghĩa là **tự đổi MAC để bảo vệ quyền riêng tư**, nên demo thật có thể nhiễu. Nên dùng controlled demo input nếu chọn kịch bản này.

---

# DS-10 — Bàn giao/lắp đặt: kỹ thuật viên test nhanh toàn bộ thiết bị

## 1. Tên kịch bản

```text
Installation and Handover Test Mode
```

Tên tiếng Việt:

```text
Bàn giao/lắp đặt: kỹ thuật viên test nhanh toàn bộ thiết bị
```

## 2. Bối cảnh

Kỹ thuật viên lắp thiết bị tại nhà khách hàng. Trước khi bàn giao, kỹ thuật viên bật chế độ demo/test để kiểm tra cảm biến, còi, LED, camera và Telegram.

## 3. Luồng demo

1. Admin bật `demo_mode = true`.
2. Chọn `demo_scenario`.
3. Test từng nhóm:
   - PIR,
   - siêu âm,
   - LDR,
   - buzzer,
   - LED đỏ/xanh,
   - camera,
   - Telegram.
4. Sau khi bàn giao:
   - `demo_mode = false`,
   - hệ thống quay về vận hành thật.

## 4. Điểm ăn điểm

Kịch bản này giúp giải thích vì sao hệ thống cần `demo_mode`: không phải giả lập để che lỗi, mà là công cụ trình diễn/bàn giao có kiểm soát.

## 5. Vị trí khuyến nghị

Nên để kịch bản này ở phần phụ hoặc phần giới thiệu chế độ demo, không nên làm top 6 chính.

---

# DS-11 — Lỗi cấu hình lịch tự động được xử lý an toàn

## 1. Tên kịch bản

```text
Schedule Configuration Safety Check
```

Tên tiếng Việt:

```text
Lỗi cấu hình lịch tự động được xử lý an toàn
```

## 2. Bối cảnh

Người dùng cấu hình giờ tự bật và tự tắt chống trộm trùng nhau, hoặc nhập giờ/phút không hợp lệ. Hệ thống không tự bật/tắt bừa mà ghi lỗi rõ ràng.

## 3. Luồng demo

1. Bật `schedule_enabled = true`.
2. Đặt:
   - `auto_arm_hour:auto_arm_minute`,
   - trùng với `auto_disarm_hour:auto_disarm_minute`.
3. Hệ thống không đổi `alarm_enabled`.
4. Hệ thống cập nhật:
   - `last_event_type = schedule_config_error`,
   - `alarm_status = ERROR` nếu cần.
5. Telegram có thể gửi cảnh báo lỗi cấu hình nếu SRS yêu cầu.

## 4. Điểm ăn điểm

Cho thấy hệ thống có xử lý lỗi cấu hình, không chạy bừa.

## 5. Vị trí khuyến nghị

Nên để làm test case kỹ thuật, không nên đưa vào top 6 demo chính vì ít hấp dẫn khách hàng.

---

# DS-12 — Camera lỗi nhưng hệ thống không sập

## 1. Tên kịch bản

```text
Camera Failure Handling
```

Tên tiếng Việt:

```text
Camera lỗi nhưng hệ thống không sập
```

## 2. Bối cảnh

Khi có đột nhập, camera chụp ảnh thất bại vì lỗi kết nối, thiếu PSRAM hoặc lỗi khởi tạo camera. Hệ thống vẫn giữ cảnh báo chính, không bị treo.

## 3. Luồng demo

1. Tạo intrusion alert.
2. Camera thử chụp ảnh.
3. Nếu chụp thất bại:
   - `photo_status = FAILED`,
   - `last_event_type = photo_capture_failed`,
   - `notification_event_type = photo_capture_failed` nếu cần.
4. Còi/LED vẫn hoạt động.
5. `intrusion_alert` vẫn giữ.
6. Parent/Admin vẫn reset được.

## 4. Điểm ăn điểm

Đây là kịch bản xử lý lỗi tốt, cho thấy hệ thống có tính ổn định.

## 5. Vị trí khuyến nghị

Không nên làm kịch bản chính nếu camera đang chạy ổn. Chỉ dùng nếu hội đồng hỏi: “Nếu camera lỗi thì sao?”

---

# DS-13 — Cảnh báo ưu tiên khi nhiều sự kiện xảy ra cùng lúc

## 1. Tên kịch bản

```text
Alarm Priority Handling
```

Tên tiếng Việt:

```text
Xử lý ưu tiên khi nhiều cảnh báo xảy ra cùng lúc
```

## 2. Bối cảnh

Đang có cảnh báo đột nhập thì trẻ em bấm SOS hoặc thiết bị bị phá hoại. Hệ thống phải ưu tiên trạng thái nguy hiểm hơn thay vì ghi đè bừa.

## 3. Luồng demo gợi ý

1. Tạo `intrusion_alert = true`.
2. Khi intrusion đang active, bấm `sos_child = true`.
3. Hệ thống phải ưu tiên:
   - `alarm_status = SOS_ALERT`,
   - `threat_level = 4`.
4. Log vẫn nên ghi nhận sự kiện SOS.
5. Reset tổng bằng `reset_alarm`.

## 4. Điểm ăn điểm

Kịch bản này thể hiện tư duy hệ thống và trạng thái. Tuy nhiên, nó khó hơn, dễ phát sinh tranh luận nếu chưa code kỹ.

## 5. Vị trí khuyến nghị

Để làm kịch bản nâng cao hoặc trả lời câu hỏi của hội đồng, không nên đưa vào top 6 chính nếu thời gian gấp.

---

## 4. Bảng so sánh để chọn nhanh

| Mã | Kịch bản | Nên làm chính? | Độ khó code | Độ rủi ro lúc demo | Độ ăn điểm |
|---|---|---:|---:|---:|---:|
| DS-01 | Auto-arm ban đêm + trộm soi đèn pin | Có | Trung bình | Trung bình | Rất cao |
| DS-02 | Người nhà về muộn + auto re-arm | Có | Trung bình | Thấp nếu dùng dashboard input | Rất cao |
| DS-03 | Đột nhập full chain | Có | Trung bình-cao | Trung bình | Rất cao |
| DS-04 | Anti-sabotage | Có | Trung bình | Thấp-trung bình | Rất cao |
| DS-05 | False positive / pet filtering | Có | Trung bình-cao | Trung bình | Cao |
| DS-06 | Child SOS + escalation | Có | Trung bình | Trung bình nếu email chậm | Cao |
| DS-07 | Manual camera check | Phụ | Trung bình | Trung bình | Khá cao |
| DS-08 | Mất Internet vẫn báo local | Phụ/nâng cao | Cao | Cao | Cao |
| DS-09 | Unknown WiFi/MAC pre-warning | Phụ/nâng cao | Cao | Cao | Khá cao |
| DS-10 | Installation/handover test mode | Phụ | Thấp-trung bình | Thấp | Trung bình |
| DS-11 | Schedule config error | Test kỹ thuật | Thấp | Thấp | Trung bình |
| DS-12 | Camera failure handling | Test kỹ thuật | Trung bình | Trung bình | Trung bình |
| DS-13 | Alarm priority handling | Nâng cao | Cao | Trung bình-cao | Cao |

---

## 5. Bộ kịch bản tôi khuyên chốt cuối cùng

Nếu chỉ được chọn 6 kịch bản, nên chọn:

```text
DS-01 — Tự bật chống trộm ban đêm và phát hiện trộm dùng đèn pin
DS-02 — Người nhà về muộn ban đêm, hệ thống không báo nhầm
DS-03 — Đột nhập thật với chuỗi báo động đầy đủ đa cảm biến
DS-04 — Trộm cố che/phá cảm biến trước khi đột nhập
DS-05 — Lọc báo động giả/vật nhỏ
DS-06 — SOS trẻ em và xác nhận khẩn cấp
```

## 6. Thứ tự trình bày khuyến nghị

Thứ tự nên kể như một câu chuyện sản phẩm:

```text
1. Ban đêm hệ thống tự bật chống trộm.
2. Người nhà về muộn, hệ thống nhận diện và không báo nhầm.
3. Người nhà rời đi, hệ thống tự re-arm.
4. Trộm vào bếp dùng đèn pin, hệ thống phát hiện và báo động.
5. Trộm cố che/phá cảm biến, hệ thống vẫn phát hiện sabotage.
6. Trẻ em bấm SOS, hệ thống gửi cảnh báo khẩn cấp.
```

Nếu cần đủ 6 scenario tách riêng, dùng đúng DS-01 đến DS-06 như trên.

---

## 7. Gợi ý chốt chiến lược demo

Nên chuẩn bị 2 tầng:

### Tầng chính — 6 kịch bản bắt buộc

Đây là phần trình bày trước hội đồng/khách hàng.

### Tầng backup — kịch bản phụ

Dùng khi hội đồng hỏi thêm:

- Nếu mất mạng thì sao? → DS-08.
- Nếu MAC lạ thì sao? → DS-09.
- Nếu camera lỗi thì sao? → DS-12.
- Nếu người dùng nhập lịch sai thì sao? → DS-11.
- Nếu nhiều cảnh báo cùng lúc thì sao? → DS-13.

---

## 8. Kết luận

Bộ kịch bản nên chốt không chỉ chứng minh thiết bị chạy được, mà còn chứng minh sản phẩm có khả năng bán cho khách hàng:

```text
Tự động bật bảo vệ
Không báo nhầm người nhà
Phát hiện đột nhập thật
Chống phá hoại
Giảm báo động giả
Hỗ trợ SOS khẩn cấp
```

Đây là bộ câu chuyện mạnh nhất cho dự án hiện tại.
