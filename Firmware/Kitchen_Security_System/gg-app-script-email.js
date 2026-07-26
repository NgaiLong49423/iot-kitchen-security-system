/**
 * Web App Google Apps Script của Kitchen Security System - Nhóm 6.
 * Mốc SRS: v0.6.7.
 *
 * Trách nhiệm của tệp:
 * 1. Điều phối quy trình xin xác nhận nâng cấp SOS.
 * 2. Gửi email cho Phụ huynh/Admin khi ESP32-S3 báo SOS.
 * 3. Nhận xác nhận cùng ghi chú sos_authority_note tùy chọn.
 * 4. Chỉ gửi email đến liên hệ cơ quan demo sau khi người có quyền xác nhận.
 * 5. Cung cấp endpoint polling để ESP32-S3 đọc trạng thái xác nhận/gửi thư.
 * 6. Nhận heartbeat và sabotage_alert cho DS-03/DS-04.
 * 7. Giám sát heartbeat, chỉ gửi một email mất kết nối và một email phục hồi.
 * 8. Nâng thành critical_security_compromise nếu mất heartbeat sau phá hoại.
 *
 * Quy tắc quan trọng:
 * - Thông báo Telegram và nâng cấp SOS bằng email là hai luồng độc lập.
 * - Script này xử lý email SOS, không thay thế cảnh báo cháy/đột nhập.
 * - Xác nhận không reset còi cục bộ; ESP32 phải giữ SOS_ALERT đến khi người dùng
 *   bật reset_alarm trên Arduino Cloud.
 * - Email nâng cấp chứa địa chỉ nhà đã cấu hình và ghi chú tùy chọn.
 * - Người nhận cơ quan phải là địa chỉ giả lập/demo.
 *
 * Ví dụ request tương thích ESP32-S3:
 *
 * 1) Tạo email xác nhận SOS từ ESP32-S3:
 *    ?event=sos_alert&source=CHILD&device=He%20thong&location=Khu%20vuc%20bep&status=SOS_ALERT&threat=4&score=0&time=2026-07-04%2013:00:00&message=SOS
 *
 * 2) ESP32-S3 thăm dò trạng thái:
 *    ?action=status&eventId=EVENT_ID_FROM_RESPONSE
 *
 * 3) Phụ huynh/Admin mở trang xác nhận từ link trong email:
 *    ?action=confirm&eventId=EVENT_ID
 */

// ==================================================
// 1. CẤU HÌNH
// ==================================================

const CONFIG = {
    PROJECT_NAME: 'Kitchen Security System - Group 6', // Tên dự án dùng trong tiêu đề/nội dung email.
    TIME_ZONE: 'Asia/Ho_Chi_Minh',                     // Múi giờ dùng để định dạng thời gian máy chủ.

    // Tùy chọn. Nếu khác rỗng, ESP32-S3 phải gửi thêm &key=YOUR_TOKEN.
    WEB_APP_TOKEN: '', // Token query tùy chọn; để trống nghĩa là không yêu cầu khóa này.

    // Link xác nhận email và link trạng thái phải cùng deployment Web App
    // mà ESP32-S3 gọi. Không thay bằng URL deployment cũ.
    PUBLIC_WEB_APP_URL: 'https://script.google.com/macros/s/AKfycbxQ3ik2vGu4f4ygHQXUtswjMmpfDSKtb381Sz5UFLzDDc2JNAnNewI6UV311sC2bKPdPQ/exec', // Endpoint dùng để tạo link xác nhận/trạng thái.

    // Các Phụ huynh/Admin sẽ nhận email xác nhận SOS.
    FAMILY_RECIPIENTS: [ // Danh sách Phụ huynh/Admin nhận email xác nhận và trạng thái.
        'longgiango511@gmail.com',
        'danhvo0147258@gmail.com',
        'anhbin160304@gmail.com',
        'yakinpvk@gmail.com'
    ],

    // Chỉ dùng người nhận cơ quan demo. Không đặt địa chỉ công an/cơ quan thật
    // nếu chủ hệ thống chưa phê duyệt hợp pháp việc triển khai.
    AUTHORITY_DEMO_RECIPIENTS: [ // Danh sách liên hệ cơ quan giả lập nhận email sau xác nhận.
        'hong123aa@gmail.com'
    ],

    // Địa chỉ demo chỉ dùng trong luồng nâng cấp của đồ án.
    // Script Property HOME_ADDRESS vẫn được ưu tiên hơn hằng số này.
    HOME_ADDRESS: 'Lô E2a-7, Đường D1, Khu Công nghệ cao, Phường Tăng Nhơn Phú, TP. Hồ Chí Minh', // Địa chỉ dự phòng cho nội dung nâng cấp.

    // Giá trị dự phòng tùy chọn khi ESP32-S3 không gửi các trường tương ứng.
    DEFAULT_DEVICE_NAME: 'Freenove ESP32-S3 Kitchen Security', // Tên dùng khi request thiếu device.
    DEFAULT_DEVICE_LOCATION: 'Khu vuc bep',                    // Vị trí dùng khi request thiếu location.

    // Script lưu bản ghi sự kiện để polling trạng thái và chống xử lý trùng.
    EVENT_PROPERTY_PREFIX: 'KSS_EVENT_', // Tiền tố khóa Script Properties của mỗi bản ghi sự kiện.
    EVENT_TTL_HOURS: 24,                 // Số giờ giữ bản ghi trước khi cleanup xóa.

    HEARTBEAT_PROPERTY_KEY: 'KSS_LAST_HEARTBEAT',                         // Khóa lưu heartbeat mới nhất.
    HEARTBEAT_CONNECTION_STATE_KEY: 'KSS_HEARTBEAT_CONNECTION_STATE',     // Khóa lưu ONLINE/OFFLINE/PAUSED.
    HEARTBEAT_MONITOR_ENABLED_KEY: 'KSS_HEARTBEAT_MONITOR_ENABLED',       // Khóa lưu quyền giám sát heartbeat.
    ACTIVE_SABOTAGE_EVENT_KEY: 'KSS_ACTIVE_SABOTAGE_EVENT_ID',            // Khóa trỏ đến sự kiện phá hoại đang mở.
    ACTIVE_SOS_EVENT_KEY: 'KSS_ACTIVE_SOS_EVENT_ID',                      // Khóa trỏ đến sự kiện SOS đang mở.
    HEARTBEAT_TIMEOUT_SECONDS: 40                                         // Số giây im lặng trước khi coi thiết bị offline.
};

// Tập mã trạng thái chuẩn dùng thống nhất trong bản ghi, response và giao diện xác nhận.
const STATUS = {
    WAITING_CONFIRMATION: 'WAITING_CONFIRMATION', // Đang chờ Phụ huynh/Admin xác nhận.
    CONFIRMED: 'CONFIRMED',                       // Người có quyền đã xác nhận.
    SENT: 'SENT',                                 // Email nâng cấp đã gửi thành công.
    FAILED: 'FAILED',                             // Tác vụ gửi/xử lý thất bại.
    NOT_CONFIGURED: 'NOT_CONFIGURED',             // Thiếu địa chỉ hoặc người nhận bắt buộc.
    UNKNOWN_EVENT: 'UNKNOWN_EVENT',               // Không tìm thấy ID sự kiện yêu cầu.
    IDLE: 'IDLE',                                 // Chưa có quy trình cần xử lý.
    MONITORING: 'MONITORING',                     // Sự kiện đang được theo dõi.
    RESOLVED: 'RESOLVED'                          // Sự kiện đã được người dùng giải quyết.
};

// ==================================================
// 2. ĐIỂM VÀO CỦA WEB APP
// ==================================================

/**
 * Điểm vào HTTP GET: xác thực, dọn dữ liệu cũ và định tuyến theo action/event.
 * @param e Sự kiện request do Apps Script cung cấp; e.parameter chứa query.
 * @return TextOutput hoặc HtmlOutput phù hợp với endpoint được gọi.
 */
function doGet(e) {
    try {
        const params = sanitizeParams((e && e.parameter) || {}); // Bộ query đã ép chuỗi và giới hạn độ dài.

        if (!isAuthorized(params)) {
            return textResponse('ERROR: Unauthorized request');
        }

        cleanupOldEvents_();

        const action = normalize(params.action || ''); // Tên thao tác chuẩn hóa dùng để định tuyến.

        if (action === 'status' || action === 'poll') {
            return handleStatus(params);
        }

        if (action === 'confirm') {
            return handleConfirmPage(params);
        }

        if (action === 'confirm_send') {
            return handleConfirmSend(params);
        }

        if (action === 'resolve' || action === 'reset') {
            return handleResolve(params);
        }

        if (action === 'monitor_heartbeat') {
            return monitorHeartbeatAfterSabotage();
        }

        if (action === 'debug_event') {
            return handleDebugEvent(params);
        }

        // Nhánh mặc định tương thích ngược với ESP32.
        // Firmware ESP32 giai đoạn 5 gửi event=sos_alert mà không có action.
        return handleIncomingEvent(params);

    } catch (err) {
        return textResponse('ERROR: ' + err.message);
    }
}

/**
 * Điểm vào HTTP POST dành cho form xác nhận và lệnh resolve/reset.
 * @param e Sự kiện POST do Apps Script cung cấp.
 * @return Phản hồi chữ hoặc HTML; không hỗ trợ action ngoài danh sách cho phép.
 */
function doPost(e) {
    try {
        const params = sanitizeParams((e && e.parameter) || {}); // Bộ tham số POST đã làm sạch.

        if (!isAuthorized(params)) {
            return textResponse('ERROR: Unauthorized request');
        }

        cleanupOldEvents_();

        const action = normalize(params.action || ''); // Tên thao tác POST đã chuẩn hóa.

        if (action === 'confirm_send') {
            return handleConfirmSend(params);
        }

        if (action === 'resolve' || action === 'reset') {
            return handleResolve(params);
        }

        return textResponse('ERROR: Unsupported POST action');

    } catch (err) {
        return textResponse('ERROR: ' + err.message);
    }
}

// ==================================================
// 3. XỬ LÝ SỰ KIỆN TỪ ESP32-S3
// ==================================================

/** Phân loại event/type đã chuẩn hóa và chuyển request ESP32-S3 đến đúng hàm xử lý. */
function handleIncomingEvent(params) {
    const eventType = normalize(params.event || params.type || ''); // Loại sự kiện ESP32 gửi, dạng chuẩn.

    if (eventType === 'heartbeat' || eventType === 'device_heartbeat') {
        return handleHeartbeat(params);
    }

    if (eventType === 'heartbeat_monitor_control') {
        return handleHeartbeatMonitorControl_(params);
    }

    if (eventType === 'sabotage_alert' || eventType === 'tamper' || eventType === 'device_tampered') {
        return handleSabotageRequest(params);
    }

    if (eventType === 'critical_security_compromise' || eventType === 'critical_damage' || eventType === 'device_compromised') {
        return handleCriticalRequest(params, 'ESP32-S3 reported critical security compromise.');
    }

    if (eventType === 'sos_alert' || eventType === 'sos' || eventType === 'sos_child' || eventType === 'sos_adult') {
        return handleSosRequest(params, eventType);
    }

    if (eventType === 'ai_person_detected' || eventType === 'person_detected') {
        return handleAiPersonDetected(params);
    }

    return textResponse(
        'IGNORED: Unsupported Google Apps Script event. ' +
        'event=' + (eventType || 'NONE')
    );
}

// Đây là cảnh báo thông tin cho gia đình, không phải SOS hay sự kiện nâng cấp
// đến cơ quan. ESP32 chỉ gửi một lần cho mỗi cảnh báo tự động sau khi Gemini
// phân loại ảnh chụp là có người.
/** Gửi email thông tin cho gia đình khi Gemini phát hiện người; không mở quy trình SOS. */
function handleAiPersonDetected(params) {
    const device = firstNonEmpty(params.device, params.deviceName, CONFIG.DEFAULT_DEVICE_NAME); // Tên thiết bị báo AI.
    const location = firstNonEmpty(params.location, params.zone, CONFIG.DEFAULT_DEVICE_LOCATION); // Vị trí phát hiện.
    const detectedAt = firstNonEmpty(params.time, params.current_time, nowText()); // Thời điểm phát hiện ưu tiên RTC.
    const message = firstNonEmpty( // Nội dung tùy chỉnh hoặc lời nhắc mặc định cho gia đình.
        params.message,
        'Hệ thống đã phát hiện có người trong nhà. Vui lòng kiểm tra ảnh trong Telegram để xác minh danh tính.'
    );

    const subject = '[Cảnh báo AI] Phát hiện có người trong nhà'; // Tiêu đề email AI.
    const plainBody = // Nội dung text/plain cho trình đọc thư không hỗ trợ HTML.
        'Thiết bị: ' + device + '\n' +
        'Khu vực: ' + location + '\n' +
        'Thời điểm: ' + detectedAt + '\n\n' +
        message + '\n\n' +
        'Vui lòng nhanh chóng kiểm tra ảnh trong Telegram để xác minh danh tính.';
    const htmlBody = // Nội dung HTML trình bày thông tin phát hiện AI.
        '<h2>Cảnh báo AI: phát hiện có người</h2>' +
        '<p><b>Thiết bị:</b> ' + escapeHtml(device) + '</p>' +
        '<p><b>Khu vực:</b> ' + escapeHtml(location) + '</p>' +
        '<p><b>Thời điểm:</b> ' + escapeHtml(detectedAt) + '</p>' +
        '<p>' + escapeHtml(message) + '</p>' +
        '<p><b>Vui lòng nhanh chóng kiểm tra ảnh trong Telegram để xác minh danh tính.</b></p>';

    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
    return textResponse('OK:AI_PERSON_EMAIL_SENT');
}

/** Lưu heartbeat mới nhất, phát email phục hồi khi cần và trả trạng thái sự kiện đang theo dõi. */
function handleHeartbeat(params) {
    const now = nowText(); // Thời gian máy chủ nhận heartbeat.
    const record = {       // Bản ghi heartbeat mới nhất sẽ lưu vào Script Properties.
        eventType: 'heartbeat',
        device: firstNonEmpty(params.device, params.deviceName, CONFIG.DEFAULT_DEVICE_NAME),
        location: firstNonEmpty(params.location, params.zone, CONFIG.DEFAULT_DEVICE_LOCATION),
        status: firstNonEmpty(params.status, 'ONLINE'),
        threat: firstNonEmpty(params.threat, params.threat_level, '0'),
        score: firstNonEmpty(params.score, params.intrusion_score, '0'),
        rtcTime: firstNonEmpty(params.time, params.current_time, 'RTC_NOT_PROVIDED'),
        serverTime: now,
        message: firstNonEmpty(params.message, 'Device heartbeat received.'),
        activeSabotageEventId: getActiveSabotageEventId_()
    };

    PropertiesService.getScriptProperties().setProperty(CONFIG.HEARTBEAT_PROPERTY_KEY, JSON.stringify(record));

    // Chỉ heartbeat bình thường xuất hiện sau thông báo offline mới làm gửi
    // email phục hồi. Lần khởi động đầu tiên không gửi email phục hồi.
    const connectionState = getHeartbeatConnectionState_(); // Trạng thái ONLINE/OFFLINE/PAUSED trước heartbeat này.
    if (isHeartbeatMonitorEnabled_() && connectionState.state === 'OFFLINE') {
        sendHeartbeatRecoveryEmail_(record);
    }
    if (isHeartbeatMonitorEnabled_()) {
        saveHeartbeatConnectionState_('ONLINE', now);
    }

    return textResponse(
        'OK:HEARTBEAT' +
        ';serverTime=' + safeValue(now) +
        ';active_sabotage=' + String(Boolean(record.activeSabotageEventId)) +
        ';active_sabotage_eventId=' + safeValue(record.activeSabotageEventId || '')
    );
}

/** Bật/tạm dừng giám sát heartbeat theo lệnh ESP32 và ngăn tạo email phục hồi giả. */
function handleHeartbeatMonitorControl_(params) {
    const enabled = String(params.enabled || '').toLowerCase() === 'true'; // Trạng thái giám sát firmware yêu cầu.
    const changedAt = firstNonEmpty(params.time, nowText()); // Thời điểm thay đổi ưu tiên thời gian thiết bị gửi.
    PropertiesService.getScriptProperties().setProperty(
        CONFIG.HEARTBEAT_MONITOR_ENABLED_KEY,
        enabled ? 'true' : 'false'
    );

    // Tạm dừng do người dùng chủ động không phải sự cố offline và không được
    // tạo email phục hồi giả khi giám sát được bật lại.
    if (!enabled) {
        saveHeartbeatConnectionState_('PAUSED', changedAt);
    }

    return textResponse(
        'OK:HEARTBEAT_MONITOR_' + (enabled ? 'ENABLED' : 'PAUSED') +
        ';changed_at=' + safeValue(changedAt)
    );
}

/** Tạo/cập nhật bản ghi phá hoại, đánh dấu đang giám sát và ghi con trỏ sự kiện hoạt động. */
function handleSabotageRequest(params) {
    const now = nowText(); // Thời gian máy chủ xử lý cảnh báo phá hoại.
    const eventId = params.eventId || getActiveSabotageEventId_() || createEventId('SABOTAGE'); // ID tái sử dụng hoặc mới.
    let record = loadEventRecord(eventId); // Bản ghi có thể được tạo mới nếu ID chưa tồn tại.

    if (!record) {
        record = buildBaseSecurityRecord(params, eventId, 'sabotage_alert', 'DEVICE');
        record.monitoring_status = STATUS.MONITORING;
        record.sabotageDetectedAt = now;
        record.criticalRaisedAt = '';
        record.lastHeartbeatAt = getLastHeartbeatServerTime_();
        record.message = firstNonEmpty(
            params.message,
            'Thiết bị phát hiện hành vi che cảm biến hoặc can thiệp phần cứng.'
        );
    }

    record.status = firstNonEmpty(params.status, record.status, 'SABOTAGE_ALERT');
    record.threat = firstNonEmpty(params.threat, params.threat_level, record.threat, '3');
    record.score = firstNonEmpty(params.score, params.intrusion_score, record.score, '0');
    record.rtcTime = firstNonEmpty(params.time, params.current_time, record.rtcTime, 'RTC_NOT_PROVIDED');
    record.serverTime = now;
    record.lastHeartbeatAt = getLastHeartbeatServerTime_();
    record.monitoring_status = STATUS.MONITORING;

    saveEventRecord(record);
    setActiveSabotageEventId_(eventId);

    return textResponse(
        'OK:SABOTAGE_MONITORING' +
        ';eventId=' + record.eventId +
        ';monitoring_status=' + record.monitoring_status +
        ';heartbeat_timeout_seconds=' + String(CONFIG.HEARTBEAT_TIMEOUT_SECONDS) +
        ';last_heartbeat_at=' + safeValue(record.lastHeartbeatAt)
    );
}

/** Nâng một sự kiện thành xâm phạm nghiêm trọng và gửi email xin xác nhận đúng một lần. */
function handleCriticalRequest(params, reason) {
    const eventId = params.eventId || getActiveSabotageEventId_() || createEventId('CRITICAL'); // ID sự kiện cần nâng cấp.
    const existing = loadEventRecord(eventId); // Bản ghi phá hoại/critical đã có, nếu tồn tại.

    if (existing && existing.eventType === 'critical_security_compromise') {
        return textResponse(formatStatusResponse(existing));
    }

    const record = existing || buildBaseSecurityRecord(params, eventId, 'critical_security_compromise', 'DEVICE'); // Bản ghi sẽ cập nhật.
    record.eventType = 'critical_security_compromise';
    record.source = normalizeSource(firstNonEmpty(params.source, record.source, 'DEVICE'));
    record.status = firstNonEmpty(params.status, 'CRITICAL_SECURITY_COMPROMISE');
    record.threat = firstNonEmpty(params.threat, params.threat_level, record.threat, '4');
    record.score = firstNonEmpty(params.score, params.intrusion_score, record.score, '0');
    record.rtcTime = firstNonEmpty(params.time, params.current_time, record.rtcTime, 'RTC_NOT_PROVIDED');
    record.serverTime = nowText();
    record.message = firstNonEmpty(
        params.message,
        reason,
        'Thiết bị đang ở trạng thái suy giảm nghiêm trọng sau cảnh báo phá hoại.'
    );
    record.monitoring_status = STATUS.MONITORING;
    record.criticalRaisedAt = nowText();
    record.lastHeartbeatAt = getLastHeartbeatServerTime_();
    record.homeAddressConfigured = isHomeAddressConfigured();
    record.emergency_escalation_status = STATUS.WAITING_CONFIRMATION;
    record.emergency_authority_message_status = 'IDLE';
    record.emergency_confirmation_requested = true;
    record.emergency_confirmed = false;
    record.confirmationSentAt = record.confirmationSentAt || nowText();
    record.confirmedAt = '';
    record.escalationSentAt = '';
    record.lastError = '';

    saveEventRecord(record);
    setActiveSabotageEventId_(eventId);
    sendCriticalConfirmationEmailOnce_(record);

    return textResponse(
        'OK:CRITICAL_WAITING_CONFIRMATION' +
        ';eventId=' + record.eventId +
        ';emergency_confirmation_requested=true' +
        ';emergency_confirmed=false' +
        ';emergency_escalation_status=' + record.emergency_escalation_status +
        ';emergency_authority_message_status=' + record.emergency_authority_message_status +
        ';home_address_configured=' + String(record.homeAddressConfigured)
    );
}

/** Tạo cấu trúc bản ghi an ninh cơ sở với đầy đủ giá trị mặc định và trạng thái ban đầu. */
function buildBaseSecurityRecord(params, eventId, eventType, sourceFallback) {
    return {
        eventId: eventId,
        eventType: eventType,
        source: normalizeSource(firstNonEmpty(params.source, sourceFallback, 'UNKNOWN')),
        device: firstNonEmpty(params.device, params.deviceName, CONFIG.DEFAULT_DEVICE_NAME),
        location: firstNonEmpty(params.location, params.zone, CONFIG.DEFAULT_DEVICE_LOCATION),
        status: firstNonEmpty(params.status, 'ACTIVE'),
        threat: firstNonEmpty(params.threat, params.threat_level, '0'),
        score: firstNonEmpty(params.score, params.intrusion_score, '0'),
        rtcTime: firstNonEmpty(params.time, params.current_time, 'RTC_NOT_PROVIDED'),
        serverTime: nowText(),
        message: firstNonEmpty(params.message, ''),
        note: firstNonEmpty(params.note, ''),
        homeAddressConfigured: isHomeAddressConfigured(),
        emergency_escalation_status: STATUS.IDLE,
        emergency_authority_message_status: 'IDLE',
        emergency_confirmation_requested: false,
        emergency_confirmed: false,
        confirmationSentAt: '',
        confirmedAt: '',
        escalationSentAt: '',
        monitoring_status: STATUS.MONITORING,
        lastError: ''
    };
}

/** Tạo sự kiện SOS duy nhất, chống gửi trùng khi còn chờ và gửi email xin xác nhận. */
function handleSosRequest(params, eventType) {
    const now = nowText(); // Thời gian máy chủ tạo SOS.
    const source = normalizeSource(params.source || eventType || 'UNKNOWN'); // Nguồn kích hoạt đã chuẩn hóa.
    const requestedEventId = params.eventId || ''; // ID do thiết bị chỉ định khi retry/callback.
    const activeEventId = getActiveSosEventId_(); // ID SOS đang chờ để chống tạo trùng.

    // Callback Dashboard hoặc lần retry không được tạo email xác nhận mới
    // khi SOS trước vẫn đang chờ xác nhận.
    if (!requestedEventId && activeEventId) {
        const activeRecord = loadEventRecord(activeEventId); // Bản ghi SOS đang mở dùng trả lại trạng thái cũ.
        if (activeRecord && activeRecord.monitoring_status !== STATUS.RESOLVED) {
            return textResponse(
                'OK:WAITING_CONFIRMATION' +
                ';eventId=' + activeRecord.eventId +
                ';emergency_confirmation_requested=' + String(Boolean(activeRecord.emergency_confirmation_requested)) +
                ';emergency_confirmed=' + String(Boolean(activeRecord.emergency_confirmed)) +
                ';emergency_escalation_status=' + activeRecord.emergency_escalation_status +
                ';emergency_authority_message_status=' + activeRecord.emergency_authority_message_status +
                ';home_address_configured=' + String(activeRecord.homeAddressConfigured)
            );
        }
    }

    const eventId = requestedEventId || createEventId('SOS'); // ID cuối cùng của SOS mới.

    const record = { // Bản ghi đầy đủ của quy trình xác nhận SOS.
        eventId: eventId,
        eventType: 'sos_alert',
        source: source,
        device: firstNonEmpty(params.device, params.deviceName, CONFIG.DEFAULT_DEVICE_NAME),
        location: firstNonEmpty(params.location, params.zone, CONFIG.DEFAULT_DEVICE_LOCATION),
        status: firstNonEmpty(params.status, 'SOS_ALERT'),
        threat: firstNonEmpty(params.threat, params.threat_level, '4'),
        score: firstNonEmpty(params.score, params.intrusion_score, '0'),
        rtcTime: firstNonEmpty(params.time, params.current_time, 'RTC_NOT_PROVIDED'),
        serverTime: now,
        message: firstNonEmpty(params.message, params.sos_message, 'SOS was triggered.'),
        note: firstNonEmpty(params.sos_authority_note, params.note, ''),
        homeAddressConfigured: isHomeAddressConfigured(),
        emergency_escalation_status: STATUS.WAITING_CONFIRMATION,
        emergency_authority_message_status: 'IDLE',
        emergency_confirmation_requested: true,
        emergency_confirmed: false,
        confirmationSentAt: now,
        confirmedAt: '',
        escalationSentAt: '',
        monitoring_status: STATUS.MONITORING,
        lastError: ''
    };

    saveEventRecord(record);
    setActiveSosEventId_(record.eventId);
    sendSosConfirmationEmail(record);

    return textResponse(
        'OK:WAITING_CONFIRMATION' +
        ';eventId=' + record.eventId +
        ';emergency_confirmation_requested=true' +
        ';emergency_confirmed=false' +
        ';emergency_escalation_status=' + record.emergency_escalation_status +
        ';emergency_authority_message_status=' + record.emergency_authority_message_status +
        ';home_address_configured=' + String(record.homeAddressConfigured)
    );
}

// ==================================================
// 4. POLLING STATUS FOR ESP32-S3
// ==================================================

/**
 * Hàm trigger định kỳ: kiểm tra mất heartbeat sau phá hoại và nâng thành sự kiện nghiêm trọng.
 * Đồng thời luôn kiểm tra trạng thái kết nối chung để phát email offline/recovery đúng một lần.
 */
function monitorHeartbeatAfterSabotage() {
    const connectionResult = monitorHeartbeatConnection_(); // Kết quả kiểm tra online/offline chung của lần trigger.
    const eventId = getActiveSabotageEventId_(); // Sự kiện phá hoại cần kiểm tra mất heartbeat sau can thiệp.

    if (!eventId) {
        return textResponse(
            'OK:' + connectionResult.state +
            ';heartbeat_age_seconds=' + String(connectionResult.ageSeconds)
        );
    }

    const record = loadEventRecord(eventId); // Bản ghi phá hoại/critical đang được theo dõi.

    if (!record || record.monitoring_status === STATUS.RESOLVED) {
        clearActiveSabotageEventId_();
        return textResponse('OK:NO_ACTIVE_SABOTAGE');
    }

    const lastHeartbeat = getLastHeartbeat_(); // Heartbeat cuối dùng tính thời gian im lặng.

    if (!lastHeartbeat) {
        return handleCriticalRequest({
            eventId: eventId,
            event: 'critical_security_compromise',
            source: 'CLOUD_MONITOR',
            device: record.device,
            location: record.location,
            status: 'CRITICAL_SECURITY_COMPROMISE',
            threat: '4',
            score: record.score || '0',
            time: 'HEARTBEAT_NOT_RECEIVED',
            message: 'Cloud không nhận được heartbeat sau khi thiết bị đã báo phá hoại.'
        }, 'Cloud không nhận được heartbeat sau khi thiết bị đã báo phá hoại.');
    }

    const ageSeconds = Math.floor((new Date().getTime() - new Date(lastHeartbeat.serverTime).getTime()) / 1000); // Tuổi heartbeat theo giây.
    record.lastHeartbeatAt = lastHeartbeat.serverTime;
    saveEventRecord(record);

    if (ageSeconds > CONFIG.HEARTBEAT_TIMEOUT_SECONDS) {
        return handleCriticalRequest({
            eventId: eventId,
            event: 'critical_security_compromise',
            source: 'CLOUD_MONITOR',
            device: record.device,
            location: record.location,
            status: 'CRITICAL_SECURITY_COMPROMISE',
            threat: '4',
            score: record.score || '0',
            time: record.rtcTime || 'RTC_NOT_PROVIDED',
            message: 'Cloud không nhận heartbeat trong ' + ageSeconds + ' giây sau cảnh báo phá hoại.'
        }, 'Cloud phát hiện mất heartbeat sau cảnh báo phá hoại.');
    }

    return textResponse(
        'OK:MONITORING' +
        ';eventId=' + eventId +
        ';heartbeat_age_seconds=' + String(ageSeconds) +
        ';heartbeat_timeout_seconds=' + String(CONFIG.HEARTBEAT_TIMEOUT_SECONDS)
    );
}

// installHeartbeatMonitorTrigger() gọi hàm này mỗi phút. Heartbeat bị mất chỉ
// tạo một email gia đình, sau đó im lặng đến khi bo mạch trở lại. Nếu đang có
// phá hoại, thiết bị dùng luồng nâng cấp nghiêm trọng riêng bên dưới để tránh
// gửi trùng email offline thông thường.
/** So tuổi heartbeat với timeout, chuyển ONLINE/OFFLINE và gửi email mất kết nối khi vừa đổi trạng thái. */
function monitorHeartbeatConnection_() {
    if (!isHeartbeatMonitorEnabled_()) {
        return { state: 'HEARTBEAT_MONITOR_PAUSED', ageSeconds: -1 };
    }

    const lastHeartbeat = getLastHeartbeat_(); // Heartbeat cuối cùng đã lưu, có thể null lúc chưa khởi động.
    if (!lastHeartbeat || !lastHeartbeat.serverTime) {
        return { state: 'WAITING_FIRST_HEARTBEAT', ageSeconds: -1 };
    }

    const ageSeconds = Math.floor( // Số giây từ heartbeat cuối đến thời điểm trigger chạy.
        (new Date().getTime() - new Date(lastHeartbeat.serverTime).getTime()) / 1000
    );
    const isOffline = ageSeconds > CONFIG.HEARTBEAT_TIMEOUT_SECONDS; // Kết quả so với ngưỡng mất kết nối.
    const activeSabotage = getActiveSabotageEventId_(); // ID phá hoại dùng đưa vào email offline.
    const connectionState = getHeartbeatConnectionState_(); // Trạng thái trước lần đánh giá này.

    if (!isOffline) {
        return { state: 'ONLINE', ageSeconds: ageSeconds };
    }

    // Nhánh nghiêm trọng tự gửi email xác nhận riêng sau cảnh báo phá hoại.
    if (activeSabotage) {
        return { state: 'SABOTAGE_TIMEOUT', ageSeconds: ageSeconds };
    }

    if (connectionState.state !== 'OFFLINE') {
        sendHeartbeatOfflineEmail_(lastHeartbeat, ageSeconds);
        saveHeartbeatConnectionState_('OFFLINE', nowText());
    }

    return { state: 'OFFLINE', ageSeconds: ageSeconds };
}

/** Đọc cờ Script Properties; mặc định bật nếu khóa chưa từng được ghi. */
function isHeartbeatMonitorEnabled_() {
    const configured = PropertiesService.getScriptProperties().getProperty( // Giá trị chuỗi true/false đã lưu.
        CONFIG.HEARTBEAT_MONITOR_ENABLED_KEY
    );
    // Deployment hiện có mặc định giám sát bình thường đến khi firmware gửi
    // một lệnh điều khiển rõ ràng từ Dashboard.
    return configured !== 'false';
}

/** Đọc và parse trạng thái kết nối đã lưu; trả trạng thái mặc định nếu chưa có hoặc JSON lỗi. */
function getHeartbeatConnectionState_() {
    const raw = PropertiesService.getScriptProperties().getProperty(CONFIG.HEARTBEAT_CONNECTION_STATE_KEY); // JSON trạng thái thô.
    if (!raw) {
        return { state: 'UNKNOWN', changedAt: '' };
    }
    try {
        return JSON.parse(raw);
    } catch (err) {
        return { state: 'UNKNOWN', changedAt: '' };
    }
}

/** Lưu mã trạng thái kết nối cùng thời điểm thay đổi vào Script Properties. */
function saveHeartbeatConnectionState_(state, changedAt) {
    PropertiesService.getScriptProperties().setProperty(
        CONFIG.HEARTBEAT_CONNECTION_STATE_KEY,
        JSON.stringify({ state: state, changedAt: changedAt })
    );
}

/** Gửi email gia đình báo thiết bị offline, kèm heartbeat cuối và số giây đã mất tín hiệu. */
function sendHeartbeatOfflineEmail_(lastHeartbeat, ageSeconds) {
    const subject = '[Cảnh báo kết nối] Thiết bị mất tín hiệu'; // Tiêu đề email offline.
    const plainBody = // Phiên bản nội dung văn bản thuần.
        'Thiết bị mất tín hiệu heartbeat trong ' + ageSeconds + ' giây.\n\n' +
        'Thiết bị: ' + safeValue(lastHeartbeat.device) + '\n' +
        'Khu vực: ' + safeValue(lastHeartbeat.location) + '\n' +
        'Heartbeat cuối: ' + safeValue(lastHeartbeat.serverTime) + '\n\n' +
        'Vui lòng kiểm tra nguồn điện, Wi-Fi và thiết bị.';
    const htmlBody = // Phiên bản nội dung HTML.
        htmlTitle('Cảnh báo: thiết bị mất tín hiệu') +
        htmlParagraph('Cloud không nhận heartbeat trong ' + ageSeconds + ' giây.') +
        htmlRow('Thiết bị', safeValue(lastHeartbeat.device)) +
        htmlRow('Khu vực', safeValue(lastHeartbeat.location)) +
        htmlRow('Heartbeat cuối', safeValue(lastHeartbeat.serverTime)) +
        htmlWarning('Vui lòng kiểm tra nguồn điện, Wi-Fi và thiết bị.');
    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
}

/** Gửi email gia đình khi heartbeat xuất hiện lại sau trạng thái OFFLINE. */
function sendHeartbeatRecoveryEmail_(heartbeat) {
    const subject = '[Khôi phục kết nối] Thiết bị đã gửi tín hiệu trở lại'; // Tiêu đề email phục hồi.
    const plainBody = // Nội dung phục hồi dạng văn bản thuần.
        'Thiết bị đã kết nối lại và đang gửi heartbeat bình thường.\n\n' +
        'Thiết bị: ' + safeValue(heartbeat.device) + '\n' +
        'Khu vực: ' + safeValue(heartbeat.location) + '\n' +
        'Thời điểm khôi phục: ' + safeValue(heartbeat.serverTime);
    const htmlBody = // Nội dung phục hồi dạng HTML.
        htmlTitle('Thiết bị đã kết nối lại') +
        htmlParagraph('Thiết bị đã gửi heartbeat trở lại và đang hoạt động bình thường.') +
        htmlRow('Thiết bị', safeValue(heartbeat.device)) +
        htmlRow('Khu vực', safeValue(heartbeat.location)) +
        htmlRow('Thời điểm khôi phục', safeValue(heartbeat.serverTime));
    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
}

/** Xóa trigger trùng và cài trigger thời gian gọi monitorHeartbeatAfterSabotage mỗi phút. */
function installHeartbeatMonitorTrigger() {
    const triggers = ScriptApp.getProjectTriggers(); // Danh sách trigger hiện có để xóa bản trùng.
    triggers.forEach(function (trigger) {
        if (trigger.getHandlerFunction && trigger.getHandlerFunction() === 'monitorHeartbeatAfterSabotage') {
            ScriptApp.deleteTrigger(trigger);
        }
    });

    ScriptApp.newTrigger('monitorHeartbeatAfterSabotage')
        .timeBased()
        .everyMinutes(1)
        .create();
}

/** Trả trạng thái một eventId cho ESP32 polling, hoặc trạng thái heartbeat khi không có eventId. */
function handleStatus(params) {
    const eventId = params.eventId || getActiveSosEventId_() || getActiveSabotageEventId_() || ''; // ID cần polling.

    if (!eventId) {
        return textResponse(
            'OK:NO_EVENT' +
            ';last_heartbeat_at=' + safeValue(getLastHeartbeatServerTime_()) +
            ';home_address_configured=' + String(isHomeAddressConfigured())
        );
    }

    const record = loadEventRecord(eventId); // Bản ghi dùng tạo chuỗi trạng thái trả firmware.

    if (!record) {
        return textResponse(
            'OK:UNKNOWN_EVENT' +
            ';eventId=' + eventId +
            ';emergency_confirmation_requested=false' +
            ';emergency_confirmed=false' +
            ';emergency_escalation_status=' + STATUS.UNKNOWN_EVENT +
            ';emergency_authority_message_status=' + STATUS.UNKNOWN_EVENT +
            ';home_address_configured=' + String(isHomeAddressConfigured())
        );
    }

    return textResponse(formatStatusResponse(record));
}

/** Chuyển bản ghi sự kiện thành chuỗi key=value ổn định để firmware phân tích. */
function formatStatusResponse(record) {
    return (
        'OK:STATUS' +
        ';eventId=' + safeValue(record.eventId) +
        ';eventType=' + safeValue(record.eventType) +
        ';monitoring_status=' + safeValue(record.monitoring_status || STATUS.IDLE) +
        ';emergency_confirmation_requested=' + String(Boolean(record.emergency_confirmation_requested)) +
        ';emergency_confirmed=' + String(Boolean(record.emergency_confirmed)) +
        ';emergency_escalation_status=' + safeValue(record.emergency_escalation_status) +
        ';emergency_authority_message_status=' + safeValue(record.emergency_authority_message_status) +
        ';home_address_configured=' + String(Boolean(record.homeAddressConfigured)) +
        ';source=' + safeValue(record.source) +
        ';last_heartbeat_at=' + safeValue(record.lastHeartbeatAt || getLastHeartbeatServerTime_()) +
        ';confirmedAt=' + safeValue(record.confirmedAt) +
        ';escalationSentAt=' + safeValue(record.escalationSentAt)
    );
}

/** Đánh dấu một hoặc các sự kiện hoạt động là RESOLVED và xóa các con trỏ tương ứng. */
function handleResolve(params) {
    const requestedEventId = params.eventId || ''; // ID cụ thể; rỗng nghĩa là đóng các sự kiện đang hoạt động.
    const eventIds = requestedEventId // Danh sách ID duy nhất thực sự sẽ được xử lý.
        ? [requestedEventId]
        : uniqueEventIds_([getActiveSosEventId_(), getActiveSabotageEventId_()]);

    if (eventIds.length === 0) {
        return textResponse('OK:RESOLVED;eventId=NONE');
    }

    eventIds.forEach(function (eventId) {
        const record = loadEventRecord(eventId); // Bản ghi tương ứng với ID đang duyệt.

        if (record) {
            record.monitoring_status = STATUS.RESOLVED;
            record.resolvedAt = nowText();
            saveEventRecord(record);
        }

        if (getActiveSabotageEventId_() === eventId) {
            clearActiveSabotageEventId_();
        }
        if (getActiveSosEventId_() === eventId) {
            clearActiveSosEventId_();
        }
    });

    return textResponse('OK:RESOLVED;eventId=' + eventIds.join(','));
}

// ==================================================
// 5. PARENT/ADMIN CONFIRMATION FLOW
// ==================================================

/** Kết xuất trang HTML xác nhận, hiển thị tóm tắt sự kiện và ô ghi chú cơ quan. */
function handleConfirmPage(params) {
    const eventId = params.eventId || ''; // ID lấy từ link trong email.

    if (!eventId) {
        return htmlResponse(pageShell(
            htmlTitle('Thiếu Event ID') +
            htmlParagraph('Không thể xác nhận vì URL thiếu eventId.')
        ));
    }

    const record = loadEventRecord(eventId); // Dữ liệu sự kiện hiển thị trên trang xác nhận.

    if (!record) {
        return htmlResponse(pageShell(
            htmlTitle('Không tìm thấy sự kiện') +
            htmlParagraph('Sự kiện có thể đã hết hạn hoặc eventId không đúng.') +
            htmlRow('Event ID', eventId)
        ));
    }

    if (record.emergency_confirmed) {
        return htmlResponse(pageShell(
            htmlTitle('Sự kiện đã được xác nhận trước đó') +
            htmlParagraph('Không cần xác nhận lại. Còi/LED trên thiết bị vẫn phải được tắt bằng reset_alarm trên Arduino Cloud.') +
            renderEventSummary(record)
        ));
    }

    const url = ScriptApp.getService().getUrl(); // URL deployment dùng làm action của form POST.
    const pageTitle = record.eventType === 'critical_security_compromise' // Tiêu đề thay đổi theo loại sự kiện.
        ? 'Xác nhận cảnh báo nghiêm trọng'
        : 'Xác nhận SOS escalation';

    const html = // Toàn bộ phần nội dung trong khung trang xác nhận.
        htmlTitle(pageTitle) +
        htmlParagraph('Bấm xác nhận để gửi email escalation đến contact cơ quan chức năng/công an mô phỏng đã cấu hình.') +
        htmlWarning('ACK/xác nhận này KHÔNG tắt còi/LED. Parent/Admin vẫn phải bấm reset_alarm trên Arduino Cloud để kết thúc cảnh báo tại thiết bị.') +
        renderEventSummary(record) +
        '<form method="get" action="' + escapeHtml(url) + '">' +
        '<input type="hidden" name="action" value="confirm_send">' +
        '<input type="hidden" name="eventId" value="' + escapeHtml(eventId) + '">' +
        '<label><b>Ghi chú bổ sung gửi cho contact tiếp nhận:</b></label><br>' +
        '<textarea name="note" rows="5" style="width:100%;max-width:720px;" placeholder="Ví dụ: Có trẻ em ở nhà, cửa bếp đang mở, vui lòng liên hệ phụ huynh trước khi đến..."></textarea><br><br>' +
        '<button type="submit" style="padding:10px 16px;background:#d93025;color:white;border:0;border-radius:6px;font-weight:bold;">Xác nhận gửi escalation</button>' +
        '</form>';

    return htmlResponse(pageShell(html));
}

/**
 * Xử lý form xác nhận: khóa chống chạy đồng thời, kiểm tra sự kiện, lưu ghi chú
 * và chỉ gửi email cơ quan demo một lần khi đủ cấu hình.
 */
function handleConfirmSend(params) {
    const eventId = params.eventId || '';             // ID sự kiện ẩn được form gửi lại.
    const note = trimToLength(params.note || '', 1000); // Ghi chú cơ quan đã trim và giới hạn 1.000 ký tự.

    if (!eventId) {
        return htmlResponse(pageShell(
            htmlTitle('Thiếu Event ID') +
            htmlParagraph('Không thể gửi escalation vì URL thiếu eventId.')
        ));
    }

    const lock = LockService.getScriptLock(); // Khóa toàn script chống hai người xác nhận/gửi thư đồng thời.
    lock.waitLock(10000);

    try {
        const record = loadEventRecord(eventId); // Bản ghi mới nhất sau khi đã lấy khóa.

        if (!record) {
            return htmlResponse(pageShell(
                htmlTitle('Không tìm thấy sự kiện') +
                htmlParagraph('Sự kiện có thể đã hết hạn hoặc eventId không đúng.') +
                htmlRow('Event ID', eventId)
            ));
        }

        if (record.emergency_confirmed) {
            return htmlResponse(pageShell(
                htmlTitle('Sự kiện đã được xử lý') +
                htmlParagraph('Đã có Parent/Admin xác nhận sự kiện này trước đó.') +
                renderEventSummary(record)
            ));
        }

        record.note = note || record.note || '';
        record.emergency_confirmed = true;
        record.emergency_escalation_status = STATUS.CONFIRMED;
        record.confirmedAt = nowText();

        if (!isHomeAddressConfigured()) {
            record.homeAddressConfigured = false;
            record.emergency_escalation_status = STATUS.NOT_CONFIGURED;
            record.emergency_authority_message_status = STATUS.NOT_CONFIGURED;
            record.lastError = 'Home address is not configured.';
            saveEventRecord(record);
            sendFamilyStatusEmail(record, 'Đã xác nhận nhưng chưa gửi escalation vì thiếu địa chỉ nhà cấu hình.');

            return htmlResponse(pageShell(
                htmlTitle('Chưa gửi được escalation') +
                htmlWarning('HOME_ADDRESS chưa được cấu hình hợp lệ trong Apps Script hoặc Script Properties.') +
                htmlParagraph('Còi/LED trên thiết bị vẫn phải được reset bằng reset_alarm trên Arduino Cloud.') +
                renderEventSummary(record)
            ));
        }

        if (!hasAuthorityRecipients()) {
            record.homeAddressConfigured = true;
            record.emergency_escalation_status = STATUS.NOT_CONFIGURED;
            record.emergency_authority_message_status = STATUS.NOT_CONFIGURED;
            record.lastError = 'Authority demo recipient is not configured.';
            saveEventRecord(record);
            sendFamilyStatusEmail(record, 'Đã xác nhận nhưng chưa gửi escalation vì thiếu contact mô phỏng.');

            return htmlResponse(pageShell(
                htmlTitle('Chưa gửi được escalation') +
                htmlWarning('AUTHORITY_DEMO_RECIPIENTS chưa được cấu hình.') +
                htmlParagraph('Còi/LED trên thiết bị vẫn phải được reset bằng reset_alarm trên Arduino Cloud.') +
                renderEventSummary(record)
            ));
        }

        try {
            sendAuthorityEscalationEmail(record);
            record.homeAddressConfigured = true;
            record.emergency_escalation_status = STATUS.SENT;
            record.emergency_authority_message_status = STATUS.SENT;
            record.escalationSentAt = nowText();
            record.lastError = '';
            saveEventRecord(record);
            sendFamilyStatusEmail(record, 'Đã gửi escalation đến contact mô phỏng sau khi Parent/Admin xác nhận.');

            return htmlResponse(pageShell(
                htmlTitle('Xác nhận thành công') +
                htmlParagraph('Đã gửi email escalation đến contact mô phỏng đã cấu hình.') +
                htmlWarning('ACK không reset cảnh báo tại chỗ. Hãy bấm reset_alarm trên Arduino Cloud khi xử lý xong.') +
                renderEventSummary(record)
            ));

        } catch (err) {
            record.emergency_escalation_status = STATUS.FAILED;
            record.emergency_authority_message_status = STATUS.FAILED;
            record.lastError = err.message;
            saveEventRecord(record);
            sendFamilyStatusEmail(record, 'Gửi SOS escalation thất bại: ' + err.message);

            return htmlResponse(pageShell(
                htmlTitle('Gửi escalation thất bại') +
                htmlWarning(err.message) +
                htmlParagraph('Còi/LED trên thiết bị vẫn phải được reset bằng reset_alarm trên Arduino Cloud.') +
                renderEventSummary(record)
            ));
        }

    } finally {
        lock.releaseLock();
    }
}

// ==================================================
// 6. EMAIL BUILDERS
// ==================================================

/** Gửi email xin xác nhận cho sự kiện nghiêm trọng nếu bản ghi chưa đánh dấu đã gửi. */
function sendCriticalConfirmationEmailOnce_(record) {
    if (record.criticalConfirmationEmailSentAt) {
        return;
    }

    sendSosConfirmationEmail(record);
    record.criticalConfirmationEmailSentAt = nowText();
    saveEventRecord(record);
}

/** Tạo link xác nhận và gửi email SOS đến toàn bộ người nhận gia đình. */
function sendSosConfirmationEmail(record) {
    const confirmUrl = buildUrl({ action: 'confirm', eventId: record.eventId }); // Link mở trang xác nhận.
    const statusUrl = buildUrl({ action: 'status', eventId: record.eventId });   // Link xem phản hồi trạng thái thô.
    const isCritical = record.eventType === 'critical_security_compromise';      // Chọn cách diễn đạt mức nghiêm trọng.

    const subject = (isCritical ? 'Cảnh báo nghiêm trọng cần xác nhận - ' : 'Yêu cầu xác nhận SOS - ') + CONFIG.PROJECT_NAME; // Tiêu đề email.
    const intro = isCritical // Đoạn mở đầu giải thích loại cảnh báo.
        ? 'Cloud phát hiện thiết bị có thể đã mất liên lạc hoặc suy giảm nghiêm trọng sau cảnh báo phá hoại. Cần Parent/Admin xác nhận trước khi gửi escalation.'
        : 'Hệ thống vừa nhận SOS và cần Parent/Admin xác nhận trước khi gửi escalation.';

    const plainBody = // Email xác nhận dạng văn bản thuần.
        intro + '\n\n' +
        plainEventSummary(record) +
        'Home address configured: ' + String(isHomeAddressConfigured()) + '\n\n' +
        'Xác nhận gửi báo cáo tại link sau:\n' + confirmUrl + '\n\n' +
        'Kiểm tra trạng thái polling:\n' + statusUrl + '\n\n' +
        'Lưu ý: Xác nhận email KHÔNG tắt còi/LED. Cần dùng reset_alarm trên Arduino Cloud để kết thúc cảnh báo tại thiết bị.';

    const htmlBody = // Email xác nhận dạng HTML có nút bấm.
        htmlTitle(isCritical ? 'Cảnh báo nghiêm trọng cần xác nhận' : 'Yêu cầu xác nhận SOS') +
        htmlParagraph(intro) +
        htmlWarning('Xác nhận này KHÔNG tắt còi/LED. Mất heartbeat không chứng minh chắc chắn đã bị cắt điện; cũng có thể do WiFi, nguồn hoặc lỗi mạng.') +
        renderEventSummary(record) +
        htmlRow('Home address configured', String(isHomeAddressConfigured())) +
        htmlButton(confirmUrl, 'Mở trang xác nhận escalation') +
        htmlParagraph('Polling status URL: ' + statusUrl);

    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
}

/** Gửi email nâng cấp cuối cùng đến liên hệ cơ quan demo với địa chỉ và ghi chú đã xác nhận. */
function sendAuthorityEscalationEmail(record) {
    const homeAddress = getHomeAddress(); // Địa chỉ đã cấu hình đưa vào báo cáo cuối.
    const isCritical = record.eventType === 'critical_security_compromise'; // Phân biệt báo cáo critical và SOS.
    const subject = (isCritical ? 'Báo cáo sự cố nghiêm trọng demo - ' : 'Báo cáo SOS demo - ') + CONFIG.PROJECT_NAME; // Tiêu đề cơ quan demo.

    const plainBody = // Báo cáo gửi cơ quan dạng text/plain.
        'Parent/Admin đã xác nhận gửi báo cáo đến contact mô phỏng.\n\n' +
        plainEventSummary(record) +
        'Địa chỉ nhà/khu vực cần hỗ trợ:\n' + homeAddress + '\n\n' +
        'Ghi chú bổ sung:\n' + (record.note || 'Không có') + '\n\n' +
        'Đây là email mô phỏng phục vụ demo đồ án. Không gửi đến cơ quan thật nếu chưa có cấu hình hợp lệ và sự đồng ý của chủ hệ thống.';

    const htmlBody = // Báo cáo gửi cơ quan dạng HTML.
        htmlTitle(isCritical ? 'Báo cáo sự cố nghiêm trọng demo' : 'Báo cáo SOS demo') +
        htmlParagraph('Parent/Admin đã xác nhận gửi báo cáo đến contact mô phỏng.') +
        renderEventSummary(record) +
        htmlRow('Địa chỉ nhà/khu vực cần hỗ trợ', homeAddress) +
        htmlRow('Ghi chú bổ sung', record.note || 'Không có') +
        htmlWarning('Đây là contact mô phỏng phục vụ demo. Không gửi đến cơ quan thật nếu chưa có cấu hình hợp lệ và sự đồng ý của chủ hệ thống.');

    sendEmail(CONFIG.AUTHORITY_DEMO_RECIPIENTS, subject, plainBody, htmlBody);
}

/** Gửi email cập nhật kết quả xác nhận/nâng cấp trở lại cho gia đình. */
function sendFamilyStatusEmail(record, message) {
    const subject = 'Cập nhật trạng thái gửi báo cáo - ' + CONFIG.PROJECT_NAME; // Tiêu đề cập nhật cho gia đình.

    const plainBody = // Nội dung trạng thái dạng văn bản thuần.
        message + '\n\n' +
        plainEventSummary(record) +
        'emergency_escalation_status: ' + record.emergency_escalation_status + '\n' +
        'emergency_authority_message_status: ' + record.emergency_authority_message_status + '\n\n' +
        'Lưu ý: ACK/escalation không reset cảnh báo tại chỗ. Cần reset_alarm trên Arduino Cloud.';

    const htmlBody = // Nội dung trạng thái dạng HTML.
        htmlTitle('Cập nhật trạng thái gửi báo cáo') +
        htmlParagraph(message) +
        renderEventSummary(record) +
        htmlRow('emergency_escalation_status', record.emergency_escalation_status) +
        htmlRow('emergency_authority_message_status', record.emergency_authority_message_status) +
        htmlWarning('ACK/escalation không reset cảnh báo tại chỗ. Cần reset_alarm trên Arduino Cloud.');

    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
}

// ==================================================
// 7. EVENT STORAGE
// ==================================================

/** Tuần tự hóa và lưu bản ghi theo khóa EVENT_PROPERTY_PREFIX + eventId. */
function saveEventRecord(record) {
    const props = PropertiesService.getScriptProperties(); // Kho lưu bền cấp script.
    props.setProperty(CONFIG.EVENT_PROPERTY_PREFIX + record.eventId, JSON.stringify(record));
}

/** Đọc và parse bản ghi theo eventId; trả null nếu thiếu ID, không tồn tại hoặc JSON hỏng. */
function loadEventRecord(eventId) {
    const props = PropertiesService.getScriptProperties(); // Kho chứa bản ghi sự kiện.
    const raw = props.getProperty(CONFIG.EVENT_PROPERTY_PREFIX + eventId); // Chuỗi JSON thô của eventId.

    if (!raw) {
        return null;
    }

    try {
        return JSON.parse(raw);
    } catch (err) {
        return null;
    }
}

/** Xóa bản ghi quá EVENT_TTL_HOURS và dọn các con trỏ hoạt động không còn hợp lệ. */
function cleanupOldEvents_() {
    const props = PropertiesService.getScriptProperties(); // Kho dữ liệu cần cleanup.
    const all = props.getProperties();                      // Snapshot tất cả cặp khóa/giá trị hiện có.
    const prefix = CONFIG.EVENT_PROPERTY_PREFIX;            // Tiền tố nhận diện riêng khóa sự kiện.
    const nowMs = new Date().getTime();                      // Timestamp hiện tại tính bằng mili giây.
    const ttlMs = CONFIG.EVENT_TTL_HOURS * 60 * 60 * 1000;  // Thời gian sống sự kiện quy đổi sang mili giây.

    Object.keys(all).forEach(function (key) {
        if (key.indexOf(prefix) !== 0) {
            return;
        }

        try {
            const record = JSON.parse(all[key]); // Bản ghi đang được xét tuổi.
            const createdText = record.confirmationSentAt || record.serverTime || ''; // Mốc tạo ưu tiên.
            const createdMs = new Date(createdText).getTime(); // Mốc tạo quy đổi sang timestamp.

            if (createdMs && nowMs - createdMs > ttlMs) {
                props.deleteProperty(key);
            }
        } catch (err) {
            // Corrupt record; remove it.
            props.deleteProperty(key);
        }
    });

    clearStaleActiveEventPointers_();
}

/** Đọc heartbeat cuối từ Script Properties; trả null nếu chưa có hoặc dữ liệu lỗi. */
function getLastHeartbeat_() {
    const raw = PropertiesService.getScriptProperties().getProperty(CONFIG.HEARTBEAT_PROPERTY_KEY); // JSON heartbeat thô.

    if (!raw) {
        return null;
    }

    try {
        return JSON.parse(raw);
    } catch (err) {
        return null;
    }
}

/** Trả chuỗi serverTime của heartbeat gần nhất hoặc chuỗi rỗng nếu chưa có. */
function getLastHeartbeatServerTime_() {
    const record = getLastHeartbeat_(); // Bản ghi heartbeat cuối dùng lấy serverTime.
    return record ? record.serverTime : '';
}

/** Đọc ID sự kiện phá hoại đang hoạt động từ Script Properties. */
function getActiveSabotageEventId_() {
    return PropertiesService.getScriptProperties().getProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY) || '';
}

/** Ghi ID sự kiện phá hoại đang hoạt động vào Script Properties. */
function setActiveSabotageEventId_(eventId) {
    PropertiesService.getScriptProperties().setProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY, eventId);
}

/** Đọc ID sự kiện SOS đang hoạt động từ Script Properties. */
function getActiveSosEventId_() {
    return PropertiesService.getScriptProperties().getProperty(CONFIG.ACTIVE_SOS_EVENT_KEY) || '';
}

/** Ghi ID sự kiện SOS đang hoạt động vào Script Properties. */
function setActiveSosEventId_(eventId) {
    PropertiesService.getScriptProperties().setProperty(CONFIG.ACTIVE_SOS_EVENT_KEY, eventId);
}

/** Xóa con trỏ SOS hoạt động sau khi sự kiện đã được giải quyết. */
function clearActiveSosEventId_() {
    PropertiesService.getScriptProperties().deleteProperty(CONFIG.ACTIVE_SOS_EVENT_KEY);
}

/** Xóa con trỏ phá hoại hoạt động sau khi sự kiện đã được giải quyết. */
function clearActiveSabotageEventId_() {
    PropertiesService.getScriptProperties().deleteProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY);
}

/** Lọc chuỗi ID rỗng/trùng và trả danh sách ID duy nhất theo thứ tự đầu vào. */
function uniqueEventIds_(eventIds) {
    const unique = []; // Kết quả ID không rỗng và không trùng.
    eventIds.forEach(function (eventId) {
        if (eventId && unique.indexOf(eventId) < 0) {
            unique.push(eventId);
        }
    });
    return unique;
}

/** Xóa con trỏ hoạt động nếu bản ghi đích không còn tồn tại hoặc đã RESOLVED. */
function clearStaleActiveEventPointers_() {
    const activeSosEventId = getActiveSosEventId_(); // Con trỏ SOS cần xác thực còn hiệu lực.
    if (activeSosEventId && !loadEventRecord(activeSosEventId)) {
        clearActiveSosEventId_();
    }

    const activeSabotageEventId = getActiveSabotageEventId_(); // Con trỏ phá hoại cần xác thực.
    if (activeSabotageEventId && !loadEventRecord(activeSabotageEventId)) {
        clearActiveSabotageEventId_();
    }
}

// ==================================================
// 8. CONFIG HELPERS
// ==================================================

/** Kiểm tra token request bằng WEB_APP_TOKEN; tự cho phép khi token cấu hình đang rỗng. */
function isAuthorized(params) {
    if (!CONFIG.WEB_APP_TOKEN) {
        return true;
    }
    return params.key === CONFIG.WEB_APP_TOKEN;
}

/** Lấy HOME_ADDRESS từ Script Properties nếu có, nếu không dùng giá trị CONFIG. */
function getHomeAddress() {
    const fromProperty = PropertiesService.getScriptProperties().getProperty('HOME_ADDRESS'); // Địa chỉ quản trị cấu hình.
    const configuredProperty = fromProperty === 'DEMO_ADDRESS_NOT_CONFIGURED' ? '' : fromProperty; // Loại placeholder demo.
    return firstNonEmpty(configuredProperty, CONFIG.HOME_ADDRESS, '');
}

/** Trả về true khi địa chỉ nhà sau trim khác rỗng. */
function isHomeAddressConfigured() {
    const address = getHomeAddress(); // Địa chỉ cuối cùng sau khi áp ưu tiên property/cấu hình.
    return Boolean(address && address !== 'DEMO_ADDRESS_NOT_CONFIGURED');
}

/** Trả về true khi danh sách liên hệ cơ quan demo có ít nhất một email. */
function hasAuthorityRecipients() {
    return Array.isArray(CONFIG.AUTHORITY_DEMO_RECIPIENTS) && CONFIG.AUTHORITY_DEMO_RECIPIENTS.length > 0;
}

/** Ghép object query thành URL Web App với các khóa/giá trị đã encodeURIComponent. */
function buildUrl(query) {
    const baseUrl = CONFIG.PUBLIC_WEB_APP_URL || ScriptApp.getService().getUrl(); // URL gốc của deployment hiện tại.
    const parts = []; // Các cặp key=value đã encode sẽ được nối bằng dấu &.

    Object.keys(query).forEach(function (key) {
        parts.push(encodeURIComponent(key) + '=' + encodeURIComponent(query[key]));
    });

    if (CONFIG.WEB_APP_TOKEN) {
        parts.push('key=' + encodeURIComponent(CONFIG.WEB_APP_TOKEN));
    }

    return baseUrl + '?' + parts.join('&');
}

// ==================================================
// 9. DEBUG / MANUAL TEST
// ==================================================

/** Endpoint chẩn đoán trả bản ghi JSON thô theo eventId; không dùng trong luồng vận hành chính. */
function handleDebugEvent(params) {
    const eventId = params.eventId || ''; // ID được yêu cầu xem dữ liệu chẩn đoán.
    const record = eventId ? loadEventRecord(eventId) : null; // Bản ghi hoặc null nếu thiếu/không tồn tại.

    if (!record) {
        return textResponse('DEBUG: Event not found. eventId=' + eventId);
    }

    return textResponse(JSON.stringify(record, null, 2));
}

/** Hàm kiểm thử thủ công: giả lập một request SOS và ghi phản hồi vào Execution log. */
function testCreateSosRequest() {
    const fake = { // Bộ tham số giả lập SOS trẻ em cho test thủ công.
        event: 'sos_alert',
        source: 'CHILD_TEST',
        device: CONFIG.DEFAULT_DEVICE_NAME,
        location: CONFIG.DEFAULT_DEVICE_LOCATION,
        status: 'SOS_ALERT',
        threat: '4',
        score: '0',
        time: nowText(),
        message: 'Manual test SOS from Apps Script editor.'
    };
    return handleSosRequest(fake, 'sos_alert');
}

/** Hàm kiểm thử thủ công: giả lập heartbeat của ESP32-S3. */
function testHeartbeat() {
    return handleHeartbeat({
        event: 'heartbeat',
        device: CONFIG.DEFAULT_DEVICE_NAME,
        location: CONFIG.DEFAULT_DEVICE_LOCATION,
        status: 'ONLINE',
        time: nowText(),
        message: 'Manual heartbeat test.'
    });
}

/** Hàm kiểm thử thủ công: tạo sự kiện phá hoại để kiểm tra quy trình theo dõi. */
function testCreateSabotage() {
    return handleSabotageRequest({
        event: 'sabotage_alert',
        source: 'TEST',
        device: CONFIG.DEFAULT_DEVICE_NAME,
        location: CONFIG.DEFAULT_DEVICE_LOCATION,
        status: 'SABOTAGE_ALERT',
        threat: '3',
        time: nowText(),
        message: 'Manual sabotage test.'
    });
}

/** Hàm kiểm thử thủ công: làm cũ heartbeat để kiểm tra nâng cấp nghiêm trọng sau phá hoại. */
function testForceHeartbeatTimeoutAfterSabotage() {
    const eventId = getActiveSabotageEventId_() || createEventId('SABOTAGE_TEST'); // ID phá hoại test.

    if (!loadEventRecord(eventId)) {
        handleSabotageRequest({
            eventId: eventId,
            event: 'sabotage_alert',
            source: 'TEST',
            message: 'Manual sabotage before timeout.'
        });
    }

    PropertiesService.getScriptProperties().setProperty(CONFIG.HEARTBEAT_PROPERTY_KEY, JSON.stringify({
        eventType: 'heartbeat',
        device: CONFIG.DEFAULT_DEVICE_NAME,
        location: CONFIG.DEFAULT_DEVICE_LOCATION,
        status: 'ONLINE',
        serverTime: Utilities.formatDate(new Date(new Date().getTime() - 120000), CONFIG.TIME_ZONE, 'yyyy-MM-dd HH:mm:ss'),
        rtcTime: 'TEST_OLD_HEARTBEAT',
        message: 'Old heartbeat for timeout test.'
    }));

    return monitorHeartbeatAfterSabotage();
}

/** Hàm kiểm thử thủ công: đóng các sự kiện hiện đang được con trỏ hoạt động tham chiếu. */
function testResolveCurrentEvent() {
    return handleResolve({});
}

// ==================================================
// 10. RENDERING HELPERS
// ==================================================

/** Kết xuất bảng HTML tóm tắt các trường quan trọng của một bản ghi sự kiện. */
function renderEventSummary(record) {
    return (
        htmlRow('Event ID', record.eventId) +
        htmlRow('Loại sự kiện', record.eventType) +
        htmlRow('Nguồn', record.source) +
        htmlRow('Thiết bị', record.device) +
        htmlRow('Khu vực', record.location) +
        htmlRow('Trạng thái hệ thống', record.status) +
        htmlRow('Threat level', record.threat) +
        htmlRow('Intrusion score', record.score) +
        htmlRow('Thời gian RTC', record.rtcTime) +
        htmlRow('Thời gian server', record.serverTime) +
        htmlRow('Heartbeat gần nhất', record.lastHeartbeatAt || getLastHeartbeatServerTime_() || 'Chưa có') +
        htmlRow('Tin nhắn', record.message) +
        htmlRow('Ghi chú escalation', record.note || 'Chưa có') +
        htmlRow('emergency_escalation_status', record.emergency_escalation_status) +
        htmlRow('emergency_authority_message_status', record.emergency_authority_message_status)
    );
}

/** Kết xuất tóm tắt sự kiện dạng văn bản thuần để dùng trong email không hỗ trợ HTML. */
function plainEventSummary(record) {
    return (
        'Event ID: ' + safeValue(record.eventId) + '\n' +
        'Loai su kien: ' + safeValue(record.eventType) + '\n' +
        'Nguon: ' + safeValue(record.source) + '\n' +
        'Thiết bị: ' + safeValue(record.device) + '\n' +
        'Khu vực: ' + safeValue(record.location) + '\n' +
        'Trạng thái hệ thống: ' + safeValue(record.status) + '\n' +
        'Threat level: ' + safeValue(record.threat) + '\n' +
        'Intrusion score: ' + safeValue(record.score) + '\n' +
        'Thời gian RTC: ' + safeValue(record.rtcTime) + '\n' +
        'Thời gian server: ' + safeValue(record.serverTime) + '\n' +
        'Heartbeat gan nhat: ' + safeValue(record.lastHeartbeatAt || getLastHeartbeatServerTime_()) + '\n' +
        'Tin nhắn: ' + safeValue(record.message) + '\n' +
        'Ghi chú escalation: ' + safeValue(record.note || 'Chưa có') + '\n'
    );
}

/** Bọc nội dung trong khung trang HTML có charset, viewport và CSS dùng chung. */
function pageShell(innerHtml) {
    return (
        '<html><head>' +
        '<meta name="viewport" content="width=device-width, initial-scale=1">' +
        '<style>' +
        'body{font-family:Arial,sans-serif;line-height:1.5;padding:16px;max-width:860px;margin:auto;}' +
        'p{margin:8px 0;}' +
        '.warning{background:#fff3cd;border-left:4px solid #fbbc04;padding:10px;margin:12px 0;}' +
        'textarea{font-family:Arial,sans-serif;font-size:14px;padding:8px;}' +
        '</style>' +
        '</head><body>' +
        innerHtml +
        '</body></html>'
    );
}

/** Tạo thẻ tiêu đề HTML sau khi escape nội dung đầu vào. */
function htmlTitle(text) {
    return '<h2>' + escapeHtml(text) + '</h2>';
}

/** Tạo đoạn văn HTML an toàn từ nội dung đầu vào. */
function htmlParagraph(text) {
    return '<p>' + escapeHtml(text) + '</p>';
}

/** Tạo hộp cảnh báo HTML an toàn với kiểu hiển thị nổi bật. */
function htmlWarning(text) {
    return '<div class="warning"><b>Lưu ý:</b> ' + escapeHtml(text) + '</div>';
}

/** Tạo một hàng nhãn/giá trị trong bảng HTML tóm tắt. */
function htmlRow(label, value) {
    return '<p><b>' + escapeHtml(label) + ':</b> ' + escapeHtml(value || 'N/A') + '</p>';
}

/** Tạo liên kết được trình bày như nút bấm, escape cả URL lẫn nhãn. */
function htmlButton(url, text) {
    return (
        '<p><a href="' + escapeHtml(url) + '" ' +
        'style="display:inline-block;padding:10px 16px;background:#d93025;color:white;text-decoration:none;border-radius:6px;font-weight:bold;">' +
        escapeHtml(text) +
        '</a></p>'
    );
}

/** Tạo HtmlOutput từ chuỗi HTML và đặt tiêu đề trang Web App. */
function htmlResponse(html) {
    return HtmlService.createHtmlOutput(html);
}

/** Tạo TextOutput dạng text/plain để ESP32 có thể phân tích ổn định. */
function textResponse(text) {
    return ContentService
        .createTextOutput(text)
        .setMimeType(ContentService.MimeType.TEXT);
}

// ==================================================
// 11. GENERAL HELPERS
// ==================================================

/**
 * Gửi một email bằng MailApp đến danh sách người nhận sau khi lọc địa chỉ rỗng.
 * Ném lỗi khi không có người nhận; không trả về dữ liệu.
 */
function sendEmail(recipientList, subject, plainBody, htmlBody) {
    if (!recipientList || recipientList.length === 0) {
        throw new Error('Recipient list is empty.');
    }

    MailApp.sendEmail({
        to: recipientList.join(','),
        subject: subject,
        body: plainBody,
        htmlBody: htmlBody
    });
}

/** Tạo object tham số mới, ép mọi giá trị thành chuỗi và giới hạn độ dài để giảm dữ liệu bất thường. */
function sanitizeParams(params) {
    const clean = {}; // Object kết quả chỉ chứa chuỗi đã giới hạn độ dài.
    Object.keys(params).forEach(function (key) {
        clean[key] = trimToLength(String(params[key] || ''), 2000);
    });
    return clean;
}

/** Ép đầu vào thành chuỗi, trim khoảng trắng và cắt tối đa maxLength ký tự. */
function trimToLength(text, maxLength) {
    const value = String(text || '').trim(); // Chuỗi đầu vào sau ép kiểu và loại khoảng trắng hai đầu.
    if (value.length <= maxLength) {
        return value;
    }
    return value.substring(0, maxLength);
}

/** Chuẩn hóa mã điều khiển thành chữ thường và thay khoảng trắng bằng dấu gạch dưới. */
function normalize(text) {
    return String(text || '').trim().toLowerCase();
}

/** Chuẩn hóa nguồn sự kiện thành chữ hoa, dấu gạch dưới; mặc định UNKNOWN. */
function normalizeSource(text) {
    const raw = String(text || '').trim(); // Mã nguồn sự kiện trước khi thay ký tự.
    if (!raw) {
        return 'UNKNOWN';
    }
    return raw.toUpperCase().replace(/[^A-Z0-9_\-]/g, '_');
}

/** Trả về đối số đầu tiên có nội dung sau trim; trả chuỗi rỗng nếu tất cả rỗng. */
function firstNonEmpty() {
    for (let i = 0; i < arguments.length; i++) {
        const value = arguments[i]; // Đối số hiện tại đang được kiểm tra.
        if (value !== undefined && value !== null && String(value).trim() !== '') {
            return String(value).trim();
        }
    }
    return '';
}

/** Biến giá trị thành chuỗi response an toàn bằng cách loại dấu xuống dòng và dấu chấm phẩy. */
function safeValue(value) {
    if (value === undefined || value === null || value === '') {
        return 'N/A';
    }
    return String(value);
}

/** Trả thời gian máy chủ hiện tại theo CONFIG.TIME_ZONE và định dạng yyyy-MM-dd HH:mm:ss. */
function nowText() {
    return Utilities.formatDate(new Date(), CONFIG.TIME_ZONE, 'yyyy-MM-dd HH:mm:ss');
}

/** Tạo ID sự kiện gần như duy nhất từ tiền tố, timestamp và UUID rút gọn. */
function createEventId(prefix) {
    return String(prefix || 'EVENT') + '_' + Utilities.formatDate(new Date(), CONFIG.TIME_ZONE, 'yyyyMMdd_HHmmss') + '_' + Math.floor(Math.random() * 100000);
}

/** Escape năm ký tự đặc biệt để dữ liệu động không thể phá cấu trúc HTML. */
function escapeHtml(value) {
    return String(value || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}
