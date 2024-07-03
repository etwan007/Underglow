#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#define LED_PIN D1
#define NUM_LEDS 60
#define SWITCH D2
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 120

CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
uint8_t gHue = 0;

// Default color (red in HSV)
CHSV defaultColor = CHSV(0, 255, 255);
CHSV currentColor = defaultColor;
bool colorChanged = false;
CHSV newColor = defaultColor;

// Function declarations
void handleRoot();
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void setColor(CHSV color);
void rainbow_wave();

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Set up WiFi AP
  WiFi.softAP("BimmerColor", "Cbears04"); 
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Set up web server
  server.on("/", HTTP_GET, handleRoot);
  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/iro.min.js", LittleFS, "/iro.min.js");
  server.begin();

  // Set up WebSocket server
  webSocket.begin();
  webSocket.onEvent(handleWebSocketEvent);

  // Initialize LED strip with default color
  pinMode(LED_PIN, OUTPUT);
  setColor(defaultColor);

  // Initialize switch
  pinMode(SWITCH, OUTPUT);
  digitalWrite(SWITCH, HIGH);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Check switch state to toggle between modes
  if (digitalRead(SWITCH) == HIGH) {
    if (colorChanged) {
      setColor(newColor);
      FastLED.show();
      colorChanged = false;
      delay(50);
    }
  } else {
    rainbow_wave();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
}

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_TEXT:
      if (length > 0) {
        String message = String((char *)payload);
        if (message.startsWith("hsv:")) {
          String hsvValue = message.substring(4); // Skip "hsv:" prefix
          int firstComma = hsvValue.indexOf(',');
          int secondComma = hsvValue.indexOf(',', firstComma + 1);

          uint8_t h = hsvValue.substring(0, firstComma).toInt();
          uint8_t s = hsvValue.substring(firstComma + 1, secondComma).toInt();
          uint8_t v = hsvValue.substring(secondComma + 1).toInt();

          newColor = CHSV(h, s, v);
          colorChanged = true;
        } else if (message.startsWith("animation:")) {
          // Handle animation message if needed
        }
      }
      break;
    default:
      break;
  }
}

void setColor(CHSV color) {
  fill_solid(leds, NUM_LEDS, color);

  // Print HSV components
  Serial.printf("H: %d, S: %d, V: %d\n", color.hue, color.saturation, color.value);
}

void rainbow_wave() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
  
  gHue++;
}
