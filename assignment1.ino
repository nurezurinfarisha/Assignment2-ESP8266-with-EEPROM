#include <ESP8266WiFi.h>       // Library for WiFi functionality
#include <ESP8266WebServer.h>  // Library to create a web server
#include <EEPROM.h>            // Library to read/write EEPROM

#define EEPROM_SIZE 96
#define SSID_ADDR 0
#define PASSWORD_ADDR 32
#define DEVICE_ID_ADDR 64
#define LED_STATUS_ADDR 80
#define LED_PIN 2  // GPIO2 (D4 on NodeMCU)

ESP8266WebServer server(80);

void writeStringToEEPROM(int addr, const String &str) {
  for (int i = 0; i < str.length(); ++i) {
    EEPROM.write(addr + i, str[i]);
  }
  EEPROM.write(addr + str.length(), '\0');  // Null-terminate the string
  EEPROM.commit();  // Save changes to EEPROM
}

String readStringFromEEPROM(int addr) {
  char data[32];
  int len = 0;
  unsigned char k;
  k = EEPROM.read(addr);
  while (k != '\0' && len < 32) {  // Read until null-terminator or max length
    k = EEPROM.read(addr + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';  // Ensure null-termination
  return String(data);
}

void writeIntToEEPROM(int addr, int value) {
  EEPROM.write(addr, value);
  EEPROM.commit();  // Save changes to EEPROM
}

int readIntFromEEPROM(int addr) {
  return EEPROM.read(addr);
}

void startAPMode() {
  // Start the ESP8266 in AP mode with the given SSID and password
  WiFi.softAP("ESP8266_Config", "password");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Define the web server routes
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/display", handleDisplay);  // New route to display saved details
  server.begin();  // Start the server
  Serial.println("HTTP server started");

  // Set up the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Turn the LED off initially
}

void handleRoot() {
  // HTML form to collect WiFi credentials, device ID, and LED status
  String html = "<html><body><h1>ESP8266 Configuration</h1>"
                "<style>"
                "body { font-family: Arial, sans-serif; }"
                "form { max-width: 400px; margin: auto; padding: 10px; border: 1px solid #ccc; border-radius: 5px; }"
                "input[type='text'], input[type='password'] { width: 100%; padding: 8px; margin: 6px 0; border: 1px solid #ccc; border-radius: 4px; }"
                "input[type='submit'] { width: 100%; padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; }"
                "input[type='submit']:hover { background-color: #45a049; }"
                "</style>"
                "<form action='/save' method='POST'>"
                "SSID: <input type='text' name='ssid'><br>"
                "Password: <input type='password' name='password'><br>"
                "Device ID: <input type='text' name='device_id'><br>"
                "LED Status: <input type='radio' name='led_status' value='1'>On"
                "<input type='radio' name='led_status' value='0'>Off<br><br>"
                "<input type='submit' value='Save'>"
                "</form></body></html>";
  server.send(200, "text/html", html);  // Send the form to the client
}

void handleSave() {
  // Retrieve the form data
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String device_id = server.arg("device_id");
  int led_status = server.arg("led_status").toInt();

  // Write the data to EEPROM
  writeStringToEEPROM(SSID_ADDR, ssid);
  writeStringToEEPROM(PASSWORD_ADDR, password);
  writeStringToEEPROM(DEVICE_ID_ADDR, device_id);
  writeIntToEEPROM(LED_STATUS_ADDR, led_status);

  // Create a confirmation message with the saved details
  String html = "<html><body><h1>Configuration Saved!</h1>"
                "<p>SSID: " + ssid + "</p>"
                "<p>Password: " + password + "</p>"
                "<p>Device ID: " + device_id + "</p>"
                "<p>LED Status: " + String(led_status == 1 ? "On" : "Off") + "</p>"
                "<p>Restarting...</p></body></html>";
  server.send(200, "text/html", html);  // Send the confirmation message

  delay(2000);  // Wait for 2 seconds before restarting
  ESP.restart();  // Restart the ESP8266
}

void handleDisplay() {
  // Retrieve the stored data from EEPROM
  String ssid = readStringFromEEPROM(SSID_ADDR);
  String password = readStringFromEEPROM(PASSWORD_ADDR);
  String device_id = readStringFromEEPROM(DEVICE_ID_ADDR);
  int led_status = readIntFromEEPROM(LED_STATUS_ADDR);

  // Display the stored configuration details
  String html = "<html><body><h1>Stored Configuration</h1>"
                "<p>SSID: " + ssid + "</p>"
                "<p>Password: " + password + "</p>"
                "<p>Device ID: " + device_id + "</p>"
                "<p>LED Status: " + String(led_status == 1 ? "On" : "Off") + "</p>"
                "</body></html>";
  server.send(200, "text/html", html);  // Send the stored details to the client
}

void connectToWiFi(String ssid, String password) {
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi!");
    int led_status = readIntFromEEPROM(LED_STATUS_ADDR);
    digitalWrite(LED_PIN, led_status == 1 ? HIGH : LOW);  // Set the LED status
  } else {
    Serial.println("Failed to connect to WiFi. Starting AP mode...");
    startAPMode();  // If connection fails, start AP mode again
  }
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  EEPROM.begin(EEPROM_SIZE);  // Initialize EEPROM

  // Read stored WiFi credentials from EEPROM
  String ssid = readStringFromEEPROM(SSID_ADDR);
  String password = readStringFromEEPROM(PASSWORD_ADDR);

  if (ssid.length() == 0 || password.length() == 0) {
    // If no valid credentials, start AP mode
    startAPMode();
  } else {
    // Otherwise, attempt to connect to WiFi
    connectToWiFi(ssid, password);
  }
}

void loop() {
  server.handleClient();  // Handle web server client requests
}
