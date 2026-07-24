# Software Requirements Specification - Kitchen Security System

## 1. Purpose and Scope

This specification defines the current, tested Version 3 prototype. It supersedes historical documents and is the source of truth for the public repository.

The system monitors a residential kitchen with a Freenove ESP32-S3 WROOM, OV3660 camera, PIR sensor, HY-SRF05 ultrasonic sensor, LDR, DS1307 RTC, buzzer, and LEDs. Arduino IoT Cloud provides remote controls and state; Telegram and Google Apps Script are optional connected services.

### In Scope

- PIR and ultrasonic sensor intrusion confirmation.
- LDR/distance-based anti-sabotage monitoring.
- Local buzzer and LED alarm outputs.
- RTC scheduling, manual control, and dashboard status reporting.
- Event-driven and manual camera capture, Telegram photos, optional Gemini classification.
- Heartbeat monitoring and simulated, human-confirmed escalation.

### Out of Scope

RFID, flame sensing, WiFi/MAC scanning, ESP32-CAM AI Thinker hardware, automatic contact with public authorities, facial recognition, and certified security operation are out of scope.

## 2. Architecture

```text
PIR + ultrasonic + LDR + RTC -> ESP32-S3 firmware -> buzzer, LEDs, OV3660
                                             -> Arduino IoT Cloud dashboard
                                             -> Telegram and optional services
```

## 3. Functional Requirements

| ID | Requirement |
| --- | --- |
| FR-01 | The device shall read PIR, ultrasonic, LDR, and RTC data. |
| FR-02 | The dashboard shall allow alarm enablement, scheduling, sensitivity, camera, and notification controls. |
| FR-03 | The device shall confirm an intrusion from the configured PIR and ultrasonic conditions after the boot grace period. |
| FR-04 | The device shall identify configured LDR-cover and near-distance sabotage conditions. |
| FR-05 | The device shall latch alarm events until a permitted reset action occurs. |
| FR-06 | The device shall activate local LED and buzzer outputs for active alarm states. |
| FR-07 | The device shall capture a photo on eligible events or manual dashboard request when the camera is enabled. |
| FR-08 | The device shall send a Telegram photo only when valid configuration and network connectivity are available. |
| FR-09 | The device shall publish status, event, and health properties to Arduino IoT Cloud. |
| FR-10 | The device shall publish heartbeat data while connected when heartbeat monitoring is enabled. |
| FR-11 | The Apps Script workflow shall require human confirmation for simulated SOS or critical escalation. |
| FR-12 | Gemini image classification, when enabled, shall provide supplementary person/no-person evidence only. |

## 4. Key Rules

- SOS and active sabotage have priority over ordinary intrusion monitoring.
- A heartbeat timeout indicates loss of contact, not confirmed physical sabotage.
- AI output never replaces sensor-driven alarm logic.
- A reset acknowledges a local alarm state; it does not prove that a remote incident has been resolved.
- Secrets, recipient addresses, and deployment URLs must remain outside version control.

## 5. Interface Contract

The Arduino IoT Cloud Thing uses properties including `alarm_enabled`, `system_armed`, `intrusion_alert`, `sabotage_alert`, `alarm_status`, `last_event`, `photo_status`, `notification_sent_status`, `heartbeat_status`, `last_heartbeat_time`, `manual_capture_photo`, and SOS controls. The exact property declarations in `Firmware/.../thingProperties.h` are authoritative.

## 6. Non-Functional Requirements

- The firmware shall recover normal monitoring after WiFi reconnection.
- The system shall avoid duplicate notifications for the same latched event where practical.
- The public repository shall not contain credentials or personally identifying evidence.
- Documentation shall distinguish tested behavior from historical ideas.

## 7. Acceptance Criteria

The prototype is accepted when the seven scenarios in [Demo Scenarios](DEMO_SCENARIOS_V1.0.0.md) produce their stated observable results using the current firmware and configured services.

## 8. Security Configuration

Create `arduino_secrets.h` locally from `arduino_secrets.example.h`. Configure WiFi, device keys, Telegram credentials, optional Gemini credentials, and the private Apps Script deployment URL there. Never commit the completed secrets file.
