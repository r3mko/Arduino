// Version 1.0
// Author: Remko Kleinjan
// Date: 240525

#include <LiquidCrystal_I2C.h>

#define printByte(args) write(args);

uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t clck[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};
  
// The LCD display is connected to D1/GPIO5 (SCL) and D2/GPIO4 (SDA)
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  lcd.init(); 
  
  lcd.createChar(0, bell);
  lcd.createChar(1, note);
  lcd.createChar(2, clck);
  lcd.createChar(3, heart);
  lcd.createChar(4, duck);
  lcd.createChar(5, check);
  lcd.createChar(6, cross);
  lcd.createChar(7, retarrow);

  lcd.clear();
  lcd.print("Hello world...");

  lcd.setCursor(0, 1);
  lcd.print("I ");
  lcd.printByte(3);
  lcd.print(" LCDs!");

  delay(5000);
}

// Display all keycodes
void displayKeyCodes() {
  uint8_t i = 0;

  while (1) {
    lcd.clear();

    // Print HEX codes
    lcd.print("Codes 0x");
    lcd.print(i, HEX);
    lcd.print("-0x");
    lcd.print(i + 15, HEX);

    // Print Chars
    lcd.setCursor(0, 1);
    for (int j = 0; j < 16; j++) {
      lcd.printByte(i + j);
    }
    i += 16;
    
    delay(5000);
  }
}

void loop() {
  displayKeyCodes();
}
