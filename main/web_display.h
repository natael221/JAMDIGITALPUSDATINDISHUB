#ifndef WEB_DISPLAY_H
#define WEB_DISPLAY_H

#include <WiFi.h>
#include <WebServer.h>
#include <time.h>// Gunakan AsyncWebServer jika diperlukan
#include <Wire.h>
#include <RTClib.h>
#include "data.h"

// Deklarasi variabel yang ada di main.ino
extern const char* sta_ssid;
extern const char* sta_password;
extern const char* ap_ssid;
extern const char* ap_password;

extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;

extern WebServer server;

extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

#define MAX_SENTENCES 10
extern String sentences[MAX_SENTENCES];
extern int sentenceCount;

extern String prayerTimes[5];
// extern String previousPrayerTimes[5];
extern String prayerNames[5];

extern String previousSSID;
extern String subuhTime, dzuhurTime, ashrTime, maghribTime, isyaTime;

extern int currentVolume;

// Deklarasi fungsi
void updatePrayerTimesArray();
bool is_authenticated();
String getPage(String content);
void handleInfo();
void handleHome();
void handleTampilan1();
void handleEditPrayerTimes();
void handleSavePrayerTimes();
void handleEditTime();
void handleEditDate();
void handleSaveTime();
void handleSaveDate();
void handleTampilan2();
void handleSaveSentence();
void handleEditSentence();
void handleUpdateSentence();
void handleRemoveSentence();
void handleSetVolume();
void handleserver();

#endif
