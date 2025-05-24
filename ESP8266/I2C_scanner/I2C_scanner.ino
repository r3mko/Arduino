// Version 1.0
// Author: Remko Kleinjan
// Date: 240525

#include <Wire.h>

void setup() {
  Serial.begin(9600);

  Serial.println();
  Serial.println("== I2C scanner ==");

  Wire.begin();
}

void loop() {
  byte count = 0;

  Serial.println("Scanning...");
  Serial.println();
  
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);

    if (Wire.endTransmission() == 0) {
      Serial.print("Address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
    }
  }

  Serial.println();
  Serial.println("Done.");
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" device(s).");

  delay(5000);
}
