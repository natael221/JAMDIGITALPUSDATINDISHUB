#include "Scene2.h"
#include <Arduino.h>

// Variabel waktu
extern volatile byte currentHour;
extern volatile byte currentMinute;
extern volatile byte currentSecond;
extern volatile byte currentTemperature;
extern volatile byte currentHumidity;
extern volatile byte currentDay;
extern volatile byte currentMonth;
extern volatile byte currentYear;

extern String subuhTime;
extern String dzuhurTime;
extern String ashrTime;
extern String maghribTime;
extern String isyaTime;
extern String currentQuote;

// Dimensi layar
extern const int DISPLAYS_ACROSS;
extern const int DISPLAYS_DOWN;

// Variabel untuk kontrol waktu
unsigned long previousMillis = 0;
unsigned long moveDownMillis = 0;
unsigned long stopMillis = 0;
const unsigned long interval = 1000;      // Interval untuk detik
const unsigned long moveInterval = 50;    // Interval untuk animasi move down
const unsigned long stopDuration = 5000;  // Durasi berhenti di tengah layar (3 detik)

// Variabel untuk posisi Y
int moveDownY = -16;         // Posisi awal (move down dimulai di atas layar)
int moveDownStopY = (DISPLAYS_DOWN * 16 - 16) / 2; // Posisi berhenti di tengah layar
int moveDownEndY = DISPLAYS_DOWN * 16;    // Posisi akhir (move down)

bool moveDownStopped = false;  // Apakah teks sudah berhenti
byte currentStage = 0; // Untuk melacak urutan yang ditampilkan (0: TEMP, 1: HUMID, 2: DATE)

byte lastSecond = 255;

unsigned long moveUpMillis = 0;
const unsigned long moveUpInterval = 50; // Interval untuk animasi move up
const unsigned long salatStopDuration = 5000;  // Durasi berhenti di tengah layar (3 detik)

int moveUpY = DISPLAYS_DOWN * 16;       // Posisi awal (di luar layar bawah)
int moveUpStopY = (DISPLAYS_DOWN * 16 - 16) / 2; // Posisi berhenti di tengah layar
int moveUpEndY = -16;                   // Posisi akhir (keluar layar atas)

bool moveUpStopped = false;
byte salatStage = 0; // Tahap waktu salat yang sedang ditampilkan

// Variabel global untuk quote
String quote = "This is a sample quote.";  // Variabel untuk menyimpan kutipan

void tampilMoveDown(const char* nama, const char* waktu, int yPos) {
    dmd.drawFilledBox(33, 0, DISPLAYS_ACROSS * 32 - 1, DISPLAYS_DOWN * 16 - 1, GRAPHICS_INVERSE);
    dmd.selectFont(ElektronMart6x8);
    dmd.drawString(35, yPos, nama, strlen(nama), GRAPHICS_NORMAL);
    dmd.drawString(35, yPos + 9, waktu, strlen(waktu), GRAPHICS_NORMAL);
}

void tampilMoveUp(const char* nama, const char* waktu, int yPos) {
    dmd.drawFilledBox(33, 0, DISPLAYS_ACROSS * 32 - 1, DISPLAYS_DOWN * 16 - 1, GRAPHICS_INVERSE);
    dmd.selectFont(ElektronMart6x8);
    dmd.drawString(35, yPos, nama, strlen(nama), GRAPHICS_NORMAL);
    dmd.drawString(35, yPos + 9, waktu, strlen(waktu), GRAPHICS_NORMAL);
}

void tampilJam() {
    // Hapus area waktu
    dmd.drawFilledBox(0, 0, 32, DISPLAYS_DOWN * 16 - 1, GRAPHICS_INVERSE);

    // Tampilkan jam, menit, dan detik
    dmd.selectFont(Angka6x16);
    char jam[3];
    sprintf(jam, "%02d", currentHour);
    dmd.drawString(1, 0, jam, strlen(jam), GRAPHICS_NORMAL);

    dmd.selectFont(ElektronMart6x8);
    char menit[3];
    sprintf(menit, "%02d", currentMinute);
    dmd.drawString(16, 0, menit, strlen(menit), GRAPHICS_NORMAL);

    char detik[3];
    sprintf(detik, "%02d", currentSecond);
    dmd.drawString(16, 9, detik, strlen(detik), GRAPHICS_NORMAL);
}

void tampilDataMoveDown() {
  if (currentStage == 1) {
    char suhu[10];
    sprintf(suhu, "%d}C", currentTemperature);
    tampilMoveDown("TEMP", suhu, moveDownY);
  } else if (currentStage == 2) {
    char humid[10];
    sprintf(humid, "%d{", currentHumidity);
    tampilMoveDown("HUMIDITY", humid, moveDownY);
  } else if (currentStage == 0) {
    const char* namaBulan[] = {"JAN", "FEB", "MAR", "APR", "MEI", "JUN",
                               "JUL", "AGU", "SEP", "OKT", "NOV", "DES"};
    char tanggal[16];
    if (currentMonth >= 1 && currentMonth <= 12) {
      sprintf(tanggal, "%02d %s %02d", currentDay, namaBulan[currentMonth - 1], currentYear % 100);
    } else {
      sprintf(tanggal, "%02d ??? %02d", currentDay, currentYear % 100);
    }
    tampilMoveDown("DATE", tanggal, moveDownY);
  }
}

void tampilSalatMoveUp() {
    const char* namaSalat[] = {"SUBUH", "DZUHUR", "ASHAR", "MAGHRIB", "ISYA"};
    String waktuSalat[] = {subuhTime, dzuhurTime, ashrTime, maghribTime, isyaTime};

    if (salatStage >= 0 && salatStage <= 4) {
        dmd.drawFilledBox(33, 0, DISPLAYS_ACROSS * 32 - 1, DISPLAYS_DOWN * 16 - 1, GRAPHICS_INVERSE); // Hapus layar
        dmd.selectFont(ElektronMart6x8);

        // Tampilkan nama salat dan waktu salat
        dmd.drawString(35, moveUpY, namaSalat[salatStage], strlen(namaSalat[salatStage]), GRAPHICS_NORMAL);
        dmd.drawString(35, moveUpY + 9, waktuSalat[salatStage].c_str(), strlen(waktuSalat[salatStage].c_str()), GRAPHICS_NORMAL);  // Menggunakan c_str() untuk konversi
    }
}

void tampilQuotesScrolling() {
    static int scrollX = DISPLAYS_ACROSS * 32;  // Mulai di posisi kanan layar
    static unsigned long lastScrollMillis = 0;  // Variabel untuk kontrol kecepatan scrolling
    unsigned long currentMillis = millis();
    
    // Tentukan interval untuk memperlambat kecepatan scrolling (misal 50ms)
    if (currentMillis - lastScrollMillis >= 55) {
        lastScrollMillis = currentMillis; // Update last scroll time

        // Hapus area untuk quotes (bukan seluruh layar)
        dmd.drawFilledBox(33, 0, DISPLAYS_ACROSS * 32 - 1, DISPLAYS_DOWN * 16 - 1, GRAPHICS_INVERSE);

        // Tampilkan teks quotes
        dmd.selectFont(Arial_Black_16_ISO_8859_1);
        dmd.drawString(scrollX, 0, currentQuote.c_str(), currentQuote.length(), GRAPHICS_NORMAL);

        // Update posisi scrolling (gerak ke kiri)
        scrollX--;  // Menggerakkan teks ke kiri

        // Jika teks sudah keluar layar kiri, reset posisi ke kanan dan mulai ulang
        if (scrollX + currentQuote.length() * 6 <= 0) {  // Memastikan teks sudah keluar layar kiri
            Serial.println("Quote selesai, kembali ke stage awal.");
            
            // Reset scrollX ke posisi kanan
            scrollX = DISPLAYS_ACROSS * 32;  // Posisi kanan

            // Tunggu sampai teks selesai bergerak keluar layar sebelum kembali ke stage awal
            // Reset ke stage pertama setelah menampilkan quotes
            currentStage = 0;  // Reset ke stage pertama (TEMP)
            moveDownY = -16;   // Reset posisi untuk move down
            moveUpY = DISPLAYS_DOWN * 16;  // Reset posisi untuk move up
            moveDownStopped = false;
            moveUpStopped = false;
            salatStage = 0;  // Reset stage salat
            lastSecond = 255; // Reset detik terakhir agar tampilJam bisa aktif
            previousMillis = millis(); // Reset timer utama
            moveDownMillis = millis();
            moveUpMillis = millis();
        }
    }
}

void scene2() {
    unsigned long currentMillis = millis();

    // Perbarui waktu
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        currentSecond++;
        if (currentSecond >= 60) {
            currentSecond = 0;
            currentMinute++;
            if (currentMinute >= 60) {
                currentMinute = 0;
                currentHour++;
                if (currentHour >= 24) {
                    currentHour = 0;
                }
            }
        }
    }

    // Tampilkan jam hanya jika bukan dalam mode tampilan quote
    if (currentStage != 4 && currentSecond != lastSecond) {
        lastSecond = currentSecond;
        tampilJam();  // Tampilkan waktu
    }

    // Logika move down (Temperature, Humidity, Date)
    if (currentStage < 3 && currentMillis - moveDownMillis >= moveInterval) {
        moveDownMillis = currentMillis;

        if (!moveDownStopped) {
            moveDownY++;
            if (moveDownY >= moveDownStopY) {
                moveDownY = moveDownStopY;
                moveDownStopped = true;
                stopMillis = currentMillis;
            }
        } else if (currentMillis - stopMillis >= stopDuration) {
            moveDownY++;
            if (moveDownY >= moveDownEndY) {
                moveDownY = -16;
                moveDownStopped = false;
                currentStage++;
            }
        }

        tampilDataMoveDown();  // Tampilkan Temperature, Humidity, Date
    }

    // Logika move up (Salat times)
    if (currentStage == 3 && currentMillis - moveUpMillis >= moveUpInterval) {
        moveUpMillis = currentMillis;

        if (!moveUpStopped) {
            moveUpY--;
            if (moveUpY <= moveUpStopY) {
                moveUpY = moveUpStopY;
                moveUpStopped = true;
                stopMillis = currentMillis;
            }
        } else if (currentMillis - stopMillis >= salatStopDuration) {
            moveUpY--;
            if (moveUpY <= moveUpEndY) {
                moveUpY = DISPLAYS_DOWN * 16;
                moveUpStopped = false;
                salatStage++;
                if (salatStage > 4) {
                    salatStage = 0;
                    currentStage = 4;  // Ubah ke stage 4 untuk menampilkan quotes
                }
            }
        }

        tampilSalatMoveUp();  // Tampilkan waktu salat
    }

    // Tampilkan quotes jika sudah mencapai stage ke-4
    if (currentStage == 4) {
        tampilQuotesScrolling();  // Tampilkan quotes scrolling dari kanan ke kiri
    } else {
        // Garis vertikal hanya ditampilkan jika bukan sedang menampilkan quotes
        dmd.drawLine(31, 0, 31, DISPLAYS_DOWN * 16 - 1, GRAPHICS_NORMAL);
        dmd.drawLine(32, 0, 32, DISPLAYS_DOWN * 16 - 1, GRAPHICS_NORMAL);
    }
}
