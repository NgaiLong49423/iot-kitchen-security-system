# Flowchart Review

**Reviewed:** 2026-07-12
**Scope:** Current ESP32-S3 Version 3 implementation

## Required Corrections

- Show PIR and ultrasonic readings as separate inputs before intrusion confirmation.
- Keep anti-sabotage and normal intrusion as distinct event paths.
- Show camera capture and Telegram delivery as conditional actions that require network and configuration availability.
- Represent heartbeat loss as a monitoring condition, not proof of power removal or physical sabotage.
- Label the authority-contact path as a simulated, human-confirmed escalation.

## Simplified Current Flow

```text
Read sensors and RTC
  -> Evaluate SOS
  -> Evaluate active sabotage
  -> Evaluate intrusion conditions
  -> Update local alarm and dashboard state
  -> Capture and notify when enabled
  -> Send heartbeat while connected
```

## Presentation Notes

Describe Gemini as optional photo evidence, not as the system's sole decision maker. State that an offline heartbeat may be caused by WiFi, power, or network failure and requires human review.

## Reference Files

- [System Requirements Specification](SRS.md)
- [Demo Scenarios](DEMO_SCENARIOS_V1.0.0.md)
- `docs/images/Flowchart*.mdj` for editable diagrams
