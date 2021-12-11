/* Firmansyah Wahyu @2021
   Repo :https://github.com/bywahjoe/pakanUNP.git
*/
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include "pinku.h"
#include "datapakan.h"
#include "RTClib.h"

#define isYES digitalRead(buttonYES)
#define isNO digitalRead(buttonNO)
#define getPot analogRead(potensio)

#define buzzON digitalWrite(buzz,HIGH)
#define buzzOFF digitalWrite(buzz,LOW)

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

//Address 0,1
int hari, mode;
int maxPakan[91];

String waktu = "18:30:67";
String pagi = "07:00";
String sore = "17:00";
int nowConfig[3];

bool isRepeat = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buttonYES, INPUT_PULLUP);
  pinMode(buttonNO, INPUT_PULLUP);
  pinMode(buzz, OUTPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.setCursor(0, 0);
    lcd.print("Timer Error");
    while (1) delay(10);
  }

  //  EEPROM.update(0,0);
  //  EEPROM.update(1,0);
  checkMemory();
  rewriteArr();
  loadLCD();

}

void loop() {
  char format[] = "hh:mm:ss";
  DateTime now = rtc.now();
  nowConfig[0] = now.year();
  nowConfig[1] = now.month();
  nowConfig[2] = now.day();
  waktu = now.toString(format);

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
        EEPROM.put(0, hari);
      }
      runAction();
    }
  } else {
    isRepeat = false;
  }

  displayLCD();
  delay(1000);
}
int getBibit() {
  int bibit = 2000;
  if (mode == 2)bibit = 3000;
  return bibit;
}
int getBerat() {
  return 10000;
}
int getUltra() {
  return 150;
}
void runAction() {

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
          EEPROM.put(1, var);
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
          EEPROM.put(0, var);
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
  hari = EEPROM.read(0);
  mode = EEPROM.read(1);

  if (hari == 255 || hari == 0) {
    EEPROM.put(0, 1);
    hari = 1;
  }
  if (mode == 255 || mode == 0) {
    EEPROM.put(1, 1);
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

  lcd.setCursor(0, 3); lcd.print("Cap :"); lcd.print(getBerat()); lcd.print("gr/ ");
  lcd.setCursor(12, 3); lcd.print("~"); lcd.print(getUltra());
  lcd.setCursor(18, 3); lcd.print("cm");

}
void buka() {


}
void tutup() {
}
