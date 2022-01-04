#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
  Serial.begin(9600);
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("Hello, world!");
 delay(1500);
}

void loop()
{
  uint16_t a0 = analogRead(A0);
  uint16_t a1 = analogRead(A1);
  float v0 = a0*5.00/1024.00;
  float v1 = a1*5.00/1024.00;
  float vcc = 12;
  float k = 0.02; // sensor calibration constant
  float s = 2.21; // sensor zero
  float i0 = (v0-s)/k;
  float i1 = (v1-s)/k;
  float vr = analogRead(A2);
  vr = vr*5;
  vr = vr / 1024;
  vr = vr*300;
  vr = vr/5;
  lcd.clear();
  Serial.print(v0);
  Serial.print('\t');
  Serial.print(i0);
  Serial.print('\t');
  Serial.println(vr);
  lcd.print(vr);
  delay(750);
}
