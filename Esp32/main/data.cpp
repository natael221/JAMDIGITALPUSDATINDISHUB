#include "data.h"

// Inisialisasi variabel global
DHT dht(DHTPIN, DHTTYPE);
HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;
RTC_DS3231 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

String subuhTime, dzuhurTime, ashrTime, maghribTime, isyaTime;

void syncRTCwithNTP() {
    timeClient.begin();
    if (timeClient.forceUpdate()) {
        unsigned long epochTime = timeClient.getEpochTime();
        DateTime ntpTime = DateTime(epochTime);
        rtc.adjust(ntpTime);
    }
}

void sendTimeToI2C(int hour, int minute, int second, int day, int month, int year) {
    Wire1.beginTransmission(8);
    Wire1.write(0x01);
    Wire1.write(hour);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x02);
    Wire1.write(minute);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x09);
    Wire1.write(second);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x05);
    Wire1.write(day);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x06);
    Wire1.write(month);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x07);
    Wire1.write(year % 100);
    Wire1.endTransmission();
    delay(30);
}

void sendPrayerToNano(byte dataType, String data) {
    Wire1.beginTransmission(8);
    Wire1.write(dataType);
    Wire1.write((const uint8_t *)data.c_str(), data.length());
    Wire1.endTransmission();
    delay(30);
}

void getPrayerTimes() {
  if (WiFi.status() == WL_CONNECTED) {
    DateTime now = rtc.now();
    String apiURL = "https://api.myquran.com/v2/sholat/jadwal/1301/" 
                    + String(now.year()) + "/" + String(now.month());

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, apiURL);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        return;
      }
      
      char currentDate[30];
      snprintf(currentDate, sizeof(currentDate), "%04d-%02d-%02d", now.year(), now.month(), now.day());

      JsonArray jadwalArray = doc["data"]["jadwal"].as<JsonArray>();
      for (JsonObject jadwal : jadwalArray) {
        if (String(jadwal["date"].as<const char*>()) == currentDate) {
          subuhTime = jadwal["subuh"].as<String>();
          dzuhurTime = jadwal["dzuhur"].as<String>();
          ashrTime = jadwal["ashar"].as<String>();
          maghribTime = jadwal["maghrib"].as<String>();
          isyaTime = jadwal["isya"].as<String>();

          // Perbarui array waktu salat
          updatePrayerTimesArray();

          // Kirim data ke I2C
          sendPrayerToNano(0x10, subuhTime);  // Subuh
          delay(50);
          sendPrayerToNano(0x11, dzuhurTime); // Dzuhur
          delay(50);
          sendPrayerToNano(0x12, ashrTime);   // Ashar
          delay(50);
          sendPrayerToNano(0x13, maghribTime);// Maghrib
          delay(50);
          sendPrayerToNano(0x14, isyaTime);   // Isya
          delay(50);
          break;
        }
      }
    }
    http.end();
  }
}

void sendDHTToI2C(float temperature, float humidity) {
    Wire1.beginTransmission(8);
    Wire1.write(0x03);
    Wire1.write((int)temperature);
    Wire1.endTransmission();
    delay(30);

    Wire1.beginTransmission(8);
    Wire1.write(0x04);
    Wire1.write((int)humidity);
    Wire1.endTransmission();
    delay(30);
}

void fetchAndSendQuote() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.setTimeout(5000);
    http.begin(client, "http://192.168.60.57:4300/pkb/book/runningtxt");

    int httpCode = http.GET();
    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        String quote = doc["quotes"].as<String>();
        // Kirim ke Arduino Nano atau perangkat lain
        sendLongStringToNano(0x20, quote);
      }
    }
    http.end();
  } 
}

void sendLongStringToNano(byte dataType, String data) {
    size_t length = data.length();
    size_t chunkSize = 30;
    for (size_t i = 0; i < length; i += chunkSize) {
        size_t remaining = length - i;
        size_t currentChunkSize = remaining < chunkSize ? remaining : chunkSize;
        String chunk = data.substring(i, i + currentChunkSize);

        Wire1.beginTransmission(8);
        Wire1.write(dataType);
        Wire1.write((i + currentChunkSize >= length));
        Wire1.write((const uint8_t *)chunk.c_str(), chunk.length());
        Wire1.endTransmission();
        delay(50);
    }
}

void checkPrayerTimes(int hour, int minute) {
  String currentTime = String(hour) + ":" + String(minute);
  static String lastPlayedTime = "";
  
  if (currentTime == subuhTime && lastPlayedTime != subuhTime) {
    myDFPlayer.play(1);
    lastPlayedTime = subuhTime;
  } else if (currentTime == "16:45" && lastPlayedTime != dzuhurTime) {
    myDFPlayer.play(1);
    lastPlayedTime = dzuhurTime;
  } else if (currentTime == ashrTime && lastPlayedTime != ashrTime) {
    myDFPlayer.play(1);
    lastPlayedTime = ashrTime;
  } else if (currentTime == maghribTime && lastPlayedTime != maghribTime) {
    myDFPlayer.play(1);
    lastPlayedTime = maghribTime;
  } else if (currentTime == isyaTime && lastPlayedTime != isyaTime) {
    myDFPlayer.play(1);
    lastPlayedTime = isyaTime;
  }
}

String getRTCDateTime() {
    DateTime now = rtc.now();
    char dateTimeBuffer[20];
    snprintf(dateTimeBuffer, sizeof(dateTimeBuffer), "%02d:%02d:%02d %02d-%02d-%04d",
             now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
    return String(dateTimeBuffer);
}

String getRTCTime() {
    DateTime now = rtc.now();
    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    return String(timeBuffer);
}

String getRTCDate() {
    DateTime now = rtc.now();
    char dateBuffer[12];
    snprintf(dateBuffer, sizeof(dateBuffer), "%02d-%02d-%04d", now.day(), now.month(), now.year());
    return String(dateBuffer);
}
