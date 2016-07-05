//Brent Graham, 2016

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include "RTClib.h"
#define PIN 6

//IF CLOCK TIME IS WRONG: RUN FILE->EXAMPLE->DS1307RTC->SET TIME

RTC_DS3231 rtc;

// matrix of 11 - 16 bit ints (shorts) for displaying the leds
uint16_t mask[11];

/* notes for my arduino wiring:
 * pin 2: top button (set)
 * pin 3: middle button (up)
 * pin 4: bottom button (down)
 * pin 6: LED strand data
 * pin A0: photosensor
 * pin A4: RTC Module SDA
 * pin A5: RTC Module SCL
 */

  int buttonSet=2;
  int buttonUp=3;
  int buttonDown=4;
  int photoResistor=A0;
  int photoRead;
  int dimmer=1;

  //////////////////////////////////SET COLORS/////////////////////////////

  //WORDCLOCK COLORS
  int wordred   = 80;
  int wordblue  = 255;
  int wordgreen = 160;

  //DIGIT COLORS (HOURS)
  int hourred   = 130;
  int hourblue  = 130;
  int hourgreen = 130;
  //DIGIT COLORS (MINUTES)
  int minred    = 250;
  int minblue   = 250;
  int mingreen  = 250;

  //DATE COLORS (MONTH)
  int monthred   = 125;
  int monthblue  = 25;
  int monthgreen = 50;
  //DATE COLORS (DAY)
  int dayred    = 250;
  int dayblue   = 50;
  int daygreen  = 100;

  //DIMABLE (once photoresistor is programmed)
  int dimred=wordred/dimmer;
  int dimblue=wordblue/dimmer;
  int dimgreen=wordgreen/dimmer;
  
//////////////////////////////////////////////////////////////////////////////

  int mytimemonth;
  int mytimeday;
  int mytimehr;
  int mytimemin;
  int mytimesec;
  
  int j; //an integer for the color shifting effect

  int mode = 0;
  int lastState = LOW;   // the previous reading from the input pin
  int buttonState;
  // the following variables are long's because the time, measured in miliseconds,
  // will quickly become a bigger number than can be stored in an int.
  long lastDebounceTime = 0;  // the last time the output pin was toggled
  long debounceDelay = 50;    // the debounce time; increase if the output flickers
  
#define WORD_MODE 0
#define DIGIT_MODE 1
#define DATE_MODE 2
#define BDAY_MODE 3
#define MAX_MODE 4

#define phraseITS        mask[0]  |= 0xE000
#define phraseA          mask[0]  |= 0x800
#define phraseFIVE       mask[2]  |= 0x1E0
#define phraseTEN        mask[1]  |= 0x7000
#define phraseQUARTER    mask[1]  |= 0xFE0
#define phraseTWENTY     mask[2]  |= 0xFC00
#define phraseHALF       mask[0]  |= 0x3C0
#define phrasePAST       mask[3]  |= 0x3C0
#define phraseTIL        mask[3]  |= 0x1C00
#define hourONE          mask[9]  |= 0xE000
#define hourTWO          mask[7]  |= 0xE0
#define hourTHREE        mask[9]  |= 0x3E0
#define hourFOUR         mask[10] |= 0xF000
#define hourFIVE         mask[7]  |= 0xF000
#define hourSIX          mask[9]  |= 0x1C00
#define hourSEVEN        mask[4]  |= 0x7C00
#define hourEIGHT        mask[8]  |= 0x3E0
#define hourNINE         mask[7]  |= 0xF00
#define hourTEN          mask[6]  |= 0xE0
#define hourELEVEN       mask[8]  |= 0xFC00
#define hourNOON         mask[4]  |= 0x1E0
#define hourMIDNIGHT     mask[6]  |= 0xFF00
#define phraseOCLOCK     mask[10] |= 0x7E0
#define phraseHAPPY      mask[0]  |= 0x200, mask[1]  |= 0x200, mask[2]  |= 0x200, mask[3]  |= 0x200, mask[4]  |= 0x200
#define phraseBIRTHDAY   mask[5]  |= 0x7DC0

typedef uint8_t Character[7];
const Character charmap[] = {
  {
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01110
  },
  {
    0b00100,
    0b01100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b01110
  },
  {
    0b01110,
    0b10001,
    0b00001,
    0b00010,
    0b00100,
    0b01000,
    0b11111
  },
  {
    0b11111,
    0b00010,
    0b00100,
    0b00010,
    0b00001,
    0b10001,
    0b01110
  },
  {
    0b00010,
    0b00110,
    0b01010,
    0b10010,
    0b11111,
    0b00010,
    0b00010
  },
  {
    0b11111,
    0b10000,
    0b11110,
    0b00001,
    0b00001,
    0b10001,
    0b01110
  },
  {
    0b00110,
    0b01000,
    0b10000,
    0b11110,
    0b10001,
    0b10001,
    0b01110
  },
  {
    0b11111,
    0b00001,
    0b00010,
    0b00100,
    0b01000,
    0b01000,
    0b01000
  },
  {
    0b01110,
    0b10001,
    0b10001,
    0b01110,
    0b10001,
    0b10001,
    0b01110
  },
  {
    0b01110,
    0b10001,
    0b10001,
    0b01111,
    0b00001,
    0b00010,
    0b01100
  }
};

// define pins
#define NEOPIN 6

// define delays
#define FLASHDELAY 500  // delay for startup "flashWords" sequence
#define SHIFTDELAY 100   // controls color shifting speed

// Parameter 1 = number of pixels in matrix
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel matrix = Adafruit_NeoPixel(64, NEOPIN, NEO_GRB + NEO_KHZ800);

// configure for 11x11 neopixel matrix
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(11, 11, NEOPIN,
      NEO_MATRIX_TOP  + NEO_MATRIX_LEFT +
      NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
      NEO_GRB         + NEO_KHZ800);

void rainbowCycle(uint8_t wait);
void flashWords(void);
void pickAPixel(uint8_t x, uint8_t y);

void setup() {
   matrix.begin();
   Serial.begin(9600);  //Begin serial communcation (for photoresistor to display on serial monitor)
   
   pinMode(buttonSet, INPUT_PULLUP);
   pinMode(buttonUp, INPUT_PULLUP);
   pinMode(buttonDown, INPUT_PULLUP);
   pinMode(photoResistor, INPUT);


   // This info pulled from RTClib.h
 if (! rtc.begin()) {
    Serial.println("Couldn't find RTC"); //program LEDs to show "NO CLOCK"
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!"); //program LEDs to show "SET CLOCK"
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void applyMask() {

   for (byte row = 0; row < 11; row++) 
   {
      for (byte col = 0; col < 16; col++) 
      {
         boolean masker = bitRead(mask[row], 15 - col); // bitread is backwards because bitRead reads rightmost digits first. could have defined the word masks differently
         switch (masker) 
         {
            case 0:
               matrix.drawPixel(col, row, 0);
               break;
            case 1:
              // matrix.drawPixel(col, row, Wheel(((col * 256 / matrix.numPixels()) + j) & 255));
              // word_mode color set
               matrix.drawPixel(col, row, matrix.Color(dimred, dimgreen, dimblue));
               break;
         }
      }
      // reset mask for next time
      mask[row] = 0;
   }


   matrix.show(); // show it!
}

void readModeButton() {
  int currentState = digitalRead(buttonSet);
  // If the switch changed, due to noise or pressing:
  if (currentState == HIGH && lastState == LOW) {
    delay(1);
  }
  else if (currentState == LOW && lastState == HIGH)
  {
    // if the button state has changed:
      mode++;
      if (mode >= MAX_MODE)
          mode = WORD_MODE; 
     delay(1);
  }
  lastState = currentState;
}


void loop() {
  
    DateTime now = rtc.now();
    mytimemonth=now.month();
    mytimeday=now.day();
    mytimehr=now.hour();
    mytimemin=now.minute();
    mytimesec=now.second();
//////////////////////////////////////////PHOTORESISTOR/////////////////////////////////////////////
    //Photoresistor settings
    photoRead = analogRead(photoResistor);  
//    Serial.print(photoRead);     // the raw analog reading
//      if (photoRead < 200) {
//        dimmer=5;
//      } else if (photoRead < 400) {
//        dimmer=4;
//      } else if (photoRead < 600) {
//        dimmer=2;
//      } else {
//        dimmer=1;
//      }
      delay(100);
////////////////////////////////////////////////////////////////////////////////////////////////


    
    //Serial.print(now.year(), DEC);
    //Serial.print('/');
    //Serial.print(now.month(), DEC);
    //Serial.print('/');
    //Serial.print(now.day(), DEC);
    //Serial.print(" (");
    //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    //Serial.print(") ");
    //Serial.print(now.hour(), DEC);
    //Serial.print(':');
    //Serial.print(now.minute(), DEC);
    //Serial.print(':');
    //Serial.print(now.second(), DEC);
    //Serial.println();

  readModeButton();

  if (mode == WORD_MODE)
    displayWords();
  else if (mode == DIGIT_MODE)
    displayDigits();
  else if (mode == DATE_MODE)
    displayDate();
  else if (mode == BDAY_MODE)
    displayBday();
    
}

void draw(uint8_t x, uint8_t y, const Character &c, uint16_t color) {
  for (int i = 0; i < 7; i++) for (int j = 0; j < 5; j++) {
    if (bitRead(c[i], j)) matrix.drawPixel(x+4-j, y+i, color);

  }
};

void displayDigits() {
  uint8_t units, tens;
  uint16_t color;
   
  if (mytimesec/2 % 2) {
    units = mytimemin % 10;
    tens  = mytimemin / 10;
    //digit_mode color, minutes
    color = matrix.Color(minred,mingreen,minblue);
  } else {
    units = mytimehr % 10;
    tens  = mytimehr / 10;
    //digit_mode color, hours
    color = matrix.Color(hourred,hourgreen,hourblue);
  }
  matrix.clear();
 
  draw(0, 2, charmap[tens],  color);
  draw(6, 2, charmap[units], color);
 
  matrix.show();
}

void displayDate() {
  uint8_t units, tens;
  uint16_t color;
   
  if (mytimesec/2 % 2) {
    units = mytimeday % 10;
    tens  = mytimeday / 10;
    //digit_mode color, minutes
    color = matrix.Color(dayred,daygreen,dayblue);
  } else {
    units = mytimemonth % 10;
    tens  = mytimemonth / 10;
    //digit_mode color, hours
    color = matrix.Color(monthred,monthgreen,monthblue);
  }
  matrix.clear();
 
  draw(0, 2, charmap[tens],  color);
  draw(6, 2, charmap[units], color);
 
  matrix.show();
}

void displayBday() {
  //Always on
   phraseHAPPY;
   phraseBIRTHDAY;
     applyMask(); 
}

void displayWords() {
  //Always on
   phraseITS;


  //calculate minutes on the hour
    if(mytimemin>57 && mytimemin<3){
    }
    // do nothing, no minutes it's on the hour
    
    if(mytimemin>2 && mytimemin<8){
      
      phraseFIVE;
      phrasePAST;
    }
    
    if(mytimemin>7 && mytimemin<13){
      
      phraseTEN;
      phrasePAST;
    }
    if(mytimemin>12 && mytimemin<18){
      
      phraseA;
      phraseQUARTER;
      phrasePAST;
    }
    if(mytimemin>17 && mytimemin<23){
      
      phraseTWENTY;
      phrasePAST;
    }
    if(mytimemin>22 && mytimemin<28){
      
      phraseTWENTY;
      phraseFIVE;
      phrasePAST;
    }
    if(mytimemin>27 && mytimemin<33){
      
      phraseHALF;
      phrasePAST;
    }
    if(mytimemin>32 && mytimemin<38){
      
      phraseTWENTY;
      phraseFIVE;
      phraseTIL;
    }
    if(mytimemin>37 && mytimemin<43){
      
      phraseTWENTY;
      phraseTIL;
    }
    if(mytimemin>42 && mytimemin<48){
      
      phraseA;
      phraseQUARTER;
      phraseTIL;
    }    
    if(mytimemin>47 && mytimemin<53){
      
      phraseTEN;
      phraseTIL;
    }
    if(mytimemin>52 && mytimemin<58){
      
      phraseFIVE;
      phraseTIL;
    }


  // Calculate hour & oclocks
  if(mytimehr==1){
    if(mytimemin>32){
      hourTWO;
      phraseOCLOCK;
    }
    else
    {
      hourONE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==2){
    if(mytimemin>32){
      hourTHREE;
      phraseOCLOCK;
    }
    else
    {
      hourTWO;
      phraseOCLOCK;
    }
  }
    if(mytimehr==3){
    if(mytimemin>32){
      hourFOUR;
      phraseOCLOCK;
    }
    else
    {
      hourTHREE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==4){
    if(mytimemin>32){
      hourFIVE;
      phraseOCLOCK;
    }
    else
    {
      hourFOUR;
      phraseOCLOCK;
    }
  }
  if(mytimehr==5){
    if(mytimemin>32){
      hourSIX;
      phraseOCLOCK;
    }
    else
    {
      hourFIVE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==6){
    if(mytimemin>32){
      hourSEVEN;
      phraseOCLOCK;
    }
    else
    {
      hourSIX;
      phraseOCLOCK;
    }
  }
  if(mytimehr==7){
    if(mytimemin>32){
      hourEIGHT;
      phraseOCLOCK;
    }
    else
    {
      hourSEVEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==8){
    if(mytimemin>32){
      hourNINE;
      phraseOCLOCK;
    }
    else
    {
      hourEIGHT;
      phraseOCLOCK;
    }
  }
  if(mytimehr==9){
    if(mytimemin>32){
      hourTEN;
      phraseOCLOCK;
    }
    else
    {
      hourNINE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==10){
    if(mytimemin>32){
      hourELEVEN;
      phraseOCLOCK;
    }
    else
    {
      hourTEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==11){
    if(mytimemin>32){
      hourNOON;
    }
    else
    {
      hourELEVEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==12){
    if(mytimemin>32){
      hourONE;
      phraseOCLOCK;
    }
    else
    {
      hourNOON;
    }
  }
      if(mytimehr==13){
    if(mytimemin>32){
      hourTWO;
      phraseOCLOCK;
    }
    else
    {
      hourONE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==14){
    if(mytimemin>32){
      hourTHREE;
      phraseOCLOCK;
    }
    else
    {
      hourTWO;
      phraseOCLOCK;
    }
  }
    if(mytimehr==15){
    if(mytimemin>32){
      hourFOUR;
      phraseOCLOCK;
    }
    else
    {
      hourTHREE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==16){
    if(mytimemin>32){
      hourFIVE;
      phraseOCLOCK;
    }
    else
    {
      hourFOUR;
      phraseOCLOCK;
    }
  }
  if(mytimehr==17){
    if(mytimemin>32){
      hourSIX;
      phraseOCLOCK;
    }
    else
    {
      hourFIVE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==18){
    if(mytimemin>32){
      hourSEVEN;
      phraseOCLOCK;
    }
    else
    {
      hourSIX;
      phraseOCLOCK;
    }
  }
  if(mytimehr==19){
    if(mytimemin>32){
      hourEIGHT;
      phraseOCLOCK;
    }
    else
    {
      hourSEVEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==20){
    if(mytimemin>32){
      hourNINE;
      phraseOCLOCK;
    }
    else
    {
      hourEIGHT;
      phraseOCLOCK;
    }
  }
  if(mytimehr==21){
    if(mytimemin>32){
      hourTEN;
      phraseOCLOCK;
    }
    else
    {
      hourNINE;
      phraseOCLOCK;
    }
  }
  if(mytimehr==22){
    if(mytimemin>32){
      hourELEVEN;
      phraseOCLOCK;
    }
    else
    {
      hourTEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==23){
    if(mytimemin>32){
      hourMIDNIGHT;
    }
    else
    {
      hourELEVEN;
      phraseOCLOCK;
    }
  }
  if(mytimehr==0){
    if(mytimemin>32){
      hourONE;
      phraseOCLOCK;
    }
    else
    {
      hourMIDNIGHT;
    }
  }
  applyMask(); 
}

