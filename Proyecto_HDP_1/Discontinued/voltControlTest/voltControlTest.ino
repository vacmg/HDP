void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

}
int vmin = 12;
int vmax = 16;
void serialEvent()
{
  int volt = Serial.readStringUntil('\n').toInt();
  Serial.println(volt);
  if (volt >= vmax)
    {
      Serial.println(1);
    }
    if ((volt < vmin))
    {
      Serial.println(0);
    }
}
