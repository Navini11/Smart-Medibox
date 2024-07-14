# Smart Medibox

The Smart Medibox is an advanced device designed to assist individuals in managing their medication schedules efficiently. Combining automated reminders with environmental monitoring, it ensures that medications are stored under optimal conditions.

## Overview

The Smart Medibox helps users adhere to their medication regimens while maintaining the best possible storage environment. It provides timely reminders, monitors temperature and humidity, and controls light exposure to ensure medication efficacy.Here is the WOKWI Simulation for this medibox.

![image](https://github.com/user-attachments/assets/db45de31-4978-43d6-9e5d-a644787671c9)

## 🛠️ Key Features

- **Automated Medication Reminders:** Set alarms to remind users to take their medications on time.
- **Environmental Monitoring:** Continuously tracks temperature and humidity levels, alerting users if conditions fall outside the recommended range.
- **Light Control:** Utilizes a motorized curtain to regulate light exposure, preserving the integrity of light-sensitive medications.

## 📚 Technologies and Components

- **OLED Display:** ADAFRUIT SSD 1306 OLED Monochrome Display (128x64) for clear and accessible information display.
- **Microcontroller:** ESP32 Devkit V1, providing robust performance and connectivity.
- **Sensors:** DHT11 for temperature and humidity monitoring (can be configured for DHT22).
- **Actuators:** SG90 Micro Servo Motor for light control.
- **Additional Components:** Light Dependent Resistors (LDRs), push buttons, and 10kΩ resistors.

## 🚀 Getting Started

### Prerequisites

#### 🖥️ Software:
- Git for repository management
- PlatformIO with Arduino Framework for development and deployment
- Node-RED for dashboard creation
- MQTT broker for communication

#### 🔩 Hardware:
- ESP32 Devkit V1
- ADAFRUIT SSD 1306 OLED Display
- DHT11 or DHT22 sensor
- SG90 Micro Servo Motor
- Light Dependent Resistors (LDRs)
- Push Buttons
- 10kΩ Resistors

![image](https://github.com/user-attachments/assets/7f1c5ef1-1b38-4f43-b822-88c9c68d2d82)

### Installation

1. **Clone the Repository:**
    ```bash
    git clone https://github.com/Navini11/Smart-Medibox
    ```
2. **For Simulation:**
    ```bash
    git clone -b Wokwi-Simulation https://github.com/Navini11/Smart-Medibox
    ```

3. **Set Up Development Environment:**
    Ensure PlatformIO is set up with the Arduino Framework. The necessary libraries are typically installed automatically, but you can refer to the `platform.ini` file for manual installations.

### Usage

#### 🔩 Hardware Setup:
Connect all components according to the provided wiring diagram in the repository.

#### 📤 Upload Code:
Upload the code to the ESP32 Devkit V1 using PlatformIO.

#### ⚙️ Configure Settings:
- Set medication reminders.
- Define temperature and humidity thresholds.
- Adjust light control preferences.

#### 🌐 Node-RED Dashboard:
- Import `flows.json` to Node-RED.
- Configure MQTT server parameters and deploy.

  

## 🧩 Software Architecture

The software is organized into modular components for efficient management:

- **Hardware Abstraction Layer:** Simplifies interaction with hardware components.
- **Sensor Management:** Manages data from the DHT sensor and LDRs.
- **Alarm Management:** Handles medication reminders and notifications.
- **Time Management:** Synchronizes with an NTP server and allows time zone configuration.
- **User Interface:** Provides a menu-driven interface on the OLED display.
- **Communication Management:** Manages MQTT communication for remote data transmission and control.

## 🏷️ About

This project originated as a Semester 4 assignment for the EN2853 - Embedded Systems & Applications module.

## 🎥 Demo video

https://github.com/user-attachments/assets/1137beab-cba0-48df-80ab-6a865d24aaf9


## 🤝 Contributing

We welcome contributions to enhance the Smart Medibox:

- **Bug Reports:** Submit issues if you encounter any bugs.
- **Feature Requests:** Suggest new features or improvements.

---

Feel free to customize this README file with your specific details and links.


