# Firmware Source Code

Thư mục này chứa mã nguồn chạy trên vi điều khiển chính: **Freenove ESP32-S3 WROOM + Camera OV3660**.

Firmware hiện hành nằm tại `Kitchen_Security_System_-_Group_6_jun18a/` và đã được test với Arduino Cloud, Telegram, Google Apps Script heartbeat/critical escalation, anti-sabotage và chụp ảnh thủ công. Các mô tả cũ về ESP32-CAM AI Thinker, ESP8266, RFID, Flame Sensor hoặc quét WiFi/MAC không thuộc phạm vi bản demo hiện tại.

## Cấu trúc thư mục

*   **[`Kitchen_Security_System_-_Group_6_jun18a/`](./Kitchen_Security_System_-_Group_6_jun18a/)**:
    *   Chứa firmware chính, `thingProperties.h` và `arduino_secrets.h` cho ESP32-S3.
    *   Điều khiển camera OV3660, PIR, cảm biến siêu âm, LDR, DS1307, LED và buzzer.
    *   Thực hiện intrusion, anti-sabotage, SOS, chụp ảnh tự động/thủ công, Telegram, heartbeat và Google Apps Script escalation.
    *   Kết nối Arduino IoT Cloud để nhận lệnh Dashboard và xuất trạng thái đã chuẩn hóa cho demo.

---

## Hướng dẫn cho Thành viên Phát triển (Võ Trần Công Danh & Ngô Gia Long)

1.  **Đặt code vào đúng thư mục**:
    *   Dùng sketch [Kitchen_Security_System_-_Group_6_jun18a.ino](./Kitchen_Security_System_-_Group_6_jun18a/Kitchen_Security_System_-_Group_6_jun18a.ino) trong thư mục cùng tên.
2.  **Quản lý Thư viện (Libraries)**:
    *   Biên dịch với FQBN `esp32:esp32:esp32s3`; các thư viện được dùng được liệt kê trong log compile hoặc Arduino IDE của máy phát triển.
3.  **Thông tin Bảo mật (Secrets)**:
    *   Không commit mật khẩu WiFi hay khóa API của Cloud trực tiếp lên Git.
    *   Sử dụng file cấu hình riêng (như `arduino_secrets.h` hoặc `secrets.h`) và đảm bảo các file này đã được khai báo trong [.gitignore](../../.gitignore) của dự án.
