

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include "router_control.h"
#include <string.h>

/******************* UI details */
#define BUTTON_X 180//40
#define BUTTON_Y 75//100
#define BUTTON_W 48
#define BUTTON_H 30
#define BUTTON_SPACING_X 6//20
#define BUTTON_SPACING_Y 6//20
#define BUTTON_TEXTSIZE 2

// text box where numbers go
#define TEXT_X 160
#define TEXT_Y 5
#define TEXT_W 150
#define TEXT_H 30
#define TEXT_TSIZE 2
#define TEXT_TCOLOR ILI9341_YELLOW
// the data we store in the textfield
#define TEXT_LEN 12
char textfield[TEXT_LEN+1] = "";
uint8_t textfield_i=0;

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

//Touch For New ILI9341 TP
#define TS_MINX 120
#define TS_MAXX 980//900

#define TS_MINY 70
#define TS_MAXY 920
#define MINPRESSURE 10
#define MAXPRESSURE 1000
// We have a status line for like, is FONA working
#define STATUS_X 170
#define STATUS_Y 45
#define MIDJOG  400
#define SHORTJOG  60
#define LONGJOG 3000
// fence accelleration constants
#define RAMPST 150
 #define RAMPLEN 1000
#define FULLRAMP 1250
//Machine states
#define HOMESTATE 0
#define MANUALSTATE 1
#define JOGSTATE  2
#define ZEROSTATE 3
#define LIMITSTATE 4
#define CHANGESTATE 5
#define APPSTATE  6
#define FINGERSTATE 7
#define FCUTSTATE 8
#define PARKPOS 7000  //70mm

void ListDisplay(int);
void DrawManual(void);
void DrawJogscreen(void);
void ReadManual(void);
void ReadJog(void);
void ShowHeight(void);
void readDRO(void);
void DisplayDRO(void);
void DrawHscreen(void);
void ReadH(void);
void InitDRO(void);
void DrawZeroscreen(void);
void ReadZero(void);
void DrawChscreen(void);
void DrawAppscreen(void);
 void ReadApp(void);
void ReadCh(void);
void DrawFingerscreen(void);
 void ReadFinger(void);
int StepFence(int);
int MoveFence(int);
int setBitHeight(int);
int ZeroFence(void);
void TestDRO(void);
void ReadL(void);
void DrawLscreen(void);
void DrawFCutscreen(void);
void ReadFCut(void);
void Steponemm(void);
void DrawCalc(void);
void ButtonsState(int);
void ClearText(void);
void ErrorMess(int);

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

Elegoo_GFX_Button buttons[19];
/* create 15 buttons, in classic candybar phone style */
char jogbuttons[11][9] = {"Lift+","Lift-","Exit","Fence-","Fence+","10.0","1.0","0.1","Limit","Apps","About"};
char buttonlabels[15][5] = {"Q", "Clr", "<", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "-" };
char hbuttons[6][9] = {"Zero","Man","Apps","Chg","Jog","reset"};
char lbuttons[6][9] = {"Force <","Force >","Exit","Chg","Jog","reset"};
char zbuttons[7][8] = {"Down","Stop","In","Bit","Set","Set","Exit"};
char cbuttons[3][8] = {"Change","Restore","Exit"};
uint16_t buttoncolors[15] = {ILI9341_RED, ILI9341_ORANGE, ILI9341_DARKGREEN, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_ORANGE, ILI9341_BLUE, ILI9341_ORANGE};
Elegoo_GFX_Button DownList;   
char DownListText[5] = "Down";
 Elegoo_GFX_Button UpList;   
char UpListText[5] = "Up";              
  char AppList[20][20] = {"dowel hole ", "box joint  ","dovetail   ","domino     ","dado slot  ","rebate     ","multi-slot ","plunge slot ","reserved1  ","reserved1  " ,"reserved2  " ,"reserved3  " ,"reserved4  " ,"reserved5  " ,"reserved6  " ,"reserved7  " ,"reserved8  " ,"reserved9  " ,"reserved10 " ,"reserved11 " }; 
  int MenuPointer = 0;  
  int16_t tmp;    
  TSPoint p;      
  int MCstate = 0;   
  char str[20];
//  float BitHeight = 0.0; 
  float fHeightTarget;
  float FenceDist = 0.0;    
  int dro_bits[24];
  int DROreading = 0;
  int DROzero = 0;
  int CurrHeight = 0;
  int CurrDir;
  int heightTarget = 0;
  int SpeedD = 0;
  int Tcount = 0;
  int Jogtime = 200;
  long CurrPC = 0;      //distance from zero in stepper pulses
  int JogStep = 100;
  char BitValid = 0;
  /******************** I/O definitions ****************************/
 //#define DriveU 46
 //#define DriveD 44
 #define Bpulse 46
 #define Bdir   44
 #define Sensor 40
  int clk = 50; 
  int data = 52;
  int Fpulse = 48;
  int Fdir = 42;
/************************ app variables ***************************/
float BitSize;
float Width;
float Depth;
float CurrWidth;
char CutStart = 0;
 /************************************************************************************************/ 
void setup(void) {
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));
  pinMode(clk, INPUT_PULLUP);
  pinMode(data, INPUT_PULLUP);
  pinMode(Bpulse, OUTPUT);
  pinMode(Bdir,OUTPUT);
  pinMode(Fpulse, OUTPUT);
  pinMode(Fdir,OUTPUT);
  pinMode(Sensor,INPUT_PULLUP);
  tft.reset();

  uint16_t identifier = tft.readID();
  /*
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else */
  if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    identifier=0x9341;
   
  }
  TestDRO();
 
  tft.begin(identifier);
  DrawHscreen();   //start point
}
/**************************************************************************************************************/
void loop(void) {

  digitalWrite(13, HIGH);
  p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);
 
   if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // scale from 0->1023 to tft.width
      tmp =tft.height() - map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
      p.x = tft.width() - map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
      p.y = tmp;
 //  Serial.print("Pressing:x "); Serial.print(p.x);Serial.print(" y "); Serial.println(p.y);
   }
   else
   {    // if not pressed, these are not valid!
     p.x = 0;
     p.y = 0;
   }
   switch(MCstate)    //which screen to service
   {
      case 0:
        ReadH();
        break;
      case 1:
        ReadManual();
        break;
      case 2:
        ReadJog();
        break;
      case 3:
        ReadZero();
      break;
      case LIMITSTATE:
        ReadL();
        break;
      case CHANGESTATE:
        ReadCh();
        break;
      case APPSTATE:
        ReadApp();
        break;
      case FINGERSTATE:
        ReadFinger();
        break;
      case FCUTSTATE:
        ReadFCut();
        break;

   }
}
/*****************************************************************************************************************
                              HOME SCREEN
*****************************************************************************************************************/
void DrawHscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 44, 120, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[0], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 120, 120, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[1], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 196, 120, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[2], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
     buttons[3].initButton(&tft, 272, 120, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[3], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 44, 170,64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 230, 215,100, 35, ILI9341_WHITE, ILI9341_DARKGREEN, ILI9341_WHITE, hbuttons[5], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    /*
    buttons[6].initButton(&tft, 220, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[6], 1);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 
   */
 
  tft.drawRect(10, 35, 120, 30, ILI9341_WHITE);
  tft.drawRect(170, 35, 120, 30, ILI9341_WHITE);
  tft.setCursor(50, 70);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.print("Bit height");
  tft.setCursor(190, 70);
  tft.print("Fence position");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80, 10);
  tft.print("ROUTE-MASTER");
  tft.setCursor(40, 42);
  tft.print(((float)CurrHeight) / 100.0);
  // ShowHeight();
   tft.setCursor(200, 42);
  tft.print(((float)CurrPC) / 320.0); 
    if(!rDRO) //no DRO found
    {
      tft.setCursor(110,165);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setTextSize(2);
       tft.print("DRO error!");
    }
 // tft.setCursor(200, 12);
//  tft.print("0.0");
}
/******************************************************************************************************************/
 void ReadH(void)
  {
    
    #define HKNUM  6
    for (uint8_t b=0; b<HKNUM; b++)
     {
    if(buttons[b].contains(p.x,p.y))
      buttons[b].press(true);
    else
     buttons[b].press(false);
    }
    for (uint8_t b=0; b<HKNUM; b++)
     {
     if(buttons[b].justReleased())
      buttons[b].drawButton();
     }
     if(p.z) //worth checking for press
     {
       Serial.println("pressed");
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          Serial.println(b);
          switch(b)
            {
              case 0: //zero
                MCstate = 3;
                DrawZeroscreen();
                return;
              case 1:     //manual
                MCstate = 1;
                DrawManual();
                
                return;
              case 2:   //apps
               MCstate = APPSTATE;
               DrawAppscreen();
                return;
              case 3:       //change bit
                MCstate = CHANGESTATE;
                DrawChscreen();
                break;
              case 4:
                MCstate = 2; //
                DrawJogscreen();
                return;
              case 5:
               TestDRO();      //check DRO
               DrawHscreen();  //and redraw the display
                return;
            }
        }
        delay(100);   
     }
  tft.setCursor(40,220);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(1);   
if(digitalRead(Sensor))
  tft.print("No  "); 
  else
  tft.print("Yes"); 
     
/*
    if(rDRO && ((++Tcount & 0x07) == 0))  //not every time- it is slow
    {
      readDRO();
      DisplayDRO();
    }
*/
   // Serial.println("RH end ");
  } 
/*****************************************************************************************************************************
                                            MANUAL SCREEN
*****************************************************************************************************************************/

void DrawManual(void) //MCstate 1
{
  textfield[0] = 0; //empty it
  textfield_i = 0;
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create calc buttons
  for (uint8_t row=0; row<5; row++) {
    for (uint8_t col=0; col<3; col++) {
      buttons[col + row*3].initButton(&tft, BUTTON_X+col*(BUTTON_W+BUTTON_SPACING_X), 
                 BUTTON_Y+row*(BUTTON_H+BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text
                  BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col+row*3], ILI9341_WHITE,
                  buttonlabels[col + row*3], BUTTON_TEXTSIZE); 
      buttons[col + row*3].drawButton();
    }
  }
  buttons[15].initButton(&tft,40,120,65,40,ILI9341_WHITE,ILI9341_BLUE,ILI9341_WHITE,"Bit",2);
  buttons[15].drawButton();
  buttons[16].initButton(&tft,110,120,65,40,ILI9341_WHITE,ILI9341_BLUE,ILI9341_WHITE,"Fence",2);
  buttons[16].drawButton();
  buttons[17].initButton(&tft,75,175,110,40,ILI9341_WHITE,ILI9341_GREEN,ILI9341_WHITE,"GO",2);
  buttons[17].drawButton();
  buttons[18].initButton(&tft,100,220,60,30,ILI9341_WHITE,ILI9341_DARKGREEN,ILI9341_WHITE,"Exit",2);
  buttons[18].drawButton();
  // create 'text field'
  tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);
  tft.drawRect(15, 5, 120, 30, ILI9341_WHITE);
  tft.drawRect(15, 50, 120,30, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(TEXT_TSIZE); 
  tft.setCursor(40, 60);
  tft.print(((float)CurrPC) / 320.0);
  tft.setCursor(40, 15);
 // tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
 // tft.setTextSize(TEXT_TSIZE);
  tft.print(((float)CurrHeight) / 100.0); //white for actual
  tft.setTextSize(1); 
  tft.setCursor(65, 40);
  tft.print("Bit");
  tft.setCursor(60, 81);
  tft.print("Fence");
  //ListDisplay(0);
}
/*******************************************************************************************************************/
void ReadManual(void)
{
  float Disp;
  int Errcode;
  
  // go thru all the buttons, checking if they were pressed
  for (uint8_t b=0; b<19; b++) {
 //   if( p.x != tmp) Serial.print("Error "); Serial.println(b);
    if (buttons[b].contains(p.x, p.y))  {
 //     Serial.print("Buttons "); Serial.print(p.x);Serial.print(" was "); Serial.println(tmp);
      buttons[b].press(true);  // tell the button it is pressed
    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // now we can ask the buttons if their state has changed
  for (uint8_t b=0; b<19; b++) {
    if (buttons[b].justReleased()) {
      // Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
    }
  } 
  if(!p.z)
    return; //cannot be just pressed
  for (uint8_t b=0; b<19; b++) { 
    if (buttons[b].justPressed()) {
        buttons[b].drawButton(true);  // draw invert!
//Serial.print("Bpress ");  Serial.println(b);        
        // if a numberpad button, append the relevant # to the textfield
        if ((b >= 3) && (b < 15)) {
//Serial.print("Kpad "); Serial.print(textfield_i);Serial.print(" butt "); Serial.println(b);
          if (textfield_i < TEXT_LEN) {
            textfield[textfield_i] = buttonlabels[b][0];
            textfield_i++;
	          textfield[textfield_i] = 0; // zero terminate
            
           // fona.playDTMF(buttonlabels[b][0]);
          }
        }
        switch(b)
        {
          case 0: //top left keypad
             MCstate = 0;
            break;
          case 1: //clear key
             textfield[textfield_i] = 0;
             while (textfield_i > 0) 
             {
                textfield_i--;
                textfield[textfield_i] = ' ';
             }  
            break;
          case 2:  //backspace
             textfield[textfield_i] = 0;
             if (textfield > 0) 
             {
              textfield_i--;
              textfield[textfield_i] = ' ';
             }
            break;
            case 15:  //bit transfer
              fHeightTarget = atof(textfield);
              tft.setCursor(40, 15);
              tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
              tft.setTextSize(TEXT_TSIZE);
              tft.print(fHeightTarget);
              tft.print("  ");
              BitValid = 1;
               textfield[textfield_i] = 0;
             while (textfield_i > 0) 
             {
                textfield_i--;
                textfield[textfield_i] = ' ';
             } 
              break;
           case 16: //fence transfer
              FenceDist = atof(textfield);      //target
              tft.setCursor(40, 60);
              tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
              tft.setTextSize(TEXT_TSIZE);
              tft.print(FenceDist);           //target in yellow
              tft.print("  ");
               textfield[textfield_i] = 0;
             while (textfield_i > 0) 
             {
                textfield_i--;
                textfield[textfield_i] = ' ';
             } 
              break;
           case 17: //action 
              if(BitValid)
              {
                heightTarget = (int)(fHeightTarget * 100);
                tft.setCursor(40, 15);
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); //white for actual
                tft.setTextSize(TEXT_TSIZE);
                if(Errcode = setBitHeight(heightTarget))    //move bit
                  {
                      tft.print("error "); 
                      tft.print(Errcode);
                  }
                else                
                  tft.print(((float) CurrHeight)/100.0); //will have been changed to actual
              }
              BitValid = 0;
              tft.setCursor(40, 60);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.setTextSize(TEXT_TSIZE);
              if(StepFence((int)(FenceDist * 100)))
              {
                  tft.print("Limit!");   
                  break;  
              }
              tft.print(((float)CurrPC) / 320.0); //white for actual
              break;
           case 18: //exit screen
              MCstate = 0;
              break;            
        }
        // update the current text field
       // Serial.println(textfield);
        tft.setCursor(TEXT_X + 10, TEXT_Y+10);
        tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
        tft.setTextSize(TEXT_TSIZE);
        tft.print(textfield);

       
      }
    }
   // Serial.print("F ");
    if(MCstate == 0)
       DrawHscreen();
    delay(100); // UI debouncing
  }
/**************************************************************************************************************************************************
                                            JOG SCREEN
***************************************************************************************************************************************************/
void DrawJogscreen(void)    //MCstate 2
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 65, 80,70, 55, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, jogbuttons[0], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 65, 150,70, 55, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, jogbuttons[1], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 70, 210,90, 30, ILI9341_WHITE, ILI9341_OLIVE, ILI9341_WHITE, jogbuttons[2], 1);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
     buttons[3].initButton(&tft, 170, 80,77, 55, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, jogbuttons[3], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 260, 80,77, 55, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, jogbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 160, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[5], 1);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    buttons[6].initButton(&tft, 220, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[6], 1);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 
    buttons[7].initButton(&tft, 280, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[7], 1);  // x, y, w, h, outline, fill, text
    buttons[7].drawButton(); 
    buttons[8].initButton(&tft, 160, 195,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[8], 1);  // x, y, w, h, outline, fill, text
    buttons[8].drawButton(); 
    buttons[9].initButton(&tft, 220, 195,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[9], 1);  // x, y, w, h, outline, fill, text
    buttons[9].drawButton(); 
    buttons[10].initButton(&tft, 280, 195,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[10], 1);  // x, y, w, h, outline, fill, text
    buttons[10].drawButton();  
 
  tft.drawRect(10, 5, 100, 30, ILI9341_WHITE);
  tft.drawRect(195, 5, 100, 30, ILI9341_WHITE);
 // tft.setCursor(30, 12);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
   ShowHeight();
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(130, 12);
  tft.print("1.0 ");
   tft.setCursor(210, 12);
  tft.print(((float) CurrPC)/320.0);
  Jogtime = MIDJOG;
  JogStep = 320;
   tft.drawFastVLine(115,55,120,ILI9341_WHITE);
}
/*
// Print something in the mini status bar with either flashstring
void status(const __FlashStringHelper *msg) {
  tft.fillRect(STATUS_X, STATUS_Y, 240, 8, ILI9341_BLACK);
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(msg);
  }
*/

/**************************************************************************************************************************************************/
  void ReadJog(void)
  {
    
    #define HOMEKNUM  11
    float DispPos = 0.0;
    for (uint8_t b=0; b<HOMEKNUM; b++)
     {
    if(buttons[b].contains(p.x,p.y))
      buttons[b].press(true);
    else
     buttons[b].press(false);
    }
    for (uint8_t b=0; b<HOMEKNUM; b++)
     {
     if(buttons[b].justReleased())
      buttons[b].drawButton();
     }
     if(p.z) //worth checking for press
     {
//       Serial.println("pressed");
      for (uint8_t b=0; b<HOMEKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
//          Serial.println(b);
          switch(b)
            {
              case 0: //bit up
              //  analogWrite(DriveD,0);
                digitalWrite(Bdir,1);
                analogWrite(Bpulse,230);
                delay(Jogtime);
                analogWrite(Bpulse, 0);
                break;
              case 1: //bit down
           //     analogWrite(DriveU,0);
                digitalWrite(Bdir,0);
                analogWrite(Bpulse,180);
                delay(Jogtime);
                analogWrite(Bpulse, 0);
                break;
              case 2: //exit
                MCstate = 0;
                DrawHscreen();
                break;
             
              case 3: //fence in
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setTextSize(2);
                tft.setCursor(210, 12);
                if(MoveFence(-JogStep))
                {
                  tft.print("Limit!");   
                  break;
                }        
                DispPos = (float(CurrPC)) / 320;
                tft.print(DispPos);
                break;
               case 4:     //fence out 
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setTextSize(2);
                tft.setCursor(210, 12);   
                if(MoveFence(JogStep))
                {
                  tft.print("Limit!");   
                  break;
                }     
                DispPos = (float(CurrPC)) / 320;
                tft.print(DispPos);         
                return;
              case 5:
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setTextSize(2);
                tft.setCursor(130, 12);
                tft.print("10.0");
                Jogtime = LONGJOG;
                JogStep = 3200;
                break;
              case 6:
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setTextSize(2);
                tft.setCursor(130, 12);
                tft.print("1.0 ");
                Jogtime = MIDJOG;
                JogStep = 320;
                break;
              case 7:
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setTextSize(2);
                tft.setCursor(130, 12);
                tft.print("0.1 ");
                Jogtime = SHORTJOG;
                JogStep = 32;
                break;
              case 9:
               // MCstate = 1;
              //  Serial.println("case 9 ");
              //  setBitHeight(heightTarget);
              
               // DrawManual();
                return;
              case 8:   //escape from limit condition
                MCstate = LIMITSTATE;
                DrawLscreen();
                return;
              case 10:
                MCstate = 0;
                DrawHscreen();
                break;            
            }
        }
        delay(100);
     }   
    if(rDRO && ((++Tcount & 0x07) == 0))  //not every time- it is slow
    {
      readDRO();
      DisplayDRO();
    }
  }
  /*
  void ShowHeight(void)
  {
  tft.setCursor(40, 12);
  if(((heightTarget + 10) > CurrHeight) || ((heightTarget - 10) < CurrHeight))
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  else
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
 
  sprintf(str,"%4i ",heightTarget);//,BitHeight);
  tft.print(str);
  }
  void DisplayDRO(void)
  {
    static int readingCopy = 69; // make it print first time...
    if(CurrHeight != readingCopy)
    {
      tft.setCursor(160, 12);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(2);
      sprintf(str,"%4i ",CurrHeight);//,BitHeight);
      tft.print(str);
      readingCopy = CurrHeight;
    }
  }
void ListDisplay(int cmd)
{
  tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
  tft.setTextSize(TEXT_TSIZE);
  for(int i;i<6;i++)
  {
  tft.setCursor(5, 40 + 30 * i);   
        tft.print(AppList[i + cmd]);
        if((i + cmd) >= 19)
          break;
  }
}
*/
/**************************************************************************************************************************************************
                                            ZERO SCREEN
***************************************************************************************************************************************************/
void DrawZeroscreen(void)    //MCstate 
{ 
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 44, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, zbuttons[0], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 120, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, zbuttons[1], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 196, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, zbuttons[2], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
     buttons[3].initButton(&tft, 272, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, zbuttons[3], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 82, 160,80, 50, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, zbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 233, 160,80, 50, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, zbuttons[5], 2);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    buttons[6].initButton(&tft, 160, 220,80, 30, ILI9341_WHITE, ILI9341_OLIVE, ILI9341_WHITE, zbuttons[6], 2);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 

 // tft.setCursor(30, 12);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(140, 12);
  tft.print("ZERO");
   tft.setCursor(70, 50);
  tft.print("Bit");
   tft.setCursor(195, 50);
  tft.print("Fence");
  tft.drawRect(10, 5, 80, 30, ILI9341_WHITE);
  tft.drawRect(235, 5, 80, 30, ILI9341_WHITE);
 // tft.setCursor(30, 12);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(15, 12);
  tft.setTextSize(2);
  tft.print(((float) CurrHeight)/100.0); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
   tft.setCursor(240, 12);
  tft.print((float(CurrPC)) / 320.0);
  tft.drawFastVLine(159,40,160,ILI9341_WHITE); 
}

/**************************************************************************************************************************************************/
  void ReadZero(void)
  {
    
    #define HOMEKNUM  7
    for (uint8_t b=0; b<HOMEKNUM; b++)
     {
    if(buttons[b].contains(p.x,p.y))
      buttons[b].press(true);
    else
     buttons[b].press(false);
    }
    for (uint8_t b=0; b<HOMEKNUM; b++)
     {
     if(buttons[b].justReleased())
      buttons[b].drawButton();
     }
     if(p.z) //worth checking for press
     {
//       Serial.println("pressed");
      for (uint8_t b=0; b<HOMEKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
//          Serial.println(b);
          switch(b)
            {
              case 0: //bit down
                digitalWrite(Bdir,0);
                analogWrite(Bpulse,150);
                break;
              case 1: //bit stop
                analogWrite(Bpulse, 0);
               // analogWrite(DriveU, 0);
                break;
              case 2: //fence step
                MoveFence(-320);  //move in 1mm
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setCursor(240, 12);
                tft.print((float(CurrPC)) / 320.0);
                break;
             
              case 3: //fence bit set
                digitalWrite(Bdir,1);
             //    analogWrite(DriveD,0);  //raise bit to give contact
                analogWrite(Bpulse,210);
                delay(LONGJOG);
                analogWrite(Bpulse, 0);
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                tft.setCursor(15, 12);
                tft.setTextSize(2);
                tft.print(((float) CurrHeight)/100.0); 
                break;
               case 4:    //zero bit
                digitalWrite(Bdir,1);
                analogWrite(Bpulse, 240);
                 while(digitalRead(Sensor)) //wait for a touch
                  ;
                analogWrite(Bpulse, 0);
                InitDRO();
                 tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                  tft.setCursor(15, 12);
                  tft.setTextSize(2);
                  tft.print(((float) CurrHeight)/100.0); 
                return;
              case 5: //zero fence 
               
                 tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                  tft.setTextSize(2);
                  tft.setCursor(240, 12);
                if(ZeroFence())
                {                                                   //moved 10mm without finding zero- or limit switch.
                  tft.print("Error ");
                }
                else
                {
                  tft.print("0.00   ");
                  delay(2000); //time to move sensor
                  setBitHeight(0);  //return bit
                  CurrPC = 0;
                }
                break;
              case 6:   //exit
                MCstate = 0;
                DrawHscreen();
                break;    
            }
        }
        delay(100);
     }   
  }
  /*****************************************************************************************************************
                              LIMIT SCREEN
*****************************************************************************************************************/
void DrawLscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 90, 100, 100, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, lbuttons[0], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 230, 100, 100, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, lbuttons[1], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 196, 190, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, lbuttons[2], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
      /*
     buttons[3].initButton(&tft, 272, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[3], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 44, 150,64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 230, 215,100, 35, ILI9341_WHITE, ILI9341_DARKGREEN, ILI9341_WHITE, hbuttons[5], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    buttons[6].initButton(&tft, 220, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[6], 1);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 
   */
 

  tft.setCursor(80, 15);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("Limit escape");
}
/******************************************************************************************************************/
 void ReadL(void)
  {
    
    #define HKNUM  3
    for (uint8_t b=0; b<HKNUM; b++)
     {
    if(buttons[b].contains(p.x,p.y))
      buttons[b].press(true);
    else
     buttons[b].press(false);
    }
    for (uint8_t b=0; b<HKNUM; b++)
     {
     if(buttons[b].justReleased())
      buttons[b].drawButton();
     }
     if(p.z) //worth checking for press
     {
       Serial.println("pressed");
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          Serial.println(b);
          switch(b)
            {
              case 0: //force in
                digitalWrite(Fdir, 1);
                Steponemm();
                break;
              case 1:     //force out
                digitalWrite(Fdir, 0);
                Steponemm();
                break;
              case 2:   //exit
                MCstate = JOGSTATE;
                DrawJogscreen();
                return;
              case 3:       //change bit
      
                break;
              case 4:
               break;
              case 5:
               break;
            }
        }
        delay(50);   
     }
  tft.setCursor(40,190);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(2);   
if(digitalRead(Sensor))
  tft.print("No  "); 
  else
  tft.print("Yes"); 
     
/*
    if(rDRO && ((++Tcount & 0x07) == 0))  //not every time- it is slow
    {
      readDRO();
      DisplayDRO();
    }
*/
   // Serial.println("RH end ");
  } 

/*****************************************************************************************************************

                              CHANGE SCREEN
*****************************************************************************************************************/
void DrawChscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 90, 100, 100, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, cbuttons[0], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 230, 100, 100, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, cbuttons[1], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 196, 190, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, cbuttons[2], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
      /*
     buttons[3].initButton(&tft, 272, 100, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[3], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 44, 150,64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 230, 215,100, 35, ILI9341_WHITE, ILI9341_DARKGREEN, ILI9341_WHITE, hbuttons[5], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    buttons[6].initButton(&tft, 220, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[6], 1);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 
   */
 

  tft.setCursor(90, 15);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("Change bit");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
   tft.setCursor(85, 40);
    tft.print("Release lock!");
}
/******************************************************************************************************************/
 void ReadCh(void)
  {
    static long SavePos = CurrPC;
    
    #define HKNUM  3
     ButtonsState(HKNUM);
     if(p.z) //worth checking for press
     {
       Serial.println("pressed");
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          Serial.println(b);
          switch(b)
            {
              case 0: //raise bit to max
                SavePos = CurrPC; //remember
                StepFence(PARKPOS); //move fence out of the way
             
              digitalWrite(Bdir,1);
                 analogWrite(Bpulse,230);   
                while(digitalRead(Sensor))
                ;
                 analogWrite(Bpulse,0);   
                break;
              case 1:     //return
             //   analogWrite(DriveU,0);
             digitalWrite(Bdir,0);
                analogWrite(Bpulse,240);
                delay(7000);
                analogWrite(Bpulse, 0);
                StepFence(SavePos); //return the fence
                break;
              case 2:   //exit
                MCstate = HOMESTATE;
                DrawHscreen();
                return;
              case 3:      
      
                break;
              case 4:
               break;
              case 5:
               break;
            }
        }
        delay(50);   
     }

  tft.setCursor(40,190);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(2);   
if(!digitalRead(Sensor))
  tft.print("Up  "); 
  }
  
/*****************************************************************************************************************
                              APP SCREEN
*****************************************************************************************************************/
void DrawAppscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  // create buttons
      buttons[0].initButton(&tft, 160, 40, 300, 40, ILI9341_BLACK, ILI9341_BLACK, ILI9341_WHITE,"Finger joint", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 160, 80, 300, 40, ILI9341_BLACK, ILI9341_BLACK,  ILI9341_WHITE, "Channel", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 160, 120, 300, 40, ILI9341_BLACK, ILI9341_BLACK,  ILI9341_WHITE, "Dovetail", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
     buttons[3].initButton(&tft, 272, 190, 64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Exit", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      /*
      buttons[4].initButton(&tft, 44, 150,64, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, hbuttons[4], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton();
    buttons[5].initButton(&tft, 230, 215,100, 35, ILI9341_WHITE, ILI9341_DARKGREEN, ILI9341_WHITE, hbuttons[5], BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
    buttons[5].drawButton(); 
    buttons[6].initButton(&tft, 220, 155,50, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, jogbuttons[6], 1);  // x, y, w, h, outline, fill, text
    buttons[6].drawButton(); 
   */
 

  tft.setCursor(90, 10);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("Applications");
  
}
/******************************************************************************************************************/
 void ReadApp(void)
  {
    
    #define HKNUM  4
     ButtonsState(HKNUM);
  
     if(p.z) //worth checking for press
     {
       Serial.println("pressed");
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          Serial.println(b);
          switch(b)
            {
              case 0: //
                MCstate = FINGERSTATE;
                DrawFingerscreen();
                break;
              case 1:     //return
            
                break;
              case 2:

                break;
              case 3:   //exit
                MCstate = HOMESTATE;
                DrawHscreen();
                return;
              
              case 4:
               break;
              case 5:
               break;
            }
        }
        delay(50);   
     }

  tft.setCursor(40,190);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(2);   
if(!digitalRead(Sensor))
  tft.print("Up  "); 
  }
   
/*****************************************************************************************************************
                              FINGER SCREEN
*****************************************************************************************************************/
void DrawFingerscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
  DrawCalc();
  /*
      buttons[0].initButton(&tft, 160, 40, 300, 40, ILI9341_BLACK, ILI9341_BLACK, ILI9341_WHITE,"Finger joint", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
     buttons[1].initButton(&tft, 160, 80, 300, 40, ILI9341_BLACK, ILI9341_BLACK,  ILI9341_WHITE, "Channel", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
    buttons[2].initButton(&tft, 160, 120, 300, 40, ILI9341_BLACK, ILI9341_BLACK,  ILI9341_WHITE, "Dovetail", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
   */   
     buttons[15].initButton(&tft, 80, 45, 100, 30, ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "Bit size", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[15].drawButton(); 
      
      buttons[16].initButton(&tft, 80, 105,100, 30, ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "Width", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[16].drawButton();
    buttons[17].initButton(&tft, 80, 165,100, 30, ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "Depth", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
    buttons[17].drawButton(); 
    
    buttons[18].initButton(&tft, 80, 220,100, 30, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, "GO", 2);  // x, y, w, h, outline, fill, text
    buttons[18].drawButton(); 
   
 

  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("Finger joint");
  Width = 0.0;
  Depth = 0.0;
  BitSize = 0.0;
  
}
/******************************************************************************************************************/
 void ReadFinger(void)
  {
    static char valid = 0;
    #define HKNUM  19
    ButtonsState(HKNUM);
   
     if(p.z) //worth checking for press
     {
       
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          Serial.println(b);
          if ((b >= 3) && (b < 15)) 
          {
            if (textfield_i < TEXT_LEN) 
            {
              textfield[textfield_i] = buttonlabels[b][0];
              textfield_i++;
              textfield[textfield_i] = 0; // zero terminate
              valid = 1;
            // fona.playDTMF(buttonlabels[b][0]);
            }
          }
          Serial.print("F ");
        switch(b)
        {
          
          case 0: //top left keypad
            MCstate = HOMESTATE;
             DrawHscreen();
             return;
          case 1: //clear key
             ClearText();
             valid = 1;
            break;
          case 2:  //backspace
             textfield[textfield_i] = 0;
             if (textfield > 0) 
             {
              textfield_i--;
              textfield[textfield_i] = ' ';
             }
             valid = 1;
            break;
           case 15:
              BitSize = atof(textfield);
              tft.setCursor(50, 65);
              tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
              tft.setTextSize(2);
              tft.print(BitSize);
              ClearText();
             valid = 1;
           break;
           case 16:
               Width = atof(textfield);
              tft.setCursor(50, 125);
              tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
              tft.setTextSize(2);
              tft.print(Width);
              ClearText();
             valid = 1;
           break;
           case 17:
               Depth = atof(textfield);
              tft.setCursor(50, 185);
              tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
              tft.setTextSize(2);
              tft.print(Depth);
              ClearText();
             valid = 1;

           break; 
           case 18:
              if(!BitSize)
                {
                ErrorMess(1);
                break;
                }
              if(!Width)
                {
                ErrorMess(1);
                break;
                }
              if(!Depth)
                {
                ErrorMess(1);
                break;
                }
              if(BitSize > Width)
              {
                ErrorMess(2);
                break;
              }
              MCstate = FCUTSTATE;
              DrawFCutscreen();

            break;
        }
        delay(50);   
     }
    }
    if(valid)
    {
       tft.setCursor(TEXT_X + 10, TEXT_Y+10);
        tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
        tft.setTextSize(TEXT_TSIZE);
        tft.print(textfield);
        valid = 0;
        ErrorMess(0);
     }
  }

     
/*****************************************************************************************************************
                              FINGER SCREEN
*****************************************************************************************************************/
void DrawFCutscreen(void)
{
  
  tft.setRotation(3); //2 is original
  tft.fillScreen(BLACK);
 
  
      buttons[0].initButton(&tft, 50, 100, 90, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE,"Start", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[0].drawButton();
      buttons[1].initButton(&tft, 50, 150, 90, 40, ILI9341_WHITE, ILI9341_BLUE,  ILI9341_WHITE, "Next", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[1].drawButton(); 
      buttons[2].initButton(&tft, 180, 60, 80, 40, ILI9341_WHITE, ILI9341_BLUE,  ILI9341_WHITE, "Mark", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[2].drawButton(); 
      buttons[3].initButton(&tft, 280, 60, 80, 40, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Space", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[3].drawButton(); 
      buttons[4].initButton(&tft, 280, 180, 60, 40, ILI9341_WHITE, ILI9341_GREEN, ILI9341_WHITE, "Exit", BUTTON_TEXTSIZE);  // x, y, w, h, outline, fill, text
      buttons[4].drawButton(); 
 
  tft.setTextSize(2);
  tft.setCursor(70, 10);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.print("Finger cut");
  tft.setCursor(30, 40);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if(CutStart)
    tft.print("Mark   ");
  else
    tft.print("Space");
  
  
}
/******************************************************************************************************************/
 void ReadFCut(void)
  {
    static int count,partwo;
    #define HKNUM  5
    ButtonsState(HKNUM);
   
     if(p.z) //worth checking for press
     {
       
      for (uint8_t b=0; b<HKNUM; b++)
        if(buttons[b].justPressed())
          {
          buttons[b].drawButton(true);
          switch(b)
          {     
          case 0: //start
            count = 1;
            partwo = 0;
             tft.setTextSize(2);
            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            tft.setCursor(140, 180);
              tft.print("       "); //not ready
            if(CutStart)    //starting with a mark
              CurrWidth = Width + BitSize;
            else
              CurrWidth = BitSize;
            tft.setCursor(140, 100);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.print("No. ");
              tft.print(count);
              if(Width > BitSize)
              {
                tft.print(" Part 1 ");
                partwo = 1;
              }
              else
              {
                tft.print("       ");
                partwo = 0;
              }
               heightTarget = (int)(Depth * 100);           //bit first
                if(setBitHeight(heightTarget))    //move bit
                  {
                      tft.setTextSize(2);
                      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
                      tft.setCursor(10, 180);
                      tft.print("Bit set error! "); 
                      break;
                  }
              StepFence((int)(CurrWidth * 100.0));
             // delay(2000);// move stuff
              tft.setCursor(140, 180);
              tft.print("READY ");
             break;
          case 1: //next
             tft.setTextSize(2);
            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            tft.setCursor(140, 180);
              tft.print("       "); //not ready
            if(partwo) //need intrim cut
            {
              CurrWidth += Width;
              CurrWidth -= BitSize;
              partwo = 0;
              tft.setCursor(200, 100);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.print(" Part 2 ");
            }
            else
            {
              CurrWidth += (Width + BitSize); //to next slot
              tft.setCursor(140, 100);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.print("No. ");
              tft.print(++count);
              if(Width > BitSize)
              {
                partwo = 1;
                tft.print(" Part 1 ");
              }
              else
                tft.print("       ");
            }
            StepFence((int)(CurrWidth * 100.0));//move fence
            tft.setCursor(140, 180);
              tft.print("READY ");
            break;
          case 2:  //mark
             CutStart = 1;
              tft.setCursor(10, 40);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.print("Mark   ");
              break;
           case 3://space
              CutStart = 0;
              tft.setCursor(10, 40);
              tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
              tft.print("Space");
           break;
           case 4://exit
               MCstate = 0;
               DrawHscreen();
               return;
          }
        delay(50);   
      }
    }
   
  } 
  /****************************************************************************************/
  void ShowHeight(void)
  {
  tft.setCursor(40, 12);
  if(((heightTarget + 10) > CurrHeight) || ((heightTarget - 10) < CurrHeight))
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  else
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
 
  sprintf(str,"%4i ",heightTarget);//,BitHeight);
  tft.print(str);
  }
 
void ListDisplay(int cmd)
{
  tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
  tft.setTextSize(TEXT_TSIZE);
  for(int i;i<6;i++)
  {
  tft.setCursor(5, 40 + 30 * i);   
        tft.print(AppList[i + cmd]);
        if((i + cmd) >= 19)
          break;
  }
}
/******************************** DRO stuff **********************************************/
void InitDRO(void)
{
  readDRO();
  DROzero = DROreading;
  CurrHeight = 0;
}
void TestDRO(void)
{
   for(uint8_t i = 0; i< 10;i++)
  {
    if(!digitalRead(data)) // it is a low- so something connected
    {
        rDRO = 1;
        InitDRO();
        break;
    }
    delay(1);
  }
}
 void DisplayDRO(void)
  {
    static int readingCopy = 69; // make it print first time...
    if(CurrHeight != readingCopy)
    {
      tft.setCursor(30, 12);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(2);
      
      tft.print(((float) CurrHeight)/100.0);
      tft.print("  ");
      readingCopy = CurrHeight;
    }
  }
  void readDRO(void)
{
    DROreading = 0;
    unsigned long syncTimer = 0;
    bool syncronized = false;
    int Sign;

    while(syncronized == false)
    {
      syncTimer = millis();
      while (digitalRead(clk) == LOW) {} //wait for start
      if(millis() - syncTimer > 5) 
        syncronized = true;  // we have the first clock high
      else
        syncronized = false;
    }
    // ready to read
    for(int i = 0; i < 21;i++)
    {
      while(digitalRead(clk) == HIGH){}  //wait for fall
      if(i < 15)
        DROreading += (digitalRead(data)) << i;
     if(i == 20)
      Sign = digitalRead(data);
      while(digitalRead(clk) == LOW) {}
    }
    if(!Sign)
      DROreading = -DROreading;
    CurrHeight = DROreading - DROzero;// requires zero to be done!
}
/***************************************** BIT ROUTINES ******************************************/
int setBitHeight(int target) //goto
{
  long Timer;
  if(!rDRO)
    return(1);
  Timer = millis();
  readDRO();
   if((CurrHeight > (target - 8)) && (CurrHeight < (target + 8))) //near enough
    return;                             //double press, etc
  if(CurrHeight > target)               //too high
  {
    digitalWrite(Bdir,0);
    analogWrite(Bpulse,160);
    while (CurrHeight > (target-150)) 
    { 
      readDRO();
       if(Timer < (millis() - 9000))
          {
            analogWrite(Bpulse,0);
            return(2);
          } 
    } //drop below 
    analogWrite(Bpulse,0);
  }
  digitalWrite(Bdir,1);
  delay(50); //let it stop, and relay change
  analogWrite(Bpulse,230);                        // fast to start
  while(CurrHeight < (target - 150))
   { 
        readDRO();
        if(Timer < (millis() - 9000))
          {
            analogWrite(Bpulse,0);
            return(3);
          }
    }
  analogWrite(Bpulse,200);
  while(CurrHeight < (target - 15)) 
    {
      readDRO(); 
      if(Timer < (millis() - 6000))
          {
            analogWrite(Bpulse,0);
            return(4);
          }
    }
  analogWrite(Bpulse,0);
 
  return(0);
}
  /*
 // Serial.print("S ");
   if(UpList.contains(p.x,p.y))
      UpList.press(true);
    else
     UpList.press(false);
    if(UpList.justReleased())
      UpList.drawButton();
    if(UpList.justPressed())
    {
      UpList.drawButton(true);
      if(MenuPointer < 13)
        MenuPointer++;
        ListDisplay(MenuPointer);
    }
   if(DownList.contains(p.x,p.y))
      DownList.press(true);
    else
     DownList.press(false);
    if(DownList.justReleased())
      DownList.drawButton();
    if(DownList.justPressed())
    {
      DownList.drawButton(true);
      if(MenuPointer)
        MenuPointer--;
        ListDisplay(MenuPointer);
    } 
    */
    /********************************* FENCE ROUTINES *********************************************/
    int StepFence(int movto)   //move to absolue position, in 0.01mm units
    {
   
      char Dir = 0;
      long NewPC,Move,i;
      long Pause = RAMPST;
      NewPC = (((long)movto) *16)/5; //absolute step position
      Move = NewPC - CurrPC;
      if(Move == 0)                   //avoid!
        return(0);                    //but valid
      if(Move < 0)
      {
        Dir = 1;
        Move = -Move;
      }
      digitalWrite(Fdir, Dir);
      for(i = 0; i < Move; i++)
      {
        Pause = RAMPST;
        digitalWrite(Fpulse, 0);
        if(i < RAMPLEN)
        {
          Pause = FULLRAMP - i;
          Pause >>= 1;
        }
        if((Move - i) < RAMPLEN)
        {
          Pause = FULLRAMP - (Move - i);
          Pause >>= 1;
        }
        digitalWrite(Fpulse, 1);
        delayMicroseconds(Pause);
        if(!digitalRead(Sensor))  //limit switch or zero true
        {
          if(Dir)         // adjust for move so far
            CurrPC -= i;
          else
            CurrPC += i;
          return(1);
        }
      }
      CurrPC = NewPC;      //where I am     
      return(0);      
    }
    int MoveFence(int dist)    //relative move in internal steps - limited to 32000
    {
      char Dir = 0;
      int i;
      int Pause = RAMPST;
     
      if(dist == 0)                   //avoid!
        return;
      CurrPC += (long)dist; // set it before it gets modified...
      if(dist < 0)
      {
        Dir = 1;
        dist = -dist;
      }
      digitalWrite(Fdir, Dir);
      for(i = 0; i < dist; i++)
      {
        Pause = RAMPST;
        digitalWrite(Fpulse, 0);
        if(i < RAMPLEN)
        {
          Pause = FULLRAMP - i;
          Pause >>= 1;
        }
        if((dist - i) < RAMPLEN)
        {
          Pause = FULLRAMP - (dist - i);
          Pause >>= 1;
        }
        digitalWrite(Fpulse, 1);
        delayMicroseconds(Pause);
        if(!digitalRead(Sensor))  //limit switch or zero true
        {
          if(Dir)
            CurrPC += (dist -i);
          else
            CurrPC -= (dist - i);
            return(1);
        }
      }    
    return(0);
    }
int ZeroFence(void)
{
   int i;
   digitalWrite(Fdir, 0); //always out
   for(i = 0;i<3200; i++)
   {
     if(digitalRead(Sensor)) //until we get zero- or limit sw
     {
        digitalWrite(Fpulse, 0);
        delayMicroseconds(RAMPST);
        digitalWrite(Fpulse, 1);
        delayMicroseconds(1000);
     }
      else
        return(0);      //found it
   }
   return(1);           // out of range- or error
}
void Steponemm(void)  //direction already set
{
  int i;
  for(i = 0; i<320; i++)
  {
        digitalWrite(Fpulse, 0);
        delayMicroseconds(RAMPST);
        digitalWrite(Fpulse, 1);
        delayMicroseconds(1000);
     }
}
void DrawCalc(void)
{
   // create calc buttons
  for (uint8_t row=0; row<5; row++) {
    for (uint8_t col=0; col<3; col++) {
      buttons[col + row*3].initButton(&tft, BUTTON_X+col*(BUTTON_W+BUTTON_SPACING_X), 
                 BUTTON_Y+row*(BUTTON_H+BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text
                  BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col+row*3], ILI9341_WHITE,
                  buttonlabels[col + row*3], BUTTON_TEXTSIZE); 
      buttons[col + row*3].drawButton();
    }
  }
   tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);
}
void ButtonsState(int Num)
{
   for (uint8_t b=0; b<Num; b++)
     {
      if(buttons[b].contains(p.x,p.y))
        buttons[b].press(true);
      else
        buttons[b].press(false);
    }
    for (uint8_t b=0; b<Num; b++)
     {
      if(buttons[b].justReleased())
        buttons[b].drawButton();
     }
}
void ClearText(void)
{
   textfield[textfield_i] = 0;
             while (textfield_i > 0) 
             {
                textfield_i--;
                textfield[textfield_i] = ' ';
             }  
}
void ErrorMess(int Mess)
{
    tft.setCursor(180, 40);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setTextSize(1);
      switch(Mess)
      {
        case 0:
        tft.print("               ");
        break;
        case 1:
          tft.print("invalid value");
        break;
        case 2:
          tft.print("Invalid tool");
          break;
      }

}