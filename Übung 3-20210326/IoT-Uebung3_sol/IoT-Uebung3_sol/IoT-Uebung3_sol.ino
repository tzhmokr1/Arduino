//------------------------------------------------------------------------------
//  File: IoT-Uebung3_sol.ino
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
//  Framework for Uebung3 on IoT Grundlagen @ OST
//
// History:
//  16-Jan-2021  Thomas Hoeltschi, Created
//  14-Feb-2021  Thomas Hoeltschi, make it compatible with broker.hivemq.com
//  21-Mar-2021  Thomas Hoeltschi, Id & UART speed changed
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <MqttClient.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <TinyGPS++.h>

//------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------
const char* ssid     = "Zackenbarsch 5G"; // Your ssid
const char* password = "Ap3r1end!"; // Your Password

#define MQTT_ID        "IoT15" // Your MQTT ID from the list

// Enable MqttClient logs
#define MQTT_LOG_ENABLED 1
// Uart Speed
#define HW_UART_SPEED    9600L

#define LOG_PRINTFLN(fmt, ...)  logfln(fmt, ##__VA_ARGS__)
#define LOG_SIZE_MAX 128
void logfln(const char *fmt, ...) 
{
  char buf[LOG_SIZE_MAX];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, LOG_SIZE_MAX, fmt, ap);
  va_end(ap);
  Serial.println(buf);
}

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

char MQTT_TOPIC_GPS[] = "IoT-Master3/GPS/Raw";
char MQTT_TOPIC_LON[] = MQTT_ID "/Lon";
char MQTT_TOPIC_LAT[] = MQTT_ID "/Lat";
char MQTT_TOPIC_SPD[] = MQTT_ID "/Speed";

static MqttClient *mqtt = NULL;
static WiFiClient network;

// The TinyGPS++ object
TinyGPSPlus gps;
String      gpsRaw;
bool        newGpsData;
double      latitude;
double      longitude;
double      speed;


Adafruit_SSD1306 display(OLED_RESET);

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------
// ============== Object to supply system functions ============================
class System: public MqttClient::System 
{
  public:
    unsigned long millis() const 
    {
      return ::millis();
    }

    void yield(void) 
    {
      ::yield();
    }
};

// ============== Subscription callback ========================================
void processMessageGps(MqttClient::MessageData& md) 
{
  const MqttClient::Message& msg = md.message;
  char payload[msg.payloadLen + 1];
  memcpy(payload, msg.payload, msg.payloadLen);
  payload[msg.payloadLen] = '\0';
  LOG_PRINTFLN(
    "Message arrived Temp: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
    msg.qos, msg.retained, msg.dup, msg.id, payload
  );
  gpsRaw     = payload;
  newGpsData = true;
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

  newGpsData = false;
  speed = 1079252848.8;
    
  // Init buttons
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);

  // Setup WiFi network
  WiFi.hostname(MQTT_ID);
  WiFi.begin(ssid, password);

  LOG_PRINTFLN("\n");
  LOG_PRINTFLN("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) 
	{
    delay(500);
    Serial.print(".");
  }
 
  Serial.println(".");
  LOG_PRINTFLN("Connected to WiFi");
  LOG_PRINTFLN("IP: %s", WiFi.localIP().toString().c_str());

  // Setup MqttClient
  MqttClient::System *mqttSystem = new System;
  MqttClient::Logger *mqttLogger = new MqttClient::LoggerImpl<HardwareSerial>(Serial);
  MqttClient::Network * mqttNetwork = new MqttClient::NetworkClientImpl<WiFiClient>(network, *mqttSystem);
  //// Make 128 bytes send buffer
  MqttClient::Buffer *mqttSendBuffer = new MqttClient::ArrayBuffer<128>();
  //// Make 128 bytes receive buffer
  MqttClient::Buffer *mqttRecvBuffer = new MqttClient::ArrayBuffer<128>();
  //// Allow up to 2 subscriptions simultaneously
  MqttClient::MessageHandlers *mqttMessageHandlers = new MqttClient::MessageHandlersImpl<2>();
  //// Configure client options
  MqttClient::Options mqttOptions;
  ////// Set command timeout to 10 seconds
  mqttOptions.commandTimeoutMs = 10000;
  //// Make client object
  mqtt = new MqttClient(
    mqttOptions, *mqttLogger, *mqttSystem, *mqttNetwork, *mqttSendBuffer,
    *mqttRecvBuffer, *mqttMessageHandlers
  );
}

//------------------------------------------------------------------------------
// Application Mainloop
//------------------------------------------------------------------------------
void loop() 
{
  // Check connection status
  if (!mqtt->isConnected()) 
  {
    // Close connection if exists
    network.stop();
    // Re-establish TCP connection with MQTT broker
    LOG_PRINTFLN("Connecting");
    network.connect("broker.hivemq.com", 1883);
    if (!network.connected()) 
    {
      LOG_PRINTFLN("Can't establish the TCP connection");
      delay(5000);
      ESP.reset();
    }
    // Start new MQTT connection
    MqttClient::ConnectResult connectResult;
    { // Connect
      MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
      options.MQTTVersion = 4;
      options.clientID.cstring = (char*)MQTT_ID;
      options.cleansession = true;
      options.keepAliveInterval = 15; // 15 seconds
      MqttClient::Error::type rc = mqtt->connect(options, connectResult);
      if (rc != MqttClient::Error::SUCCESS) 
      {
        LOG_PRINTFLN("Connection error: %i", rc);
        return;
      }
    }
    { // Subscribe to GPS Raw
      MqttClient::Error::type rc = mqtt->subscribe(MQTT_TOPIC_GPS, MqttClient::QOS0, processMessageGps);
      if (rc != MqttClient::Error::SUCCESS) 
      {
        LOG_PRINTFLN("Subscribe error: %i", rc);
        LOG_PRINTFLN("Drop connection");
        mqtt->disconnect();
        return;
      }
    }
  }
  else
  {
    if (newGpsData)
    {
      // Decode GPS data
      for (int n = 0; n < gpsRaw.length(); n++)
      {
        // We have to read the data cahracter by character as the library request it so (normally used with Serial port from GPS).      
        Serial.print(gpsRaw.charAt(n));
        if (gps.encode(gpsRaw.charAt(n)))
        {
          Serial.println(" ");
          if (gps.location.isValid())
          {
            latitude = gps.location.lat();
            longitude = gps.location.lng();


            { // Publish Longitude 
              char buf[20];
              sprintf(buf,"%.6f", longitude);
              MqttClient::Message message;
              message.qos = MqttClient::QOS0;
              message.retained = false;
              message.dup = false;
              message.payload = (void*) buf;
              message.payloadLen = strlen(buf);
              mqtt->publish(MQTT_TOPIC_LON, message);
              Serial.print("Longitude published: ");Serial.println(longitude);
            }
            // Give mqtt time to run
            mqtt->yield(50);
  
            { // Publish Latitude  
              char buf[20];
              sprintf(buf,"%.6f", latitude);
              MqttClient::Message message;
              message.qos = MqttClient::QOS0;
              message.retained = false;
              message.dup = false;
              message.payload = (void*) buf;
              message.payloadLen = strlen(buf);
              mqtt->publish(MQTT_TOPIC_LAT, message);
              Serial.print("Latitude published: "); Serial.println(latitude);
            }
            // Give mqtt time to run
            mqtt->yield(50);
          }
          else
          {
            // We mark it as invalid with the position of Atlantis!
            // As long as we don't go outside iof Europe, it's no problem.
            latitude  = 0;
            longitude = 0;
          }
          if (gps.speed.isValid())
          {
            speed = gps.speed.kmph();
            { // Publish speed  
              char buf[20];
              sprintf(buf,"%.2f", speed);
              MqttClient::Message message;
              message.qos = MqttClient::QOS0;
              message.retained = false;
              message.dup = false;
              message.payload = (void*) buf;
              message.payloadLen = strlen(buf);
              mqtt->publish(MQTT_TOPIC_SPD, message);
              Serial.print("Speed published: "); Serial.println(speed);
            }
          }
          else
          {
            // Invalid speed is marked as Warp 1.
            // It's ok as long as we don't use it with our Enterprise.
            speed = 1079252848.8;
          }
        }
      }
      newGpsData = false;
    }
  
    // Display network data on button request
    if (digitalRead(buttonA) == LOW)
    {
      // Display text
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("ID:");
      display.println(MQTT_ID);
      display.println("local");
      display.println(WiFi.localIP().toString());
      display.display();
    }
    else if (digitalRead(buttonB) == LOW)
    {
      // Display text
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("subnet");
      display.println(WiFi.subnetMask().toString());
      display.println("gateway");
      display.println(WiFi.gatewayIP().toString());
      display.display();
    }
    else
    {
      // Display data
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Lat:");
      display.println(latitude, 6);
      display.println("Lon:");
      display.println(longitude, 6);
      display.println("Speed:");
      display.println(speed, 2);
      display.display();
    }

    // Give mqtt time to run
    mqtt->yield(50);
  }
}
