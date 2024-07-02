#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include "animations.h" // Include the animations header

#define LED_PIN D1 // GPIO5 (D1 on most ESP8266 boards)
const int NUM_LEDS = 200; // Update this to the number of LEDs in your strip


CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);
int i = 0;

bool colorChanged = false;
bool animationChanged = false;
byte newR, newG, newB;
int newAnimationIndex = 0;

// Set your static IP address
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Function declarations
void handleRoot();
void handleColor();
void handleAnimationRequest();
void setColor(byte r, byte g, byte b);

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(99);
  fill_solid(leds, NUM_LEDS, CRGB::Red); // Initialize all pixels to 'off'
  FastLED.show();

  // Set up the ESP8266 as an access point with a static IP
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("BimmerColor", "Cbears04");

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", HTTP_GET, handleRoot);
  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/iro.min.js", LittleFS, "/iro.min.js");

  server.on("/color", HTTP_GET, handleColor);
  server.on("/animation", HTTP_GET, handleAnimationRequest);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
 server.handleClient();

  if (colorChanged) {
    setColor(newR, newG, newB);
    colorChanged = false; // Reset the flag after updating
    FastLED.show();
    Serial.println("Color updated");
    delay(800);
  }

  if (animationChanged) {
    handleAnimation(newAnimationIndex);
    animationChanged = false; // Reset the flag after updating
    Serial.println("Animation updated");
  }

  fill_solid(leds, NUM_LEDS, CRGB(190, 0, 255));
  FastLED.show();
  
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

void handleColor() {
  String color = server.arg("value"); // Changed from 'c' to 'value' to match the client-side code
  long number = strtol(color.c_str(), NULL, 16); // No need to remove '#'
  newR = (number >> 16) & 255;
  newG = (number >> 8) & 255;
  newB = number & 255;
  colorChanged = true; // Set the flag to indicate the color needs updating
  server.send(200, "text/plain", "Color set");
  Serial.printf("Color set to: R=%d, G=%d, B=%d\n", newR, newG, newB);
}

void handleAnimationRequest() {
  String anim = server.arg("a");
  newAnimationIndex = anim.toInt();
  animationChanged = true; // Set the flag to indicate the animation needs updating
  server.send(200, "text/plain", "Animation started");
  Serial.printf("Animation set to: %d\n", newAnimationIndex);
}

void setColor(byte r, byte g, byte b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
}

