# 🤖 JA3 Control

**A phone-controlled web interface for an ESP32-based line follower robot.**

JA3 Control is a personal robotics project focused on controlling and configuring a line follower robot through a web interface hosted directly on an ESP32.

The goal is to replace traditional physical control buttons with a simple dashboard accessible from a smartphone.

## ✨ Features

-  Phone-based robot control through a local web interface
-  ESP32 Wi-Fi Access Point
-  Manual PID parameter configuration (Kp, Ki, Kd)
-  Adjustable base speed, maximum speed, and turn speed
-  START, STOP, and CALIBRATE commands
-  Settings stored locally in the browser
-  LittleFS filesystem for hosting website files

##  Technologies

- ESP32 DevKit V1 (ESP32-WROOM-32)
- Arduino IDE
- C/C++
- HTML
- CSS
- JavaScript
- ArduinoJson
- LittleFS

##  Current Progress

The ESP32 web server and phone communication are working.

Currently implemented and tested:

- ESP32 Wi-Fi connection
- Website hosting through LittleFS
- PID and speed settings transmission
- START, STOP, and CALIBRATE command transmission
- Basic dashboard status updates

**Note:** The current commands update software flags on the ESP32. Physical motor control and actual sensor calibration have not yet been integrated.

##  Future Improvements

- [ ] Integrate the motor driver
- [ ] Integrate QTR line sensor calibration
- [ ] Implement the PID control algorithm
- [ ] Connect the dashboard commands to the actual robot
- [ ] Test and tune the robot's performance
- [ ] Improve the dashboard and add more features

##  Project Goal

Build a functional line follower robot with a smartphone-based control and configuration system, combining embedded programming, robotics, and web development.

---

**JA3 Control — A robotics project built step by step.** ❤️
