#pragma once

// Sao chép tệp mẫu này thành arduino_secrets.h rồi thay mọi giá trị giữ chỗ.
// Không commit arduino_secrets.h thật vì tệp đó chứa thông tin đăng nhập bí mật.
#define SECRET_SSID "YOUR_WIFI_SSID"                              // Tên mạng Wi-Fi mà thiết bị sử dụng.
#define SECRET_OPTIONAL_PASS "YOUR_WIFI_PASSWORD"                 // Mật khẩu của mạng Wi-Fi.
#define SECRET_DEVICE_KEY "YOUR_ARDUINO_CLOUD_DEVICE_KEY"         // Khóa xác thực thiết bị Arduino IoT Cloud.
#define SECRET_DEVICE_NAME "YOUR_ARDUINO_CLOUD_DEVICE_NAME"       // Tên thiết bị được đưa vào nội dung thông báo.
#define SECRET_DEVICE_LOCATION "YOUR_ARDUINO_CLOUD_DEVICE_LOCATION" // Vị trí lắp thiết bị được đưa vào cảnh báo.
#define SECRET_TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"       // Token dùng để gọi Telegram Bot API.
#define SECRET_TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"           // ID cuộc trò chuyện nhận thông báo Telegram.
#define SECRET_GEMINI_API_KEY "YOUR_OPTIONAL_GEMINI_API_KEY"      // Khóa Gemini API; có thể để trống nếu không dùng AI.
#define SECRET_GOOGLE_SCRIPT_URL "YOUR_PRIVATE_APPS_SCRIPT_URL"   // URL Web App riêng của Google Apps Script.
