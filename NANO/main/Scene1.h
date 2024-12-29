#ifndef SCENE1_H
#define SCENE1_h

#include <DMD.h>
// #include "fonts/SystemFont5x7.h"
#include "fonts/sevseg2.h"
#include "fonts/ElektronMart6x12.h"
#include "fonts/ElektronMart6x8.h"
#include "fonts/TimesNewRoman12.h"

extern const int DISPLAYS_ACROSS;
extern const int DISPLAYS_DOWN;

extern DMD dmd;

// Fungsi-fungsi DMD
void ScanDMD();
void tampilTeksMoveUp(const char *nama, const char *waktu);
void tampilTanggal();
void tampilJamBerkedip();
void tampilSuhuDanKelembaban();
void tampilTeksBergulir(const char *teks);
void tampilJadwalSholat();
void tampilQuotes(String quote);
void scene1();

#endif
