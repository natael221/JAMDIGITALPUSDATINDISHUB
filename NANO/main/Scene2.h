#ifndef SCENE2_H
#define SCENE2_H
#include <DMD.h>
#include "fonts/Angka6x16.h"
#include "fonts/ElektronMart6x16.h"
#include "fonts/ElektronMart6x8.h"
#include "fonts/Arial_Black_16_ISO_8859_1.h"

// extern volatile byte currentHour;
// extern volatile byte currentMinute;
// extern volatile byte currentSecond;

// Fungsi untuk menampilkan Scene 2
extern DMD dmd;
void scene2();
// void tampilJamPanel1();
// void tampilMoveUp(const char* nama, const char* waktu, int yPos);
// void tampilMoveDown(const char* nama, const char* waktu, int yPos);
void tampilSalatMoveUp();
void tampilDataMoveDown();
void tampilMoveDown(const char* nama, const char* waktu, int yPos);
void tampilMoveUp(const char* nama, const char* waktu, int yPos);
void tampilJam();
void tampilQuotesScrolling();
#endif // SCENE2_H
