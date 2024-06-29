# ESP32 LED and Display Controller Firmware

## Overview

This repository contains the Arduino code for controlling LEDs and an OLED display using an ESP32. The code enables the ESP32 to act as an access point, allowing an Android app to connect and send commands to control LEDs and update the display.

## Features

- **WiFi Access Point**: ESP32 creates its own WiFi network for direct communication.
- **LED Control**: Turn on/off individual LEDs (Red, Green, Yellow) with a specified duration.
- **OLED Display Control**: Update and display the current time, train number, train name, track number, information text, and route information with scrolling text.
- **HTTP Server**: The ESP32 runs a web server to receive HTTP requests from the Android app and execute the corresponding actions.

## Pin Configuration

### LEDs
- **Red LED**: GPIO 12
- **Green LED**: GPIO 14
- **Yellow LED**: GPIO 27
- **Common Ground**: GND

### OLED Display
- **SDA**: GPIO 21
- **SCL**: GPIO 22
- **VCC**: 3.3V
- **GND**: GND

## Setup

### Hardware Requirements

- ESP32 Development Board
- LEDs (Red, Green, Yellow) with appropriate resistors
- AZ-Delivery 0.91 inch OLED I2C display with 128x32 pixels
- Breadboard and jumper wires

### Android Client

The Client for the ESP32 can be found in the following repository: [ESP32 LED and Display Controller](https://github.com/Cabzla/LED_OLED_Control)


### Software Requirements

- Arduino IDE

### Arduino IDE Setup

1. **Install ESP32 Board**:
   - Open Arduino IDE.
   - Go to `File` -> `Preferences`.
   - In the "Additional Board Manager URLs" field, add: `https://dl.espressif.com/dl/package_esp32_index.json`.
   - Go to `Tools` -> `Board` -> `Board Manager` and install the `esp32` platform.

2. **Install Required Libraries**:
   - `WiFi.h` for WiFi functionality.
   - `Adafruit_GFX.h` and `Adafruit_SSD1306.h` for OLED display control.

3. **Upload Code to ESP32**:
   - Open the provided `ESP32_LED_Display_Controller.ino` file in the Arduino IDE.
   - Select your ESP32 board and the correct port under `Tools`.
   - Click the upload button to flash the code to the ESP32.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For any inquiries or support, please contact Michael Krüger at [kruegermichael90@gmail.com].
