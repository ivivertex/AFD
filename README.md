# Pump Control System 🌊
The Pump Control System is an Arduino-based project designed to manage and control a pump using a web-based interface and MQTT protocol. The system provides a comprehensive set of features, including scheduling, calibration, and real-time monitoring, to ensure efficient and reliable pump operation.

## 🚀 Features
* Web-based interface for remote monitoring and control
* MQTT protocol for real-time communication and data exchange
* Scheduling feature for automated pump operation
* Calibration feature for accurate pump performance
* Real-time monitoring of pump state, level sensor readings, and system status
* WiFi connectivity for easy setup and configuration
* LittleFS file system for storing configuration data and scheduled jobs

## 🛠️ Tech Stack
* Arduino framework
* ESP8266WebServer library for web server functionality
* PubSubClient library for MQTT client functionality
* LittleFS library for file system operations
* ArduinoJson library for JSON parsing and generation
* WiFiManager library for WiFi operations
* ConfigManager library for configuration data management
* PumpControl library for pump control functionality
* SchedulerManager library for scheduling jobs
* SystemStatus library for system status management
* TaskRunner library for task management

## 📦 Installation
To install the Pump Control System, follow these steps:
1. Clone the repository to your local machine
2. Install the required libraries using the Arduino Library Manager
3. Configure the WiFi settings and MQTT broker details in the `ConfigManager.h` file
4. Upload the code to your Arduino board
5. Access the web-based interface using a web browser

## 💻 Usage
To use the Pump Control System, follow these steps:
1. Access the web-based interface using a web browser
2. Configure the pump settings, scheduling, and calibration using the web interface
3. Monitor the pump state, level sensor readings, and system status in real-time
4. Use the MQTT protocol to receive real-time updates and control the pump remotely

## 📂 Project Structure
```markdown
pump_control_system/
|-- pump_control.ino
|-- PumpControl.h
|-- PumpControl.cpp
|-- SchedulerManager.h
|-- SchedulerManager.cpp
|-- ConfigManager.h
|-- ConfigManager.cpp
|-- WebServer.h
|-- WebServer.cpp
|-- MQTTManager.h
|-- MQTTManager.cpp
|-- SystemStatus.h
|-- SystemStatus.cpp
|-- TaskRunner.h
|-- TaskRunner.cpp
|-- LittleFS/
|-- WiFiManager/
|-- PubSubClient/
|-- ArduinoJson/
|-- README.md
```

## 📸 Screenshots

## 🤝 Contributing
To contribute to the Pump Control System, please fork the repository and submit a pull request with your changes.

## 📝 License
The Pump Control System is licensed under the MIT License.
