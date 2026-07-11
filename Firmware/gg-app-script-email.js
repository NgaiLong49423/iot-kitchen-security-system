/**
 * Google Apps Script Web App
 * Project: Kitchen Security System - Group 6
 * SRS checkpoint: v0.6.7
 *
 * Scope in this script:
 * 1. Handle SOS escalation confirmation flow.
 * 2. Send Parent/Admin confirmation email when ESP32-S3 reports SOS.
 * 3. Let Parent/Admin confirm escalation and optionally add sos_authority_note.
 * 4. Send authority escalation email to demo authority contact only after confirmation.
 * 5. Expose a polling endpoint so ESP32-S3 can check confirmation/escalation status.
 * 6. Receive heartbeat and sabotage_alert for DS-03/DS-04.
 * 7. Monitor heartbeat only after sabotage has happened.
 * 8. Escalate to critical_security_compromise when heartbeat is lost after sabotage.
 *
 * Important SRS rules implemented here:
 * - Telegram notification and SOS email escalation are separate flows.
 * - This script is only for SOS email escalation, not fire/intrusion notification.
 * - ACK/confirmation does NOT reset the local alarm; ESP32 must keep SOS_ALERT
 *   until Parent/Admin uses reset_alarm on Arduino Cloud.
 * - Authority escalation includes configured home address and optional note.
 * - Demo recipient must be a mock/demo authority contact.
 *
 * ESP32-S3 compatible request examples:
 *
 * 1) Create SOS confirmation email from ESP32-S3:
 *    ?event=sos_alert&source=CHILD&device=He%20thong&location=Khu%20vuc%20bep&status=SOS_ALERT&threat=4&score=0&time=2026-07-04%2013:00:00&message=SOS
 *
 * 2) Poll status from ESP32-S3:
 *    ?action=status&eventId=EVENT_ID_FROM_RESPONSE
 *
 * 3) Parent/Admin confirmation is opened from email link:
 *    ?action=confirm&eventId=EVENT_ID
 */

// ==================================================
// 1. CONFIGURATION
// ==================================================

const CONFIG = {
    PROJECT_NAME: 'Kitchen Security System - Group 6',
    TIME_ZONE: 'Asia/Ho_Chi_Minh',

    // Optional. If non-empty, ESP32-S3 must call with &key=YOUR_TOKEN.
    WEB_APP_TOKEN: '',

    // Keep email confirmation and status links on the same deployed Web App
    // endpoint that the ESP32-S3 calls. Do not replace this with an old URL.
    PUBLIC_WEB_APP_URL: 'https://script.google.com/macros/s/AKfycbxQ3ik2vGu4f4ygHQXUtswjMmpfDSKtb381Sz5UFLzDDc2JNAnNewI6UV311sC2bKPdPQ/exec',

    // Parent/Admin recipients who receive SOS confirmation email.
    FAMILY_RECIPIENTS: [
        'longgiango511@gmail.com',
        'danhvo0147258@gmail.com',
        'anhbin160304@gmail.com',
        'yakinpvk@gmail.com'
    ],

    // Demo authority recipient only. Do NOT put a real police/public authority
    // address here unless deployment is legally approved by the system owner.
    AUTHORITY_DEMO_RECIPIENTS: [
        'hong123aa@gmail.com'
    ],

    // Demo address used only for the project escalation flow.
    // Script Property HOME_ADDRESS still has priority over this constant.
    HOME_ADDRESS: 'Lô E2a-7, Đường D1, Khu Công nghệ cao, Phường Tăng Nhơn Phú, TP. Hồ Chí Minh',

    // Optional fallback values when ESP32-S3 does not send them.
    DEFAULT_DEVICE_NAME: 'Freenove ESP32-S3 Kitchen Security',
    DEFAULT_DEVICE_LOCATION: 'Khu vuc bep',

    // The script stores event records for status polling and duplicate protection.
    EVENT_PROPERTY_PREFIX: 'KSS_EVENT_',
    EVENT_TTL_HOURS: 24,

    HEARTBEAT_PROPERTY_KEY: 'KSS_LAST_HEARTBEAT',
    ACTIVE_SABOTAGE_EVENT_KEY: 'KSS_ACTIVE_SABOTAGE_EVENT_ID',
    ACTIVE_SOS_EVENT_KEY: 'KSS_ACTIVE_SOS_EVENT_ID',
    HEARTBEAT_TIMEOUT_SECONDS: 60
};

const STATUS = {
    WAITING_CONFIRMATION: 'WAITING_CONFIRMATION',
    CONFIRMED: 'CONFIRMED',
    SENT: 'SENT',
    FAILED: 'FAILED',
    NOT_CONFIGURED: 'NOT_CONFIGURED',
    UNKNOWN_EVENT: 'UNKNOWN_EVENT',
    IDLE: 'IDLE',
    MONITORING: 'MONITORING',
    RESOLVED: 'RESOLVED'
};

// ==================================================
// 2. WEB APP ENTRYPOINTS
// ==================================================

function doGet(e) {
    try {
        const params = sanitizeParams((e && e.parameter) || {});

        if (!isAuthorized(params)) {
            return textResponse('ERROR: Unauthorized request');
        }

        cleanupOldEvents_();

        const action = normalize(params.action || '');

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

        // Backward/ESP32-compatible default path.
        // Current ESP32 Phase 5 code sends event=sos_alert without action.
        return handleIncomingEvent(params);

    } catch (err) {
        return textResponse('ERROR: ' + err.message);
    }
}

function doPost(e) {
    try {
        const params = sanitizeParams((e && e.parameter) || {});

        if (!isAuthorized(params)) {
            return textResponse('ERROR: Unauthorized request');
        }

        cleanupOldEvents_();

        const action = normalize(params.action || '');

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
// 3. ESP32-S3 EVENT HANDLING
// ==================================================

function handleIncomingEvent(params) {
    const eventType = normalize(params.event || params.type || '');

    if (eventType === 'heartbeat' || eventType === 'device_heartbeat') {
        return handleHeartbeat(params);
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

    return textResponse(
        'IGNORED: Unsupported Google Apps Script event. ' +
        'event=' + (eventType || 'NONE')
    );
}

function handleHeartbeat(params) {
    const now = nowText();
    const record = {
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

    return textResponse(
        'OK:HEARTBEAT' +
        ';serverTime=' + safeValue(now) +
        ';active_sabotage=' + String(Boolean(record.activeSabotageEventId)) +
        ';active_sabotage_eventId=' + safeValue(record.activeSabotageEventId || '')
    );
}

function handleSabotageRequest(params) {
    const now = nowText();
    const eventId = params.eventId || getActiveSabotageEventId_() || createEventId('SABOTAGE');
    let record = loadEventRecord(eventId);

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

function handleCriticalRequest(params, reason) {
    const eventId = params.eventId || getActiveSabotageEventId_() || createEventId('CRITICAL');
    const existing = loadEventRecord(eventId);

    if (existing && existing.eventType === 'critical_security_compromise') {
        return textResponse(formatStatusResponse(existing));
    }

    const record = existing || buildBaseSecurityRecord(params, eventId, 'critical_security_compromise', 'DEVICE');
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
        lastError: ''
    };
}

function handleSosRequest(params, eventType) {
    const now = nowText();
    const source = normalizeSource(params.source || eventType || 'UNKNOWN');
    const requestedEventId = params.eventId || '';
    const activeEventId = getActiveSosEventId_();

    // A dashboard callback or retry must not create another confirmation email
    // while the previous SOS is still awaiting confirmation.
    if (!requestedEventId && activeEventId) {
        const activeRecord = loadEventRecord(activeEventId);
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

    const eventId = requestedEventId || createEventId('SOS');

    const record = {
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

function monitorHeartbeatAfterSabotage() {
    const eventId = getActiveSabotageEventId_();

    if (!eventId) {
        return textResponse('OK:NO_ACTIVE_SABOTAGE');
    }

    const record = loadEventRecord(eventId);

    if (!record || record.monitoring_status === STATUS.RESOLVED) {
        clearActiveSabotageEventId_();
        return textResponse('OK:NO_ACTIVE_SABOTAGE');
    }

    const lastHeartbeat = getLastHeartbeat_();

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

    const ageSeconds = Math.floor((new Date().getTime() - new Date(lastHeartbeat.serverTime).getTime()) / 1000);
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

function installHeartbeatMonitorTrigger() {
    const triggers = ScriptApp.getProjectTriggers();
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

function handleStatus(params) {
    const eventId = params.eventId || getActiveSabotageEventId_() || '';

    if (!eventId) {
        return textResponse(
            'OK:NO_EVENT' +
            ';last_heartbeat_at=' + safeValue(getLastHeartbeatServerTime_()) +
            ';home_address_configured=' + String(isHomeAddressConfigured())
        );
    }

    const record = loadEventRecord(eventId);

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

function handleResolve(params) {
    const eventId = params.eventId || getActiveSabotageEventId_() || '';

    if (!eventId) {
        return textResponse('OK:RESOLVED;eventId=NONE');
    }

    const record = loadEventRecord(eventId);

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

    return textResponse('OK:RESOLVED;eventId=' + eventId);
}

// ==================================================
// 5. PARENT/ADMIN CONFIRMATION FLOW
// ==================================================

function handleConfirmPage(params) {
    const eventId = params.eventId || '';

    if (!eventId) {
        return htmlResponse(pageShell(
            htmlTitle('Thiếu Event ID') +
            htmlParagraph('Không thể xác nhận vì URL thiếu eventId.')
        ));
    }

    const record = loadEventRecord(eventId);

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

    const url = ScriptApp.getService().getUrl();
    const pageTitle = record.eventType === 'critical_security_compromise'
        ? 'Xác nhận cảnh báo nghiêm trọng'
        : 'Xác nhận SOS escalation';

    const html =
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

function handleConfirmSend(params) {
    const eventId = params.eventId || '';
    const note = trimToLength(params.note || '', 1000);

    if (!eventId) {
        return htmlResponse(pageShell(
            htmlTitle('Thiếu Event ID') +
            htmlParagraph('Không thể gửi escalation vì URL thiếu eventId.')
        ));
    }

    const lock = LockService.getScriptLock();
    lock.waitLock(10000);

    try {
        const record = loadEventRecord(eventId);

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

function sendCriticalConfirmationEmailOnce_(record) {
    if (record.criticalConfirmationEmailSentAt) {
        return;
    }

    sendSosConfirmationEmail(record);
    record.criticalConfirmationEmailSentAt = nowText();
    saveEventRecord(record);
}

function sendSosConfirmationEmail(record) {
    const confirmUrl = buildUrl({ action: 'confirm', eventId: record.eventId });
    const statusUrl = buildUrl({ action: 'status', eventId: record.eventId });
    const isCritical = record.eventType === 'critical_security_compromise';

    const subject = (isCritical ? 'Cảnh báo nghiêm trọng cần xác nhận - ' : 'Yêu cầu xác nhận SOS - ') + CONFIG.PROJECT_NAME;
    const intro = isCritical
        ? 'Cloud phát hiện thiết bị có thể đã mất liên lạc hoặc suy giảm nghiêm trọng sau cảnh báo phá hoại. Cần Parent/Admin xác nhận trước khi gửi escalation.'
        : 'Hệ thống vừa nhận SOS và cần Parent/Admin xác nhận trước khi gửi escalation.';

    const plainBody =
        intro + '\n\n' +
        plainEventSummary(record) +
        'Home address configured: ' + String(isHomeAddressConfigured()) + '\n\n' +
        'Xác nhận gửi báo cáo tại link sau:\n' + confirmUrl + '\n\n' +
        'Kiểm tra trạng thái polling:\n' + statusUrl + '\n\n' +
        'Lưu ý: Xác nhận email KHÔNG tắt còi/LED. Cần dùng reset_alarm trên Arduino Cloud để kết thúc cảnh báo tại thiết bị.';

    const htmlBody =
        htmlTitle(isCritical ? 'Cảnh báo nghiêm trọng cần xác nhận' : 'Yêu cầu xác nhận SOS') +
        htmlParagraph(intro) +
        htmlWarning('Xác nhận này KHÔNG tắt còi/LED. Mất heartbeat không chứng minh chắc chắn đã bị cắt điện; cũng có thể do WiFi, nguồn hoặc lỗi mạng.') +
        renderEventSummary(record) +
        htmlRow('Home address configured', String(isHomeAddressConfigured())) +
        htmlButton(confirmUrl, 'Mở trang xác nhận escalation') +
        htmlParagraph('Polling status URL: ' + statusUrl);

    sendEmail(CONFIG.FAMILY_RECIPIENTS, subject, plainBody, htmlBody);
}

function sendAuthorityEscalationEmail(record) {
    const homeAddress = getHomeAddress();
    const isCritical = record.eventType === 'critical_security_compromise';
    const subject = (isCritical ? 'Báo cáo sự cố nghiêm trọng demo - ' : 'Báo cáo SOS demo - ') + CONFIG.PROJECT_NAME;

    const plainBody =
        'Parent/Admin đã xác nhận gửi báo cáo đến contact mô phỏng.\n\n' +
        plainEventSummary(record) +
        'Địa chỉ nhà/khu vực cần hỗ trợ:\n' + homeAddress + '\n\n' +
        'Ghi chú bổ sung:\n' + (record.note || 'Không có') + '\n\n' +
        'Đây là email mô phỏng phục vụ demo đồ án. Không gửi đến cơ quan thật nếu chưa có cấu hình hợp lệ và sự đồng ý của chủ hệ thống.';

    const htmlBody =
        htmlTitle(isCritical ? 'Báo cáo sự cố nghiêm trọng demo' : 'Báo cáo SOS demo') +
        htmlParagraph('Parent/Admin đã xác nhận gửi báo cáo đến contact mô phỏng.') +
        renderEventSummary(record) +
        htmlRow('Địa chỉ nhà/khu vực cần hỗ trợ', homeAddress) +
        htmlRow('Ghi chú bổ sung', record.note || 'Không có') +
        htmlWarning('Đây là contact mô phỏng phục vụ demo. Không gửi đến cơ quan thật nếu chưa có cấu hình hợp lệ và sự đồng ý của chủ hệ thống.');

    sendEmail(CONFIG.AUTHORITY_DEMO_RECIPIENTS, subject, plainBody, htmlBody);
}

function sendFamilyStatusEmail(record, message) {
    const subject = 'Cập nhật trạng thái gửi báo cáo - ' + CONFIG.PROJECT_NAME;

    const plainBody =
        message + '\n\n' +
        plainEventSummary(record) +
        'emergency_escalation_status: ' + record.emergency_escalation_status + '\n' +
        'emergency_authority_message_status: ' + record.emergency_authority_message_status + '\n\n' +
        'Lưu ý: ACK/escalation không reset cảnh báo tại chỗ. Cần reset_alarm trên Arduino Cloud.';

    const htmlBody =
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

function saveEventRecord(record) {
    const props = PropertiesService.getScriptProperties();
    props.setProperty(CONFIG.EVENT_PROPERTY_PREFIX + record.eventId, JSON.stringify(record));
}

function loadEventRecord(eventId) {
    const props = PropertiesService.getScriptProperties();
    const raw = props.getProperty(CONFIG.EVENT_PROPERTY_PREFIX + eventId);

    if (!raw) {
        return null;
    }

    try {
        return JSON.parse(raw);
    } catch (err) {
        return null;
    }
}

function cleanupOldEvents_() {
    const props = PropertiesService.getScriptProperties();
    const all = props.getProperties();
    const prefix = CONFIG.EVENT_PROPERTY_PREFIX;
    const nowMs = new Date().getTime();
    const ttlMs = CONFIG.EVENT_TTL_HOURS * 60 * 60 * 1000;

    Object.keys(all).forEach(function (key) {
        if (key.indexOf(prefix) !== 0) {
            return;
        }

        try {
            const record = JSON.parse(all[key]);
            const createdText = record.confirmationSentAt || record.serverTime || '';
            const createdMs = new Date(createdText).getTime();

            if (createdMs && nowMs - createdMs > ttlMs) {
                props.deleteProperty(key);
            }
        } catch (err) {
            // Corrupt record; remove it.
            props.deleteProperty(key);
        }
    });
}

function getLastHeartbeat_() {
    const raw = PropertiesService.getScriptProperties().getProperty(CONFIG.HEARTBEAT_PROPERTY_KEY);

    if (!raw) {
        return null;
    }

    try {
        return JSON.parse(raw);
    } catch (err) {
        return null;
    }
}

function getLastHeartbeatServerTime_() {
    const record = getLastHeartbeat_();
    return record ? record.serverTime : '';
}

function getActiveSabotageEventId_() {
    return PropertiesService.getScriptProperties().getProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY) || '';
}

function setActiveSabotageEventId_(eventId) {
    PropertiesService.getScriptProperties().setProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY, eventId);
}

function getActiveSosEventId_() {
    return PropertiesService.getScriptProperties().getProperty(CONFIG.ACTIVE_SOS_EVENT_KEY) || '';
}

function setActiveSosEventId_(eventId) {
    PropertiesService.getScriptProperties().setProperty(CONFIG.ACTIVE_SOS_EVENT_KEY, eventId);
}

function clearActiveSosEventId_() {
    PropertiesService.getScriptProperties().deleteProperty(CONFIG.ACTIVE_SOS_EVENT_KEY);
}

function clearActiveSabotageEventId_() {
    PropertiesService.getScriptProperties().deleteProperty(CONFIG.ACTIVE_SABOTAGE_EVENT_KEY);
}

// ==================================================
// 8. CONFIG HELPERS
// ==================================================

function isAuthorized(params) {
    if (!CONFIG.WEB_APP_TOKEN) {
        return true;
    }
    return params.key === CONFIG.WEB_APP_TOKEN;
}

function getHomeAddress() {
    const fromProperty = PropertiesService.getScriptProperties().getProperty('HOME_ADDRESS');
    const configuredProperty = fromProperty === 'DEMO_ADDRESS_NOT_CONFIGURED' ? '' : fromProperty;
    return firstNonEmpty(configuredProperty, CONFIG.HOME_ADDRESS, '');
}

function isHomeAddressConfigured() {
    const address = getHomeAddress();
    return Boolean(address && address !== 'DEMO_ADDRESS_NOT_CONFIGURED');
}

function hasAuthorityRecipients() {
    return Array.isArray(CONFIG.AUTHORITY_DEMO_RECIPIENTS) && CONFIG.AUTHORITY_DEMO_RECIPIENTS.length > 0;
}

function buildUrl(query) {
    const baseUrl = CONFIG.PUBLIC_WEB_APP_URL || ScriptApp.getService().getUrl();
    const parts = [];

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

function handleDebugEvent(params) {
    const eventId = params.eventId || '';
    const record = eventId ? loadEventRecord(eventId) : null;

    if (!record) {
        return textResponse('DEBUG: Event not found. eventId=' + eventId);
    }

    return textResponse(JSON.stringify(record, null, 2));
}

function testCreateSosRequest() {
    const fake = {
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

function testForceHeartbeatTimeoutAfterSabotage() {
    const eventId = getActiveSabotageEventId_() || createEventId('SABOTAGE_TEST');

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

function testResolveCurrentEvent() {
    return handleResolve({});
}

// ==================================================
// 10. RENDERING HELPERS
// ==================================================

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

function htmlTitle(text) {
    return '<h2>' + escapeHtml(text) + '</h2>';
}

function htmlParagraph(text) {
    return '<p>' + escapeHtml(text) + '</p>';
}

function htmlWarning(text) {
    return '<div class="warning"><b>Lưu ý:</b> ' + escapeHtml(text) + '</div>';
}

function htmlRow(label, value) {
    return '<p><b>' + escapeHtml(label) + ':</b> ' + escapeHtml(value || 'N/A') + '</p>';
}

function htmlButton(url, text) {
    return (
        '<p><a href="' + escapeHtml(url) + '" ' +
        'style="display:inline-block;padding:10px 16px;background:#d93025;color:white;text-decoration:none;border-radius:6px;font-weight:bold;">' +
        escapeHtml(text) +
        '</a></p>'
    );
}

function htmlResponse(html) {
    return HtmlService.createHtmlOutput(html);
}

function textResponse(text) {
    return ContentService
        .createTextOutput(text)
        .setMimeType(ContentService.MimeType.TEXT);
}

// ==================================================
// 11. GENERAL HELPERS
// ==================================================

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

function sanitizeParams(params) {
    const clean = {};
    Object.keys(params).forEach(function (key) {
        clean[key] = trimToLength(String(params[key] || ''), 2000);
    });
    return clean;
}

function trimToLength(text, maxLength) {
    const value = String(text || '').trim();
    if (value.length <= maxLength) {
        return value;
    }
    return value.substring(0, maxLength);
}

function normalize(text) {
    return String(text || '').trim().toLowerCase();
}

function normalizeSource(text) {
    const raw = String(text || '').trim();
    if (!raw) {
        return 'UNKNOWN';
    }
    return raw.toUpperCase().replace(/[^A-Z0-9_\-]/g, '_');
}

function firstNonEmpty() {
    for (let i = 0; i < arguments.length; i++) {
        const value = arguments[i];
        if (value !== undefined && value !== null && String(value).trim() !== '') {
            return String(value).trim();
        }
    }
    return '';
}

function safeValue(value) {
    if (value === undefined || value === null || value === '') {
        return 'N/A';
    }
    return String(value);
}

function nowText() {
    return Utilities.formatDate(new Date(), CONFIG.TIME_ZONE, 'yyyy-MM-dd HH:mm:ss');
}

function createEventId(prefix) {
    return String(prefix || 'EVENT') + '_' + Utilities.formatDate(new Date(), CONFIG.TIME_ZONE, 'yyyyMMdd_HHmmss') + '_' + Math.floor(Math.random() * 100000);
}

function escapeHtml(value) {
    return String(value || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}
