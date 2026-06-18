# ESP32-CAM Firmware

Thư mục này chứa mã nguồn chạy trên vi điều khiển **ESP32-CAM** tích hợp Camera OV2640.

## Chức năng đảm nhận
1.  **Quản lý Camera**: Khởi tạo cấu hình cho camera OV2640, thiết lập độ phân giải và chất lượng hình ảnh phù hợp để truyền tải nhanh qua mạng.
2.  **Chụp ảnh khi đột nhập**: Nhận lệnh kích hoạt chụp ảnh từ vi điều khiển chính (ESP8266) hoặc từ Cloud khi phát hiện hành vi đột nhập hoặc khi có lệnh SOS khẩn cấp.
3.  **Tải ảnh lên Cloud**: Đẩy ảnh chụp lên máy chủ lưu trữ (Firebase Storage, Google Drive, hoặc gửi trực tiếp dạng dữ liệu mã hóa lên Arduino IoT Cloud) để hiển thị trực quan trên ứng dụng di động của chủ nhà.

## Hướng dẫn kết nối phần cứng nạp code
Để nạp code cho ESP32-CAM, bạn cần sử dụng mạch nạp USB-to-TTL (như FT232RL) và kết nối như sau:
*   `GND` -> `GND`
*   `5V` hoặc `3V3` -> `5V` hoặc `3V3` (Khuyến nghị dùng nguồn 5V ngoài nếu camera hoạt động không ổn định)
*   `TX` -> `RX`
*   `RX` -> `TX`
*   **Quan trọng**: Nối chân `GPIO 0` xuống `GND` trước khi cấp nguồn để đưa chip vào chế độ Download Mode (nạp code). Sau khi nạp xong, hãy tháo dây nối `GPIO 0` và nhấn nút `Reset` trên chip để chạy chương trình.

## Cấu trúc Code
*   Hãy tạo thư mục con trùng tên với file `.ino` chính, ví dụ: `ESP32Cam/ESP32Cam/ESP32Cam.ino`.
*   Sử dụng thư viện `esp_camera.h` có sẵn trong ESP32 Arduino Core.
