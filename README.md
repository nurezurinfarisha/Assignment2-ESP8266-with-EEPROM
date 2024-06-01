# Assignment2-ESP8266-with-EEPROM
ESP8266 WiFi Configuration and LED Control
This project uses an ESP8266 microcontroller to create a sensor-based system that reads and writes WiFi configuration data, device ID, and the last output status (LED) to EEPROM. The system includes an Access Point (AP) mode web interface for configuring these parameters. Upon saving the configuration, the ESP8266 reloads with the saved settings and restores the last output status to the output component.

Features
Configure WiFi credentials, device ID, and LED status through a web interface.
Save configuration data to EEPROM.
Automatically reconnect to WiFi using saved credentials.
Restore the last saved LED status on startup.

Components
ESP8266 microcontroller (e.g., NodeMCU)
LED
Breadboard
Jumper wires

Circuit Diagram
LED Anode (Long leg): Connect to GPIO2 (D4) on the ESP8266 through a current-limiting resistor.
LED Cathode (Short leg): Connect to GND on the ESP8266.
