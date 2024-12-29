#include "web_display.h"

// Definisi variabel
const char* sta_ssid = "RAJA SIGMA";
const char* sta_password = "pusdatin07";

const char* ap_ssid = "DIGICLOCK F";
const char* ap_password = "12345678";

IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

String sentences[MAX_SENTENCES];
int sentenceCount = 0;

String prayerTimes[5];
String prayerNames[] = {"Subuh", "Dzuhur", "Ashar", "Maghrib", "Isya"};

DateTime now; // Variabel global untuk menyimpan waktu RTC
String currentTime; // Variabel global untuk waktu dalam format string
String currentDate;

String previousSSID = "";

int currentVolume = 50;

void updatePrayerTimesArray() {
  prayerTimes[0] = subuhTime;
  prayerTimes[1] = dzuhurTime;
  prayerTimes[2] = ashrTime;
  prayerTimes[3] = maghribTime;
  prayerTimes[4] = isyaTime;
}

void updateRTCTime() {
    // Pastikan RTC diperbarui dengan NTP jika sudah beberapa waktu
    static unsigned long lastNTPUpdate = 0;
    unsigned long NTPUpdateInterval = 3600000; // Update setiap jam (3600000 ms = 1 jam)
    
    if (millis() - lastNTPUpdate >= NTPUpdateInterval) {
        syncRTCwithNTP();  // Sinkronkan RTC dengan NTP
        lastNTPUpdate = millis();  // Pembaruan timestamp
    }

    // Ambil waktu terbaru dari RTC
    DateTime now = rtc.now();
    currentTime = String(now.hour()) + ":" + String(now.minute());
    currentDate = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
}


bool is_authenticated() {
  if (!server.authenticate("admin", "admin")) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

String getPage(String content) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>DigiClock</title>";
  
  // Menambahkan favicon
  html += "<link rel='icon' href='/favicon.ico' type='image/x-icon'>";
  
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f9f9f9;color:#333;}";
  html += "header h1{margin:0;font-size:1.5em;}";
  html += "nav{background:#007BA7;padding:10px;display:flex;gap:15px;}";
  html += "nav a{color:white;text-decoration:none;padding:5px 10px;border-radius:5px;transition:background 0.3s;}";
  html += "nav a:hover{background:#005f73;}";
  html += ".container{max-width:800px;margin:20px auto;padding:20px;background:white;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
  html += "h1,h2{color:#007BA7;}";
  html += "form input[type='time'], form input[type='submit']{margin:10px 0;padding:10px;width:100%;max-width:300px;border:1px solid #ccc;border-radius:5px;}";
  html += "form input[type='submit']{background:#007BA7;color:white;border:none;cursor:pointer;}";
  html += "form input[type='submit']:hover{background:#005f73;}"; 
  html += "</style></head><body>";
  html += "<header><h1>Dinas Perhubungan</h1></header>";
  html += "<nav>";
  html += "<a href='/'>Home</a>";
  html += "<a href='/info'>WiFI-Info</a>";
  html += "<a href='/tampilan1'>Tampil 1</a>";
  html += "<a href='/tampilan2'>Tampil 2</a>";
  html += "</nav>";
  html += "<div class='container'>";
  html += content;
  html += "</div></body></html>";
  return html;
}

void handleInfo() {
  if (!is_authenticated()) {
    return;
  }
  
  // Data informasi jaringan
  String currentSSID = String(sta_ssid); // Mengubah SSID ke String untuk manipulasi
  String content = "<h1>Network Information</h1>";
  content += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  content += "<p>SSID: " + currentSSID + "</p>";
  
  // Cek perubahan SSID
  if (currentSSID != previousSSID) {
    Serial.println("SSID has been updated.");
    Serial.println("New SSID: " + currentSSID);
    previousSSID = currentSSID; // Perbarui nilai SSID sebelumnya
  }
  
  server.send(200, "text/html", getPage(content));
}

void handleHome() {
  String content = "<h1 style='color:#007BA7;text-align:center;'>Welcome to ESP32 Web Server</h1>";

  // Div untuk tombol
  content += "<div style='display:flex;flex-direction:column;align-items:center;gap:15px;margin-top:20px;'>";

  // Tombol Tampilan 1
  content += "<a href='/tampilan1' style='text-align:center;display:inline-block;width:220px;padding:15px;"
             "font-size:18px;color:white;background:#007BA7;border:none;border-radius:10px;text-decoration:none;"
             "box-shadow:0 4px 6px rgba(0,0,0,0.1);'>Tampilan 1</a>";

  // Tombol Tampilan 2
  content += "<a href='/tampilan2' style='text-align:center;display:inline-block;width:220px;padding:15px;"
             "font-size:18px;color:white;background:#007BA7;border:none;border-radius:10px;text-decoration:none;"
             "box-shadow:0 4px 6px rgba(0,0,0,0.1);'>Tampilan 2</a>";

  // Tombol WiFi Info
  content += "<a href='/info' style='text-align:center;display:inline-block;width:220px;padding:15px;"
             "font-size:18px;color:white;background:#007BA7;border:none;border-radius:10px;text-decoration:none;"
             "box-shadow:0 4px 6px rgba(0,0,0,0.1);'>WiFi Info</a>";

  content += "</div>";

  server.send(200, "text/html", getPage(content));
}

void handleTampilan1() {
    String content = "<h1 style='text-align:center;color:#007BA7;'>Tampilan 1</h1>";

    // Waktu dan Tanggal dari RTC
    String currentTime = getRTCTime();
    String currentDate = getRTCDate();

    // Menampilkan waktu
    content += "<h2 style='text-align:left;'>Current Time</h2>";
    content += "<div id='clock' style='font-size:24px;text-align:center;margin-bottom:20px;'>" + currentTime + "</div>";

    // Tombol Edit Waktu
    content += "<div style='text-align:center;'>";
    content += "<a href='/edit_time' style='display:inline-block;background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-bottom:10px;'>Edit Time</a>";
    content += "</div>";

    // Menampilkan tanggal
    content += "<h2 style='text-align:left;'>Current Date</h2>";
    content += "<div id='date' style='font-size:24px;text-align:center;margin-bottom:20px;'>" + currentDate + "</div>";

    // Tombol Edit Tanggal
    content += "<div style='text-align:center;'>";
    content += "<a href='/edit_date' style='display:inline-block;background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-bottom:10px;'>Edit Date</a>";
    content += "</div>";

    content += "<h2 style='text-align:left;color:#007BA7;'>Jadwal Salat</h2>";
    content += "<form style='max-width:340px;margin:0 auto;text-align:center;'>";

    for (int i = 0; i < 5; i++) {
      content += "<label for='prayer" + String(i) + "' style='display:block;text-align:left;margin-bottom:5px;'>" 
                + prayerNames[i] + ":</label>";
      content += "<input type='time' id='prayer" + String(i) + "' name='prayer" + String(i) + "' value='" 
                + prayerTimes[i] + "' readonly style='width:100%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;background:#f9f9f9;cursor:not-allowed;'>";
    }
    content += "</form>";

    // Tombol Back to Home
    content += "<div style='text-align:center;margin-top:20px;'>";
    content += "<a href='/' style='display:inline-block;text-decoration:none;'>";
    content += "<button style='display:inline-block;background-color:#007BA7;color:white;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;'>Back to Home</button>";
    content += "</a>";
    content += "</div>";

    server.send(200, "text/html", getPage(content));
}

void handleEditPrayerTimes() {
  String content = "<h1>Edit Prayer Times</h1>";
  
  // Form untuk mengedit waktu salat
  content += "<form action='/save_prayer_times' method='POST' style='max-width:340px;margin:0 auto;text-align:center;'>"; 
  
  for (int i = 0; i < 5; i++) {
    content += "<label for='prayer" + String(i) + "' style='display:block;text-align:left;margin-bottom:5px;'>" 
               + prayerNames[i] + ":</label>";
    content += "<input type='time' id='prayer" + String(i) + "' name='prayer" + String(i) + "' value='" 
               + prayerTimes[i] + "' style='width:100%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;'>";
  }
  
  // Tombol Save Changes
  content += "<input type='submit' value='Save Changes' style='background-color:#007BA7;color:white; padding:10px 20px;width:100%;max-width:340px;border:none;border-radius:5px;cursor:pointer;margin-bottom:10px;'>";

  content += "</form>";

  // Tombol Back to Home
  content += "<div style='text-align:center;margin-top:10px;'>";
  content += "<a href='/'><button style='background-color:#007BA7;color:white;padding:10px 20px;width:100%;max-width:340px;border:none;border-radius:5px;cursor:pointer;'>Back to Home</button></a>";
  content += "</div>";

  server.send(200, "text/html", getPage(content));  // Kirim halaman HTML
}

void handleSavePrayerTimes() {
  // Mengambil data dari form inputan
  for (int i = 0; i < 5; i++) {
    if (server.hasArg("prayer" + String(i))) {
      prayerTimes[i] = server.arg("prayer" + String(i)); // Menyimpan waktu baru
    }
  }

  // Kirim data baru ke Nano via I2C
  for (int i = 0; i < 5; i++) {
    sendPrayerToNano(0x10 + i, prayerTimes[i]);
    delay(50);  // Delay untuk menghindari collision
  }

  // Menampilkan konfirmasi bahwa data telah disimpan
  String content = "<h1>Prayer Times Updated</h1>";
  content += "<p>Your prayer times have been updated successfully!</p>";
  content += "<a href='/'><button>Back to Home</button></a>";
  
  server.send(200, "text/html", getPage(content));  // Kirim halaman konfirmasi
}

void handleEditTime() {
    if (!is_authenticated()) {
        return;
    }

    updateRTCTime(); // Baca waktu dari RTC sebelum menampilkan formulir

    String content = "<h1>Edit Time</h1>";
    content += "<form action='/save_time' method='POST' style='max-width:340px;margin:0 auto;text-align:center;'>"; // Center form and inputs
    content += "<label for='time' style='display:block;text-align:left;margin-bottom:5px;'>Time:</label>";
    content += "<input type='time' id='time' name='time' value='" + currentTime + "' style='width:100%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;'>";

    content += "<input type='submit' value='Save Time' style='background-color:#007BA7;color:white;padding:10px 20px;width:100%;max-width:340px;border:none;border-radius:5px;cursor:pointer;margin-top:10px;'>";

    content += "</form>";
    content += "<a href='/' style='background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-top:20px;display:block;text-align:center;'>Back to Home</a>";

    server.send(200, "text/html", getPage(content));
}

void handleSaveTime() {
    if (!is_authenticated()) {
        return;
    }

    String newTime = server.arg("time"); // Ambil waktu dari form
    int newHour = newTime.substring(0, 2).toInt();
    int newMinute = newTime.substring(3, 5).toInt();

    // Ambil data tanggal saat ini
    int currentSecond = now.second();
    int currentDay = now.day();
    int currentMonth = now.month();
    int currentYear = now.year();

    // Perbarui RTC dengan waktu baru
    rtc.adjust(DateTime(currentYear, currentMonth, currentDay, newHour, newMinute, currentSecond));

    // Perbarui variabel global
    updateRTCTime();

    // Kirim waktu ke perangkat lain via I2C
    sendTimeToI2C(newHour, newMinute, currentSecond, currentDay, currentMonth, currentYear);

    String content = "<h1>Time Saved Successfully!</h1>";
    content += "<p>New time: " + currentTime + "</p>";
    content += "<a href='/' style='background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-top:20px;display:block;text-align:center;'>Back to Home</a>";

    server.send(200, "text/html", getPage(content));
}

void handleEditDate() {
    if (!is_authenticated()) {
        return;
    }

    updateRTCTime(); // Baca tanggal dari RTC sebelum menampilkan formulir

    String content = "<h1>Edit Date</h1>";
    content += "<form action='/save_date' method='POST' style='max-width:340px;margin:0 auto;text-align:center;'>"; // Center form and inputs
    content += "<label for='date' style='display:block;text-align:left;margin-bottom:5px;'>Date:</label>";
    content += "<input type='date' id='date' name='date' value='" + currentDate + "' style='width:95%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;'>";

    content += "<input type='submit' value='Save Date' style='background-color:#007BA7;color:white;padding:10px 20px;width:100%;max-width:340px;border:none;border-radius:5px;cursor:pointer;margin-top:10px;'>";

    content += "</form>";
    content += "<a href='/' style='background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-top:20px;display:block;text-align:center;'>Back to Home</a>";

    server.send(200, "text/html", getPage(content));
}

void handleSaveDate() {
    if (!is_authenticated()) {
        return;
    }

    String newDate = server.arg("date"); // Ambil tanggal dari form
    int newYear = newDate.substring(0, 4).toInt();
    int newMonth = newDate.substring(5, 7).toInt();
    int newDay = newDate.substring(8, 10).toInt();

    // Ambil data waktu saat ini
    int currentHour = now.hour();
    int currentMinute = now.minute();
    int currentSecond = now.second();

    // Perbarui RTC dengan tanggal baru
    rtc.adjust(DateTime(newYear, newMonth, newDay, currentHour, currentMinute, currentSecond));

    // Perbarui variabel global
    updateRTCTime();

    // Kirim tanggal ke perangkat lain via I2C
    sendTimeToI2C(currentHour, currentMinute, currentSecond, newDay, newMonth, newYear);

    String content = "<h1>Date Saved Successfully!</h1>";
    content += "<p>New date: " + currentDate + "</p>";
    content += "<a href='/' style='background-color:#007BA7;color:white;padding:8px 15px;border-radius:5px;text-decoration:none;margin-top:20px;display:block;text-align:center;'>Back to Home</a>";

    server.send(200, "text/html", getPage(content));
}

void handleTampilan2() {
  String content = "<h1 style='text-align:center;color:#007BA7;'>Tampilan 2</h1>";

  // Form input untuk menambahkan kalimat
  content += "<form method='POST' action='/save_sentence' style='max-width:400px;margin:0 auto;text-align:center;'>";
  content += "<label for='sentence' style='display:block;text-align:left;'>Enter a sentence (max 50 words):</label>";
  content += "<textarea id='sentence' name='sentence' rows='3' maxlength='300' style='width:95%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;' required></textarea>";
  content += "<button type='submit' style='background-color:#007BA7;color:white;padding:8px 15px;border:none;border-radius:5px;cursor:pointer;'>Submit</button>";
  content += "</form>";

  // Tabel untuk menampilkan daftar kalimat yang tersimpan
  content += "<h2 style='text-align:left;color:#007BA7;margin-top:20px;'>Saved Sentences</h2>";
  content += "<table border='1' style='width:100%;max-width:400px;margin:0 auto;border-collapse:collapse;text-align:left;'>";
  content += "<tr style='background-color:#007BA7;color:white;'><th>#</th><th>Sentence</th><th>Actions</th></tr>";

  for (int i = 0; i < sentenceCount; i++) {
    content += "<tr>";
    content += "<td style='padding:8px;text-align:center;'>" + String(i + 1) + "</td>";
    content += "<td style='padding:8px;'>" + sentences[i] + "</td>";
    content += "<td style='padding:8px;text-align:center;'>";
    content += "<a href='/edit_sentence?index=" + String(i) + "' style='color:blue;text-decoration:none;'>Edit</a> | ";
    content += "<a href='/remove_sentence?index=" + String(i) + "' style='color:red;text-decoration:none;'>Remove</a>";
    content += "</td>";
    content += "</tr>";
  }

  content += "</table>";

  // Tombol Back to Home
  content += "<div style='text-align:center;margin-top:20px;'>";
  content += "<a href='/'><button style='background-color:#007BA7;color:white;padding:10px 20px;width:100%;"
             "max-width:340px;border:none;border-radius:5px;cursor:pointer;'>Back to Home</button></a>";
  content += "</div>";

  server.send(200, "text/html", getPage(content));
}

void handleSaveSentence() {
  if (server.hasArg("sentence")) {
    String sentence = server.arg("sentence");

    // Validasi jumlah kata
    int wordCount = 0;
    bool inWord = false;
    for (char c : sentence) {
      if (isspace(c)) {
        if (inWord) {
          wordCount++;
          inWord = false;
        }
      } else {
        inWord = true;
      }
    }
    if (inWord) wordCount++;

    if (wordCount <= 50) {
      if (sentenceCount < MAX_SENTENCES) {
        sentences[sentenceCount++] = sentence; // Tambahkan kalimat ke array
        server.sendHeader("Location", "/tampilan2?message=Sentence%20saved%20successfully!");
        server.send(303);
      } else {
        server.sendHeader("Location", "/tampilan2?message=Error:%20Maximum%20sentences%20reached!");
        server.send(303);
      }
    } else {
      server.sendHeader("Location", "/tampilan2?message=Error:%20Sentence%20exceeds%2050%20words!");
      server.send(303);
    }
  }
}

void handleEditSentence() {
  if (server.hasArg("index")) {
    int index = server.arg("index").toInt();

    if (index >= 0 && index < sentenceCount) {
      String content = "<h1 style='text-align:center;color:#007BA7;'>Edit Sentence</h1>";
      content += "<form method='POST' action='/update_sentence' style='max-width:400px;margin:0 auto;text-align:center;'>";
      content += "<label for='sentence' style='display:block;text-align:left;'>Edit the sentence:</label>";
      content += "<textarea id='sentence' name='sentence' rows='3' maxlength='300' style='width:95%;padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:5px;' required>";
      content += sentences[index];
      content += "</textarea>";
      content += "<input type='hidden' name='index' value='" + String(index) + "'>";
      content += "<button type='submit' style='background-color:#007BA7;color:white;padding:8px 15px;border:none;border-radius:5px;cursor:pointer;'>Update</button>";
      content += "</form>";
      content += "<div style='text-align:center;margin-top:20px;'>";
      content += "<a href='/tampilan2'><button style='background-color:#007BA7;color:white;padding:10px 20px;width:100%;"
                 "max-width:340px;border:none;border-radius:5px;cursor:pointer;'>Back</button></a>";
      content += "</div>";

      server.send(200, "text/html", getPage(content));
    } else {
      server.send(200, "text/html", "<h2 style='text-align:center;color:red;'>Error: Invalid index!</h2><a href='/tampilan2'>Back</a>");
    }
  }
}

void handleUpdateSentence() {
  if (server.hasArg("index") && server.hasArg("sentence")) {
    int index = server.arg("index").toInt();
    String sentence = server.arg("sentence");

    // Validasi jumlah kata
    int wordCount = 0;
    bool inWord = false;
    for (char c : sentence) {
      if (isspace(c)) {
        if (inWord) {
          wordCount++;
          inWord = false;
        }
      } else {
        inWord = true;
      }
    }
    if (inWord) wordCount++;

    if (wordCount <= 50) {
      if (index >= 0 && index < sentenceCount) {
        sentences[index] = sentence;
        server.sendHeader("Location", "/tampilan2?message=Sentence%20updated%20successfully!");
        server.send(303);
      } else {
        server.sendHeader("Location", "/tampilan2?message=Error:%20Invalid%20index!");
        server.send(303);
      }
    } else {
      server.sendHeader("Location", "/tampilan2?message=Error:%20Sentence%20exceeds%2050%20words!");
      server.send(303);
    }
  }
}

void handleRemoveSentence() {
  if (server.hasArg("index")) {
    int index = server.arg("index").toInt();

    if (index >= 0 && index < sentenceCount) {
      // Geser elemen ke kiri untuk menghapus
      for (int i = index; i < sentenceCount - 1; i++) {
        sentences[i] = sentences[i + 1];
      }
      sentenceCount--;

      server.sendHeader("Location", "/tampilan2?message=Sentence%20removed%20successfully!");
      server.send(303);
    } else {
      server.sendHeader("Location", "/tampilan2?message=Error:%20Invalid%20index!");
      server.send(303);
    }
  }
}  // Volume awal (dummy)

void handleSetVolume() {
  if (server.method() == HTTP_POST) {
    if (server.hasArg("volume")) {
      // Update nilai variabel volume
      currentVolume = server.arg("volume").toInt();

      // Log di serial monitor
      Serial.println("Volume updated to: " + String(currentVolume));
    }
  }
  
  // Redirect kembali ke Tampilan 1
  server.sendHeader("Location", "/tampilan1");
  server.send(303);  // Kode status HTTP 303 = Redirect
}

void handleserver() {
  server.on("/", handleHome);
  server.on("/tampilan1", handleTampilan1);
  server.on("/tampilan2", handleTampilan2);
  server.on("/info", handleInfo);
  server.on("/edit_prayer_times", HTTP_GET, handleEditPrayerTimes);  // Menampilkan form edit waktu salat
  server.on("/save_prayer_times", HTTP_POST, handleSavePrayerTimes);
  server.on("/edit_time", HTTP_GET, handleEditTime); // Menampilkan form edit waktu
  server.on("/edit_date", HTTP_GET, handleEditDate); // Menampilkan form edit tanggal
  server.on("/save_time", HTTP_POST, handleSaveTime); // Menyimpan waktu yang telah diedit
  server.on("/save_date", HTTP_POST, handleSaveDate); // Menyimpan tanggal yang telah diedit
  server.on("/tampilan2", handleTampilan2);
  server.on("/save_sentence", HTTP_POST, handleSaveSentence);
  server.on("/edit_sentence", handleEditSentence);
  server.on("/update_sentence", HTTP_POST, handleUpdateSentence);
  server.on("/remove_sentence", handleRemoveSentence);
  server.on("/", handleTampilan1);
  server.on("/set_volume", HTTP_POST, handleSetVolume); 
}