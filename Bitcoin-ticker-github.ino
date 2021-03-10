
/* 
 * Bitcoin ticker - NodeMCU with 8x32 led matrix
 * This version has special effects for big daily swings in price
 * 
 * 2 APIs - two coinbase for curent pricce and historical price
 * NTPClient to fetch date from worldtime
 * Using ArduinoJson 5
 * 
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

// pin definitions
#define DATA_PIN 16    // D0/GPIO16 on NodeMCU
#define CS_PIN 5       // D1/GPIO5
#define CLK_PIN 4      // D2/GPIO4

// customization variables (you will manually have to change the API you want to use to grab price data in another currency than CAD)
#define GMT_OFFSET -6     // in hours

const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";

// Create a new instance of the MD_MAX72XX class:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() 
{
  // Intialize the object for led display:
  myDisplay.begin();
  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setIntensity(0);
  // Clear the display:
  myDisplay.displayClear();
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print("Wi-Fi");
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
  WiFi.begin(ssid, password);
  Serial.println("Connecting...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() 
{
  float currentPriceCAD;
  float currentPriceUSD;
  float yesterdayPriceUSD;
  float dailyPercentChange;
  String priceMessage;
  String priceMessageDollar;
  String percentSign;
  unsigned long myTime;
  String previousUTCdate;
  char* date[] = {"dummy", "values", "here"};
  char* dateANDtime;
  
  if (WiFi.status() == WL_CONNECTED) 
    {
      
    ///////////// Previous date, using central timezone flip over
    // Define NTP Client to get time
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, "pool.ntp.org");
    
    // Initialize a NTPClient to get time
    timeClient.begin();
    timeClient.setTimeOffset(-24*3600 - 6*3600); // one day back (for previous price data) and GMT-6 for central time zone
    
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int monthDay = ptm->tm_mday;
    Serial.print("Month day: ");
    Serial.println(monthDay);

    int currentMonth = ptm->tm_mon+1;
    Serial.print("Month: ");
    Serial.println(currentMonth);
    
    int currentYear = ptm->tm_year+1900;
    Serial.print("Year: ");
    Serial.println(currentYear);

    String currentMonthString = String(currentMonth);
    String monthDayString = String(monthDay); 
    
    if(currentMonth < 10){
      currentMonthString = "0" +  String(currentMonth);
    }
    if(monthDay < 10){
      monthDayString = "0" + String(monthDay);
    }
    previousUTCdate = String(currentYear) + "-" + currentMonthString + "-" + monthDayString;
    Serial.println(previousUTCdate);

    
    ////////////// current CAD and USD price
    HTTPClient http; //Object of class HTTPClient
    http.begin("http://api.coindesk.com/v1/bpi/currentprice/CAD.json"); 
    int httpCode = http.GET();
    if (httpCode > 0) 
    {
      Serial.println(http.getString());
      
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      
      currentPriceCAD = root["bpi"]["CAD"]["rate_float"];
      currentPriceUSD = root["bpi"]["USD"]["rate_float"]; 

      Serial.print("Current bitcoin price in CAD: ");
      Serial.println(currentPriceCAD);
      Serial.print("Current bitcoin price in USD: ");
      Serial.println(currentPriceUSD);
      Serial.println();

      priceMessageDollar = ("$" + String(currentPriceCAD));
      priceMessage = ("%d", int(currentPriceCAD)); //String(currentPriceCAD);

    }
    else{
      Serial.println("not connected to api");
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print("ERROR");
      delay(2500);
    }
    http.end(); //Close connection


    ////////////// Previous day's price in USD
    HTTPClient http2; //Object of class HTTPClient
    http2.begin("http://api.coindesk.com/v1/bpi/historical/close.json"); 
    int httpCode2 = http2.GET();
    if (httpCode2 > 0) 
    {
      Serial.println(http2.getString());
      
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http2.getString());
      
      yesterdayPriceUSD = root["bpi"][String(previousUTCdate)]; 
      dailyPercentChange = float(currentPriceUSD)/float(yesterdayPriceUSD)*100-100;
   }
   else{
      Serial.println("not connected to api");
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print("ERROR");
      delay(2500);
      }
   http2.end(); //Close connection

   }
   
  ////// If WiFi disconnect
  else{
    WiFi.begin(ssid, password);
    Serial.println("Disconnected from network, attempting to reconnect...");
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  }

    

  ////////////////////////// Display Message
  
  myDisplay.setTextAlignment(PA_LEFT);
  myDisplay.print("BTC->$"); //"BTC   $"
  delay(2500);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(priceMessage);
  delay(2500);


  /////////////////////// Percent logic
  if(dailyPercentChange>0){
    percentSign = String("+");
  }
  else{
    percentSign = "";
  }
  myDisplay.setTextAlignment(PA_CENTER);
  String percentMessage;
  percentMessage = String(percentSign + dailyPercentChange + "%");
  

  if( dailyPercentChange > 10 ){
    for(int x = 0; x < 3; x++){
      myDisplay.print("MOOON");
      delay(500);
      myDisplay.print("SHOT");
      delay(500);
    }
  }
  else if( dailyPercentChange > 7 ){
    myDisplay.print(percentMessage);
    delay(2500);
    for(int x = 0; x < 1; x++){
      myDisplay.print("STACKS");
      delay(500);
      myDisplay.print("ON");
      delay(500);
      myDisplay.print("STACKS");
      delay(500);
      myDisplay.print("ON");
      delay(500);
      myDisplay.print("STACKS");
      delay(500);
  }
  }
  else if( dailyPercentChange > 5 ){
    myDisplay.print(percentMessage);
    delay(2500);
    for(int x = 0; x < 1; x++){
      myDisplay.print("blast");
      delay(750);
      myDisplay.print("off in");
      delay(750);
      myDisplay.setTextAlignment(PA_LEFT);
      myDisplay.print("  T-5");
      delay(500);
      myDisplay.print("  T-4");
      delay(500);
      myDisplay.print("  T-3");
      delay(500);
      myDisplay.print("  T-2");
      delay(500);
      myDisplay.print("  T-1");
      delay(500);
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print("TO THE");
      delay(1000);
      myDisplay.print("MOON!!");
      delay(1000);
    }
  }
  else if( dailyPercentChange > 2 ){
    myDisplay.print(percentMessage);
    delay(2500);
    for(int x = 0; x < 3; x++){
      myDisplay.print("BTC GO");
      delay(500);
      myDisplay.print("BRRRR");
      delay(500);
    }
  }
  else if( dailyPercentChange < -10 ){
    for(int x = 0; x < 4; x++){
      myDisplay.print("HODL!");
      delay(500);
      myDisplay.print("ape");
      delay(500);
      myDisplay.print("strong");
      delay(500);
    }
  }
  else if( dailyPercentChange < -7){
    myDisplay.print(percentMessage);
    delay(2500);
    for(int x = 0; x < 3; x++){
      myDisplay.print("brace");
      delay(500);
      myDisplay.print("for");
      delay(500);
      myDisplay.print("impact");
      delay(500);
    }
  }
  else if( dailyPercentChange < -3 ){
    myDisplay.print(percentMessage);
    delay(2500);
    for(int x = 0; x < 3; x++){
      myDisplay.print("buy");
      delay(500);
      myDisplay.print("the dip");
      delay(500);
    }
  }
  else{
    myDisplay.print(percentMessage);
    delay(2500);
  }
    
}
