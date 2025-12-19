#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BT(8, 9); // RX, TX

#define MAX_STUDENTS 60

int students[MAX_STUDENTS];
int totalStudents = 42;

int halls = 2;
int benches = 20;
int seats = 2;

bool systemReady = false;
String input = "";

// -------- SETUP --------
void setup() {
  lcd.init();
  lcd.backlight();
  BT.begin(9600);
  Serial.begin(9600);

  randomSeed(analogRead(A0));

  lcd.print("Seat System");
  lcd.setCursor(0, 1);
  lcd.print("Waiting Admin");
}

// -------- MAIN LOOP --------
void loop() {
  while (BT.available()) {
    char c = BT.read();
    if (c == '\n' || c == '\r') {
      input.trim();
      processCommand(input);
      input = "";
    } else {
      input += c;
    }
  }
}

// -------- PROCESS COMMAND --------
void processCommand(String cmd) {
  cmd.toUpperCase();

  if (cmd.startsWith("SET:")) {
    configureSystem(cmd);
  }
  else if (cmd.startsWith("ROLL:")) {
    if (!systemReady) {
      lcd.clear();
      lcd.print("System Not Set");
      delay(1500);
      resetLCD();
      return;
    }
    int roll = cmd.substring(5).toInt();
    showSeat(roll);
  }
}

// -------- CONFIGURATION --------
void configureSystem(String data) {
  int startRoll = getValue(data, "START");
  int endRoll   = getValue(data, "END");
  halls   = getValue(data, "HALLS");
  benches = getValue(data, "BENCHES");
  seats   = getValue(data, "SEATS");

  totalStudents = endRoll - startRoll + 1;

  if (totalStudents > MAX_STUDENTS) {
    lcd.clear();
    lcd.print("Too Many Stud");
    return;
  }

  int capacity = halls * benches * seats;
  if (capacity < totalStudents) {
    lcd.clear();
    lcd.print("Capacity Low");
    return;
  }

  // Generate roll list
  for (int i = 0; i < totalStudents; i++) {
    students[i] = startRoll + i;
  }

  shuffleStudents();

  systemReady = true;

  lcd.clear();
  lcd.print("Seating Ready");
  lcd.setCursor(0, 1);
  lcd.print("Students:");
  lcd.print(totalStudents);
}

// -------- SHUFFLE --------
void shuffleStudents() {
  for (int i = totalStudents - 1; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = students[i];
    students[i] = students[j];
    students[j] = temp;
  }
}

// -------- SHOW SEAT --------
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
    lcd.print("Invalid Roll");
    delay(1500);
    resetLCD();
    return;
  }

  int perHall = benches * seats;
  int hall  = (index / perHall) + 1;
  int bench = ((index / seats) % benches) + 1;
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

// -------- UTILITIES --------
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
  lcd.print("Enter Roll No");

}