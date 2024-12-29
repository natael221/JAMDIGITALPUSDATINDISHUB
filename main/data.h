#ifndef DATA_H
#define DATA_H

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "DHT.h"
#include "web_display.h"

// Definisi pin dan tipe DHT
#define DHTPIN 25
#define DHTTYPE DHT22

// Variabel global
extern DHT dht;
extern HardwareSerial mySerial;
extern DFRobotDFPlayerMini myDFPlayer;
extern RTC_DS3231 rtc;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// Variabel waktu salat
extern String subuhTime, dzuhurTime, ashrTime, maghribTime, isyaTime;

// Fungsi utama
void connectToWiFi();
void syncRTCwithNTP();
void sendTimeToI2C(int hour, int minute, int second, int day, int month, int year);
void sendDHTToI2C(float temperature, float humidity);
void sendPrayerToNano(byte dataType, String data);
void getPrayerTimes();
void fetchAndSendQuote();
void sendLongStringToNano(byte dataType, String data);
void checkWiFiReconnect();
void checkPrayerTimes(int hour, int minute);

String getRTCDate();
String getRTCTime();
String getRTCDateTime();

#endif // DATA_H
