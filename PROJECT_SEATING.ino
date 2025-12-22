#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BT(8, 9); // RX, TX

#define MAX_STUDENTS 60

int students[MAX_STUDENTS];
int totalStudents = 0;

int halls = 0;
int benches = 0;   // benches PER hall
int seats = 0;     // seats PER bench

bool systemReady = false;
String input = "";

// ---------------- SETUP ----------------
void setup() {
  lcd.init();
  lcd.backlight();
  BT.begin(9600);
  Serial.begin(9600);

  lcd.print("SEATING SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("WAITING FOR I/P");
}

// ---------------- LOOP ----------------
void loop() {
  while (BT.available()) {
    char c = BT.read();
    if (c == '\n' || c == '\r') {
      input.trim();
      input.toUpperCase();
      processCommand(input);
      input = "";
    } else {
      input += c;
    }
  }
}

// ---------------- COMMAND HANDLER ----------------
void processCommand(String cmd) {

  if (cmd.startsWith("SET:")) {
    configureSystem(cmd);
  }
  else if (cmd.startsWith("ROLL:")) {
    if (!systemReady) {
      lcd.clear();
      lcd.print("SYSTEM NOT SET");
      delay(1500);
      resetLCD();
      return;
    }
    int roll = cmd.substring(5).toInt();
    showSeat(roll);
  }
}

// ---------------- CONFIGURATION ----------------
void configureSystem(String data) {

  int startRoll = getValue(data, "START");
  int endRoll   = getValue(data, "END");
  halls   = getValue(data, "HALLS");
  benches = getValue(data, "BENCHES");
  seats   = getValue(data, "SEATS");

  totalStudents = endRoll - startRoll + 1;

  if (totalStudents <= 0 || totalStudents > MAX_STUDENTS) {
    lcd.clear();
    lcd.print("INVALID COUNT");
    return;
  }

  int capacity = halls * benches * seats;
  if (capacity < totalStudents) {
    lcd.clear();
    lcd.print("CAPACITY LOW");
    return;
  }

  // Generate roll numbers in ORDER
  for (int i = 0; i < totalStudents; i++) {
    students[i] = startRoll + i;
  }

  systemReady = true;

  lcd.clear();
  lcd.print("SEATING READY");
  lcd.setCursor(0, 1);
  lcd.print("STUDENTS:");
  lcd.print(totalStudents);
}

// ---------------- SHOW SEAT ----------------
void showSeat(int roll) {

  int index = -1;
  for (int i = 0; i < totalStudents; i++) {
    if (students[i] == roll) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    lcd.clear();
    lcd.print("INVALID ROLL NO");
    delay(1500);
    resetLCD();
    return;
  }

  // ⭐ CORRECT LOGIC (Bench resets per hall)
  int studentsPerHall = benches * seats;

  int hall  = (index / studentsPerHall) + 1;
  int bench = ((index % studentsPerHall) / seats) + 1;
  int seat  = (index % seats) + 1;

  lcd.clear();
  lcd.print("ROLL ");
  lcd.print(roll);
  lcd.setCursor(0, 1);
  lcd.print("H");
  lcd.print(hall);
  lcd.print(" B");
  lcd.print(bench);
  lcd.print(" S");
  lcd.print(seat);
}

// ---------------- UTILITIES ----------------
int getValue(String data, String key) {
  int idx = data.indexOf(key + "=");
  if (idx == -1) return 0;
  int start = idx + key.length() + 1;
  int end = data.indexOf(',', start);
  if (end == -1) end = data.length();
  return data.substring(start, end).toInt();
}

void resetLCD() {
  lcd.clear();
  lcd.print("ENTER ROLL NO");
}