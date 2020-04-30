#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Servo.h>

#define RFID_TIMEOUT 5000
#define PN532_IRQ 12
#define NEXT A4
#define SELECT A3
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
static bool taken[3];

uint8_t selected;

void eject(uitn8_t num, uint8_t fct = 1) {
  digitalWrite(FWD, HIGH);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME / fct);
  analogWrite(PWM[num], 0);
}

void retract(uitn8_t num, uint8_t fct = 1) {
  digitalWrite(FWD, LOW);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME / t);
  analogWrite(PWM[num], 0);
}

void write_database(uint8_t uid[], uint8_t uidLength, uitn8_t num) {
  Serial1.print("+");
  Serial1.print(num);
  Serial1.print(",");
  for(uint8_t i = 0; i < uidLength - 1; i++) {
    Serial1.print(uid[i]);
    Serial1.print(":");
  }
  Serial1.print(uid[uidLength - 1]);
  Serial1.print("\n\r");
}

bool request_auth(uint8_t uid[], uint8_t uidLength, uitn8_t num) {
  Serial1.print("?");
  Serial1.print(num);
  Serial1.print(",");
  for(uint8_t i = 0; i < uidLength - 1; i++) {
    Serial1.print(uid[i]);
    Serial1.print(":");
  }
  Serial1.print(uid[uidLength - 1]);
  Serial1.print("\n\r");
  delay(200);
  String s = Serial1.readString();
  if(s[0] == '1')
    return true;
  else
    return false;
}

bool read_rfid(bool op, uint8_t num) {
  bool success = false;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, RFID_TIMEOUT);
  if(success) {
    if(op) {
      bool auth;
      auth = request_auth(uid, uidLength, num);
      if(not auth)
        return false;
    }
    else {
      write_database(uid, uidLength, num);
    }
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
    eject(selected);
    delay(SEC_TIME);
    retract(selected);
  }
}

void setup(void) {
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
  String sdata = Serial1.readString();
  if(sdata[0] == '_') {
    if(sdata[1] == '1') {
      eject(0);
      delay(4 * SEC_TIME);
      retract(0);
    }
    else if(sdata[1] == '2') {
      eject(1);
      delay(4 * SEC_TIME);
      retract(1);
    }
    else if(sdata[1] == '3') {
      eject(2);
      delay(4 * SEC_TIME);
      retract(2);
    }
    else if(sdata[1] == 'a'){
      eject(0, 3);
      eject(1, 2);
      eject(2);
      delay(4 * SEC_TIME);
    }
  }
  delay(150);
}
