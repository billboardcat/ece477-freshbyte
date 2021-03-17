# FreshByte
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
- [ ] An ability to read pressure sensor data to determine if food is in the box.
- [ ] An ability to transmit data to external device for analysis over a WiFi connection.
- [ ] An ability to read battery charge level data through I2C using a PMIC.
- [ ] ability to show environmental conditions on the display.
- [ ] An ability to wake the display using a proximity sensor.

## About the repository structure
This repository contains the various STM32Cube projects used to create FreshByte.

- 📂 ece477-integration : The project for integrating all of our components and driver code.
- 📂 ece477-epd : The project used to develop our e-ink display driver code.
- 📂 spi_display_hx8357 : A project (1/2) used to develop our LCD driver code. (STM32F091)
- 📂 ece477-hx8357-nucleo : A project (2/2) used to test our LCD driver code on our MCU. (STM32L053)
- 📂 ece477-hts221-bq27441-vcnl4010 : The project used to integrate our I2C peripherals.
