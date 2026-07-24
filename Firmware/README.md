# Firmware

This directory contains the current firmware for the Freenove ESP32-S3 WROOM with an OV3660 camera.

## Current Firmware

[`Kitchen_Security_System/Kitchen_Security_System_-_Group_6_jun18a/`](Kitchen_Security_System/Kitchen_Security_System_-_Group_6_jun18a/) is the maintained Arduino sketch. It implements sensor monitoring, local alarm outputs, OV3660 camera capture, Arduino IoT Cloud properties, Telegram delivery, optional Gemini classification, heartbeat reporting, and the Google Apps Script escalation workflow.

## Prerequisites

- Arduino IDE with the ESP32 board package.
- Board target: `esp32:esp32:esp32s3`.
- The libraries referenced by the sketch, including Arduino IoT Cloud, camera, RTC, and WiFi dependencies.
- An Arduino IoT Cloud Thing with property names matching `thingProperties.h`.

## Secrets

Create `arduino_secrets.h` locally from `arduino_secrets.example.h`. Supply your own WiFi, Arduino Cloud, Telegram, Gemini, and Apps Script values. The real secrets file is ignored by Git and must never be committed.

## Google Apps Script

[`Kitchen_Security_System/gg-app-script-email.js`](Kitchen_Security_System/gg-app-script-email.js) contains the companion Apps Script source for heartbeat monitoring and simulated escalation emails. Configure a private deployment URL in your local secrets file.
