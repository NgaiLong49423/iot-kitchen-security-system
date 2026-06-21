/**
 * Google Apps Script Web App
 * Project: Kitchen Security System - Group 6
 *
 * Chức năng:
 * 1. Gửi Gmail cảnh báo SOS
 * 2. Gửi Gmail cảnh báo cháy
 * 3. Gửi Gmail cảnh báo phá hoại thiết bị
 * 4. Hỗ trợ link xác nhận gửi tiếp cho công an/cứu hỏa giả lập
 *
 * Cách gọi từ ESP32-CAM:
 * ?type=sos_child&level=2&target=family
 * ?type=fire&target=family&flame=1&temp=55
 * ?type=sabotage&target=family
 *
 * Link xác nhận sẽ tự được tạo trong email.
 */

// ==================================================
// 1. CẤU HÌNH NGƯỜI NHẬN
// ==================================================

const RECIPIENTS = {
    family: [
        "longgiango511@gmail.com",
        "danhvo0147258@gmail.com",
        "anhbin160304@gmail.com",
        "yakinpvk@gmail.com"
    ],

    police_demo: [
        "hong123aa@gmail.com"
    ],

    fire_demo: [
        "longha12345a@gmail.com"
    ],

    teacher_demo: [
        "longgiango511@gmail.com"
    ]
};

// Có thể để trống khi demo.
// Nếu muốn bảo mật hơn, đặt chuỗi bí mật ở đây,
// sau đó ESP32-CAM phải gọi thêm &key=CHUOI_BI_MAT
const WEB_APP_TOKEN = "";

const DEVICE_NAME = "ESP32-CAM Kitchen Security";
const DEVICE_LOCATION = "Khu vuc bep";
const PROJECT_NAME = "Kitchen Security System - Group 6";


// ==================================================
// 2. HÀM CHÍNH DO WEB APP GỌI
// ==================================================

function doGet(e) {
    try {
        const params = e.parameter || {};

        if (!isAuthorized(params)) {
            return textResponse("ERROR: Unauthorized request");
        }

        const action = params.action || "alert";

        if (action === "confirm") {
            return handleConfirm(params);
        }

        return handleAlert(params);

    } catch (err) {
        return textResponse("ERROR: " + err.message);
    }
}


// ==================================================
// 3. XỬ LÝ CẢNH BÁO TỪ ESP32-CAM
// ==================================================

function handleAlert(params) {
    const type = params.type || "unknown";
    const target = params.target || "family";
    const level = Number(params.level || 0);

    const eventId = params.eventId || createEventId(type);

    const flame = params.flame || "";
    const temp = params.temp || "";
    const ldr = params.ldr || "";
    const pir = params.pir || "";

    const recipientList = getRecipients(target);

    let subject = "";
    let plainBody = "";
    let htmlBody = "";

    if (type === "sos" || type === "sos_child") {
        subject = "🆘 SOS KHẨN CẤP - " + PROJECT_NAME;

        plainBody =
            "Người trong nhà vừa kích hoạt SOS.\n\n" +
            commonPlainInfo(eventId, type, target) +
            "Mức SOS: " + level + "\n\n" +
            getSosLevelExplanation(level) + "\n\n" +
            "Vui lòng kiểm tra ngay.";

        htmlBody =
            htmlTitle("🆘 SOS KHẨN CẤP") +
            commonHtmlInfo(eventId, type, target) +
            htmlRow("Mức SOS", String(level)) +
            htmlParagraph(getSosLevelExplanation(level)) +
            htmlParagraph("Vui lòng kiểm tra ngay.");

        if (level >= 3) {
            const confirmPoliceUrl = buildConfirmUrl(eventId, "sos_child", "police_demo");

            plainBody +=
                "\n\nXác nhận gửi tiếp cho công an giả lập:\n" +
                confirmPoliceUrl;

            htmlBody +=
                htmlParagraph("SOS mức 3 cần người lớn xác nhận gửi tiếp cho công an/cứu hộ giả lập.") +
                htmlButton(confirmPoliceUrl, "Xác nhận gửi công an giả lập");
        }

    } else if (type === "sos_adult") {
        subject = "🆘 SOS NGƯỜI LỚN - " + PROJECT_NAME;

        plainBody =
            "Người lớn vừa kích hoạt SOS.\n\n" +
            commonPlainInfo(eventId, type, target) +
            "Mức SOS: " + level + "\n\n" +
            "Hệ thống đã bật cảnh báo tại chỗ bằng LED đỏ và buzzer.";

        htmlBody =
            htmlTitle("🆘 SOS NGƯỜI LỚN") +
            commonHtmlInfo(eventId, type, target) +
            htmlRow("Mức SOS", String(level)) +
            htmlParagraph("Hệ thống đã bật cảnh báo tại chỗ bằng LED đỏ và buzzer.");

    } else if (type === "fire") {
        subject = "🔥 CẢNH BÁO CHÁY - " + PROJECT_NAME;

        plainBody =
            "Flame Sensor phát hiện dấu hiệu ngọn lửa.\n\n" +
            commonPlainInfo(eventId, type, target) +
            "Flame detected: " + flame + "\n" +
            "Kitchen temperature: " + temp + "\n\n" +
            "Vui lòng kiểm tra khu vực bếp ngay lập tức.";

        htmlBody =
            htmlTitle("🔥 CẢNH BÁO CHÁY") +
            commonHtmlInfo(eventId, type, target) +
            htmlRow("Flame detected", flame || "N/A") +
            htmlRow("Kitchen temperature", temp || "N/A") +
            htmlParagraph("Vui lòng kiểm tra khu vực bếp ngay lập tức.");

        const confirmFireUrl = buildConfirmUrl(eventId, "fire", "fire_demo");

        plainBody +=
            "\n\nXác nhận gửi tiếp cho cứu hỏa giả lập:\n" +
            confirmFireUrl;

        htmlBody +=
            htmlParagraph("Nếu xác nhận nguy hiểm, người lớn có thể bấm link dưới để gửi tiếp cho cứu hỏa giả lập.") +
            htmlButton(confirmFireUrl, "Xác nhận gửi cứu hỏa giả lập");

    } else if (type === "sabotage") {
        subject = "🛠️ CẢNH BÁO PHÁ HOẠI THIẾT BỊ - " + PROJECT_NAME;

        plainBody =
            "Hệ thống phát hiện dấu hiệu phá hoại hoặc che cảm biến.\n\n" +
            commonPlainInfo(eventId, type, target) +
            "LDR covered: " + ldr + "\n" +
            "PIR detected: " + pir + "\n\n" +
            "Khả năng: Có người che LDR hoặc can thiệp thiết bị trong lúc có chuyển động.";

        htmlBody =
            htmlTitle("🛠️ CẢNH BÁO PHÁ HOẠI THIẾT BỊ") +
            commonHtmlInfo(eventId, type, target) +
            htmlRow("LDR covered", ldr || "N/A") +
            htmlRow("PIR detected", pir || "N/A") +
            htmlParagraph("Khả năng: Có người che LDR hoặc can thiệp thiết bị trong lúc có chuyển động.");

    } else if (type === "intrusion") {
        subject = "🚨 CẢNH BÁO ĐỘT NHẬP - " + PROJECT_NAME;

        plainBody =
            "Hệ thống phát hiện chuyển động bất thường.\n\n" +
            commonPlainInfo(eventId, type, target) +
            "PIR detected: " + pir + "\n" +
            "LDR covered/light abnormal: " + ldr + "\n\n" +
            "Hành động: Bật còi, bật LED đỏ, gửi cảnh báo.";

        htmlBody =
            htmlTitle("🚨 CẢNH BÁO ĐỘT NHẬP") +
            commonHtmlInfo(eventId, type, target) +
            htmlRow("PIR detected", pir || "N/A") +
            htmlRow("LDR/light abnormal", ldr || "N/A") +
            htmlParagraph("Hành động: Bật còi, bật LED đỏ, gửi cảnh báo.");

    } else {
        subject = "⚠️ CẢNH BÁO KHÔNG XÁC ĐỊNH - " + PROJECT_NAME;

        plainBody =
            "Hệ thống gửi một cảnh báo không xác định.\n\n" +
            commonPlainInfo(eventId, type, target);

        htmlBody =
            htmlTitle("⚠️ CẢNH BÁO KHÔNG XÁC ĐỊNH") +
            commonHtmlInfo(eventId, type, target);
    }

    sendEmail(recipientList, subject, plainBody, htmlBody);

    return textResponse(
        "OK: Email sent. type=" + type +
        ", target=" + target +
        ", level=" + level +
        ", eventId=" + eventId
    );
}


// ==================================================
// 4. XỬ LÝ LINK XÁC NHẬN GỬI TIẾP
// ==================================================

function handleConfirm(params) {
    const eventId = params.eventId || "";
    const originalType = params.originalType || "unknown";
    const escalationTarget = params.escalationTarget || "";

    if (!eventId || !escalationTarget) {
        return textResponse("ERROR: Missing eventId or escalationTarget");
    }

    const lockKey = "CONFIRMED_" + eventId;
    const props = PropertiesService.getScriptProperties();

    const oldValue = props.getProperty(lockKey);

    if (oldValue) {
        return htmlResponse(
            htmlTitle("Yêu cầu đã được xử lý") +
            htmlParagraph("Sự kiện này đã có người xác nhận trước đó.") +
            htmlParagraph("Event ID: " + escapeHtml(eventId))
        );
    }

    props.setProperty(lockKey, new Date().toISOString());

    const recipientList = getRecipients(escalationTarget);

    let subject = "";
    let plainBody = "";
    let htmlBody = "";

    if (escalationTarget === "police_demo") {
        subject = "🚓 XÁC NHẬN GỬI CÔNG AN GIẢ LẬP - " + PROJECT_NAME;

        plainBody =
            "Người lớn đã xác nhận gửi cảnh báo đến công an giả lập.\n\n" +
            "Event ID: " + eventId + "\n" +
            "Loại cảnh báo gốc: " + originalType + "\n" +
            "Thiết bị: " + DEVICE_NAME + "\n" +
            "Khu vực: " + DEVICE_LOCATION + "\n\n" +
            "Đây là email phục vụ demo đồ án.";

        htmlBody =
            htmlTitle("🚓 XÁC NHẬN GỬI CÔNG AN GIẢ LẬP") +
            htmlRow("Event ID", eventId) +
            htmlRow("Loại cảnh báo gốc", originalType) +
            htmlRow("Thiết bị", DEVICE_NAME) +
            htmlRow("Khu vực", DEVICE_LOCATION) +
            htmlParagraph("Đây là email phục vụ demo đồ án.");

    } else if (escalationTarget === "fire_demo") {
        subject = "🚒 XÁC NHẬN GỬI CỨU HỎA GIẢ LẬP - " + PROJECT_NAME;

        plainBody =
            "Người lớn đã xác nhận gửi cảnh báo đến cứu hỏa giả lập.\n\n" +
            "Event ID: " + eventId + "\n" +
            "Loại cảnh báo gốc: " + originalType + "\n" +
            "Thiết bị: " + DEVICE_NAME + "\n" +
            "Khu vực: " + DEVICE_LOCATION + "\n\n" +
            "Đây là email phục vụ demo đồ án.";

        htmlBody =
            htmlTitle("🚒 XÁC NHẬN GỬI CỨU HỎA GIẢ LẬP") +
            htmlRow("Event ID", eventId) +
            htmlRow("Loại cảnh báo gốc", originalType) +
            htmlRow("Thiết bị", DEVICE_NAME) +
            htmlRow("Khu vực", DEVICE_LOCATION) +
            htmlParagraph("Đây là email phục vụ demo đồ án.");

    } else {
        subject = "⚠️ XÁC NHẬN CẢNH BÁO GIẢ LẬP - " + PROJECT_NAME;

        plainBody =
            "Một cảnh báo đã được xác nhận.\n\n" +
            "Event ID: " + eventId + "\n" +
            "Loại cảnh báo gốc: " + originalType + "\n" +
            "Target: " + escalationTarget;

        htmlBody =
            htmlTitle("⚠️ XÁC NHẬN CẢNH BÁO GIẢ LẬP") +
            htmlRow("Event ID", eventId) +
            htmlRow("Loại cảnh báo gốc", originalType) +
            htmlRow("Target", escalationTarget);
    }

    sendEmail(recipientList, subject, plainBody, htmlBody);

    // Gửi mail báo lại cho family rằng đã có người xác nhận.
    sendEmail(
        getRecipients("family"),
        "✅ ĐÃ XÁC NHẬN CẢNH BÁO - " + PROJECT_NAME,
        "Một người lớn đã xác nhận xử lý cảnh báo.\n\n" +
        "Event ID: " + eventId + "\n" +
        "Loại cảnh báo gốc: " + originalType + "\n" +
        "Đã gửi đến: " + escalationTarget,
        htmlTitle("✅ ĐÃ XÁC NHẬN CẢNH BÁO") +
        htmlRow("Event ID", eventId) +
        htmlRow("Loại cảnh báo gốc", originalType) +
        htmlRow("Đã gửi đến", escalationTarget)
    );

    return htmlResponse(
        htmlTitle("Xác nhận thành công") +
        htmlParagraph("Cảnh báo đã được gửi tiếp thành công.") +
        htmlRow("Event ID", eventId) +
        htmlRow("Đã gửi đến", escalationTarget)
    );
}


// ==================================================
// 5. HÀM PHỤ TRỢ
// ==================================================

function isAuthorized(params) {
    if (!WEB_APP_TOKEN) {
        return true;
    }

    return params.key === WEB_APP_TOKEN;
}

function getRecipients(target) {
    return RECIPIENTS[target] || RECIPIENTS.family;
}

function sendEmail(recipientList, subject, plainBody, htmlBody) {
    MailApp.sendEmail({
        to: recipientList.join(","),
        subject: subject,
        body: plainBody,
        htmlBody: htmlBody
    });
}

function createEventId(type) {
    return type + "_" + new Date().getTime();
}

function buildConfirmUrl(eventId, originalType, escalationTarget) {
    const baseUrl = ScriptApp.getService().getUrl();

    return baseUrl +
        "?action=confirm" +
        "&eventId=" + encodeURIComponent(eventId) +
        "&originalType=" + encodeURIComponent(originalType) +
        "&escalationTarget=" + encodeURIComponent(escalationTarget);
}

function getSosLevelExplanation(level) {
    if (level === 1) {
        return "SOS mức 1: Hệ thống chỉ cảnh báo tại chỗ bằng LED đỏ và buzzer.";
    }

    if (level === 2) {
        return "SOS mức 2: Hệ thống cảnh báo tại chỗ và gửi Gmail cho người lớn.";
    }

    if (level >= 3) {
        return "SOS mức 3: Hệ thống gửi Gmail cho người lớn và yêu cầu xác nhận gửi tiếp cho công an/cứu hộ giả lập.";
    }

    return "SOS chưa xác định mức độ.";
}

function commonPlainInfo(eventId, type, target) {
    return (
        "Project: " + PROJECT_NAME + "\n" +
        "Event ID: " + eventId + "\n" +
        "Loại cảnh báo: " + type + "\n" +
        "Nhóm nhận: " + target + "\n" +
        "Thiết bị: " + DEVICE_NAME + "\n" +
        "Khu vực: " + DEVICE_LOCATION + "\n" +
        "Thời gian server: " + new Date().toLocaleString("vi-VN", { timeZone: "Asia/Ho_Chi_Minh" }) + "\n\n"
    );
}

function commonHtmlInfo(eventId, type, target) {
    return (
        htmlRow("Project", PROJECT_NAME) +
        htmlRow("Event ID", eventId) +
        htmlRow("Loại cảnh báo", type) +
        htmlRow("Nhóm nhận", target) +
        htmlRow("Thiết bị", DEVICE_NAME) +
        htmlRow("Khu vực", DEVICE_LOCATION) +
        htmlRow("Thời gian server", new Date().toLocaleString("vi-VN", { timeZone: "Asia/Ho_Chi_Minh" }))
    );
}

function textResponse(text) {
    return ContentService
        .createTextOutput(text)
        .setMimeType(ContentService.MimeType.TEXT);
}

function htmlResponse(html) {
    return HtmlService.createHtmlOutput(
        "<html><body style='font-family: Arial, sans-serif; line-height: 1.5;'>" +
        html +
        "</body></html>"
    );
}

function htmlTitle(text) {
    return "<h2>" + escapeHtml(text) + "</h2>";
}

function htmlParagraph(text) {
    return "<p>" + escapeHtml(text) + "</p>";
}

function htmlRow(label, value) {
    return (
        "<p><b>" + escapeHtml(label) + ":</b> " +
        escapeHtml(value || "N/A") +
        "</p>"
    );
}

function htmlButton(url, text) {
    return (
        "<p>" +
        "<a href='" + escapeHtml(url) + "' " +
        "style='display:inline-block;padding:10px 16px;background:#d93025;color:white;text-decoration:none;border-radius:6px;font-weight:bold;'>" +
        escapeHtml(text) +
        "</a>" +
        "</p>"
    );
}

function escapeHtml(value) {
    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}