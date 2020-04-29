#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Servo.h>

#define RFID_TIMEOUT 5000
#define PN532_IRQ 12
#define NEXT A4
#define SELECT A3
#define LOCK1 A1
#define LOCK2 A2
#define LOCK3 A5
#define BCK 13
#define FWD 4
#define SEC_TIME 2000
#define ERR_TIME 5000

const uint8_t PWM[] = {5, 4, 3};
const uint8_t R[] = {11, 10, 9};
const uint8_t G[] = {8, 7, 6};

// boolean opcodes :
//   false - take
//   true - return

static Adafruit_PN532 nfc(PN532_IRQ, 100);
static LiquidCrystal_I2C lcd(0x27, 20, 4);
static Servo lck[3];

static bool taken[3];

char selected;

void unlock(char num) {
  lck[num].write(90);
}

void lock(char num) {
  lck[num].write(0);
}

void extrude(char num) {
  digitalWrite(FWD, HIGH);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME);
  analogWrite(PWM[num], 0);
  digitalWrite(FWD, LOW);
}

void pull_back(char num) {
  digitalWrite(BCK, HIGH);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME);
  analogWrite(PWM[num], 0);
  digitalWrite(BCK, LOW);
}

void write_database(uint8_t uid[], uint8_t uidLength, bool op, char num) {
}

bool request_auth(uint8_t uid[], uint8_t uidLength, char num) {
  return true;
}

bool read_rfid(bool op, char num) {
  bool success = false;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, RFID_TIMEOUT);
  if(success) {
    Serial.println("success");
    if(op) {
      bool auth;
      auth = request_auth(uid, uidLength, num);
      if(not auth)
        return false;
    }
    write_database(uid, uidLength, op, num);
  }
  else {
    Serial.println("govno");
  }
  return success;
}

void show_menu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CHOOSE:");
  lcd.setCursor(1, 1);
  lcd.print("LAP");
  lcd.setCursor(1, 2);
  lcd.print("HDMI");
  lcd.setCursor(1, 3);
  lcd.print("REM");
  lcd.setCursor(0, 1);
  lcd.print(">");
  selected = 0;
}

void switch_cursor() {
  if(selected == 2) {
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print('>');
    selected = 0;
  }
  else {
    lcd.setCursor(0, selected + 1);
    lcd.print(" ");
    lcd.setCursor(0, selected + 2);
    lcd.print('>');
    selected++;
  }
}

void show_error_reboot() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GOVNO");
  delay(ERR_TIME);
  show_menu();
}

void select() {
  bool success = read_rfid(taken[selected], selected);
  if(not success)
    show_error_reboot();
  else {
    if(taken[selected]) {
      digitalWrite(R[selected], LOW);
      digitalWrite(G[selected], HIGH);
    }
    else {
      digitalWrite(G[selected], LOW);
      digitalWrite(R[selected], HIGH);
    }
    taken[selected] = !taken[selected];
    unlock(selected);
    delay(100);
    extrude(selected);
    delay(SEC_TIME);
    pull_back(selected);
    delay(100);
    lock(selected);
  }
}

void setup(void) {
  lck[0].attach(LOCK1);
  lck[1].attach(LOCK2);
  lck[2].attach(LOCK3);
  taken[0] = false;
  taken[1] = false;
  taken[2] = false;
  pinMode(NEXT, INPUT);
  pinMode(SELECT, INPUT);
  pinMode(BCK, OUTPUT);
  pinMode(FWD, OUTPUT);
  pinMode(PWM[0], OUTPUT);
  pinMode(PWM[1], OUTPUT);
  pinMode(PWM[2], OUTPUT);
  pinMode(R[0], OUTPUT);
  pinMode(R[1], OUTPUT);
  pinMode(R[2], OUTPUT);
  pinMode(G[0], OUTPUT);
  pinMode(G[1], OUTPUT);
  pinMode(G[2], OUTPUT);
  digitalWrite(G[0], HIGH);
  digitalWrite(G[1], HIGH);
  digitalWrite(G[2], HIGH);
  nfc.begin();
  nfc.SAMConfig();
  lcd.begin(16, 4);
  lcd.init();
  lcd.backlight();
  digitalWrite(FWD, LOW);
  digitalWrite(BCK, LOW);
  show_menu();
}

void loop(void) {
  int nxt = digitalRead(NEXT);
  int sel = digitalRead(SELECT);
  if(nxt)
    switch_cursor();
  if(sel)
    select();
  delay(150);
}
