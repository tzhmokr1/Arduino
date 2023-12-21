//------------------------------------------------------------------------------
//  File: IoT-Uebung1.ino
//------------------------------------------------------------------------------
//  (C) Samotech Engineering GmbH, Thomas Hoeltschi, CH-8604 Volketswil
//------------------------------------------------------------------------------
// Project            : IoT-Grundlagen
// Module Reference   : .
// Target Hardware    : ESP8266 (Wemos D1 Mini)
// Language/Compiler  : Arduino IDE 1.8.13
// Author             : Thomas Hoeltschi
//------------------------------------------------------------------------------
// Description:
//    Framework for Uebung1 on IoT Grundlagen @ OST 
//
// History:
//  27-Nov-2020  Thomas Hoeltschi, Created
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//------------------------------------------------------------------------------
// Local constants
//------------------------------------------------------------------------------
#define HW_UART_SPEED                  115200L

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0

static const unsigned char PROGMEM logo32_glcd_bmp[] =
{ B00000000, B11111111, B11000111, B10000000,
  B00000111, B11111111, B11100111, B11100000,
  B00011111, B11111111, B11100111, B11111000,
  B00111111, B11111111, B11100011, B11111100,
  B00111111, B11111111, B11110011, B11111110,
  B01111111, B11111111, B11110011, B11111110,
  B01111111, B11111111, B11110011, B11111110,
  B01111111, B11111111, B11110011, B11111111,
  B11111111, B11111111, B11110001, B11111111,
  B11111111, B11111111, B11110001, B11111111,
  B11111111, B11111111, B11111001, B11111111,
  B11111111, B11111111, B11111001, B11111111,
  B11111111, B11111111, B11111001, B11111111,
  B11111111, B11111111, B11111001, B11111111,
  B11111111, B11111111, B11110001, B11111111,
  B11111111, B11111111, B11110011, B11111111,
  B11111111, B11111111, B11000011, B11111111,
  B11111111, B11111111, B10000111, B11111111,
  B11111111, B11111100, B00000111, B11111111,
  B11111111, B11000000, B00001111, B11111111,
  B01111100, B00000000, B00011111, B11111111,
  B01100000, B00000000, B00111111, B11111110,
  B00000000, B00000001, B11111111, B11111110,
  B00000000, B00001111, B11111111, B11111110,
  B00000000, B11111111, B11111111, B11111100,
  B00001111, B11111111, B11111111, B11111100,
  B00011111, B11111111, B11111111, B11111000,
  B00000111, B11111111, B11111111, B11100000,
  B00000000, B11111111, B11111111, B00000000,
  B00000000, B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000, B00000000 };


#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Button definitions
const int buttonA = D3;
const int buttonB = D4;

//------------------------------------------------------------------------------
// Variable definitions
//------------------------------------------------------------------------------

Adafruit_SSD1306 display(OLED_RESET);
int counter = 0;
bool btnA_pressed = false;
bool btnB_pressed = false;

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

void displayCounter()
{
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(2,0);
  display.println(counter);
  display.display();
}

//------------------------------------------------------------------------------
// Application setup
//------------------------------------------------------------------------------
void setup() 
{
  // Setup hardware serial for logging
  Serial.begin(HW_UART_SPEED);
  while (!Serial);
  delay(5000);
  Serial.println("Startup system...");

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  // Display Samotech Logo
  display.clearDisplay();
  display.drawBitmap(15, 1, logo32_glcd_bmp, 32, 32, 1);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(8,35);
  display.println("Samotech");
  display.display();
  delay(3000);

  // Init buttons
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);

  displayCounter();
}

//------------------------------------------------------------------------------
// Application Mainloop
//------------------------------------------------------------------------------
void loop() 
{
  if (!(digitalRead(buttonA)))
  { // button pressed
    counter++;
    displayCounter();
    Serial.println("Key A "); Serial.print(counter);
  }

  if (!(digitalRead(buttonB)))
  { // button pressed
    counter--;
    displayCounter();
    Serial.println("Key B "); Serial.print(counter);
  }
}
