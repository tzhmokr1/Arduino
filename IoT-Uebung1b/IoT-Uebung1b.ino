//------------------------------------------------------------------------------
//  File: IoT-Uebung1b.ino
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
//    Framework for Uebung1b on IoT Grundlagen @ OST 
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

//------------------------------------------------------------------------------
// Variable definitions
//------------------------------------------------------------------------------

Adafruit_SSD1306 display(OLED_RESET);
bool btn_pressed = false;

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

void displayNumber(byte num)
{
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(2,0);
  display.println(num);
  display.display();
}

#define BUTTON_EVENT_NONE 0
#define BUTTON_EVENT_PRES 1
#define BUTTON_EVENT_RELE 2

// Check state of button
byte checkButton()
{
  if (!(digitalRead(buttonA)))
  { // button pressed
    if (!btn_pressed)
    {
      delay(10);
      btn_pressed = true;
      return BUTTON_EVENT_PRES;
    }
  }
  else
  { // button released
    if (btn_pressed)
    {
      btn_pressed = false;
      return BUTTON_EVENT_RELE;
    }
  }
  return BUTTON_EVENT_NONE;
}

// Show running numer
void runningDice(void)
{
  static byte i = 1;
  i++;
  if (i > 6) i = 1;
  displayNumber(i);
}

// show a blinking go
void blinkingGo(void)
{
  static byte i = 0;
  // Show On/OFF pattern on display
  if (i %2)
  {
    display.clearDisplay();
    display.setTextSize(5);
    display.setTextColor(WHITE);
    display.setCursor(2,0);
    display.println("GO");
    display.display();
  }
  else
  {
    display.clearDisplay();
    display.display();
  }
  i++;
}

//------------------------------------------------------------------------------
// Application setup
//------------------------------------------------------------------------------
void setup() 
{
  // Setup hardware serial for logging
  Serial.begin(HW_UART_SPEED);
  while (!Serial);
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
}

//------------------------------------------------------------------------------
// Application Mainloop
//------------------------------------------------------------------------------
void loop() 
{
  // todo: Show pattern until user startsOn/OFF pattern on display


  // Start playing
  byte diceNumber = 0; // hmm, what function can be used?

  // todo: Show dice running

  // Display dice number
  displayNumber(diceNumber);

  delay(5000); // Show number for 5 seconds
}
