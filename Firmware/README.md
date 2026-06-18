# Firmware Source Code

Thư mục này chứa mã nguồn (source code) chạy trên vi điều khiển chính của hệ thống: **ESP32-CAM**.

Do dự án đã tối ưu hóa phần cứng, loại bỏ hoàn toàn vi điều khiển phụ (ESP8266) và sử dụng duy nhất một mạch **ESP32-CAM** làm bộ xử lý trung tâm, toàn bộ mã nguồn sẽ được đặt tập trung tại đây.

## Cấu trúc thư mục

*   **[`ESP32Cam_Main/`](./ESP32Cam_Main/)**:
    *   Chứa mã nguồn chính tích hợp tất cả các tính năng của hệ thống.
    *   Điều khiển và đọc dữ liệu từ camera OV2640 cũng như toàn bộ cảm biến (PIR, HC-SR04, LDR, Flame), RFID RC522, RTC DS1307, LED và còi Buzzer.
    *   Thực hiện thuật toán kết hợp đa cảm biến (Sensor Fusion), chống phá hoại (Anti-Sabotage), khôi phục trạng thái sau mất điện (EEPROM) và quét WiFi lạ (Promiscuous Mode).
    *   Kết nối WiFi, gửi dữ liệu telemetry, nhận lệnh SOS/Arm/Disarm và tải ảnh chụp hiện trường lên Arduino IoT Cloud.

---

## Hướng dẫn cho Thành viên Phát triển (Võ Trần Công Danh & Ngô Gia Long)

1.  **Đặt code vào đúng thư mục**:
    *   Hãy tạo thư mục con trùng tên với file `.ino` chính nằm trong [ESP32Cam_Main/](./ESP32Cam_Main/) (Ví dụ: `ESP32Cam_Main/ESP32Cam_Main.ino`).
2.  **Quản lý Thư viện (Libraries)**:
    *   Ghi chú rõ danh sách các thư viện Arduino cần cài đặt trong file `README.md` của thư mục [ESP32Cam_Main/](./ESP32Cam_Main/) để các thành viên khác dễ dàng cài đặt và biên dịch (compile).
3.  **Thông tin Bảo mật (Secrets)**:
    *   Không commit mật khẩu WiFi hay khóa API của Cloud trực tiếp lên Git.
    *   Sử dụng file cấu hình riêng (như `arduino_secrets.h` hoặc `secrets.h`) và đảm bảo các file này đã được khai báo trong [.gitignore](../../.gitignore) của dự án.
