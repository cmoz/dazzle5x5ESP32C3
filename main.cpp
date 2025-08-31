#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "font5x5.h"  // Your custom font

#define LED_PIN 8
#define MATRIX_WIDTH 5
#define MATRIX_HEIGHT 5
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)

Adafruit_NeoPixel matrix(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define BUTTON_PIN 9  // GPIO0 is the onboard button

// Wi-Fi credentials
const char* ssid = "XXXXXXXXX";
const char* password = "XXXXXXXXX";

// Web server on port 80
WebServer server(80);

// Message buffer
String message = "CMOZ  ";

uint32_t currentColor = matrix.Color(0, 128, 128); // default color

void displayLetter(int index, uint32_t color);
void displayMessage(const String& msg);

void displayLetter(int index, uint32_t color) {
  matrix.clear();
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      if (letters[index][y][x]) {
        int pixel = y * MATRIX_WIDTH + x;
        matrix.setPixelColor(pixel, color);
      }
    }
  }
  matrix.show();
}

void displayMessage(const String& msg) {
  for (int i = 0; i < msg.length(); i++) {
    int index = getLetterIndex(toupper(msg[i]));
    if (index >= 0) {
      displayLetter(index, currentColor);
      //displayLetter(index, matrix.Color(0, 128, 128));
      delay(500);
    }
  }
}

// HTML form

const char* htmlForm = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LED Matrix Input</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      background-color: #ffe6f0;
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 20px;
    }
    h2 {
      color: #333;
    }
    input[type="text"] {
      width: 80%;
      padding: 12px;
      font-size: 16px;
      border: 1px solid #ccc;
      border-radius: 6px;
      margin-bottom: 10px;
    }
    input[type="submit"] {
      padding: 12px 24px;
      font-size: 16px;
      background-color: #ff80aa;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
    }
    input[type="submit"]:hover {
      background-color: #ff6699;
    }
    .slider {
      width: 80%;
      margin: 10px auto;
    }
    #colorPreview {
      width: 100px;
      height: 100px;
      margin: 20px auto;
      border: 2px solid #ccc;
      border-radius: 10px;
      background-color: rgb(0, 128, 128);
    }
  </style>
</head>
<body>
  <h2>Enter Your Message</h2>
  <h5>you can also have numbers, and '!' '.' '-'</h5>

  <form action="/set" method="POST">
    <input type="text" name="msg" maxlength="20" placeholder="Type your message here"><br>

    <label>Red:</label><br>
    <input type="range" name="r" min="0" max="255" value="0" class="slider" oninput="updateColor()"><br>
    <label>Green:</label><br>
    <input type="range" name="g" min="0" max="255" value="128" class="slider" oninput="updateColor()"><br>
    <label>Blue:</label><br>
    <input type="range" name="b" min="0" max="255" value="128" class="slider" oninput="updateColor()"><br>

    <div id="colorPreview"></div>

    <input type="submit" value="Send">
  </form>

  <script>
    function updateColor() {
      const r = document.querySelector('input[name="r"]').value;
      const g = document.querySelector('input[name="g"]').value;
      const b = document.querySelector('input[name="b"]').value;
      document.getElementById('colorPreview').style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
    }
    window.onload = updateColor;
  </script>
</body>
</html>
)rawliteral";


void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

void handleSet() {
  if (server.hasArg("msg")) {
    message = server.arg("msg");

    // Get RGB values
    uint8_t r = server.hasArg("r") ? server.arg("r").toInt() : 0;
    uint8_t g = server.hasArg("g") ? server.arg("g").toInt() : 128;
    uint8_t b = server.hasArg("b") ? server.arg("b").toInt() : 128;
    currentColor = matrix.Color(r, g, b);

    server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Message Sent</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      background-color: #ffe6f0;
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 20px;
    }
    h2 {
      color: #333;
    }
    a {
      display: inline-block;
      margin-top: 20px;
      padding: 12px 24px;
      font-size: 16px;
      background-color: #ff80aa;
      color: white;
      text-decoration: none;
      border-radius: 6px;
    }
    a:hover {
      background-color: #ff6699;
    }
  </style>
</head>
<body>
  <h2>Message updated!</h2>
  <a href="/">Back to input</a>
</body>
</html>
)rawliteral");
  } else {
    server.send(400, "text/plain", "Missing message");
  }
}

void splashScreen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 255, 255)); // White sparkle
    matrix.show();
    delay(50);
    matrix.setPixelColor(i, 0); // Turn off
  }
  matrix.show();
}

void displayIP() {
  String ipMessage = WiFi.localIP().toString();
  for (int i = 0; i < 2; i++) {
    displayMessage(ipMessage);
    displayMessage("     "); // pause
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  matrix.begin();
  matrix.setBrightness(64);
  matrix.clear();
  matrix.show();

  splashScreen();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  displayIP();  // Show IP on startup

  /*
  String ipMessage = WiFi.localIP().toString();

  // Scroll IP address twice with pause
  for (int i = 0; i < 2; i++) {
    displayMessage(ipMessage);
    displayMessage("     "); // blank pause
  }
*/

  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.begin();
}

void loop() {
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    displayIP();  // Show IP when button is pressed
    delay(1000);  // Debounce
  }

  displayMessage(message);
  splashScreen();
}
