/*
  Workbee pendant controller
  M H Mangay 2021
  generates gcode on button selection

 */
 #include <Wire.h>
 
const byte Xup = 3;
const byte Xdown = 4;
byte buttonX = 1;
void setup() 
{
  pinMode(Xup,INPUT);
  pinMode(Xdown,INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(57600);
  while (!Serial) {
    ;
  }
  Wire.begin();
}

void loop() 
{ // run over and over
  if((digitalRead(Xup) == 0)&& (buttonX ==1))
   {
    buttonX = 0;
    if (Serial.availableForWrite() > 20) {
      Serial.print("G91 G0 F6000 Y10.0");
      Serial.write('\n');
      digitalWrite(LED_BUILTIN,LOW);
    }
   
   }
   Wire.beginTransmission(0x70);
   Wire.write(1);
   Wire.endTransmission(1);
   if(digitalRead(Xup)) //released
   {
    buttonX = 1;  //arm for next transision
    digitalWrite(LED_BUILTIN,HIGH);
   }
   else
    digitalWrite(LED_BUILTIN,LOW);
}
