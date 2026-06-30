# SRS 0.4.0 Update Summary

## Đã cập nhật

- SOS Simplified Demo Rule
- SOS Button Auto Reset Rule
- Email-Based SOS Escalation Confirmation Rule
- Anti-Sabotage Demo Rule
- Simple Event Logging Rule
- Simple Notification Cooldown Rule
- Demo Mode Scope Rule
- FR-09 đến FR-13 được thay từ placeholder sang bản demo rõ ràng
- Bảng vấn đề cần phân rã tiếp được rút gọn

## Còn cần phân rã không?

Các phần lõi cho demo đã đủ. Không cần phân rã sâu thêm nếu mục tiêu là hoàn thành demo và code.

Những phần chỉ nên làm nếu còn thời gian:

1. Chốt giá trị `notification_sent_status`: `PENDING`, `SENT`, `FAILED`.
2. Viết test case demo cuối cùng.
3. Viết tài liệu Google Apps Script endpoint/config.
4. Làm dashboard layout riêng nếu cần trình bày đẹp hơn.
