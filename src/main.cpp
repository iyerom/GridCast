#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <mbedtls/base64.h>
#include "config.h"


MD_MAX72XX md = MD_MAX72XX(MD_MAX72XX::FC16_HW, 25, 21, 18, 2); // DATA, CLK, CS, N

/*
#define WIFI_SSID
#define WIFI_PASSWORD 
#define Web_API_KEY 
#define DATABASE_URL
#define USER_EMAIL 
#define USER_PASS
*/

void processData(AsyncResult &aResult);
byte reverseBits(byte b);
int decodeBase64(const char* input, uint8_t* output, size_t outLen);
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

FirebaseApp app;
WiFiClientSecure sslClient;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(sslClient);
RealtimeDatabase db;


uint8_t frame[16] = {0};
String frameStr = "";
bool dirty = false;

void setup() {
  Serial.begin(115200);
  md.begin();
  md.control(MD_MAX72XX::INTENSITY, 5);
  md.clear();

  //wifi
  int bootX = 0, bootY = 0, trig = 1;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    md.setPoint(bootX, bootY, trig);
    bootX+=2;
    if(bootX > 15) {
      bootX = 0;
      bootY+=2;
      if(bootY > 7) {
        bootY = 0;
        trig++;
        if(trig > 1)
          trig = 0;
      }
    }
    
  }
  md.clear();
  Serial.println();
  sslClient.setInsecure();
  sslClient.setTimeout(1000);
  sslClient.setHandshakeTimeout(5);

  initializeApp(aClient, app, getAuth(user_auth), processData, "omb | authTask");
  app.getApp<RealtimeDatabase>(db);
  db.url(DATABASE_URL);
  md.control(MD_MAX72XX::INTENSITY, db.get<int>(aClient, "/intensity"));
  for (int i = 0; i < 16; i++) {
    frame[i] = db.get<int>(aClient, "/frame/r" + String(i));
    md.setColumn(15-i, reverseBits(frame[i]));
  }
  db.get(aClient, "/frame", processData, true, "frameStream");


}

void loop() {
  app.loop();
  if(!dirty)
    return;
  for (int i = 0; i < 16; i++) 
    md.setColumn(15-i, reverseBits(frame[i]));
  dirty = false;
}

void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (!aResult.available())
    return;
  
  Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  RealtimeDatabaseResult &rtdb = aResult.to<RealtimeDatabaseResult>();
  if(!rtdb.isStream())
    return;
  String path = rtdb.dataPath();
  Serial.println("path: " + path);
  Serial.println("value: " + rtdb.to<String>());
  if(path.equals("/")) {
    String base64 = rtdb.to<String>();
    decodeBase64(base64.c_str(), frame, sizeof(frame));
    dirty = true;
  }
}

byte reverseBits(byte b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

int decodeBase64(const char* input, uint8_t* output, size_t outLen) {
  size_t decodedLen = 0;

  int res = mbedtls_base64_decode(
    output,
    outLen,
    &decodedLen,
    (const unsigned char*)input,
    strlen(input)
  );

  return (res == 0) ? decodedLen : -1;
}
