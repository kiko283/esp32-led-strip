#include <Arduino.h>
#include "LEDStrip.h"
#include "Codes.h"
#include "Secret.h"

// /fw_version endpoint returns this value, you can
// use this to keep track of which version is in device
const char* FW_VERSION = "2.0.0";

// ##### OTHER NECESSARY VARIABLES #####

const int ONBOARD_LED_PIN = 2;
const int ONBOARD_LED_ON  = HIGH;
const int ONBOARD_LED_OFF = LOW;

const int RGB_PIN = 15;
// // SHORT TEST STRIP (4 LEDS)
// const int STRIP_LED_COUNT = 4;
// const int STRIP_EVERY_XTH = 4;
// // LONG TEST STRIP (150 LEDS = 5m)
// const int STRIP_LED_COUNT = 150;
// const int STRIP_EVERY_XTH = 10;
// KIKO'S STRIP (200 LEDS)
const int STRIP_LED_COUNT = 200;
const int STRIP_EVERY_XTH = 20;

LEDStrip strip(STRIP_LED_COUNT, RGB_PIN);

// ##### ##### ##### ##### #####

// Custom setup function, executed from main setup()
void setup1 () {

  // Serial.begin(115200) already executed in main setup()

  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(RGB_PIN, OUTPUT);

  // Start LED strip
  strip.Begin();
  digitalWrite(RGB_PIN, LOW);
  delay(500);
  // Startup pattern
  strip.SetRainbow(true);
  strip.Fade(20, 100);
}

// Custom loop function, continuously executed from main loop()
void loop1() {
  strip.Update(millis());
}

// Callback function when device connects to WiFi
void connectedToWiFiCallback() {
  uint32_t bckColor1 = strip.GetColor1();
  uint32_t bckColor2 = strip.GetColor2();
  bool bckRainbow = strip.IsRainbow();
  mode bckActiveMode = strip.GetActiveMode();
  strip.SetActiveMode(CONTINUOUS);
  strip.SetRainbow(false);
  strip.SetColor2(0x000000);
  strip.SetColor1(0x004800);
  digitalWrite(ONBOARD_LED_PIN, ONBOARD_LED_ON);
  delay(500);
  strip.SetColor1(0x484848);
  digitalWrite(ONBOARD_LED_PIN, ONBOARD_LED_OFF);
  delay(500);
  strip.SetActiveMode(bckActiveMode);
  strip.SetRainbow(bckRainbow);
  strip.SetColor1(bckColor1);
  strip.SetColor2(bckColor2);
}

// Helper function for process_command(cmd)
uint32_t stringToHex(String s) {
  uint8_t len = s.length();
  char c;
  uint32_t x = 0;
  for(uint8_t i = 0; i < len; i++) {
    c = toupper(s.charAt(i));
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
    } else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += 10 + (c - 'A');
    } else return 0;
  }
  return x;
}

// Function used to process commands sent through wifi
// * Device will print it's IP address on serial line
// * To send command to device:
//    - Make a POST request to http://<device_IP>/process_command
//    - POST body:  application/x-www-form-urlencoded
//    - POST field: command=<command_to_send>
void processCommand(String cmd) {

  uint32_t code = stringToHex(cmd);

  digitalWrite(ONBOARD_LED_PIN, ONBOARD_LED_ON);

  // Use the IR codes (numeric) for commands
  switch (code) {
    case SET_MODE_1: // Continuous
#ifdef DEBUG
      Serial.println("Continuous");
#endif
      strip.Continuous(200);
      break;
    case SET_MODE_2: // Blinking
#ifdef DEBUG
      Serial.println("Blinking");
#endif
      strip.Blinking(10, 100);
      break;
    case SET_MODE_3: // ColorWipe
#ifdef DEBUG
      Serial.println("ColorWipe");
#endif
      strip.ColorWipe(20);
      break;
    case SET_MODE_4: // TheaterChase
#ifdef DEBUG
      Serial.println("TheaterChase");
#endif
      strip.TheaterChase(200, STRIP_EVERY_XTH);
      break;
    case SET_MODE_5: // Scanner
#ifdef DEBUG
      Serial.println("Scanner");
#endif
      strip.Scanner(20);
      break;
    case SET_MODE_6: // Fade
#ifdef DEBUG
      Serial.println("Fade");
#endif
      strip.Fade(20, 100);
      break;
    case SET_MODE_7: // RainbowCycle
#ifdef DEBUG
      Serial.println("RainbowCycle");
#endif
      strip.RainbowCycle(20);
      break;
    case CLR_COLOR:
      strip.SetRainbow(false);
      strip.SetColor1(0);
      strip.SetColor2(0);
#ifdef DEBUG
      Serial.println("R, G, B: 0, 0, 0");
#endif
      break;
    default:
      if (((code >> 8) & 0xFFFFFF) == RAINBOW_PREFIX) {
        code = code & 0xFF;
#ifdef DEBUG
        Serial.print("R, G, B: RAINBOW (0x");
        Serial.print(code);
        Serial.println(")");
#endif
        strip.SetRainbow(true);
        strip.SetRainbowScale(code);
        if (strip.GetActiveMode() == COLOR_WIPE) {
          strip.ClearColor();
          strip.ColorWipe(20);
        } else if (strip.GetActiveMode() == THEATER_CHASE) {
          strip.ClearColor();
          strip.TheaterChase(200, STRIP_EVERY_XTH);
        } else if (strip.GetActiveMode() == SCANNER) {
          strip.ClearColor();
          strip.Scanner(20);
        }
      } else if (((code >> 24) & 0xFF) == SET_COLOR_PREFIX) {
        code = code & 0xFFFFFF;
        String color(code, HEX);
        color.toUpperCase();
        uint8_t leadingZeros = 6 - color.length();
        while (leadingZeros) {
          color = "0" + color;
          leadingZeros--;
        }
#ifdef DEBUG
        Serial.print("Setting color: 0x");
        Serial.println(color);
#endif
        // setting new color value
        if (strip.GetColor1() == code) {
          strip.SetColor2(code);
        }
        else {
          strip.SetColor1(code);
        }
        strip.SetRainbow(false);
        if (strip.GetActiveMode() == COLOR_WIPE) {
          strip.ClearColor();
          strip.ColorWipe(20);
        } else if (strip.GetActiveMode() == THEATER_CHASE) {
          strip.ClearColor();
          strip.TheaterChase(200, STRIP_EVERY_XTH);
        } else if (strip.GetActiveMode() == SCANNER) {
          strip.ClearColor();
          strip.Scanner(20);
        }
#ifdef DEBUG
        Serial.print("R, G, B: ");
        Serial.print((code >> 16) & 0xFF);
        Serial.print(", ");
        Serial.print((code >> 8) & 0xFF);
        Serial.print(", ");
        Serial.println((code) & 0xFF);
#endif
      }
      else {
        // unrecognised command, print on screen
#ifdef DEBUG
        Serial.print("Unrecognised: ");
        Serial.print(code, DEC);
        Serial.print(", ");
        Serial.println(code, HEX);
#endif
      }
      break;
  }

  digitalWrite(ONBOARD_LED_PIN, ONBOARD_LED_OFF);

}
