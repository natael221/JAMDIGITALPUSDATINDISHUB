#include "I2CUtils.h"
#include <Arduino.h>

// Variabel global dari main.ino
extern volatile byte currentHour, currentMinute, currentSecond, currentTemperature, currentHumidity, currentDay, currentMonth, currentYear;
extern String subuhTime, dzuhurTime, ashrTime, maghribTime, isyaTime, currentQuote;

// Variabel buffer untuk menyusun quotes
String quoteBuffer = "";
bool isReceivingQuote = false;

void receiveEvent(int howMany) {
  if (howMany > 1) {
    byte dataType = Wire.read();
    Serial.print("Data received. Type: 0x");
    Serial.print(dataType, HEX);

    if (dataType == 0x20) {  // DataType untuk quotes
      byte isLastChunk = Wire.read();  // Membaca apakah ini chunk terakhir
      String chunk = "";
      while (Wire.available()) {
        char c = Wire.read();
        chunk += c;
      }
      // Tambahkan chunk ke buffer
      quoteBuffer += chunk;

      if (isLastChunk) {
        // Jika chunk terakhir, simpan ke variabel `currentQuote`
        currentQuote = quoteBuffer;
        Serial.println("Complete quote received:");
        Serial.println(currentQuote);

        // Reset buffer
        quoteBuffer = "";
        isReceivingQuote = false;
      } else {
        // Masih menerima chunk
        isReceivingQuote = true;
      }
    } else {
      // Logika lain untuk data selain quotes
      if (howMany == 2) {
        byte dataValue = Wire.read();
        Serial.print(", Value: ");
        Serial.println(dataValue);

        if (dataType == 0x01) currentHour = dataValue;
        else if (dataType == 0x02) currentMinute = dataValue;
        else if (dataType == 0x09) currentSecond = dataValue;
        else if (dataType == 0x03) currentTemperature = dataValue;
        else if (dataType == 0x04) currentHumidity = dataValue;
        else if (dataType == 0x05) currentDay = dataValue;
        else if (dataType == 0x06) currentMonth = dataValue;
        else if (dataType == 0x07) currentYear = dataValue;
      } else {
        String dataValue = "";
        while (Wire.available()) {
          char c = Wire.read();
          dataValue += c;
        }
        Serial.print(", Value: ");
        Serial.println(dataValue);

        if (dataType == 0x10) subuhTime = dataValue;
        else if (dataType == 0x11) dzuhurTime = dataValue;
        else if (dataType == 0x12) ashrTime = dataValue;
        else if (dataType == 0x13) maghribTime = dataValue;
        else if (dataType == 0x14) isyaTime = dataValue;
      }
    }
  }
}
