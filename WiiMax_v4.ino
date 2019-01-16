/*

------------- ABM LIMITED ---------------
WIIMAX -- A wirelss wontrolled LED dot matrix display.
Project: WiiMax DMD V4
Built & Designed by A.Kimani & Mike Ngash


Changelog fron v3.2
 - Added mDNS service.
 - Save text to a file in spiffs instead of EEPROM.
 - 

 */
 
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <ESP8266mDNS.h>

IPAddress    apIP(192, 168, 254, 1);  // Defining a static IP address: local & gateway

/* This are the WiFi access point settings. Update them to your likin */
const char *ssid = "WiiMax v.4 dv1 by UNIVERSAL";
const char *password = "wiimax.v4dv1";

ESP8266WebServer server(80); //Create object for webserver
MDNSResponder mdns; //Create object for mDNS

String webPage = ""; //Variable for writing webpage

String decodedMsg ; // Variable for msg from web server


/* Declare the shift register pins */
int latchPin2 = D8; // Latch pin for scanning shift register
int clockPin2 = D4; // Clock pin for scanning shift register
int dataPin2 = D0;  // Data pin for scanning shift register
int latchPin1 = D8; // Latch pin for data shift registers
int clockPin1 = D5; // Clock pin for data shift registers
int dataPin1 = D7; // Data pin for data shift registers
 
byte bitmap[8][10]; 
int numZones = sizeof(bitmap) / 8;
int maxZoneIndex = numZones-1;
int numCols = numZones * 8;
int speed=15;

/* Font */
byte alphabets[][8] = {
 { 0,0,0,0,0},        // 32 - 'Space'
  { 95},          // 33 - '!'
  { 7, 0, 7},        // 34 - '"'
  { 20, 127, 20, 127, 20},  // 35 - '#'
   {36, 42, 127, 42, 18},  // 36 - '$'
  { 35, 19, 8, 100, 98},   // 37 - '%'
  { 54, 73, 85, 34, 80},   // 38 - '&'
  { 8, 7, 3},        // 39 - '''
  { 28, 34, 65},        // 40 - '('
  { 65, 34, 28},       // 41 - ')'
  { 42, 28, 127, 28, 42},   // 42 - '*'
  { 8, 8, 62, 8, 8},      // 43 - '+'
  { 128, 112, 48},     // 44 - ','
  { 8, 8, 8, 8, 8},    // 45 - '-'
  { 192, 192},         // 46 - '.'
  { 32, 16, 8, 4, 2},     // 47 - '/'
  { 62, 81, 73, 69, 62},    // 48 - '0'
  { 66, 127, 64},       // 49 - '1'
  { 114, 73, 73, 73, 70},   // 50 - '2'
  { 33, 65, 73, 77, 51},   // 51 - '3'
  { 24, 20, 18, 127, 16},   // 52 - '4'
  { 39, 69, 69, 69, 57},    // 53 - '5'
  { 60, 74, 73, 73, 49},    // 54 - '6'
  { 65, 33, 17, 9, 7},    // 55 - '7'
  { 54, 73, 73, 73, 54},    // 56 - '8'
  { 70, 73, 73, 41, 30},    // 57 - '9'
  { 20},            // 58 - ':'
  { 128, 104},        // 59 - ';'
  { 8, 20, 34, 65},     // 60 - '<'
  { 20, 20, 20, 20, 20},    // 61 - '='
  { 65, 34, 20, 8},     // 62 - '>'
  { 2, 1, 89, 9, 6},      // 63 - '?'
  { 62, 65, 93, 89, 78},    // 64 - '@'
  { 0,252, 254, 51, 51, 254, 252,0},  // 65 - 'A'
  { 0,255, 255, 219, 219, 255, 102,0}, // 66 - 'B'
  { 0,126, 255, 195, 195, 195, 195,0}, // 67 - 'C'
  { 0,255, 255, 195, 195, 126, 60,0}, // 68 - 'D'
  { 0,255, 255, 219, 219, 195, 195,0}, // 69 - 'E'
  { 0,255, 255, 27, 27, 27, 3,0}, // 70 - 'F'
  { 0,126, 255, 195, 211, 243, 243,0}, // 71 - 'G'
  { 0,255, 255, 24, 24, 255, 255,0},  // 72 - 'H'
  { 0,129, 129, 255, 255, 129, 129,0}, // 73 - 'I'
  { 0,195, 195, 195, 255, 127, 3,0},    // 74 - 'J'
  { 0,255, 255, 24, 60, 231, 195,0},    // 75 - 'K'
  { 0,255, 255, 192, 192, 192, 192,0},    // 76 - 'L'
  { 255, 255, 6,12, 6, 255, 255},   // 77 - 'M'
  { 0,255, 255, 12, 24, 255, 255,0},    // 78 - 'N'
  { 0,126, 255, 195, 195, 255, 126,0},    // 79 - 'O'
  { 0,255, 255, 27, 27, 27, 14,0},      // 80 - 'P'
  { 126, 255, 195, 195, 255, 126, 128},   // 81 - 'Q'
  { 0,255, 255, 27, 27, 255, 238,0},  // 82 - 'R'
  { 0,206, 223, 219, 219, 251, 115,0},  // 83 - 'S'
  { 0,3, 3, 255, 255, 3, 3,0},      // 84 - 'T'
  { 0,127, 255, 192, 192, 255, 127,0},   // 85 - 'U'
  { 0,63, 127, 224, 224, 127, 63,0},    // 86 - 'V'
  { 255, 255, 96, 48, 96, 255, 255},    // 87 - 'W'
  { 195, 231, 60, 24, 60, 231, 195},    // 88 - 'X'
  { 0,7, 15, 240, 240, 15, 7,0},     // 89 - 'Y'
  { 0,195, 227, 243, 219, 207, 199,0},    // 90 - 'Z'
  { 127, 65, 65},       // 91 - '['
  { 2, 4, 8, 16, 32},     // 92 - '\'
  { 65, 65, 127},       // 93 - ']'
  { 4, 2, 1, 2, 4},     // 94 - '^'
  { 64, 64, 64, 64, 64},    // 95 - '_'
  { 3, 7, 8},        // 96 - '`'
  { 32, 84, 84, 120, 64},  // 97 - 'a'
  { 127, 40, 68, 68, 56},   // 98 - 'b'
  { 56, 68, 68, 68, 40},    // 99 - 'c'
  { 56, 68, 68, 40, 127},   // 100 - 'd'
  { 56, 84, 84, 84, 24},    // 101 - 'e'
  { 8, 126, 9, 2},      // 102 - 'f'
  { 24, 164, 164, 156, 120},  // 103 - 'g'
  { 127, 8, 4, 4, 120},   // 104 - 'h'
  { 68, 125, 64},       // 105 - 'i'
  { 64, 128, 128, 122},   // 106 - 'j'
  { 127, 16, 40, 68},     // 107 - 'k'
  { 65, 127, 64},       // 108 - 'l'
  { 124, 4, 120, 4, 120},   // 109 - 'm'
  { 124, 8, 4, 4, 120},   // 110 - 'n'
  { 56, 68, 68, 68, 56},    // 111 - 'o'
  { 252, 24, 36, 36, 24},   // 112 - 'p'
  { 24, 36, 36, 24, 252},   // 113 - 'q'
  { 124, 8, 4, 4, 8},     // 114 - 'r'
  { 72, 84, 84, 84, 36},    // 115 - 's'
  { 4, 63, 68, 36},     // 116 - 't'
  { 60, 64, 64, 32, 124},   // 117 - 'u'
  { 28, 32, 64, 32, 28},    // 118 - 'v'
  { 60, 64, 48, 64, 60},    // 119 - 'w'
  { 68, 40, 16, 40, 68},    // 120 - 'x'
  { 76, 144, 144, 144, 124 }, // 121 - 'y'
  { 68, 100, 84, 76, 68},   // 122 - 'z'
  { 8, 54, 65},       // 123 - '{'
  { 119},           // 124 - '|'
  { 65, 54, 8},       // 125 - '}'
  { 2, 1, 2, 4, 2},     // 126 - '~'
};

long counter = -1;
int drag = 0;

void setup() {
  /* Set up the website */
  webPage += "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>WiiMax&reg;: Message Control</title>";
  webPage += "<style>body {-webkit-perspective: 800px;        perspective: 800px;height: 100vh;margin: 0;overflow: hidden;font-family: 'Gudea', sans-serif;background: #4b80c2;}body p {color: white;font-size: 12px;text-align: center;}body .login {opacity: 1;top: 10px;width: 240px;border-top: 2px solid #996515;height: 300px;position: absolute;left: 0;right: 0;margin: auto;bottom: 0;padding: 100px 40px 40px 40px;background: #35394a;}body .login .disclaimer {position: absolute;bottom: 20px;left: 35px;width: 250px;margin-top: -100px;}body .login_title {color: #afb1be;height: 100px;margin-top: -50px;text-align: center;font-size: 16px;}body .login_fields {height: 208px;position: absolute;left: 0;}body .button  {border-radius: 50px;width: relative;background: transparent;margin: 10px;padding: 10px 50px;border: 2px solid #996515;color: white;text-transform: uppercase;font-size: 14px;-webkit-transition-property: background,color;        transition-property: background,color;-webkit-transition-duration: .2s;        transition-duration: .2s;}body .button:focus {box-shadow: none;outline: none;}body .button:hover {color: white;background: #996515;cursor: pointer;-webkit-transition-property: background,color;        transition-property: background,color;-webkit-transition-duration: .2s;        transition-duration: .2s;}form {padding: 10px 0 0 30px;position: relative;z-index: 2;}form input {-webkit-appearance: none;    -moz-appearance: none;        appearance: none;outline: 0;border: 1px solid rgba(255, 255, 255, 0.4);background-color: rgba(255, 255, 255, 0.2);width: relative;border-radius: 3px;padding: 10px 15px;margin: 0 auto 10px auto;display: block;text-align: center;font-size: 18px;color: white;-webkit-transition-duration: 0.25s;        transition-duration: 0.25s;font-weight: 300;}form input:hover {background-color: rgba(255, 255, 255, 0.4);}form input:focus {background-color: white;width: relative;color: #53e3a6;}.love {position: absolute;right: 20px;bottom: 0px;font-size: 11px;font-weight: normal;color: white;}";
  webPage +="@media only screen and (max-width: 500px) {body p {color: white;font-size: 12px;width: auto;text-align: center;}body .login {opacity: 1;top: 50px;position: relative;width: auto;border-top: 2px solid #996515;height: 200px;margin: 10px;padding: 100px 40px 40px 40px;background: #35394a;}body .login .disclaimer {padding: 40px 0 0 0;margin: 100px 10px 0 0;position: absolute;top: 100px;width: auto;}body .login_title {color: #afb1be;height: 100px;margin-top: -100px;text-align: center;font-size: 16px;}body .login_fields {height: auto;position: relative;text-align: center;}body .button  {border-radius: 50px;width: relative;background: transparent;margin: 10px;padding: 10px 50px;border: 2px solid #996515;color: white;text-transform: uppercase;font-size: 14px;-webkit-transition-property: background,color;    transition-property: background,color;-webkit-transition-duration: .2s;    transition-duration: .2s;}body .button:focus {box-shadow: none;outline: none;}body .button:hover {color: white;background: #996515;cursor: pointer;-webkit-transition-property: background,color;    transition-property: background,color;-webkit-transition-duration: .2s;    transition-duration: .2s;}form {padding: 10px 0 0 10px;position: relative;z-index: 2;}form input {-webkit-appearance: none;    -moz-appearance: none;        appearance: none;outline: 0;border: 1px solid rgba(255, 255, 255, 0.4);background-color: rgba(255, 255, 255, 0.2);width: auto;border-radius: 3px;padding: 10px 15px;margin: auto;display: block;text-align: center;font-size: 18px;color: white;-webkit-transition-duration: 0.25s;        transition-duration: 0.25s;font-weight: 300;}form input:hover {background-color: rgba(255, 255, 255, 0.4);}form input:focus {background-color: white;width: relative;color: #53e3a6;}.love {position: absolute;right: 20px;bottom: 0px;font-size: 11px;font-weight: normal;color: white;}}</style></head>";
  webPage +="<body><div class='login'><div class='login_title'><h2>WiiMax&reg;</h2><span>Change you message here:</span></div>";
  webPage +="<div class='login_fields'><form class='form' id='data_form' action='msg'><input type='text' name='msg' placeholder='Type your text here:'><button class='button' type='submit'>Send to Display</button></form></div>";
  webPage +="<div class='disclaimer'><p>WiiMax&reg; is a wireless controlled scroll dot matrix display. This product was built by ABM Corporation. </p><p> For more info or assistance call: +254736588558.</p></div></div>";
  webPage +="<div class='love'><p>&copy; ABM Corporation. All rights reserved.</p></div></body></html>";

  start_wifi(); //Start Wifi AP.

  /* Initialize pins */
  pinMode(latchPin1, OUTPUT);
  pinMode(clockPin1, OUTPUT);
  pinMode(dataPin1, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  pinMode(clockPin2, OUTPUT);
  pinMode(dataPin2, OUTPUT);
  
  Clear(); //clear gabage from display.

  EEPROM.begin(1024); //Start EEPROM
  SPIFFS.begin(); //Start Spiffs
}

void loop() {
  
  server.handleClient(); // Handle any client connections.

  start_wifi();
  AlphabetSoup();
  RefreshDisplay(); // Call refresh as often as you can.
}

void start_wifi(){
  /* Setup Wifi in Access Point Mode */
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  
  /* Start and Configure mDNS name */
  mdns.begin("wiimax",WiFi.softAPIP());
  mdns.begin("www.wiimax",WiFi.softAPIP());

  /* Set up pages and respose for the web server */
  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });
  server.on("/msg", [](){
    handle_msg();
    server.send(200, "text/html", webPage);
  });
  server.begin();
   
  /* Start mDNS service */
  mdns.addService("http", "tcp", 80);
 }

 void handle_msg() {
  /* 
    Function that handles incomming message from server.
    The fuction accepts incomming message, formats it then saves to eeprom.
  */
 
  String msgs = server.arg("msg"); //Capture the message from webpage
  decodedMsg = msgs;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");      
  decodedMsg.replace("%21", "!");  
  decodedMsg.replace("%22", "");  
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");  
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");  
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");  
  decodedMsg.replace("%2C", ",");  
  decodedMsg.replace("%2F", "/");   
  decodedMsg.replace("%3A", ":");    
  decodedMsg.replace("%3B", ";");  
  decodedMsg.replace("%3C", "<");  
  decodedMsg.replace("%3D", "=");  
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");  
  decodedMsg.replace("%40", "@");
   
  /* Open file for editing */
  File msgF = SPIFFS.open("/msg.txt", "w");
  msgF.print(decodedMsg);
  msgF.close();
}

void Clear() {
  for (int row=0; row<8; row++) {
    for (int zone=0; zone<numZones; zone++) {
      bitmap[row][zone] = 0;
      yield();
    } 
    yield();
  }
}

void RefreshDisplay()
{
  for (int row = 0; row < 8; row++) {
    int rowbit = 1 << row;
    digitalWrite(latchPin2, LOW);
    shiftOut(dataPin2, clockPin2, LSBFIRST, rowbit);
    digitalWrite(latchPin1, LOW);
    for (int zone = maxZoneIndex; zone >= 0; zone--) {
      shiftOut(dataPin1, clockPin1, MSBFIRST, bitmap[row][zone]);
      yield();
    }
    digitalWrite(latchPin1, HIGH);
    digitalWrite(latchPin2, HIGH);
    yield();
  }
}

// Converts row and colum to actual bitmap bit and turn it off/on
void Plot(int col, int row, bool isOn)
{
  int zone = col / 8;
  int colBitIndex = col % 8;
  byte colBit = 1 << colBitIndex;
  if (zone<numZones) {
    if (isOn)
      bitmap[row][zone] =  bitmap[row][zone] | colBit;
    else
      bitmap[row][zone] =  bitmap[row][zone] & (~colBit);
  }
}

// Plot each character of the message one column at a time, updated the display, shift bitmap left.
void AlphabetSoup()
{
  File msgsF = SPIFFS.open("/msg.txt", "r");
  String spiffMsg = msgsF.readStringUntil('\n');

  int msg_size = spiffMsg.length();
  char msg[msg_size];

  memset(msg, 0, strlen(msg));
  for(int i = 0;i < spiffMsg.length();i++)
  msg[i] = spiffMsg[i];
    
  for (int charIndex=0; charIndex < (sizeof(msg)); charIndex++)
  {
    int alphabetIndex = msg[charIndex] - ' ';
    if (alphabetIndex < 0) alphabetIndex=0;
    for (int col = 0; col < 8; col++)
    {
      for (int row = 0; row < 8; row++)
      {
        bool isOn = 0; 
        if (col<36) isOn = bitRead( alphabets[alphabetIndex][col], 7-row ) == 1;
        Plot( numCols-1, row, isOn); // We ALWAYS draw on the rightmost column, the shift loop below will scroll it leftward.
        yield();
      }
      for (int refreshCount=0; refreshCount < speed; refreshCount++){
        RefreshDisplay();
        yield();}

      //-- Shift the bitmap one column to left --
      for (int row=0; row<8; row++)
      {
        for (int zone=0; zone < numZones; zone++)
        {
          // This right shift would show as a left scroll on display because leftmost column is represented by least significant bit of the byte.
          bitmap[row][zone] = bitmap[row][zone] >> 1;
          
          // Roll over lowest bit from the next zone as highest bit of this zone.
          if (zone < maxZoneIndex) bitWrite(bitmap[row][zone], 7, bitRead(bitmap[row][zone+1],0));
          yield();
        }
        yield();
      }
      yield();
    }
    yield();
  }
}

void Print(int charOffset, String msg)
{
  for (int charIndex=0; charIndex < msg.length(); charIndex++)
  {
    int alphabetIndex = msg[charIndex] - ' ';
    if (alphabetIndex < 0) alphabetIndex=0;
    for (int col = 0; col < 8; col++)
    {
      for (int row = 0; row < 8; row++)
      {
        bool isOn = 0; 
        if (col<36) isOn = bitRead( alphabets[alphabetIndex][col], 7-row ) == 1;
        Plot( (charOffset+charIndex)*8 + col, row, isOn); // We ALWAYS draw on the rightmost column, the shift loop below will scroll it leftward.
        yield();
      }
      yield();
    }
    yield();
  }
      
}
