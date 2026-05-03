# FeedMate - A Multifunctional Pet Feeder

FeedMate is an Arduino-based multifunctional pet feeder designed to automate food dispensing using a servo mechanism, LCD interface, keypad navigation and configurable feeding modes.

## Features

- Manual feeding mode
- Automatic feeding mode with configurable interval
- Adjustable servo opening time
- LCD menu interface
- 4x4 keypad control
- Buzzer sound feedback
- Servo-based dispensing mechanism
- LCD sleep mode after inactivity
- Status screen with current configuration

## Technologies

- Arduino Mega 2560
- C/C++
- LiquidCrystal library
- Servo library

## Hardware Components

- Arduino Mega 2560
- 20x2 LCD display
- 4x4 matrix keypad
- Servo motor
- Buzzer
- Jumper wires
- Power supply
- Mechanical food dispensing container

## Pin Configuration

| Component | Pin |
|---|---|
| LCD RS | 12 |
| LCD EN | 11 |
| LCD D4 | 5 |
| LCD D5 | 4 |
| LCD D6 | 3 |
| LCD D7 | 2 |
| Buzzer | 9 |
| Servo | A7 |
| Keypad rows | A8-A11 |
| Keypad columns | A12-A15 |

## Controls

| Key | Function |
|---|---|
| A | Confirm / feed / repeat |
| B | Previous / edit opening time on start screen |
| C | Next |
| D | Menu / back |
| # | Status / clear input |
| * | Delete digit |

## Project Status

Prototype version. The project currently includes working LCD menu navigation, configurable feeding time, automatic feeding mode, buzzer feedback and servo-controlled dispensing.

In the future, I plan to replace the current Arduino-based hardware with an ESP32 microcontroller and develop a mobile application that will allow remote control, monitoring and configuration of the feeder.

## Future Improvements

- Replace the current Arduino-based hardware with an ESP32 microcontroller
- Develop a dedicated mobile application for remote control and monitoring
- Add Wi-Fi connectivity for real-time device communication
- Enable remote feeding activation from the mobile app
- Add feeding history and configuration management in the app
- Store settings permanently using non-volatile memory
- Add a food level sensor to monitor remaining food
- Improve the enclosure and mechanical dispensing system

## Author

Michał Jędrzejczyk
