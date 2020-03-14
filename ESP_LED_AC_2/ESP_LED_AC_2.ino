#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#define HOSTIDENTIFY  "esp8266"
#define mDNSUpdate(c)  do { c.update(); } while(0)
using WebServerClass = ESP8266WebServer;
using HTTPUpdateServerClass = ESP8266HTTPUpdateServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "HTTPUpdateServer.h"
#define HOSTIDENTIFY  "esp32"
#define mDNSUpdate(c)  do {} while(0)
using WebServerClass = WebServer;
using HTTPUpdateServerClass = HTTPUpdateServer;
#endif

#include <WiFiClient.h>
#include <AutoConnect.h>

#include <Smoothed.h>
Smoothed<int> mySensor;

#include <Artnet.h>
Artnet artnet;

#define FASTLED_ESP32_I2S true
#include <FastLED.h>
#define FASTLED_SHOW_CORE 0
#define UNIVERSE_SIZE 170

//#define LED_PIN	7
//#define CLOCK_PIN	5
#define CLOCK_PIN	14
//#define LED_PIN1	12
//#define LED_PIN2	2
//#define LED_PIN3	4
//#define LED_PIN4	5
//#define LED_PIN5	0
#define LED_PIN6	13
// #define LED_PIN7	14
// #define LED_PIN8	15
// #define LED_PIN9	16
// #define LED_PIN10	17
// #define LED_PIN11	18
// #define LED_PIN12	19
// #define LED_PIN13	21
// #define LED_PIN14	22
// #define LED_PIN15	23
// #define LED_PIN16	25

#define ledsPerStrip	720
#define NUM_STRIPS	1
#define BRIGHTNESS	255
#define LED_TYPE	APA102
#define COLOR_ORDER	BGR
#define NUM_LEDS	ledsPerStrip * NUM_STRIPS
CRGB leds1[ledsPerStrip];
//CRGB leds2[ledsPerStrip];
//CRGB leds3[ledsPerStrip];
//CRGB leds4[ledsPerStrip];
//CRGB leds5[ledsPerStrip];
//CRGB leds6[ledsPerStrip];
//CRGB leds7[ledsPerStrip];
//CRGB leds8[ledsPerStrip];
//CRGB leds9[ledsPerStrip];
//CRGB leds10[ledsPerStrip];
//CRGB leds11[ledsPerStrip];
//CRGB leds12[ledsPerStrip];
//CRGB leds13[ledsPerStrip];
//CRGB leds14[ledsPerStrip];
//CRGB leds15[ledsPerStrip];
//CRGB leds16[ledsPerStrip];

const int numberOfChannels = (NUM_LEDS) * 3; // Total number of channels you want to receive (1 led = 3 channels)

// Art-Net settings
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

unsigned long timeLastFrame;

//void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
//{
//  Serial.print("Got packet / universe ");
//  Serial.println(universe);
//
//  sendFrame = 1;
//
//  // Store which universe has got in
//  if ((universe - startUniverse) < maxUniverses) {
//    universesReceived[universe - startUniverse] = 1;
//  }
//
//  for (int i = 0; i < maxUniverses; i++)
//  {
//    if (universesReceived[i] == 0)
//    {
//      Serial.print("Broke - Need universe ");
//      Serial.println(i);
//      sendFrame = 0;
//      break;
//    }
//  }
//
//  // read universe and put into the right part of the display buffer
//  for (int i = 0; i < 170; i++)
//  {
//    int led = i + (universe - startUniverse) * (170);
//    if (led < ledsPerStrip)
//      leds1[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    //    if (led >= ledsPerStrip && led < ledsPerStrip * 2)
//    //      leds2[led - ledsPerStrip] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    //    if (led >= ledsPerStrip * 2 && led < ledsPerStrip * 3)
//    //      leds3[led - ledsPerStrip * 2] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    //    if (led >= ledsPerStrip * 3 && led < ledsPerStrip * 4)
//    //      leds4[led - ledsPerStrip * 3] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    //    if (led >= ledsPerStrip * 4 && led < ledsPerStrip * 5)
//    //      leds5[led - ledsPerStrip * 4] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 5 && led < ledsPerStrip * 6)
//    //   leds6[led - ledsPerStrip * 5] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 6 && led < ledsPerStrip * 7)
//    //   leds7[led - ledsPerStrip * 6] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 7 && led < ledsPerStrip * 8)
//    //   leds8[led - ledsPerStrip * 7] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 8 && led < ledsPerStrip * 9)
//    //   leds9[led - ledsPerStrip * 8] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 9 && led < ledsPerStrip * 10)
//    //   leds10[led - ledsPerStrip * 9] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 10 && led < ledsPerStrip * 11)
//    //   leds11[led - ledsPerStrip * 10] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 11 && led < ledsPerStrip * 12)
//    //   leds12[led - ledsPerStrip * 11] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 12 && led < ledsPerStrip * 13)
//    //   leds13[led - ledsPerStrip * 12] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 13 && led < ledsPerStrip * 14)
//    //   leds14[led - ledsPerStrip * 13] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 14 && led < ledsPerStrip * 15)
//    //   leds15[led - ledsPerStrip * 14] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//    // if (led >= ledsPerStrip * 15 && led < ledsPerStrip * 16)
//    //   leds16[led - ledsPerStrip * 15] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//  }
//
//  if (sendFrame)
//  {
//    Serial.println("Got all universes");
//
//    unsigned long timeSinceLastFrame = millis() - timeLastFrame;
//    Serial.print(timeSinceLastFrame);
//    Serial.print(",");
//
//    timeLastFrame = millis();
//
//    //    FastLED.show();
//    FastLEDshowESP322();
//      artnet.resetsync();
//
//
//    unsigned long showTime = millis() - timeLastFrame;
//    Serial.print(showTime);
//    Serial.print(",");
//
//    int fps = 1000 / timeSinceLastFrame;
//    Serial.print(fps);
//    Serial.print(",");
//
//    mySensor.add(fps);
//    int averageFPS = mySensor.get();
//    Serial.println(averageFPS);
//
//    // Reset universesReceived to 0
//    memset(universesReceived, 0, maxUniverses);
//  }
//}

static TaskHandle_t FastLEDshowTaskHandle2 = 0;
static TaskHandle_t userTaskHandle = 0;
void FastLEDshowESP322()
{
  if (userTaskHandle == 0) {
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    // noInterrupts();
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle2);
    //to thge contrary to the other one we do not wait for the display task to come back
  }
}



void FastLEDshowTask2(void *pvParameters)
{
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 500 );
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    unsigned long timeSinceLastFrame = millis() - timeLastFrame;
    Serial.print(timeSinceLastFrame);
    Serial.print(",");

    timeLastFrame = millis();


    // memcpy(leds,Tpic,LED_WIDTH*LED_HEIGHT*sizeof(CRGB));

    memcpy(leds1, artnet.getframe(), NUM_LEDS * sizeof(CRGB));
    FastLED.show();
    //    controller.showPixels();

    unsigned long showTime = millis() - timeLastFrame;
    Serial.print(showTime);
    Serial.print(",");

    int fps = 1000 / timeSinceLastFrame;
    Serial.print(fps);
    Serial.print(",");

    mySensor.add(fps);
    int averageFPS = mySensor.get();
    Serial.print(averageFPS);
    Serial.println(",");

    userTaskHandle = 0; //so we can't have two display tasks at the same time

  }
}

// This page for an example only, you can prepare the other for your application.
static const char AUX_AppPage[] PROGMEM = R"(
{
  "title": "Hello world",
  "uri": "/",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "<h2>ESP-LED-LavaTube-YvesArtnet</h2>",
      "style": "text-align:center;color:#2f4f4f;padding:10px;"
    },
    {
      "name": "content",
      "type": "ACText",
      "value": "In this page, place the custom web page handled by the sketch application."
    }
  ]
}
)";

// Fix hostname for mDNS. It is a requirement for the lightweight update feature.
static const char* host = HOSTIDENTIFY "-webupdate";
#define HTTP_PORT 80

// ESP8266WebServer instance will be shared both AutoConnect and UpdateServer.
WebServerClass  httpServer(HTTP_PORT);

#define USERNAME "anticlockwise"   //*< Replace the actual username you want */
#define PASSWORD "poledance"   //*< Replace the actual password you want */
// Declare AutoConnectAux to bind the HTTPWebUpdateServer via /update url
// and call it from the menu.
// The custom web page is an empty page that does not contain AutoConnectElements.
// Its content will be emitted by ESP8266HTTPUpdateServer.
HTTPUpdateServerClass httpUpdater;
AutoConnectAux  update("/update", "Update");

// Declare AutoConnect and the custom web pages for an application sketch.
AutoConnect     portal(httpServer);
AutoConnectAux  hello;

void setup(){
  delay(1000);
  Serial.begin(500000);
  Serial.println("\nBooting Sketch...");
   xTaskCreatePinnedToCore(FastLEDshowTask2, "FastLEDshowTask2", 1000, NULL,3, &FastLEDshowTaskHandle2, FASTLED_SHOW_CORE);

  // Prepare the ESP8266HTTPUpdateServer
  // The /update handler will be registered during this function.
  httpUpdater.setup(&httpServer, USERNAME, PASSWORD);
  // Load a custom web page for a sketch and a dummy page for the updater.
  hello.load(AUX_AppPage);
  portal.join({ hello, update });
  
  AutoConnectConfig acc;
  acc.apid = "LavaTube";
  acc.psk = "poledance";
//  acc.portalTimeout = 1;
//  acc.retainPortal = true;                   // Retains the portal function after timed-out
  portal.config(acc);
    
  if (portal.begin()) {
    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    }
    else
      Serial.println("Error setting up MDNS responder");
  }
  else {
    Serial.println("portal.begin failed");
  }
  
  Serial.println(WiFi.localIP());

  FastLED.addLeds<LED_TYPE, LED_PIN6, CLOCK_PIN, COLOR_ORDER>(leds1, ledsPerStrip);
//  FastLED.addLeds<LED_TYPE, LED_PIN1, COLOR_ORDER>(leds1, ledsPerStrip);
//  FastLED.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds2, ledsPerStrip);
//  FastLED.addLeds<LED_TYPE, LED_PIN3, COLOR_ORDER>(leds3, ledsPerStrip);
//  FastLED.addLeds<LED_TYPE, LED_PIN4, COLOR_ORDER>(leds4, ledsPerStrip);
//  FastLED.addLeds<LED_TYPE, LED_PIN5, COLOR_ORDER>(leds5, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN6, COLOR_ORDER>(leds6, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN7, COLOR_ORDER>(leds7, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN8, COLOR_ORDER>(leds8, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN9, COLOR_ORDER>(leds9, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN10, COLOR_ORDER>(leds10, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN11, COLOR_ORDER>(leds11, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN12, COLOR_ORDER>(leds12, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN13, COLOR_ORDER>(leds13, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN14, COLOR_ORDER>(leds14, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN15, COLOR_ORDER>(leds15, ledsPerStrip);
  // FastLED.addLeds<LED_TYPE, LED_PIN16, COLOR_ORDER>(leds16, ledsPerStrip);

  FastLED.setBrightness(BRIGHTNESS);
  //    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
  FastLED.clear();
  FastLED.show();
  
  mySensor.begin(SMOOTHED_AVERAGE, 25);

artnet.begin(NUM_LEDS,UNIVERSE_SIZE,1); //the number of pixels and the maximum size of your iniverses 1 represent the buffer
//  artnet.setArtDmxCallback(onDmxFrame);

  timeLastFrame = millis();
}

void loop()
{
  artnet.read();
  FastLEDshowESP322();
  artnet.resetsync();
  mDNSUpdate(MDNS);
  portal.handleClient();
  //  delay(1);
}
