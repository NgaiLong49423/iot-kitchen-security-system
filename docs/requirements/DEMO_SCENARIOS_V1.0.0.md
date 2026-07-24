# Official Demo Scenarios - Version 3

**Version:** 1.0.0
**Scope:** Current ESP32-S3 firmware, Arduino IoT Cloud, Telegram, and optional Google Apps Script services.

> Safety: authority-contact emails are a simulation. Do not use real emergency-service contacts, real home addresses, or personal recipient data in public demonstrations.

![Parent Control Dashboard overview](../images/Screenshot%202026-07-24%20192117.png)

*Figure 1. Parent Control Dashboard overview with alarm, camera, notification, and monitoring controls.*

## Preparation

- Verify the RTC, camera, WiFi, and Arduino IoT Cloud connection in Serial Monitor.
- Configure local secrets and enable only the services required for the selected scenario.
- Enable `camera_enabled` for camera scenarios, `telegram_enabled` for Telegram delivery, and `gemini_enabled` only for the optional AI scenario.
- Do not interpret a heartbeat timeout as proof of sabotage; it can result from WiFi, power, or service failure.

## Scenario 1 - Normal Operation and Connection Recovery

Enable heartbeat monitoring, wait for a fresh `last_heartbeat_time`, then temporarily disconnect WiFi. The monitoring service should report an ordinary connection loss and later report recovery after WiFi returns. No simulated escalation should occur without an active sabotage event.

## Scenario 2 - Intrusion Detection and Telegram Photo

Enable alarm, camera, automatic photo capture, and Telegram. After the post-boot grace period, create PIR motion while an object remains between 15 cm and 50 cm from the ultrasonic sensor for at least two seconds. Expect `intrusion_alert`, local alarm output, photo capture, and a Telegram photo notification.

## Scenario 3 - Active Sabotage and Degraded Operation

Trigger the configured LDR-cover or near-distance condition. Expect a sabotage alert and local warning. Then demonstrate that loss of telemetry after sabotage is monitored as a critical condition requiring human review; it is not an automatic claim that the device was physically disabled.

## Scenario 4 - Scheduled Arming and Disarming

![Scheduling controls on the dashboard](../images/Screenshot%202026-07-24%20192142.png)

*Figure 2. Dashboard controls for sensitivity and scheduled arming/disarming.*

Set a short future arm/disarm schedule, enable scheduling, and verify the `system_armed` status before and after each configured time.

## Scenario 5 - SOS and Simulated Escalation

Activate either SOS control from the dashboard. The device should latch the local alarm and submit the event to the monitoring workflow. A parent or administrator must acknowledge the simulated request before any escalation email is sent.

## Scenario 6 - Manual Camera Capture

Enable the camera and request manual capture from the dashboard. Confirm the resulting `photo_status` and notification status. This scenario demonstrates remote observation, not continuous video streaming.

## Scenario 7 - Optional Gemini Photo Classification

With valid credentials and camera operation, enable Gemini and trigger a photo event. The result may report person/no-person evidence. It supplements the sensor-driven decision and must not be described as facial recognition or as the sole intrusion decision.

## Acceptance Summary

| Scenario | Required evidence |
| --- | --- |
| Normal operation | Fresh heartbeat and recovery after reconnection. |
| Intrusion | Alarm state, local output, photo, and Telegram status. |
| Sabotage | Sabotage state and human-review wording. |
| Schedule | Armed state changes at the configured times. |
| SOS | Latched alarm and simulated confirmation workflow. |
| Manual capture | Updated photo and notification states. |
| Gemini | Optional image classification result. |
