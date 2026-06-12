# IoT-Based Multi-Sensor Anti-Theft System

<div align="center">

**Hệ thống chống trộm IoT đa cảm biến cho khu vực phòng bếp**

![Project](https://img.shields.io/badge/Project-IoT-blue)
![Version](https://img.shields.io/badge/Version-2.0.4-green)
![Course](https://img.shields.io/badge/Class-SE2034-orange)
![Team](https://img.shields.io/badge/Team-Group%206-purple)

</div>

---

## Giới thiệu

Dự án xây dựng một hệ thống an ninh thông minh ứng dụng công nghệ IoT để giám sát khu vực phòng bếp trong gia đình. Hệ thống kết hợp nhiều cảm biến nhằm phát hiện hành vi bất thường, giảm báo động giả, gửi cảnh báo theo thời gian thực và hỗ trợ ghi nhận hình ảnh hiện trường.

Dự án được thực hiện bởi **Nhóm 6 – lớp SE2034**.

## Mục tiêu

* Phát hiện hành vi xâm nhập bằng nhiều cảm biến đồng thời.
* Giảm báo động giả do môi trường hoặc vật nuôi.
* Khôi phục trạng thái bảo vệ sau khi mất điện.
* Cảnh báo khi phát hiện thiết bị Wi-Fi lạ xuất hiện gần khu vực giám sát.
* Tự động kích hoạt hoặc vô hiệu hóa chế độ bảo vệ.
* Phát hiện hành vi che khuất hoặc phá hoại cảm biến.
* Hỗ trợ cảnh báo cháy, chụp ảnh hiện trường và ghi nhật ký sự kiện.

## Chức năng chính

| Chức năng                      | Mô tả                                                           |
| ------------------------------ | --------------------------------------------------------------- |
| Nhận diện đột nhập đa cảm biến | Kết hợp PIR, HC-SR04 và LDR để xác nhận hành vi xâm nhập.       |
| Lọc nhiễu vật nuôi             | Hạn chế cảnh báo do vật nuôi di chuyển trong khu vực giám sát.  |
| Khôi phục sau mất điện         | Tự động phục hồi chế độ bảo vệ khi nguồn điện được cấp lại.     |
| Cảnh báo thiết bị Wi-Fi lạ     | Phát hiện thiết bị lạ xuất hiện liên tục gần khu vực phòng bếp. |
| Tự động bật/tắt bảo vệ         | Dựa trên sự hiện diện của các thiết bị đã được xác thực.        |
| Chống phá hoại thiết bị        | Nhận diện hành vi che khuất hoặc can thiệp cảm biến.            |
| Cảnh báo nguy cơ cháy          | Phát hiện dấu hiệu ngọn lửa và kích hoạt cảnh báo.              |
| Chụp ảnh khi đột nhập          | ESP32-CAM chụp ảnh hiện trường khi hệ thống xác nhận xâm nhập.  |
| SOS trên điện thoại            | Cho phép chủ nhà chủ động kích hoạt cảnh báo khẩn cấp từ xa.    |
| Ghi nhật ký cảm biến           | Lưu thời gian, trạng thái cảm biến và hành động của hệ thống.   |

## Phần cứng sử dụng

* RFID RC522
* ESP32-CAM Module Wi-Fi ESP32 Camera OV3660
* ESP8266
* DS1307 và pin CR2032
* Cảm biến chuyển động PIR 5050
* Cảm biến siêu âm HC-SR04
* Cảm biến ánh sáng LDR
* Flame Sensor
* Còi Buzzer
* Đèn LED đỏ

## Nguyên lý hoạt động tổng quát

1. Các cảm biến liên tục thu thập dữ liệu từ khu vực phòng bếp.
2. Bộ điều khiển xử lý và kết hợp tín hiệu từ nhiều cảm biến.
3. Khi phát hiện sự kiện bất thường, hệ thống xác định mức độ cảnh báo.
4. Còi và đèn LED được kích hoạt để cảnh báo tại chỗ.
5. Thông báo và hình ảnh hiện trường có thể được gửi lên nền tảng Cloud/App.
6. Thông tin sự kiện được lưu lại để theo dõi và kiểm thử.

## Kịch bản trình diễn

Hệ thống được thiết kế để trình diễn các tình huống chính:

* Phát hiện người xâm nhập sử dụng đèn pin vào ban đêm.
* Phân biệt chuyển động của vật nuôi với người.
* Khôi phục chế độ bảo vệ sau khi mất điện.
* Phát hiện thiết bị Wi-Fi lạ xuất hiện gần khu vực giám sát.
* Tự động kích hoạt chế độ bảo vệ khi gia đình rời khỏi nhà.
* Phát hiện hành vi che khuất hoặc phá hoại cảm biến.
* Cảnh báo cháy và chụp ảnh hiện trường.
* Kích hoạt cảnh báo SOS từ điện thoại.
* Xem lại nhật ký hoạt động của hệ thống.

## Thành viên

| Thành viên        | MSSV     | Vai trò             |
| ----------------- | -------- | ------------------- |
| Ngô Gia Long      | SE190732 | Trưởng nhóm         |
| Nguyễn Nhật Anh   | SE192077 | Phụ trách phần cứng |
| Võ Trần Công Danh | SE190824 | Phụ trách phần mềm  |
| Nguyễn An Vương   | SE190996 | Kiểm thử và hỗ trợ  |

## Lộ trình thực hiện

| Giai đoạn | Nội dung                                                 |
| --------- | -------------------------------------------------------- |
| Tuần 3    | Ghép nối phần cứng và đọc tín hiệu cảm biến.             |
| Tuần 4    | Xây dựng thuật toán kết hợp đa cảm biến.                 |
| Tuần 5    | Lọc nhiễu, chống phá hoại và hạn chế spam cảnh báo.      |
| Tuần 6    | Xử lý vật nuôi, mất mạng và lưu trạng thái khi mất điện. |
| Tuần 7    | Quét thiết bị Wi-Fi lạ và bổ sung cơ chế chống treo máy. |
| Tuần 8    | Hàn mạch và hoàn thiện vỏ sản phẩm.                      |
| Tuần 9    | Kiểm thử tổng thể và quay video demo.                    |
| Tuần 10   | Hoàn thiện báo cáo, slide và bảo vệ dự án.               |

## Thông tin dự án

| Thuộc tính   | Nội dung                    |
| ------------ | --------------------------- |
| Project Name | IoT based anti-theft system |
| Version      | 2.0.4                       |
| Prepared By  | Group 6                     |
| Date         | Jun 5, 2026                 |
| Class        | SE2034                      |

---

<div align="center">

**Group 6 – SE2034**

</div>
::: ​​
