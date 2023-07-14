/*
  Workbee pendant controller
  M H Mangay 2021
  generates gcode on button selection
 note that checksum is not generated- turn off requirement with
 M575 P1 S0
 */
 void DoMove(char);
 //define buttons
const byte Zup = 3;
const byte Zdown = 2;
const byte Yplus = 5;
const byte Yminus = 4;
const byte Xplus = 6;
const byte Xminus = 7;
//define switch
const byte Sunits = 8;
const byte Stens = 9;
const byte Shundreds = 10;
const byte macros = 11;
const byte moremacros = 14;
const byte BrushLift = 13;
const char Axis[] = {'X','Y','Z'};
byte buttonwas = 0;
byte BrushUpMode = 0;
byte ButtonState = 0;
void setup() 
{
  pinMode(Zup,INPUT_PULLUP);
  pinMode(Zdown,INPUT_PULLUP);
  pinMode(Yplus,INPUT_PULLUP);
  pinMode(Yminus,INPUT_PULLUP);
  pinMode(Xplus,INPUT_PULLUP);
  pinMode(Xminus,INPUT_PULLUP);
  pinMode(Sunits,INPUT_PULLUP);
  pinMode(Stens,INPUT_PULLUP);
  pinMode(Shundreds,INPUT_PULLUP);
  pinMode(macros,INPUT_PULLUP);
  pinMode(moremacros,INPUT_PULLUP);
  pinMode(BrushLift,INPUT_PULLUP);
  
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(57600);
  while (!Serial) {
    ;
  }
}

void loop() 
{ // run over and over

  if(BrushUpMode) //cutter lifted for brush; do nothing else
  {
    if(digitalRead(BrushLift))  //has been released
    {
      if(BrushUpMode)
      {
        Serial.print("G91 G0 F6000 Z-30.0");  //return cutter
        Serial.write('\n');
        delay(300);  
      }
      BrushUpMode = 0;          //so clear it
    }
      
  }
  else
  {
    if(!digitalRead(BrushLift))
    {
      if(!BrushUpMode)
      {
        Serial.print("G91 G0 F6000 Z30.0"); //raise cutter
        Serial.write('\n');
        delay(300);
      }
      BrushUpMode = 1;  //so we can lock other commands
    }
    else
    {
      ButtonState = 0;
      if(!digitalRead(Xplus))
        ButtonState =1;
      if(!digitalRead(Xminus))
        ButtonState =4;
      if(!digitalRead(Yplus))
        ButtonState =2;
      if(!digitalRead(Yminus))
        ButtonState =5;
      if(!digitalRead(Zup))
        ButtonState =3;
      if(!digitalRead(Zdown))
        ButtonState =6;
      if(ButtonState) //one is pressed
      {
        if(!buttonwas) //was not pressed last time
        {
          DoMove(ButtonState - 1);
          buttonwas = 1;  //remember for next time       
        }
      }
    else
      buttonwas = 0;  //trap release.
    }
  }
}

void DoMove(char type)
{
  Serial.print("G91 G0 F6000 ");
  Serial.write(Axis[type%3]);
  if(type > 2)
    Serial.write('-');
  if((type == 2) || (type == 5)) //Z axis
  {
   if(!digitalRead(Shundreds))
      Serial.print("10.0");
    else if(!digitalRead(Stens))
      Serial.print("1.0");
    else if(!digitalRead(Sunits))
      Serial.print("0.1");
    else
      Serial.print("0.01");     
  }
  else
  {
    if(!digitalRead(Shundreds))
      Serial.print("100.0");
    else if(!digitalRead(Stens))
      Serial.print("10.0");
    else if(!digitalRead(Sunits))
      Serial.print("1.0");
    else
      Serial.print("0.1"); 
  } 
  Serial.write('\n');
  delay(200);
}
