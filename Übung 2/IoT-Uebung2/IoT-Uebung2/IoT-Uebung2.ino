//------------------------------------------------------------------------------
//  File: IoT-Uebung2.ino
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
//  Framework for Uebung2 on IoT Grundlagen @ OST
//
// History:
//  27-Nov-2020  Thomas Hoeltschi, Created
//  13-Feb-2021  Thomas Hoeltschi, Online broker added.
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

//------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------
const char* ssid     = "Zackenbarsch 5G"; // Your ssid
const char* password = "Ap3r1end!"; // Your Password

#define MQTT_ID        "IoT-Client_Area51" // Your MQTT ID

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
// Topics to subscribe
char MQTT_TOPIC_TEMP[] = "IoT-Master/SensorData/Temperature";
char MQTT_TOPIC_HUMI[] = "IoT-Master/SensorData/Humidity";

static MqttClient *mqtt = NULL;
static WiFiClient network;

String temp;
String humi;

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
void processMessageTemp(MqttClient::MessageData& md) 
{
    const MqttClient::Message& msg = md.message;
    char payload[msg.payloadLen + 1];
    memcpy(payload, msg.payload, msg.payloadLen);
    payload[msg.payloadLen] = '\0';
    LOG_PRINTFLN(
      "Message arrived Temp: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
      msg.qos, msg.retained, msg.dup, msg.id, payload
    );
    // todo: display temperature
}

void processMessageHum(MqttClient::MessageData& md) 
{
    const MqttClient::Message& msg = md.message;
    char payload[msg.payloadLen + 1];
    memcpy(payload, msg.payload, msg.payloadLen);
    payload[msg.payloadLen] = '\0';
    LOG_PRINTFLN(
      "Message arrived Hum: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
      msg.qos, msg.retained, msg.dup, msg.id, payload
    );
    // todo: display humidity
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
    { // Subscribe to temperature
      MqttClient::Error::type rc = mqtt->subscribe(MQTT_TOPIC_TEMP, MqttClient::QOS0, processMessageTemp);
      if (rc != MqttClient::Error::SUCCESS) 
      {
        LOG_PRINTFLN("Subscribe error: %i", rc);
        LOG_PRINTFLN("Drop connection");
        mqtt->disconnect();
        return;
      }
    }
    { // Subscribe to humidity
      MqttClient::Error::type rc = mqtt->subscribe(MQTT_TOPIC_HUMI, MqttClient::QOS0, processMessageHum);
      if (rc != MqttClient::Error::SUCCESS) 
      {
        LOG_PRINTFLN("Subscribe error: %i", rc);
        LOG_PRINTFLN("Drop connection");
        mqtt->disconnect();
        return;
      }
    }
  }

  // todo: Implement button here with their pages to display data

  // Give mqtt time to run
  mqtt->yield(200);
}
