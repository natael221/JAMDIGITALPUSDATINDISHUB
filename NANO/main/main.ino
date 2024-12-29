#include <Wire.h>
#include <SPI.h>
#include <TimerOne.h>
#include "Scene1.h"
#include "Scene2.h"
#include "I2CUtils.h"

const int DISPLAYS_ACROSS = 2; // Jumlah layar secara horizontal
const int DISPLAYS_DOWN = 1; 

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

volatile byte currentHour = 0;
volatile byte currentMinute = 0;
volatile byte currentSecond = 0;
volatile byte currentTemperature = 0;
volatile byte currentHumidity = 0;
volatile byte currentDay = 0;
volatile byte currentMonth = 0;
volatile byte currentYear = 0;

String subuhTime = "";
String dzuhurTime = "";
String ashrTime = "";
String maghribTime = "";
String isyaTime = "";
String currentQuote = "";

void setup() {
  Serial.begin(9600);  // Untuk debugging

  Timer1.initialize(1000);           
  Timer1.attachInterrupt(ScanDMD);   
  dmd.clearScreen(true);             

  Wire.begin(8);  // Inisialisasi dengan alamat I2C 8
  Wire.onReceive(receiveEvent);  // Daftarkan fungsi untuk menerima data
}

void loop() {
  scene2();  // Tampilkan scene2 yang berisi quote
}
