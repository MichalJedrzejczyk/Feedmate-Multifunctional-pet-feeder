# Wiring

## LCD Display

The LCD display is connected using the LiquidCrystal library in 4-bit mode.

| LCD Pin | Arduino Pin |
|---|---|
| RS | 12 |
| EN | 11 |
| D4 | 5 |
| D5 | 4 |
| D6 | 3 |
| D7 | 2 |

## Servo Motor

| Servo Pin | Arduino Pin |
|---|---|
| Signal | A7 |

## Buzzer

| Buzzer Pin | Arduino Pin |
|---|---|
| Signal | 9 |

## Keypad

The 4x4 matrix keypad is connected to analog pins A8-A15.

| Keypad Line | Arduino Pin |
|---|---|
| Row 1 | A8 |
| Row 2 | A9 |
| Row 3 | A10 |
| Row 4 | A11 |
| Column 1 | A12 |
| Column 2 | A13 |
| Column 3 | A14 |
| Column 4 | A15 |

## Notes

The current prototype uses Arduino Mega 2560 because it provides enough available pins for the LCD display, keypad, buzzer and servo motor.

A future version may use an ESP32 microcontroller with a redesigned wiring layout, Wi-Fi connectivity and mobile app integration.
