#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <time.h>
#include <data.h>
#include <web_display.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");

    // Setup WiFi Access Point
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial.println("Access Point started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    server.begin();
    handleserver();
    Serial.println("HTTP server started");

    // Setup WiFi Station
    WiFi.begin(sta_ssid, sta_password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("Station IP address: ");
    Serial.println(WiFi.localIP());

    // Inisialisasi RTC
    if (!rtc.begin()) {
        Serial.println("RTC initialization failed!");
        while (1);
    }

    // Sinkronkan RTC dengan NTP
    syncRTCwithNTP();

    // Inisialisasi DFPlayer
    mySerial.begin(9600, SERIAL_8N1, 16, 17);
    if (!myDFPlayer.begin(mySerial)) {
        Serial.println("DFPlayer initialization failed!");
        while (1);
    }
    myDFPlayer.volume(30);

    // Setup I2C
    Wire.begin();
    Wire1.begin(18, 19);

    // Kirim waktu pertama kali setelah RTC disinkronkan
    DateTime now = rtc.now();
    sendTimeToI2C(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

    // Inisialisasi DHT dan Prayer Times serta Quotes
    dht.begin();
    getPrayerTimes();
    fetchAndSendQuote();
}


void loop() {
    DateTime now = rtc.now();
    static unsigned long lastTimeMillis = 0;
    server.handleClient();

    if (millis() - lastTimeMillis >= 60000) {
        sendTimeToI2C(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
        lastTimeMillis = millis();
    }

    // Update DHT setiap 5 detik
    static unsigned long lastDHTMillis = 0;
    if (millis() - lastDHTMillis >= 5000) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        if (!isnan(temperature) && !isnan(humidity)) {
            sendDHTToI2C(temperature, humidity);
            Serial.println(temperature);
            Serial.println(humidity);
        }
        lastDHTMillis = millis();
    }

    static unsigned long lastQuoteMillis = 0;
    if (millis() - lastQuoteMillis >= 60000) {
        if (WiFi.status() == WL_CONNECTED) {
            fetchAndSendQuote();
        }
        lastQuoteMillis = millis();
    }

    // Reconnect WiFi jika disconnected
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck >= 5000) {
        lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected. Retrying...");
            WiFi.begin(sta_ssid, sta_password);
        }
        Serial.println("Sigma Connected");
        Serial.print("Free memory: ");
        Serial.println(ESP.getFreeHeap());
    }
}
