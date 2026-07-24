# Báo Cáo Rà Soát Flowchart

**Dự án:** Kitchen Security System - Group 6  
**Ngày rà soát:** 2026-07-12  
**Nguồn đối chiếu:** Firmware ESP32-S3 và Google Apps Script hiện hành  
**Kết luận:** Flowchart đúng cấu trúc tổng quát, nhưng cần sửa các điều kiện quyết định và các mốc thời gian dưới đây trước khi dùng cho demo/báo cáo.

## Các Điểm Cần Sửa

| STT | Nội dung trên flowchart | Trạng thái đúng của code | Cách sửa trên flowchart |
|---:|---|---|---|
| 1 | `Intrusion Score >= Threshold?` kích hoạt Intrusion Alert | `intrusion_score` chỉ dùng hiển thị/mức nguy hiểm. Cảnh báo đột nhập yêu cầu `alarm_enabled = true`, PIR phát hiện, vật trong vùng 15-50 cm và duy trì 2 giây. | Thay decision bằng: `PIR detected AND distance 15-50 cm for >= 2 s AND system armed?` |
| 2 | Anti-sabotage kiểm tra `LDR Covered OR Object Too Close` | Phá hoại chỉ kích hoạt khi **đồng thời** LDR bị che và vật cách cảm biến <= 15 cm trong ít nhất 3 giây. | Thay bằng: `LDR covered AND distance <= 15 cm for >= 3 s?` |
| 3 | Nhánh sabotage chỉ bật còi/LED và gửi Telegram chữ | Sau sabotage, firmware gửi Telegram chữ, tự chụp một ảnh camera và gửi ảnh Telegram. | Thêm bước: `Capture security photo -> Send photo to Telegram`. |
| 4 | Heartbeat mỗi 10 giây | Firmware hiện gửi heartbeat mỗi 30 giây. | Đổi nhãn thành: `Send heartbeat to Google Apps Script every 30 seconds`. |
| 5 | Device lost chỉ ghi `No Heartbeat` | Apps Script chỉ tạo critical khi đã có sabotage active và heartbeat cuối cũ hơn 60 giây. Trigger Apps Script kiểm tra mỗi 1 phút. | Thay decision bằng: `Sabotage active AND last heartbeat > 60 s?` và ghi chú email đến khoảng 60-120 giây sau heartbeat cuối. |
| 6 | Heartbeat Monitoring nằm như một bước chỉ sau sabotage | ESP32 gửi heartbeat liên tục khi còn chạy. Apps Script chỉ bắt đầu đánh giá timeout/escalation sau sabotage. | Vẽ heartbeat là nhánh nền từ `Continuous Monitoring`; nối nhánh critical từ trạng thái sabotage active. |
| 7 | `System Armed?` được mô tả là `alarm_enabled AND no alarm active` | Code gán trực tiếp `system_armed = alarm_enabled`. Cảnh báo active không tự làm `system_armed` thành false. | Thay điều kiện bằng: `alarm_enabled?`. |
| 8 | Nhánh RTC chỉ thể hiện Auto Arm | Code chỉ xét lịch khi `schedule_enabled` và RTC hợp lệ; có cả Auto Arm và Auto Disarm. | Dùng decision: `Schedule enabled and RTC valid?`, sau đó tách `Auto arm time?` và `Auto disarm time?`. |
| 9 | Reset Alarm chỉ dừng còi/LED | Reset xóa intrusion, sabotage, SOS, critical, notification, demo state; đồng thời gửi resolve event sang Apps Script. | Thêm bước: `Clear local alerts + resolve active Apps Script event -> outputs normal`. |
| 10 | Confirmation rồi Send Authority Contact có thể bị hiểu là tự động | Email đến contact mô phỏng chỉ được gửi sau khi Parent/Admin bấm xác nhận trong email/trang Apps Script. | Ghi rõ decision: `Parent/Admin confirmed?` trước bước `Send escalation to demo contact`. |

## Luồng Đúng Rút Gọn

```text
Continuous monitoring
  |- Send heartbeat to Apps Script every 30 s
  |- Schedule enabled + RTC valid
  |- alarm_enabled?
       |- yes: PIR AND distance 15-50 cm for >= 2 s
       |       -> Intrusion Alert
       |       -> Buzzer + red LED + capture/send Telegram photo
       |
       |- LDR covered AND distance <= 15 cm for >= 3 s
               -> Sabotage Alert
               -> Buzzer + red LED + Telegram text + capture/send Telegram photo
               -> Apps Script starts evaluating heartbeat timeout
               -> last heartbeat > 60 s?
                    -> Critical Security Compromise
                    -> Confirmation email
                    -> Parent/Admin confirmed?
                    -> Send escalation to demo contact
```

## Ghi Chú Trình Bày

- `intrusion_score` vẫn có thể giữ trong flowchart, nhưng nên đặt sau phần đọc cảm biến với nhãn `Display threat score`; không dùng nó làm decision tạo `Intrusion Alert`.
- Không ghi “kẻ trộm đã cắt điện” ở nhánh critical. Cụm đúng là “mất liên lạc sau phá hoại” hoặc “có nguy cơ bị vô hiệu hóa”.
- Nhánh critical chạy từ Google Apps Script khi ESP32 đã offline; dashboard có thể không nhận được trạng thái critical mới nếu board đã mất nguồn.

## File Đối Chiếu

- `Firmware/Kitchen_Security_System_-_Group_6_jun18a/Kitchen_Security_System_-_Group_6_jun18a.ino`
- `Firmware/gg-app-script-email.js`
- `Docs/SRS.md`
