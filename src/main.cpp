#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>

#define LED_PIN D1 // GPIO5 (D1 on most ESP8266 boards)
#define SWITCH D2
const int NUM_LEDS = 10; // Update this to the number of LEDs in your strip

CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);

bool colorChanged = false;
bool animationChanged = false;
byte newR = 255, newG = 0, newB = 0; // Default to red
int newAnimationIndex = -1;

// Set your static IP address
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Function declarations
void handleRoot();
void handleColor();
void handleAnimationRequest();
void setColor(byte r, byte g, byte b);
void displayAnimation(int index);
void rainbow_wave();
void confetti();
void sinelon();
void juggle();
void bpm();

bool isAnimationActive = false; // To track if an animation is active
bool colorSet = false; // To track if the color has been set

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(99);

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

  server.on("/color", HTTP_POST, handleColor);
  server.on("/animation", HTTP_GET, handleAnimationRequest);

  server.begin();
  Serial.println("HTTP server started");

  pinMode(SWITCH, OUTPUT);
  digitalWrite(SWITCH, HIGH);
  setColor(newR, newG, newB); // Initialize with the default color
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {rainbow_wave, confetti, sinelon, juggle, bpm};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  server.handleClient();

  if (digitalRead(SWITCH) == HIGH) {
    if (isAnimationActive) {
      isAnimationActive = false;
      fill_solid(leds, NUM_LEDS, CRGB(newR, newG, newB));
      FastLED.show();
      Serial.println("Switching to solid color");
    }
  } 
  else {
    if (!isAnimationActive) {
      isAnimationActive = true;
      Serial.println("Switching to animation");
    }
    displayAnimation(newAnimationIndex >= 0 ? newAnimationIndex : 0);
    FastLED.show(); // Call FastLED.show() only in animation mode
    delay(10);
  }

  delay(100);
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
  if (server.hasArg("value")) { // Check if the 'value' argument is present
    String color = server.arg("value");
    Serial.print(color);
    long number = strtol(color.c_str(), NULL, 16);
    newR = (number >> 16) & 255;
    newG = (number >> 8) & 255;
    newB = number & 255;

    // Set the color and reset animation index
    fill_solid(leds, NUM_LEDS, CRGB(newR, newG, newB));
    FastLED.show();
    newAnimationIndex = -1;
    isAnimationActive = false;
    colorSet = true; // Mark that the color has been set
    Serial.printf("Color set to: R=%d, G=%d, B=%d\n", newR, newG, newB);
  }
}

void handleAnimationRequest() {
  String anim = server.arg("a");
  newAnimationIndex = anim.toInt();
  animationChanged = true; // Set the flag to indicate the animation needs updating
  server.send(200, "text/plain", "Animation set");
  Serial.printf("Animation set to: %d\n", newAnimationIndex);
}

void setColor(byte r, byte g, byte b) {
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  colorSet = true; // Ensure the color has been set
  delay(10);
  
  FastLED.show();
}

void displayAnimation(int index) {
  if (index >= 0 && index < 5) {
    gPatterns[index]();
  }
}

void rainbow_wave() {
  uint8_t thisHue = beat8(10, 255);  // A simple rainbow march.
  fill_rainbow(leds, NUM_LEDS, thisHue, 10);  // Use FastLED's fill_rainbow routine.
}

void confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
