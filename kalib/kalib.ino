#include "HX711.h"
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

HX711 scale;

void setup() {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();
  long zero_factor = scale.read_average();
  Serial.print("Zero:");Serial.println(zero_factor);
  
//scale.set_scale(22.90);
}
void loop() {
  if(Serial.available()){
      scale.set_scale();
    scale.tare();
  }
  scale.set_scale(22.90);
  int scl=scale.get_units(10);
  Serial.println(scl);
//  delay(100);
}
