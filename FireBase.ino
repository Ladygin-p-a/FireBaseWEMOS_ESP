/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "MoltiWiFi"
#define WIFI_PASSWORD "Point-7142462"

// Insert Firebase project API Key
#define API_KEY "AIzaSyARgv99kxHkM2jDQwaL-2v5SGkWa8NVssA"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://klimatkameradata-default-rtdb.europe-west1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(10800);

}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 60000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    timeClient.update();

    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
  
    time_t epochTime = timeClient.getEpochTime();
    
    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime);

    int monthDay = ptm->tm_mday;
    /*Serial.print("Month day: ");
    Serial.println(monthDay);*/

    int currentMonth = ptm->tm_mon + 1;
    /*Serial.print("Month: ");
    Serial.println(currentMonth);*/

    int currentYear = ptm->tm_year + 1900;
    /*Serial.print("Year: ");
    Serial.println(currentYear);*/


    String dataPath_T(monthDay);
    dataPath_T += "-";
    dataPath_T += currentMonth;
    dataPath_T += "-";
    dataPath_T += currentYear;
    dataPath_T += "/";
    dataPath_T += currentHour;
    dataPath_T += "-";
    dataPath_T += currentMinute;
    dataPath_T += "/T";

    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setFloat(&fbdo, dataPath_T.c_str(), 0.01 + random(0, 100))) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;

    String dataPath_H(monthDay);
    dataPath_H += "-";
    dataPath_H += currentMonth;
    dataPath_H += "-";
    dataPath_H += currentYear;
    dataPath_H += "/";
    dataPath_H += currentHour;
    dataPath_H += "-";
    dataPath_H += currentMinute;    
    dataPath_H += "/H";

    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, dataPath_H.c_str(), 0.01 + random(0, 100))) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
