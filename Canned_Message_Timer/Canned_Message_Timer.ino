#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h> 
#include <WebConfig.h>

int ledState = LOW;
int RELAY = 4;

unsigned long previousMillis = 0;
unsigned long interval = 60000;
unsigned long holdinterval = 1000;
const long aminute = 60000;
const long asecond = 1000;

// GPIO12 LED
// GPIO4 Relay
// GPIO12 Button

#define PINR      12
#define NUMPIXELS 1
Adafruit_NeoPixel pixel(NUMPIXELS, PINR, NEO_GRB + NEO_KHZ800);

String params = "["
  "{"
  "'name':'duration',"
  "'label':'Time Between Message(minutes 1-30)',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':30,"
  "'default':'1'"
  "},"
  "{"
  "'name':'holdtime',"
  "'label':'Realy Hold Time(seconds 1-120)',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':120,"
  "'default':'1'"
  "},"
  "{"
  "'name':'switch',"
  "'label':'Momentary',"
  "'type':"+String(INPUTCHECKBOX)+","
  "'default':'1'"
  "}"
  "]";


ESP8266WebServer server;
WebConfig conf;

boolean initWiFi() {
    boolean connected = false;
    WiFi.mode(WIFI_STA);
    Serial.print("Verbindung zu ");
    Serial.print(conf.values[0]);
    Serial.println(" herstellen");
    if (conf.values[0] != "") {
      WiFi.begin(conf.values[0].c_str(),conf.values[1].c_str());
      uint8_t cnt = 0;
      while ((WiFi.status() != WL_CONNECTED) && (cnt<20)){
        delay(500);
        Serial.print(".");
        cnt++;
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP-Adresse = ");
        Serial.println(WiFi.localIP());
        connected = true;
      }
    }
    if (!connected) {
          WiFi.mode(WIFI_AP);
          WiFi.softAP(conf.getApName(),"",1);  
    }
    return connected;
}

void handleRoot() {
  conf.handleFormRequest(&server);
  if (server.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Configuration ************");
    for (uint8_t i = 0; i<cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
    if (conf.getBool("switch")) Serial.printf("%s %s %i %5.2f \n",
                                conf.getValue("ssid"),
                                conf.getString("continent").c_str(), 
                                conf.getInt("amount"), 
                                conf.getFloat("float"));
  }
}

void setup() {
  Serial.begin(74880);
  Serial.println(params);
  conf.setDescription(params);
  conf.readConfig();
  initWiFi();
  char dns[30];
  sprintf(dns,"%s.local",conf.getApName());
  if (MDNS.begin(dns)) {
    Serial.println("MDNS responder gestartet");
  }
  server.on("/",handleRoot);
  server.begin(80);
  pixel.begin();
  pixel.clear();
  pixel.setBrightness(110);
  pixel.setPixelColor(0, pixel.Color(255, 0, 0)); //GREEN WHILE PLAYING
  pixel.show();
  interval = atoi(conf.getValue("duration"))*aminute;
  holdinterval = atoi(conf.getValue("holdtime"))*asecond;
  Serial.print("length = ");
  Serial.println(interval);
  pinMode(RELAY, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
//   if (ledState == LOW) {
//      ledState = HIGH;  // Note that this switches the LED *off*
      Serial.println("RELAY ON");
      digitalWrite(RELAY, HIGH);
      pixel.clear();
      pixel.setBrightness(110);
      pixel.setPixelColor(0, pixel.Color(0, 255, 0)); //GREEN WHILE PLAYING
      pixel.show();
      delay(holdinterval);
      Serial.println("RELAY OFF");
      digitalWrite(RELAY, LOW);
      pixel.clear();
      pixel.setBrightness(110);
      pixel.setPixelColor(0, pixel.Color(0, 0, 255));//BLUE WHILE STANDBY
      pixel.show();
//      ledState = LOW;
//    } else {
//      ledState = LOW;  // Note that this switches the LED *on*
//      Serial.println("RELAY OFF");
//    }
    digitalWrite(RELAY, ledState);
    
  }
  server.handleClient();
  MDNS.update();
}
