#include "Scene1.h"
#include <Arduino.h>

extern String subuhTime;
extern String dzuhurTime;
extern String ashrTime;
extern String maghribTime;
extern String isyaTime;
extern String currentQuote;

extern volatile byte currentHour;
extern volatile byte currentMinute;
extern volatile byte currentTemperature;
extern volatile byte currentHumidity;
extern volatile byte currentDay;
extern volatile byte currentMonth;
extern volatile byte currentYear;

extern const int DISPLAYS_ACROSS;
extern const int DISPLAYS_DOWN;

void ScanDMD() {
  dmd.scanDisplayBySPI();
}

void scene1(){
  static unsigned long lastSwitchTime = 0;
  static int displayState = 0;  // 0: waktu, 1: suhu/kelembaban, 2: tanggal, 3: jadwal sholat

  if (millis() - lastSwitchTime >= 5000) {  // Ganti tampilan setiap 5 detik
    displayState = 4;
    lastSwitchTime = millis();
  }

  // Mengatur tampilan berdasarkan status displayState
  if (displayState == 0) {
     dmd.selectFont(SevSeg2);
    // Menampilkan waktu berkedip
    dmd.clearScreen(true);
    char jam[6];
    sprintf(jam, "%02d:%02d", currentHour, currentMinute); // Format jam dengan 2 digit
    int spacing = 1;
    int totalWidth = 0;

    for (int i = 0; jam[i] != '\0'; i++) {
      totalWidth += (dmd.charWidth(jam[i]) + spacing);
    }
    totalWidth -= spacing;

    int screenWidth = DISPLAYS_ACROSS * 32;
    int screenHeight = DISPLAYS_DOWN * 16;
    int startX = (screenWidth - totalWidth) / 2;
    int startY = (screenHeight - 15) / 2;

    int x = startX;
    for (int i = 0; jam[i] != '\0'; i++) {
      dmd.drawChar(x, startY, jam[i], GRAPHICS_NORMAL);
      x += dmd.charWidth(jam[i]) + spacing;
    }

    static bool colonVisible = true;
    static unsigned long lastBlinkTime = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 500) {
      lastBlinkTime = currentMillis;
      colonVisible = !colonVisible;

      int colonX = startX + dmd.charWidth(jam[0]) + spacing + dmd.charWidth(jam[1]) + spacing;
      dmd.drawChar(colonX, startY, ':', colonVisible ? GRAPHICS_OR : GRAPHICS_NOR);
    }

  } else if (displayState == 2) {
    // Menampilkan suhu dan kelembaban
    dmd.selectFont(ElektronMart6x12);
    char teks[50];
    sprintf(teks, "Temp: %dC       |       Hum: %d%%", currentTemperature, currentHumidity);

    // Bersihkan layar DMD
    dmd.clearScreen(true);

    // Tampilkan teks bergulir dengan pengaturan ulang jarak spasi
    int panjangTeks = strlen(teks);
    String teksKustom = "";

    for (int i = 0; i < panjangTeks; i++) {
      if (teks[i] == ' ') {
        teksKustom += '\xFF'; // Gunakan placeholder khusus untuk spasi lebih kecil
      } else {
        teksKustom += teks[i];
      }
    }

    dmd.drawMarquee(teksKustom.c_str(), teksKustom.length(), (32 * DISPLAYS_ACROSS) - 0, 2);

    boolean selesai = false;
    long timer = millis();

    while (!selesai) {
      if (millis() - timer > 30) { // Perpindahan setiap 30ms
        selesai = dmd.stepMarquee(-1, 0); // Gerak teks ke kiri
        timer = millis();
      }
    }

  } else if (displayState == 1) {
    // Menampilkan tanggal dengan nama bulan
    dmd.clearScreen(true);    
    dmd.selectFont(ElektronMart6x12);
    // Array nama bulan
    const char *bulan[] = {
        "JAN", "FEB", "MAR", "APR", "MEI", "JUN", 
        "JUL", "AGU", "SEP", "OKT", "NOV", "DES"
    };

    // Pastikan currentMonth berada dalam rentang 1-12
    const char *namaBulan = (currentMonth >= 1 && currentMonth <= 12) ? bulan[currentMonth - 1] : "???";

    // Format tanggal
    char tanggal[20];
    snprintf(tanggal, sizeof(tanggal), "%02d %s %02d", currentDay, namaBulan, currentYear);  // Format: DD MON YY

    // Hitung lebar total teks
    int totalWidth = 0;
    for (int i = 0; tanggal[i] != '\0'; i++) {
      totalWidth += dmd.charWidth(tanggal[i]) + 1;
    }
    totalWidth -= 1; // Hapus spasi terakhir

    // Posisi teks di layar
    int screenWidth = DISPLAYS_ACROSS * 32;
    int startX = (screenWidth - totalWidth) / 2;
    int startY = (16 * DISPLAYS_DOWN - 12) / 2;

    // Gambar teks di layar
    int x = startX;
    for (int i = 0; tanggal[i] != '\0'; i++) {
      dmd.drawChar(x, startY, tanggal[i], GRAPHICS_NORMAL);
      x += dmd.charWidth(tanggal[i]) + 1;
    }
}
 else if (displayState == 3) {
    // Menampilkan jadwal sholat
    dmd.selectFont(ElektronMart6x8);
    tampilJadwalSholat();
  }else if (displayState == 4) {
    tampilQuotes(currentQuote);
  }

  delay(2000);  // Penundaan untuk penggantian tampilan
}

void tampilJadwalSholat() {
  // Tampilkan waktu salat satu per satu
  tampilTeksMoveUp("SUBUH", subuhTime.c_str());
  tampilTeksMoveUp("DZUHUR", dzuhurTime.c_str());
  tampilTeksMoveUp("ASHAR", ashrTime.c_str());
  tampilTeksMoveUp("MAGHRIB", maghribTime.c_str());
  tampilTeksMoveUp("ISYA", isyaTime.c_str());
}

void tampilTeksMoveUp(const char *nama, const char *waktu) {
  // Menghitung lebar teks nama salat dan waktu salat
  char teksNama[20], teksWaktu[20];
  snprintf(teksNama, sizeof(teksNama), "%s", nama);
  snprintf(teksWaktu, sizeof(teksWaktu), "%s", waktu);

  // Menghitung ukuran layar
  int layarTinggi = DISPLAYS_DOWN * 16;  // Tinggi layar dalam pixel
  int layarLebar = DISPLAYS_ACROSS * 32; // Lebar layar dalam pixel

  // Hitung lebar teks nama dan waktu salat
  int teksNamaLebar = 0;
  int teksWaktuLebar = 0;
  for (int i = 0; teksNama[i] != '\0'; i++) {
    teksNamaLebar += dmd.charWidth(teksNama[i]) + 1;
  }
  teksNamaLebar -= 1;

  for (int i = 0; teksWaktu[i] != '\0'; i++) {
    teksWaktuLebar += dmd.charWidth(teksWaktu[i]) + 1;
  }
  teksWaktuLebar -= 1;

  // Posisi X untuk teks di tengah layar
  int posisiX = (layarLebar - max(teksNamaLebar, teksWaktuLebar)) / 2;

  // Posisi awal dan akhir teks untuk gerakan naik
  int posisiAwalY = layarTinggi;      // Di bawah layar
  int posisiAkhirY = -16;             // Di luar layar atas
  int posisiBerhentiY = (layarTinggi - 16) / 2; // Posisi Y berhenti di tengah layar

  // Gerakan dari bawah ke atas
  for (int y = posisiAwalY; y > posisiAkhirY; y--) {
    dmd.clearScreen(true);  // Bersihkan layar

    // Gambar nama salat di bagian atas
    dmd.drawString(posisiX, y, teksNama, strlen(teksNama), GRAPHICS_NORMAL);

    // Gambar waktu salat di bawah nama salat
    dmd.drawString(posisiX, y + 9, teksWaktu, strlen(teksWaktu), GRAPHICS_NORMAL);

    // Jika teks berada di tengah layar, berhenti sementara
    if (y == posisiBerhentiY) {
      delay(2000);  // Tunggu 2 detik di tengah
    }

    delay(50);  // Kecepatan gerakan
  }
}

// Fungsi untuk menampilkan quote dengan efek scrolling
void tampilQuotes(String quote) {
  dmd.selectFont(TimesNewRoman12);
  int startPosition = (DISPLAYS_ACROSS * 32) - 1;  // Posisi awal di sebelah kanan layar
  int endPosition = -(quote.length() * (dmd.charWidth('A') + 1));  // Posisi akhir untuk scroll
  int currentX = startPosition;
  long timer = millis();
  int speed = 40; // Kecepatan scroll

  while (currentX > endPosition) {
    if (millis() - timer > speed) {
      timer = millis();
      dmd.clearScreen(true);  // Membersihkan layar
      int xBawah = currentX;
      int startY = 1;  // Posisi Y untuk teks scrolling
      for (int i = 0; i < quote.length(); i++) {
        dmd.drawChar(xBawah, startY, quote[i], GRAPHICS_NORMAL);  // Gambar karakter
        xBawah += dmd.charWidth(quote[i]) + 1;  // Geser untuk karakter berikutnya
      }
      currentX--;  // Geser posisi X untuk efek scroll
    }
  }
}

