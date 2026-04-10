# Posture Monitoring Cushion

A smart cushion system for posture monitoring using \*\*ESP32\*\* and \*\*Force Sensitive Resistors (FSRs)\*\*.  

The goal of this project is to detect sitting presence and basic sitting posture, such as:
- no user sitting
- leaning left
- sitting properly
- leaning right

This project is currently in the \*\*initial prototype phase\*\* and focuses on hardware integration, sensor reading, posture logic, and embedded software structure.


## Project Overview
The cushion uses \*\*4 FSR sensors\*\* placed at different positions under the seat:
- Right Bottom
- Right Top
- Left Top
- Left Bottom

The ESP32 reads analog values from these sensors, compares pressure distribution across the cushion, and determines the user’s posture.  
LED indicators are used for quick visual feedback during development and testing.

## Current Features
- ESP32-based embedded implementation
- 4 FSR sensor inputs
- posture state detection:
&#x20; - \*\*NO\_SIT\*\*
&#x20; - \*\*LEAN\_LEFT\*\*
&#x20; - \*\*PROPER\*\*
&#x20; - \*\*LEAN\_RIGHT\*\*

- LED posture indication
- onboard LED heartbeat for system status
- structured serial logging for debugging
- FreeRTOS-style task-based design
- object-oriented code structure for future expansion

## Hardware Setup
### Microcontroller
- ESP32 development board
### Sensors
- 4 × FSR (Force Sensitive Resistor)
### Sensor Pin Mapping
- \*\*GPIO 33\*\* → Right Bottom
- \*\*GPIO 32\*\* → Right Top
- \*\*GPIO 35\*\* → Left Top
- \*\*GPIO 34\*\* → Left Bottom
### LED Pin Mapping
- \*\*GPIO 2\*\*  → onboard heartbeat LED
- \*\*GPIO 22\*\* → NO\_SIT indicator
- \*\*GPIO 21\*\* → LEAN\_RIGHT indicator
- \*\*GPIO 19\*\* → PROPER posture indicator
- \*\*GPIO 18\*\* → LEAN\_LEFT indicator

## Circuit Notes
Each FSR is connected as a \*\*voltage divider\*\* and requires its \*\*own resistor\*\*.  
Current prototype testing uses:

- \*\*3.3V supply\*\*
- \*\*1 resistor per FSR\*\*
- shared \*\*3.3V\*\* and \*\*GND\*\* rails
- ESP32 ADC pins for analog reading


## Notes
FSRs are highly non-linear and may vary from one sensor to another, so threshold tuning and calibration will be important for reliable posture detection.
This repository will continue evolving as the hardware and software prototype becomes more robust.

