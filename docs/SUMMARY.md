# Project Summary

The IoT Kitchen Security System is an academic prototype that monitors a kitchen with multiple sensors and provides both local and remote security responses.

## Current Implementation

- **Controller:** Freenove ESP32-S3 WROOM.
- **Camera:** OV3660 for manual and event-driven photos.
- **Sensors:** PIR motion sensor, HY-SRF05 ultrasonic sensor, LDR module, and DS1307 RTC.
- **Outputs:** Buzzer, red LED, and green LED.
- **Services:** Arduino IoT Cloud, Telegram, optional Gemini image classification, and Google Apps Script monitoring.

## Safety and Scope

The system is an educational prototype, not a certified security or emergency-response system. The authority-contact workflow is a simulation and must not be presented as an official emergency-service integration. AI classification is supplementary evidence; sensor logic remains responsible for security decisions.

## Where to Continue

- Read the [requirements specification](requirements/SRS.md) for the current behavior.
- Use the [hardware reference](requirements/HARDWARE_WIRING.md) before connecting components.
- Follow the [demo scenarios](requirements/DEMO_SCENARIOS_V1.0.0.md) for reproducible testing.
