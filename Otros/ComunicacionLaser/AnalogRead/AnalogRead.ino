void setup() {
  // put your setup code here, to run once:
  analogReadResolution(12);
  analogWriteResolution(12);
  //SerialUSB.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
//SerialUSB.println(analogRead(A0));

  analogWrite(DAC0,analogRead(A0));
//delay(1);
}
