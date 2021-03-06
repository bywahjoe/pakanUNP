/* Firmansyah Wahyu @2021
   Repo :https://github.com/bywahjoe/pakanUNP.git
*/
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include "pinku.h"
#include "datapakan.h"
#include "RTClib.h"
#include <NewPing.h>
#include <Servo.h>
#include "HX711.h"

//Macro
#define isYES digitalRead(buttonYES)
#define isNO digitalRead(buttonNO)
#define getPot analogRead(potensio)
#define isMaxKanan digitalRead(maxKanan)
#define isMaxKiri digitalRead(maxKiri)

NewPing sonar(trig, echo, 200);     //Ultrasonik
LiquidCrystal_I2C lcd(0x27, 20, 4); //LCD
RTC_DS3231 rtc;                     //RTC Modul
Servo myservo;                      //Servo
HX711 scale;                        //Sensor Berat

//Address EEPROM 10,20
const int addHari = 10;
const int addMode = 20;
int hari, mode;
int maxPakan[91];
float valCalib=-18.90;

//Set Servo
int sBuka=25,sTutup=90; 

//Timer Config
String waktu = "18:30:67";
String pagi = "07:00";
String sore = "17:00";
int nowConfig[3];

int ultra = 0;
bool isRepeat = false;
bool togKanan= false;
bool togKiri=false;

void setup() {
  Serial.begin(9600);
  
  //Init
  pinMode(buttonYES, INPUT_PULLUP);
  pinMode(buttonNO, INPUT_PULLUP);
  pinMode(maxKanan, INPUT_PULLUP);
  pinMode(maxKiri, INPUT_PULLUP);
  
  pinMode(buzz, OUTPUT);
  pinMode(panelA, OUTPUT);
  pinMode(panelB, OUTPUT);
  pinMode(relayMotor, OUTPUT);

  //Servo
  myservo.attach(servopin);
  myservo.write(sTutup);
  
  //Timbangan
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();

  //LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  //RTC Check
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.setCursor(0, 0);
    lcd.print("Timer Error");
    while (1) delay(10);
  }
  
  //First Start Config
  checkMemory();
  rewriteArr();
  loadLCD();
}

void loop() {
  int jam;
  char format[] = "hh:mm:ss";

  DateTime now = rtc.now();
  nowConfig[0] = now.year();
  nowConfig[1] = now.month();
  nowConfig[2] = now.day();
  waktu = now.toString(format); //Timer Config All
  jam = now.hour();             //Timer Config Rotate Panel Surya

  //Check Ultra < 26 habis
  ultra = getUltra();
  if (ultra > kosong)buzzON();
  else buzzOFF();

  //Panel Rotation
  if (jam <= 13)panelKanan();
  else panelKiri();

  //Go Menu
  if (!isYES) menu();

  //Check Time is Valid
  if (waktu.startsWith(pagi) || waktu.startsWith(sore)) {
    Serial.print("TRUEE-"); Serial.println(isRepeat);

    //Protect Repeat Action
    if (!isRepeat) {

      //Update EEPROM
      if (waktu.startsWith(sore)) {
        hari++;
        if (hari > 90)hari = 1;
        EEPROM.put(addHari, hari);
      }
      runAction();
      isRepeat = true;
    }
  } else {
    isRepeat = false;
  }
  
  //Show Status LCD
  displayLCD();
  delay(1000);
}
int getBibit() {
  int bibit = 2000;
  if (mode == 2)bibit = 3000;
  return bibit;
}
int getPersen(int val) {
  int myval= constrain(val, 1, 45);
  int persen=map(myval, 1, 45, 1, 100);
  return 100-persen;
}
int getBerat(){
  scale.set_scale(valCalib);
  int scl=scale.get_units(10);
  
  Serial.print("BERAT: ");Serial.println(scl);
  return scl;
}
int getUltra() {
  return sonar.ping_cm();
}
void buzzON() {
  digitalWrite(buzz, HIGH);
}
void buzzOFF() {
  digitalWrite(buzz, LOW);
}
void buka() {
  myservo.write(sBuka);
}
void tutup() {
  myservo.write(sTutup);
}
void pelontarON() {
  digitalWrite(relayMotor, HIGH);
}
void pelontarOFF() {
  digitalWrite(relayMotor, LOW);
}
void panelOFF() {
  digitalWrite(panelA, LOW);
  digitalWrite(panelB, LOW);
}
void panelKanan() {
  if (!isMaxKanan) {
    panelOFF();
    togKanan=true;
    togKiri=false;
  } 
  if(!togKanan){
    digitalWrite(panelA, HIGH);
    digitalWrite(panelB, LOW);
  }
}
void panelKiri() {
  if (!isMaxKiri) {
    panelOFF();
    togKanan=false;
    togKiri=true;
  }
  if(!togKiri){
    digitalWrite(panelA, LOW);
    digitalWrite(panelB, HIGH);
  }
}
void runAction() {
  int nowGram=getBerat();
  int targetGram=nowGram-maxPakan[hari];
  Serial.print("Target Gram:");Serial.println(targetGram);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM IS RUNNING!");
  lcd.setCursor(0,1);
  
  lcd.print("Est. :");lcd.print(maxPakan[hari]);lcd.print(" gr");
  
  pelontarON();
  buka();
  
  while(getBerat()>=targetGram); 

  pelontarOFF();
  tutup();

  lcd.setCursor(0,2);
  lcd.print("DONE !OK");
  delay(3000);
}
void viewButton() {
  lcd.setCursor(15, 0); lcd.print("~OKAY");
  lcd.setCursor(15, 3); lcd.print("~EXIT");
}
void menu() {
  int option, go = 0;
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Load Menu....");
  delay(3000);

  lcd.clear();
  lcd.setCursor(4, 0); lcd.print("My Menu");
  lcd.setCursor(1, 1); lcd.print("Timer:"); lcd.print(waktu);
  lcd.setCursor(1, 2); lcd.print("Mode :"); lcd.print(mode); lcd.print("~"); lcd.print(getBibit());
  lcd.setCursor(1, 3); lcd.print("Day  :"); lcd.print(hari); lcd.print("/90");
  viewButton();
  while (1) {

    for (int i = 1; i <= 3; i++) {
      option = map(getPot, 0, 1023, 1, 3);
      lcd.setCursor(0, i);
      if (i == option) lcd.print("~");
      else lcd.print(" ");
    }
    if (!isYES) {
      go = option;
      lcd.clear();
      lcd.print("Load Selected Menu");
      Serial.print("GO: "); Serial.println(go);
      delay(1000);
      break;
    }
    if (!isNO) {
      break;
    }
    delay(50);
  }

  lcd.clear();
  viewButton();
  bool jalan = true;

  while (jalan && option > 0) {
    if (option == 1) {
      int setHM[2] = {0, 0};
      int counter = 0;
      int var;
      lcd.setCursor(0, 0);
      lcd.print("Set Your Timer");
      lcd.setCursor(0, 1);
      lcd.print("~HOUR  ~0");
      lcd.setCursor(0, 2);
      lcd.print("~MINUTE~00");

      while (1) {
        if (counter == 0) {//HOUR
          var = map(getPot, 0, 1023, 0, 23);
          lcd.setCursor(8, 1);
        } else {//MINUTE
          lcd.setCursor(8, 2);
          var = map(getPot, 0, 1023, 0, 59);
        }

        if (var <= 9)lcd.print("0"); lcd.print(var);
        if (!isYES) {
          lcd.print(" OK!");
          delay(3000);
          setHM[counter] = var;
          counter++;
          if (counter >= 2) {
            rtc.adjust(DateTime(nowConfig[0], nowConfig[1], nowConfig[2], setHM[0], setHM[1], 0));
            jalan = false; break;
          }
        }
        if (!isNO) {
          jalan = false; break;
        }
      }

    }
    else if (option == 2) {
      lcd.setCursor(0, 0);
      lcd.print("Set Your Mode");
      lcd.setCursor(0, 1);
      lcd.print("~Mode:");

      while (1) {
        lcd.setCursor(6, 1);
        int var = map(getPot, 0, 1023, 1, 2);
        lcd.print(var); lcd.print("~"); lcd.print(var * 1000 + 1000);
        if (!isYES) {
          lcd.print("!OK");
          EEPROM.put(addMode, var);
          mode = var;
          rewriteArr();
          delay(2000);
          jalan = false; break;
        }
        if (!isNO) {
          jalan = false;
          break;
        }
      }
    } else if (option == 3) {
      lcd.setCursor(0, 0);
      lcd.print("Set Your Day");
      lcd.setCursor(0, 1);
      lcd.print("~Day :");

      while (1) {
        lcd.setCursor(6, 1);
        int var = map(getPot, 0, 1023, 1, 90);
        if (var <= 9)lcd.print(" ");
        lcd.print(var); lcd.print("/"); lcd.print("90");
        if (!isYES) {
          lcd.print("!OK");
          EEPROM.put(addHari, var);
          hari = var;
          delay(2000);
          jalan = false; break;
        }
        if (!isNO) {
          jalan = false; break;
        }
      }
    }
  }
}
void copy(int* src, int* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}
void rewriteArr() {
  if (mode == 1) copy(data1, maxPakan, maxDays);
  else copy(data2, maxPakan, maxDays);
}
void checkMemory() {
  hari = EEPROM.read(addHari);
  mode = EEPROM.read(addMode);

  if (hari == 255 || hari == 0) {
    EEPROM.put(addHari, 1);
    hari = 1;
  }
  if (mode == 255 || mode == 0) {
    EEPROM.put(addMode, 1);
    mode = 1;
  }

  Serial.print("HARI: "); Serial.println(hari);
  Serial.print("MODE: "); Serial.println(mode);
}
void loadLCD() {

  lcd.setCursor(1, 1);
  lcd.print("CatFish Automation");
  lcd.setCursor(4, 2);
  lcd.print("V1.0 [~2021]");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Load User Config....");
  lcd.setCursor(0, 1);
  lcd.print("[1]Days  :"); lcd.print(hari); lcd.print("/90");
  lcd.setCursor(0, 2);
  lcd.print("[2]Mode  :"); lcd.print(mode); lcd.print("~"); lcd.print(getBibit());
  delay(3000);
}
void displayLCD() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Day :"); lcd.print(hari); lcd.print("/90");
  lcd.setCursor(12, 0); lcd.print(waktu);

  lcd.setCursor(0, 1); lcd.print("Run :"); lcd.print(pagi);
  lcd.setCursor(12, 1); lcd.print(sore);

  lcd.setCursor(0, 2); lcd.print("Mode:"); lcd.print(getBibit());
  lcd.setCursor(12, 2); lcd.print("~"); lcd.print(maxPakan[hari]);
  lcd.setCursor(18, 2); lcd.print("gr");

  lcd.setCursor(0, 3); lcd.print("Cap :"); lcd.print(getPersen(ultra)); lcd.print("%/ ");
  lcd.setCursor(12, 3); lcd.print("~"); lcd.print(ultra);
  lcd.setCursor(18, 3); lcd.print("cm");
}
