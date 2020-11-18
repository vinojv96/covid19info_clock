/*ESP8266 MAX7219 CLOCK WITH covid19 cases DISPLAY
 *code created 09/05/2020
 *author: https:github.com/vinojv96
 * 
 */
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
int pinCS = D8; 
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 50; // In milliseconds

int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

//************ Setup *****************
void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);  

//INSERT YOUR SSID AND PASSWORD HERE

  WiFi.begin("Enter SSID","Enter PASSWORD");

  //CHANGE THE POOL WITH YOUR CITY. SEARCH AT https://www.ntppool.org/zone/@
  
configTime(0 * 3600, 0, "in.pool.ntp.org");
  
  setenv("TZ", "IST-5:30",1);  //reference for Timezone: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  
  matrix.setIntensity(7); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 3);    // The first display is position upside down
  matrix.setRotation(1, 3);    // The first display is position upside down
  matrix.setRotation(2, 3);    // The first display is position upside down
  matrix.setRotation(3, 3);    // The first display is position upside down
  matrix.fillScreen(LOW);
  matrix.write();
  
  while ( WiFi.status() != WL_CONNECTED ) {
    matrix.drawChar(2,0, 'W', HIGH,LOW,1); // H
    matrix.drawChar(8,0, 'I', HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,'-', HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,'F', HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,'I', HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    delay(250);
    matrix.fillScreen(LOW);
    matrix.write();
    delay(250);
  }
}

void loop() {
  matrix.setIntensity(6);//setting Brightness
  matrix.fillScreen(LOW);
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  //Serial.println(time);
  time.substring(11,19).toCharArray(time_value, 10); 
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display
       
  delay(30000);
      
  matrix.fillScreen(LOW);
  HTTPClient http;    //Declare object of class HTTPClient
  //GET Data
  String Link = "http://api.thingspeak.com/apps/thinghttp/send_request?api_key=1563LF9UBWI617RB"; //link for http get request
  
  http.begin(Link);     //Specify request destination
  
  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload

  String stringOne = payload;
  String stringTwo = stringOne;
  stringTwo.replace("=>", ":");
  // print the original:
  Serial.println("Original string: " + stringOne);
  // and print the modified string:
  Serial.println("Modified string: " + stringTwo);
 // Serial.println(httpCode);   //Print HTTP return code
 const size_t capacity = JSON_OBJECT_SIZE(11) + 210;
DynamicJsonDocument doc(capacity);
deserializeJson(doc, stringTwo);
const char* active = doc["active"]; // "42576"
const char* confirmed = doc["confirmed"]; // "65084"
const char* deaths = doc["deaths"]; // "2153"
const char* deltaconfirmed = doc["deltaconfirmed"]; // "2219"
const char* deltadeaths = doc["deltadeaths"]; // "52"
const char* deltarecovered = doc["deltarecovered"]; // "1050"
const char* lastupdatedtime = doc["lastupdatedtime"]; // "10/05/2020 21:51:22"
const char* recovered = doc["recovered"]; // "203
serializeJson(doc, Serial);
String a,b,c,d,e,f;
a = active;b=confirmed;c=deaths;
Serial.println(confirmed);
Serial.println(deaths);
Serial.println(deltaconfirmed);
Serial.println(deltadeaths);
Serial.println(deltarecovered);
Serial.println(lastupdatedtime);
Serial.println(recovered);

  
  display_message("covid19 Information > Total Cases: " + b + " Active Now: " + a + " Total Deaths: " + c );
  http.end();  //Close connection
  
}

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait);
  }
}