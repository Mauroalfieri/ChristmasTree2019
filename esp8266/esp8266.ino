#include <FastLED.h>

FASTLED_USING_NAMESPACE

#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h> // NEW
#include <FS.h>

/****************************************************************************************/
const byte DNS_PORT = 53;   // NEW 
DNSServer dnsServer;        // NEW

ESP8266WebServer server(80);

IPAddress apIP(192, 168, 4, 1);       // NEW
IPAddress netMsk(255, 255, 255, 0);   // NEW

WiFiClient espClient;

/****************************************************************************************/

const char* wifi_ssid     = "christmas";
const char* wifi_password = "2019";
unsigned char red,green,blue;

/****************************************************************************************/

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    40
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96

/****************************************************************************************/

void handle_root() { 
  Serial.println("Handle Root"); 

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String html = "<!DOCTYPE html> <html lang=\"en\"> <head> <title>Color Picker</title> <meta charset=\"UTF-8\"/> <link href=\"common.css\" rel=\"stylesheet\" /> <script src=\"colorpicker.js\"></script> <style>.inputs-list {margin-top: 34px\; \n }</style> </head>\n";
  html += "<body> <div class=\"example-wrap\"> <div class=\"inputs-list\"> ";
  html += "<input type=\"hidden\" id=\"blackCode\" value=\"#000000\" /> ";
  html += "<input type=\"hidden\" id=\"oldHexCode\" value=\"\" /> ";
  html += "<input type=\"button\" onclick=\"sendData(getElementById('blackCode'))\" onchange=\"\" class=\"io-input\" style=\"background-color:#000\; border: 2px solid #fff\; color: #fff\; \" value=\"0\" /> ";
  html += "<input type=\"button\" onclick=\"sendData(getElementById('oldHexCode'))\" onchange=\"\" class=\"io-input\" style=\"background-color:#fff\; border: 2px solid #fff\; color: #000\; \" value=\"1\" /> ";
  html += "</div> <canvas id=\"canvas\"></canvas> <div class=\"inputs-list\"> ";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\" class=\"multi-input input-quad\" value=\"#8825eb\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#2439eb\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#24e9eb\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#eb9524\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#eb2497\" />";
  html += "</div> <div class=\"inputs-list\">";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#ffffff\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#ffe83f\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#ff0000\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#00ff00\" />";
  html += "<input  type=\"button\" onclick=\"sendData(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#0000ff\" />";
  html += "</div> </div> \n";
  html += "<script> var picker = new KellyColorPicker({ place : 'canvas',    size : 700, userEvents : { change : function(self) { if (!self.selectedInput) return\; \n  if (self.getCurColorHsv().v < 0.5) { self.selectedInput.style.color = \"#FFFFFF\"\; \n  } else { self.selectedInput.style.color = \"#000000\"\; \n  } self.selectedInput.value = self.getCurColorHex()\; \n     self.selectedInput.style.background = self.selectedInput.value\; \n    } } })\; \n  picker.editInput = function(target) { if (picker.selectedInput) picker.selectedInput.classList.remove('selected')\; \n    if (target) picker.selectedInput = target\; \n  if (!picker.selectedInput) return false\; \n  picker.selectedInput.classList.add('selected')\; \n     picker.setColor(picker.selectedInput.value)\; \n  picker.selectedInput.style.color=picker.selectedInput.value\; \n  } \n";
  html += "var mInputs = document.getElementsByClassName('multi-input')\; \n  for (var i = 0\; \n  i < mInputs.length\; \n  i++) { picker.editInput(mInputs[i])\; \n  } \n";
  html += "picker.getWheel().width += 60\; \n  picker.getSvFigCursor().radius += 15\; \n  picker.getWheelCursor().height += 15\; \n  var alpha = picker.getAlphaFig()\; \n  picker.updateView(true)\; \n  \n";
  html += "</script> </body> </html>";
 
  server.send(200, "text/html", html); 
  delay(100);
}


void handle_css()  { 
  Serial.println("Handle Css"); 
  String css    = "body { background : #000; font-family: Calibri; padding-top: 48px; text-align : center; } input { border: 0px; height: 32px; text-align: center;}.example-wrap { background: #000; width: 500px; height: 900px; margin: 0 auto; -webkit-box-shadow: 1px 2px 10px 0px rgba(0,0,0,0.75); -moz-box-shadow: 1px 2px 10px 0px rgba(0,0,0,0.75); box-shadow: 1px 2px 6px 0px rgba(0,0,0,0.4);}.example-wrap-simple { width : 98%; height : 98%; padding-bottom : 64px;}a { text-decoration : none;}.input-quad { width : 64px; height : 64px; display : inline-block;}.examples-list { max-width : 640px; margin : 0 auto; text-align : left; padding : 24px; box-sizing: border-box;}.examples-list ul { text-align : left; margin : 0px; padding : 0px;}.examples-list li { list-style-type: none; padding: 6px; padding-left : 0px; }.examples-list li a { font-size: 16px; color: #6d5ad6; font-weight: bold;}.multi-input { border: 2px ddsolid rgba(70, 70, 70, 0.18); box-sizing: border-box; width: 90px; margin-right: 5px; border-radius: 6px; text-align : center; color: #fff;}.multi-input.selected { border: 2px solid #000;}.io-input { box-sizing: border-box; width: 190px; height: 55px; margin: 40px 10px; border-radius: 6px; text-align : center; font-weight: bold; font-size: 30px;}";

  server.send(200, "text/html", css);
  delay(100);
}

void handle_outputs() {
  Serial.print("Handle output");
  
  // Strings to strore the client output
  String RMsg;
  String GMsg;
  String BMsg;

  // Parse client output
  RMsg=server.arg("r");
  GMsg=server.arg("g"); 
  BMsg=server.arg("b");

  Serial.print(" RMsg: "); Serial.print(RMsg);
  Serial.print(" GMsg: "); Serial.print(GMsg);
  Serial.print(" BMsg: "); Serial.print(BMsg);
  Serial.print("\n"); 
  
  // Convert to number to pass to Neopixel library
  red=RMsg.toInt();
  green=GMsg.toInt(); 
  blue=BMsg.toInt();   

  String result = "<!DOCTYPE html> <html lang=\"en\"> <head> <title>Color Picker</title> </head> <body>message</body> </html>";
  server.send(200, "text/html", result);
}

void getSpiffFile(String path, String TyPe) { 
 if(SPIFFS.exists(path)){ 
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, TyPe);
    file.close();
  }
}

// Initialize WiFi, web server and handles
void setup() {
  Serial.begin(115200);
  Serial.println("Setup start execution");
  
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAPConfig(apIP, apIP, netMsk);  // NEW
  WiFi.softAP(wifi_ssid, wifi_password);  //Start HOTspot removing password will disable security
  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);     // NEW
  dnsServer.start(DNS_PORT, "*", apIP);                   // NEW
  
  red=127;
  green=127;
  blue=127;
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
    
  server.on("/", handle_root);
  server.on("/common.css", []() { getSpiffFile("/common.css", "text/css"); } );
  server.on("/colorpicker.js", []() { getSpiffFile("/colorpicker.js", "application/javascript"); } );
  server.on("/setcolor.html", handle_outputs);
  //server.onNotFound( handle_root );
  
  server.begin();
  SPIFFS.begin();

  setColorPixel();

  Serial.println("Setup successfully executed");
}

void loop() { 
  server.handleClient();
  dnsServer.processNextRequest(); 
  setColorPixel();
  FastLED.show();  
}

void setColorPixel() {

  Serial.print(" R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.print(blue);
  Serial.print("\n"); 
  
  for(int i=0;i<NUM_LEDS;i++){ 
    leds[i] = CRGB( red, green, blue);
    FastLED.show();  
    FastLED.delay(1000/30);
  }
}
