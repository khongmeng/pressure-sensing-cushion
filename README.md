# \# Posture Monitoring Cushion

# 

# A smart cushion system for posture monitoring using \*\*ESP32\*\* and \*\*Force Sensitive Resistors (FSRs)\*\*.  

# The goal of this project is to detect sitting presence and basic sitting posture, such as:

# 

# \- no user sitting

# \- leaning left

# \- sitting properly

# \- leaning right

# 

# This project is currently in the \*\*initial prototype phase\*\* and focuses on hardware integration, sensor reading, posture logic, and embedded software structure.

# 

# \## Project Overview

# 

# The cushion uses \*\*4 FSR sensors\*\* placed at different positions under the seat:

# 

# \- Right Bottom

# \- Right Top

# \- Left Top

# \- Left Bottom

# 

# The ESP32 reads analog values from these sensors, compares pressure distribution across the cushion, and determines the user’s posture.  

# LED indicators are used for quick visual feedback during development and testing.

# 

# \## Current Features

# 

# \- ESP32-based embedded implementation

# \- 4 FSR sensor inputs

# \- posture state detection:

# &#x20; - \*\*NO\_SIT\*\*

# &#x20; - \*\*LEAN\_LEFT\*\*

# &#x20; - \*\*PROPER\*\*

# &#x20; - \*\*LEAN\_RIGHT\*\*

# \- LED posture indication

# \- onboard LED heartbeat for system status

# \- structured serial logging for debugging

# \- FreeRTOS-style task-based design

# \- object-oriented code structure for future expansion

# 

# \## Hardware Setup

# 

# \### Microcontroller

# \- ESP32 development board

# 

# \### Sensors

# \- 4 × FSR (Force Sensitive Resistor)

# 

# \### Sensor Pin Mapping

# \- \*\*GPIO 33\*\* → Right Bottom

# \- \*\*GPIO 32\*\* → Right Top

# \- \*\*GPIO 35\*\* → Left Top

# \- \*\*GPIO 34\*\* → Left Bottom

# 

# \### LED Pin Mapping

# \- \*\*GPIO 2\*\*  → onboard heartbeat LED

# \- \*\*GPIO 22\*\* → NO\_SIT indicator

# \- \*\*GPIO 21\*\* → LEAN\_RIGHT indicator

# \- \*\*GPIO 19\*\* → PROPER posture indicator

# \- \*\*GPIO 18\*\* → LEAN\_LEFT indicator

# 

# \## Circuit Notes

# 

# Each FSR is connected as a \*\*voltage divider\*\* and requires its \*\*own resistor\*\*.  

# Current prototype testing uses:

# 

# \- \*\*3.3V supply\*\*

# \- \*\*1 resistor per FSR\*\*

# \- shared \*\*3.3V\*\* and \*\*GND\*\* rails

# \- ESP32 ADC pins for analog reading

# 

# \## Software Design

# 

# The current embedded software is organized with an object-oriented approach and uses FreeRTOS-style tasks for modularity.

# 

# Main responsibilities include:

# 

# \- sensor reading

# \- posture classification

# \- LED control

# \- heartbeat/status indication

# \- serial debugging output

# 

# Example serial tags used for debugging:

# 

# \- `\[SYS]` for system messages

# \- `\[FSR]` for sensor values

# \- `\[SUM]` for grouped pressure values

# \- `\[POSTURE]` for interpreted posture state

# 

# \## Current Posture Logic

# 

# A simple left-right posture check is used for the initial version:

# 

# \- left side pressure = left top + left bottom

# \- right side pressure = right top + right bottom

# 

# Classification:

# 

# \- low total pressure → \*\*NO\_SIT\*\*

# \- left side significantly higher → \*\*LEAN\_LEFT\*\*

# \- right side significantly higher → \*\*LEAN\_RIGHT\*\*

# \- otherwise → \*\*PROPER\*\*

# 

# This logic will be improved as testing and calibration continue.

# 

# \## Development Status

# 

# This project is still in the \*\*early prototyping phase\*\*.  

# The current focus is on:

# 

# \- validating FSR behavior

# \- testing pressure distribution

# \- refining threshold values

# \- improving wiring and physical integration

# \- building a clean embedded software structure

# 

# \## Future Work

# 

# \- add sensor smoothing / filtering

# \- add calibration routine for each FSR

# \- detect forward and backward leaning

# \- improve physical wiring and connector design

# \- replace temporary breadboard setup with a more reliable prototype layout

# \- explore wireless data transmission

# \- log posture data for future analysis

# \- integrate with higher-level software or ML pipeline if needed

# 

# \## Notes

# 

# FSRs are highly non-linear and may vary from one sensor to another, so threshold tuning and calibration will be important for reliable posture detection.

# 

# This repository will continue evolving as the hardware and software prototype becomes more robust.

