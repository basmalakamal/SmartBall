#include "Ubidots.h"   
#include <SFE_BMP180.h>
#include <Wire.h>
SFE_BMP180 pressure;
#define ALTITUDE 1655.0
const int microphonePin = A0;
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WifiLocation.h>
#include "time.h"

const char* googleApiKey = "AIzaSyC4mFCfdvfZK5AvARqbReam_deYzJNZc4Y";
const char* UBIDOTS_TOKEN = "BBFF-75PGecjVLJRStZcoe9JU8BaNOLcQJr";  
const char* WIFI_SSID = "Murakami";      // Put here your Wi-Fi SSID
const char* WIFI_PASS = "F=Gm1m2/r2";      // Put here your Wi-Fi password 
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

WifiLocation location (googleApiKey);

//for calculate time and date
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;   
const int   daylightOffset_sec = 0;  

void set_Clock (){
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  Serial.println(asctime(timeinfo));
  delay(1000);
  }


void setup() {                       
  
  Serial.begin(115200);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  // ubidots.setDebug(true);  
  WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
     Serial.print ("Connected to ");
     Serial.print (WIFI_SSID);
     Serial.println (" WIFI");

    //init and get the time
     configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
     
  Serial.println("REBOOT");
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); 
  }                  
}

void loop() {
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  double T,P,p0,a;
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  char status;
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          p0 = pressure.sealevel(P,ALTITUDE);
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  
  int mn = 1024;
  int mx = 0;

  for (int i = 0; i < 10000; ++i) {

    int val = analogRead(microphonePin);
    
    mn = min(mn, val);
    mx = max(mx, val);
  }

  int delta = mx - mn;

  Serial.print("Min=");
  Serial.print(mn);
  Serial.print(" Max=");
  Serial.print(mx);
  Serial.print(" Delta=");
  Serial.println(delta);

  Serial.print ("time and date : ");
    set_Clock ();
    location_t loc = location.getGeoFromWiFi();
    Serial.println("Location request data");
    Serial.println(location.getSurroundingWiFiJson()+"\n");
    Serial.println ("Latitude: " + String (loc.lat, 15));
    Serial.println ("Longitude: " + String(loc.lon, 15));
    Serial.println ("Accuracy: " + String (loc.accuracy));
    Serial.println ("Result: " + location.wlStatusStr (location.getStatus ()));
  
  ubidots.add("temperature", T);
  ubidots.add("pressure", P);
  ubidots.add("sound", mx);
  ubidots.add("deltasound", delta);
  ubidots.add("Latitude", loc.lat);
  ubidots.add("Longitude", loc.lon);
  ubidots.add("Accuracy", loc.accuracy);
  //ubidots.add("location", value4);
  
  
  bool bufferSent = false;
  bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

  if (bufferSent) {
  // Do something if values were sent properly
   Serial.println("Values sent by the device");
  }
  
  delay(100);
}
