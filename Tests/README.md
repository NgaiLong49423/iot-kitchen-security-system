# Hardware & Software Tests

Thư mục này chứa các chương trình kiểm thử đơn lẻ (Unit Tests / Sensor Tests) phục vụ cho việc kiểm tra chất lượng phần cứng và hiệu chuẩn cảm biến trước khi tích hợp vào hệ thống chính.

## Tại sao cần thư mục này?

Trong các dự án nhúng và IoT, việc gộp tất cả các cảm biến vào một chương trình lớn ngay từ đầu thường rất khó gỡ lỗi (debug) nếu xảy ra lỗi phần cứng (ví dụ: hỏng dây, sai địa chỉ I2C, cảm biến lỗi).
Việc viết các bài test nhỏ giúp:
1.  **Xác minh phần cứng hoạt động**: Đảm bảo từng linh kiện hoạt động đúng thông số kỹ thuật.
2.  **Hiệu chuẩn cảm biến**: Tìm ra các giá trị ngưỡng (threshold) phù hợp cho cảm biến ánh sáng LDR, cảm biến lửa Flame, hay khoảng cách HC-SR04 trước khi nạp vào mạch chính.
3.  **Hỗ trợ kiểm thử (Nguyễn An Vương)**: Dễ dàng chạy lại các bài test độc lập khi nghi ngờ có linh kiện bị hỏng trong quá trình lắp ráp hoặc hàn mạch.

## Cấu trúc thư mục khuyến nghị

Hãy tạo các thư mục con tương ứng với từng bài test trong thư mục này:

*   `Tests/Test_PIR_HC-SR04/`: Test phát hiện chuyển động kết hợp đo khoảng cách siêu âm.
*   `Tests/Test_RFID_RC522/`: Đọc UID của các thẻ từ và in ra Serial Monitor để lấy dữ liệu làm Whitelist.
*   `Tests/Test_DS1307_RTC/`: Thiết lập giờ ban đầu và kiểm tra khả năng lưu giờ của pin CR2032.
*   `Tests/Test_Flame_Buzzer/`: Kiểm tra ngưỡng kích hoạt của cảm biến lửa và độ lớn của còi.
*   `Tests/Test_ESP32Cam_Capture/`: Test chụp ảnh đơn giản và ghi vào thẻ nhớ SD hoặc gửi qua Serial để test camera.
*   `Tests/Test_WiFi_Scan/`: Chạy thử chế độ quét WiFi và bắt địa chỉ MAC để kiểm tra chức năng phát hiện thiết bị lạ.

---

*Lưu ý: Code test nên được viết ngắn gọn, dễ đọc, chỉ tập trung vào một tính năng cụ thể và in kết quả trực tiếp ra Serial Monitor để tiện theo dõi.*
