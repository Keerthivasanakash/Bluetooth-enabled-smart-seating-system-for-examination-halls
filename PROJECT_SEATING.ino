#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ---------------- OBJECTS ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BT(8, 9);   // RX, TX

// ---------------- CONSTANTS ----------------
#define MAX_STUDENTS 60
#define MAX_HALLS 10
#define MAX_STAFF 10

// ---------------- VARIABLES ----------------
int students[MAX_STUDENTS];
int totalStudents = 0;

int halls = 0;
int benches = 0;
int seats = 0;

bool systemReady = false;
String input = "";

// Exam related
String examName = "DEFAULT";
int examOffset = 0;

// Staff related
int staffID[MAX_STAFF];
int staffHall[MAX_STAFF];
int totalStaff = 0;

// ---------------- SETUP ----------------
void setup() {
  lcd.init();
  lcd.backlight();

  BT.begin(9600);
  Serial.begin(9600);

  randomSeed(analogRead(0));

  lcd.print("SEATING SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("WAITING INPUT");
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

  // CONFIGURE SYSTEM
  if (cmd.startsWith("SET:")) {
    configureSystem(cmd);
  }

  // REGISTER STAFF ID
  else if (cmd.startsWith("STAFFREG:")) {
    if (totalStaff >= MAX_STAFF) return;

    int id = cmd.substring(9).toInt();
    staffID[totalStaff] = id;
    staffHall[totalStaff] = -1;
    totalStaff++;

    lcd.clear();
    lcd.print("STAFF ADDED");
    lcd.setCursor(0, 1);
    lcd.print("ID:");
    lcd.print(id);
  }

  // SET EXAM (SHUFFLES STUDENTS + STAFF)
  else if (cmd.startsWith("EXAM:")) {
    examName = cmd.substring(5);
    examOffset = random(0, totalStudents);

    // Assign halls sequentially
    for (int i = 0; i < totalStaff; i++) {
      staffHall[i] = (i % halls) + 1;
    }

    // Shuffle halls (Fisher-Yates)
    for (int i = totalStaff - 1; i > 0; i--) {
      int j = random(0, i + 1);
      int temp = staffHall[i];
      staffHall[i] = staffHall[j];
      staffHall[j] = temp;
    }

    lcd.clear();
    lcd.print("EXAM SET");
    lcd.setCursor(0, 1);
    lcd.print(examName);
  }

  // STAFF CHECK DUTY
  else if (cmd.startsWith("STAFFID:")) {
    int id = cmd.substring(8).toInt();
    int found = -1;

    for (int i = 0; i < totalStaff; i++) {
      if (staffID[i] == id) {
        found = i;
        break;
      }
    }

    if (found == -1 || staffHall[found] == -1) {
      lcd.clear();
      lcd.print("INVALID STAFF");
      delay(1500);
      resetLCD();
      return;
    }

    lcd.clear();
    lcd.print("ID:");
    lcd.print(id);
    lcd.setCursor(0, 1);
    lcd.print("HALL:");
    lcd.print(staffHall[found]);
  }

  // STUDENT ROLL SEARCH
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

  for (int i = 0; i < totalStudents; i++) {
    students[i] = startRoll + i;
  }

  totalStaff = 0;

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
    int rotatedIndex = (i + examOffset) % totalStudents;
    if (students[rotatedIndex] == roll) {
      index = rotatedIndex;
      break;
    }
  }

  if (index == -1) {
    lcd.clear();
    lcd.print("INVALID ROLL");
    delay(1500);
    resetLCD();
    return;
  }

  int studentsPerHall = benches * seats;

  int hall  = (index / studentsPerHall) + 1;
  int bench = ((index % studentsPerHall) / seats) + 1;
  int seat  = (index % seats) + 1;

  lcd.clear();
  lcd.print("ROLL-");
  lcd.print(roll);
  lcd.print(" HALL-");
  lcd.print(hall);

  lcd.setCursor(0, 1);
  lcd.print("BENCH-");
  lcd.print(bench);
  lcd.print(" SEAT-");
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
  lcd.print("ENTER COMMAND");
}
