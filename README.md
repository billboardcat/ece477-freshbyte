# FreshByte 🍎🍌🍋🥭
### ECE477 - Digital Systems Senior Design - Purdue University - Spring 2021
### Group 10: Sabriya Alam, Parker Crain, Natalie Rodeghier, Jimmy Sung

## About FreshByte
FreshByte is a sensor-based food quality assurance system that monitors and issues real-time alerts
regarding food longevity based on environmental conditions and type of food.

Data reported from a pressure sensor, timer, thermometer, hygrometer, and gas sensors will be used to monitor the food
and notify the user if the conditions are out of specification as well as how much longer the food can be safely consumed.

Users will input the type of food through the use of a display and buttons. The display will be used to notify users of when a certain
threshold of longevity has been reached.

### Project Specific Success Criteria
- [x] An ability to read pressure sensor data to determine if food is in the box.
- [x] An ability to transmit data to external device for analysis over a WiFi connection.
- [x] An ability to read battery charge level data through I2C using a PMIC.
- [x] An ability to show environmental conditions on the display.
- [x] An ability to wake the display using a proximity sensor.

## About the repository structure
This repository contains the various STM32CubeIDE projects used to create FreshByte. This was done to facilitate the writing and testing of driver code on different development boards.

### Folder Descriptions
- 📂 Archive : Contains projects used to develop driver code and related functions.
- 📂 Data : Contains the data we collected in order to generate our predictive model for banana spoilage.
- 📂 ece477-nucleo-integration : The project for integrating all of our components, driver code, and functionality (Nucleo-64, STM32L053)

### Repository Structure
```
ece477-freshbyte
 |-- Archive
 |   |-- ece477-edp
 |   |-- ece477-esp8266
 |   |-- ece477-hts221-bq27441-vcnl4010
 |   |-- ece477-hts221demo
 |   |-- ece477-hx8357-nucleo
 |   |-- ece477-i2c-integrated-nucleo
 |   |-- ece477-integration
 |   |-- ece477-methane-r0-calc
 |   |-- ece477-pressure
 |   |-- ece477-tim6demo
 |   |-- Micropython
 |   `-- spi_display_hx8357
 |
 |-- Data
 |   |-- ECE477_freshbyte - Sheet1.csv
 |   |-- ECE477_freshbyte (1) - Sheet1.csv
 |   `-- ECE477_freshbyte (2) - Sheet1.csv
 |
 `-- ece477-nucleo-integration
```

## Components
### These are the final components used in the `ece477-nucleo-integration` project.
Not listed are the resistors and capacitors required by some of these peripherals / used provide filtering on our analog inputs.

* Main MCU: `STM32L053 Nucleo-64`
* Wi-Fi MCU: `Ai-Thinker ESP8266 (ESP-12E)`
* 3.3V Buck-Boost: `Pololu 3.3V Step-Up/Step-Down (S9V11F3S5)`
* 5V Buck-Boost: `Adafruit Miniboost 5V @ 1A (TPS61023)`
* Battery: `Adafruit Li-Ion Battery Pack, 3.7V 4400 mAh`
* Battery PMIC: `Sparkfun Battery Babysitter`
* Temp/RH: `Adafruit HTS221 Breakout Board`
* Proximity: `Adafruit VCNL4010 Breakout Board`
* Display: `Adafruit 2.7" Tri-color eInk Shield`
* Pressure: `Adafruit Square Force-Sensitive Resistor`
* Methane: `Sparkfun MQ-4 Methane Sensor + Breakout Board`

