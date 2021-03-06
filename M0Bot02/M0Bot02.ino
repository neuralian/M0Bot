// M0Bot01
// Trinket M0 Bristlebot
// MGP October 2019

// Load hardware definitions & infrared codes
#include "M0BOT.h"

// RGB LED control

//Adafruit_DotStar RGBLED = Adafruit_DotStar(
//  1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

// Infrared receiver
#include <IRremote.h>
IRrecv irrecv(IRRCV_PIN);
decode_results results;



unsigned Tick;
unsigned MODE = -99;
unsigned Message = 0;
unsigned LastMessage = 0;
unsigned Speed = 0;
bool Left_Turn = false;
bool Right_Turn = false;

// colour sequences
uint8_t Colourseq_RG[2][3] = {{64,0,0}, {0,64,0}};
uint8_t Colourseq_RedBlink[2][3] = {{64,0,0}, {0,0,0}};
uint8_t Colourseq_RGB[3][3] = {{64,0,0}, {0,64,0}, {0,0,64}};

class Flasher: public  Adafruit_DotStar{
public:
  uint8_t n;        // number of colours in sequence
  uint8_t i;        // current colour
  unsigned long onTime = 0; // time when current colour switched on (from millis())
  uint8_t (*pClr)[3];   // pointer to nx3 colour array
  uint16_t msFlash;     // ms per flash

  // default constructor 
  // can be used for manual color setting via colour() method
  Flasher(): Adafruit_DotStar(1, M0_DOTSTAR_DATA, M0_DOTSTAR_CLK, DOTSTAR_BGR){
    i=0;
    n = 0;  // ensures that flash() is harmless when colour data not specified
    }
  // constructor for sequence of n colours each on for ms
  Flasher(uint8_t nColours, uint8_t (*pC)[3], uint16_t ms): 
      Adafruit_DotStar(1, M0_DOTSTAR_DATA, M0_DOTSTAR_CLK, DOTSTAR_BGR){
        i = 0;
        n = nColours;
        pClr = pC;  // points to (global) colour array
        msFlash = ms; // ms per flash     
        };
  // set LED color RGB
  void colour(uint8_t r, uint8_t g, uint8_t b);
  void vcolour(uint8_t rgb[3]);
  
};

void Flasher::colour(uint8_t r, uint8_t g, uint8_t b){

 setPixelColor(0, r, g, b);
 show();
  
}

void Flasher::vcolour(uint8_t* rgb){

 setPixelColor(0, rgb[0], rgb[1], rgb[2]);
 show();
  
}

Flasher noFlasher = Flasher();
Flasher rgFlasher = Flasher(2,Colourseq_RG, 200);
Flasher redBlinkFlasher = Flasher(2,Colourseq_RedBlink, 200);
Flasher rgbFlasher = Flasher(3,Colourseq_RGB, 100);
Flasher* theFlasher = &noFlasher;

// flash theFlasher
void flash(void){

  // if flash time has elapsed
  if(millis()-theFlasher->onTime > theFlasher->msFlash){
   // next colour index; cycling
   theFlasher->i++; if (theFlasher->i>=theFlasher->n) theFlasher->i=0;
   // set colour
   theFlasher->vcolour(*(theFlasher->pClr+theFlasher->i));
   // reset timer
   theFlasher->onTime = millis();
    
  }

  
}

void setup() {

  Serial.begin(115200); // open serial port

  irrecv.enableIRIn(); // Start the IR receiver

//  noFlasher.begin();   // start RGB LED controller
  noFlasher.colour(0,0,64);

  pinMode(LMOTOR, OUTPUT);
  pinMode(RMOTOR, OUTPUT);
  digitalWrite(LMOTOR, LOW);
  digitalWrite(RMOTOR, LOW);

  theFlasher = &rgbFlasher;

}

void loop() {

  flash();

  // get IR message
  if (irrecv.decode(&results)) { // if new message received
    Message = results.value;
    if (Message==IRCODE_REPEAT) Message = LastMessage;
    else LastMessage = Message;
   // Serial.println(Message);

    if (MODE==MODE_MODE_SELECT) {
      switch (Message) {
        case IRCODE_0:
          MODE = MODE_REMOTE_CONTROL;
          Serial.println("Remote control mode");
          break;
        case IRCODE_1:
          MODE = MODE_LINEFOLLOW;
          Serial.println("Line follow mode");
          break;
      }
    }
    else {
      if (Message == IRCODE_STAR){
           MODE = MODE_MODE_SELECT;
           Serial.println("Mode select mode");
      }
      else switch (MODE) {

       case MODE_REMOTE_CONTROL:
          Left_Turn = false;
          Right_Turn = false;
          switch (Message){
            case IRCODE_UP:
              Speed += ACCELERATION;
              if (Speed>100) Speed = 100;
              break;
            case IRCODE_DOWN:
              // nb Speed is unsigned, can't go <0
              if (Speed>=ACCELERATION) Speed -= ACCELERATION;
              else Speed = 0;
              break;
            case IRCODE_LEFT:
              Left_Turn = true;
              break;
            case IRCODE_RIGHT:
              Right_Turn = true;
              break;
        }
      }
//    Serial.print("Speed = " );  Serial.print(Speed);
//    Serial.print(", Left_Turn = "); Serial.print(Left_Turn);
//    Serial.print(", Right_Turn = "); Serial.println(Right_Turn);

    }
    irrecv.resume(); // Receive the next value

  }

  // move
  digitalWrite(LMOTOR, LOW);
  digitalWrite(RMOTOR, LOW);
  if (Left_Turn) {
    digitalWrite(RMOTOR, HIGH);
  }
  else if (Right_Turn) {
    digitalWrite(LMOTOR, HIGH);
  }
      else {
        if (random(100) < Speed ) digitalWrite(LMOTOR, HIGH);
        if (random(100) < Speed) digitalWrite(RMOTOR, HIGH);
      }

  delay(1);
  Tick++;
}
