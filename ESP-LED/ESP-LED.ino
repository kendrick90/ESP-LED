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

#include <ArtnetWifi.h>

#include <FastLED.h>
#define FASTLED_ALLOW_INTERRUPTS 0

//#define LED_PIN     7
//#define CLOCK_PIN   5
#define CLOCK_PIN   14
#define LED_PIN     13
#define NUM_LEDS    720
#define BRIGHTNESS  25
#define LED_TYPE    APA102
#define COLOR_ORDER BGR
CRGB leds[NUM_LEDS];
const int numberOfChannels = (NUM_LEDS) * 3; // Total number of channels you want to receive (1 led = 3 channels)

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

unsigned long timeLastFrame;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  Serial.print("Got packet / universe ");
  Serial.println(universe);

  sendFrame = 1;

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses) {
    universesReceived[universe - startUniverse] = 1;
  }

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      Serial.print("Broke - Need universe ");
      Serial.println(i);
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < 510 / 3; i++)
  {
    int led = i + (universe - startUniverse) * (510 / 3);
    if (led < NUM_LEDS)
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  //  previousDataLength = length;

  //  // read universe and put into the right part of the display buffer
  //  for (int i = 0; i < length / 3; i++)
  //  {
  //    int led = i + (universe - startUniverse) * (previousDataLength / 3);
  //    if (led < NUM_LEDS)
  //      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  //  }
  //  previousDataLength = length;
  //  Serial.print("previousDataLength: ");
  //  Serial.println(previousDataLength);


  if (sendFrame)
  {
    //    Serial.println("Got all universes");
    unsigned long msBTW = millis() - timeLastFrame;
    Serial.print(msBTW);
    Serial.print(",");
    //    Serial.println("ms since last time full frame received");
    timeLastFrame = millis();
    FastLED.show();
    unsigned long msShow = millis() - timeLastFrame;
    //    Serial.println("FastLED.show took:");
    Serial.print(msShow);
    Serial.print(",");
    //    Serial.println("Total time:");
    Serial.println(msShow + msBTW);
    //    Serial.println("ms");
    // Reset universesReceived to 0
    memset(universesReceived, 0, maxUniverses);

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
      "value": "<h2>ESP-LED-LavaTube</h2>",
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

void setup() {
  delay(1000);
  Serial.begin(500000);
  Serial.println("\nBooting Sketch...");

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

    FastLED.addLeds<LED_TYPE, LED_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS); //rm ".setCorrection( TypicalLEDStrip )"
    FastLED.setBrightness(  BRIGHTNESS );
//    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
artnet.begin();
artnet.setArtDmxCallback(onDmxFrame);
timeLastFrame = millis();
}

void loop() {
  // Sketches the application here.
  artnet.read();

  // Invokes mDNS::update and AutoConnect::handleClient() for the menu processing.
  mDNSUpdate(MDNS);
  portal.handleClient();
  //  delay(1);
}
