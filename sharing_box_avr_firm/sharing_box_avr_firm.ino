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

const uint8_t PWM[] = {5, 4, 13};
const uint8_t R[] = {11, 10, 9};
const uint8_t G[] = {8, 7, 6};
String tcp_db = "";
const char cpage[814] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>SHARING BOX by 444 Tech Team</title><style>div { display: flex; }</style></head><body><h1>SHARING BOX</h1><h3> by 444 Tech Team</h3><h2>Control panel</h2><hr><div><p>Sec. 1 (Laptop):</p></div><button type=\"button\" onclick=\"window.location.href='/?action=1'\">Unlock</button><div><p>Sec. 2 (HDMI): </p></div><button type=\"button\" onclick=\"window.location.href='/?action=2'\">Unlock</button><div><p>Sec. 3 (Remote): </p></div><button type=\"button\" onclick=\"window.location.href='/?action=3'\">Unlock</button><hr><button type=\"button\" onclick=\"window.location.href='/?action=a'\">Unlock all</button><button type=\"button\" onclick=\"window.location.href='/?action=d'\">This device is a database server</button></body></html>";
const char hexstr[16] = "0123456789ABCDEF";

// boolean opcodes :
//   false - take
//   true - return

static Adafruit_PN532 nfc(PN532_IRQ, 100);
static LiquidCrystal_I2C lcd(0x27, 20, 4);
static bool taken[3];

uint8_t selected;

void eject(uint8_t num, uint8_t fct = 1) {
  digitalWrite(FWD, HIGH);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME / fct);
  analogWrite(PWM[num], 0);
}

void retract(uint8_t num, uint8_t fct = 1) {
  digitalWrite(FWD, LOW);
  analogWrite(PWM[num], 255);
  delay(SEC_TIME / fct);
  analogWrite(PWM[num], 0);
}

void write_database(uint8_t uid[], uint8_t uidLength, uint8_t num) {
  String request = "+";
  char sec = '0' + num;
  request += sec;
  Serial1.print("AT+CIPSTART=9,TCP,");
  Serial1.print(tcp_db);
  Serial1.print(",41\r\n");
  Serial1.print("AT+CIPSEND=9,");
  request += ',';
  for(uint8_t i = 0; i < uidLength - 1; i++) {
    request += hexstr[uid[i] / 16];
    request += hexstr[uid[i] % 16];
  }
  request += hexstr[uid[uidLength - 1] / 16];
  request += hexstr[uid[uidLength - 1] % 16];
  Serial1.print(request.length());
  Serial1.print("\r\n");
  Serial1.print(request);
  Serial1.print("\r\n");
}

bool request_auth(uint8_t uid[], uint8_t uidLength, uint8_t num) {
  String request = "?";
  char sec = '0' + num;
  request += sec;
  Serial1.print("AT+CIPSTART=9,TCP,");
  Serial1.print(tcp_db);
  Serial1.print(",41\r\n");
  Serial1.print("AT+CIPSEND=9,");
  request += ',';
  for(uint8_t i = 0; i < uidLength - 1; i++) {
    request += hexstr[uid[i] / 16];
    request += hexstr[uid[i] % 16];
  }
  request += hexstr[uid[uidLength - 1] / 16];
  request += hexstr[uid[uidLength - 1] % 16];
  Serial1.print(request.length());
  Serial1.print("\r\n");
  Serial1.print(request);
  Serial1.print("\r\n");
  delay(1000);
  Serial1.print("AT+CIPCLOSE=9");
  if(Serial1.find("+IPD,9,1:1"))
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
  lcd.print("ERROR!");
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
    eject(selected, 1);
    delay(SEC_TIME);
    retract(selected, 1);
  }
}

void setup(void) {
  Serial.begin(9600);
  selected = 0;
  Serial1.begin(9600);
  delay(1000);
  Serial1.print("AT+CWMODE=3\r\n");
  delay(200);
  Serial1.print("AT+CWSAP=\"444\",\"444techteam\",1,3");
  delay(200);
  Serial1.print("AT+CIPMUX=1\r\n");
  delay(200);
  Serial1.print("AT+CIPSERVER=1\r\n");
  delay(200);
  Serial1.print("AT+CIFSR");
  delay(200);
  while(Serial1.available())
    Serial.print(Serial1.read());
  Serial.print("\r\n");
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
  if(Serial1.available()) {
    if(Serial1.find("+IPD,")) {
      delay(1000);
      uint8_t c_id = Serial1.read() - 48;
      if(Serial1.find("action=")) {
        char action = Serial1.read();
        if(action == '1') {
          eject(1);
          String response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE HTML>\n<html>\nUnlocking Sec. 1...\n</html>\r\n";
          Serial1.print("AT+CIPSEND=");
          Serial1.print(c_id);
          Serial1.print(",");
          Serial1.print(response.length());
          Serial1.print("\r\n");
          Serial1.print(response);
          Serial1.print("AT+CIPCLOSE=");
          Serial1.print(c_id);
          Serial1.print("\r\n");
          delay(10000);
          retract(1);
        }
        else if(action == '2') {
          eject(2);
          String response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE HTML>\n<html>\nUnlocking Sec. 2...\n</html>\r\n";
          Serial1.print("AT+CIPSEND=");
          Serial1.print(c_id);
          Serial1.print(",");
          Serial1.print(response.length());
          Serial1.print("\r\n");
          Serial1.print(response);
          Serial1.print("AT+CIPCLOSE=");
          Serial1.print(c_id);
          Serial1.print("\r\n");
          delay(10000);
          retract(2);
        }
        else if(action == '3') {
          eject(3);
          String response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE HTML>\n<html>\nUnlocking Sec. 3...\n</html>\r\n";
          Serial1.print("AT+CIPSEND=");
          Serial1.print(c_id);
          Serial1.print(",");
          Serial1.print(response.length());
          Serial1.print("\r\n");
          Serial1.print(response);
          Serial1.print("AT+CIPCLOSE=");
          Serial1.print(c_id);
          Serial1.print("\r\n");
          delay(10000);
          retract(3);
        }
        else if(action == 'a') {
          eject(1, 3);
          eject(2, 2);
          eject(3);
          String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nUnlocking all...\r\n</html>\r\n";
          Serial1.print("AT+CIPSEND=");
          Serial1.print(c_id);
          Serial1.print(",");
          Serial1.print(response.length());
          Serial1.print("\r\n");
          Serial1.print(response);
          Serial1.print("AT+CIPCLOSE=");
          Serial1.print(c_id);
          Serial1.print("\r\n");
          delay(10000);
          retract(1, 3);
          retract(2, 2);
          retract(3);
        }
        else if(action == 'd') {
          Serial1.find("Host: ");
          tcp_db = "";
          char p = Serial1.read();
          while(((p - 48 >= 0) && (p - 48 <= 9)) || (p == '.')) {
            tcp_db += p;
            p = Serial1.read();
          }
          String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nUnlocking Sec. 3...\r\n</html>\r\n";
          Serial1.print("AT+CIPSEND=");
          Serial1.print(c_id);
          Serial1.print(",");
          Serial1.print(response.length());
          Serial1.print("\r\n");
          Serial1.print(response);
          Serial1.print("AT+CIPCLOSE=");
          Serial1.print(c_id);
          Serial1.print("\r\n");
        }
      }
      else {
        Serial1.print("AT+CIPSEND=");
        Serial1.print(c_id);
        Serial1.print(",");
        Serial1.print(814);
        Serial1.print("\r\n");
        Serial1.print(cpage);
        Serial1.print("AT+CIPCLOSE=");
        Serial1.print(c_id);
        Serial1.print("\r\n");
      }
    }
    char temp;
    while(Serial1.available()) {
      temp = Serial1.read();
    }
  }
  delay(150);
}
