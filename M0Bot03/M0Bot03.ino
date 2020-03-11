// M0Bot01
// Trinket M0 Bristlebot
// MGP October 2019

// Load hardware definitions & infrared codes
#include "M0BOT.h"

// Infrared library
#include <IRremote.h>
IRrecv irrecv(IRRCV_PIN);
decode_results IRmessage;

unsigned MODE = -99;
unsigned keyPress = 0;
unsigned lastKeyPress = 0;
unsigned Speed = 0;
bool Left_Turn = false;
bool Right_Turn = false;
unsigned Lthreshold;
unsigned Rthreshold; //line color sensor thresholds


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
  Flasher(): Adafruit_DotStar(1, 7, 8, DOTSTAR_BGR){
    i=0;
    n = 0;  // ensures that flash() is harmless when colour data not specified
    }
  // constructor for sequence of n colours each on for ms
  Flasher(uint8_t nColours, uint8_t (*pC)[3], uint16_t ms): 
      Adafruit_DotStar(1, 7, 8, DOTSTAR_BGR){
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
  
  pinMode(LMOTOR, OUTPUT);
  pinMode(RMOTOR, OUTPUT);

  
  theFlasher = &rgbFlasher;


}


void move(int leftSpeed, int rightSpeed) {

  // nb left speed increase causes right turn etc.

  if (random(100) < leftSpeed) digitalWrite(LMOTOR, HIGH);
  else digitalWrite(LMOTOR, LOW);

  if (random(100) < rightSpeed) digitalWrite(RMOTOR, HIGH);
  else digitalWrite(RMOTOR, LOW);
       
}


bool keyPressed(){return (irrecv.decode(&IRmessage));}


void getKeyPressed(){
  // updates global variables keyPressed and lastKeyPressed
  // if (and only if) a message was received from the IR remote

  if (keyPressed()) { // if new message received
    keyPress = IRmessage.value;
    if (keyPress==IRCODE_REPEAT) keyPress = lastKeyPress;
    else lastKeyPress = keyPress;
    irrecv.resume();  // reset receiver
  }

}  

void selectMode() {
  // Runs when mode select button (*) has been pressed
  // sets behaviour mode depending on the next button pressed
  
  while (keyPress == IRCODE_STAR) {
    
    getKeyPressed();
    
    switch (keyPress) {
      case IRCODE_0:
        MODE = MODE_REMOTE;
        Serial.println("Remote control mode");
        break;     
      case IRCODE_1:
        MODE = MODE_LEARN_LINE;
        Serial.println("Learn line mode: move light sensors back and forth across line");
        break;                    
      case IRCODE_2:
        MODE = MODE_FOLLOW_LINE;
        Serial.println("Follow line mode");
        break;   
       default: 
        Serial.println("Waiting for Mode command");
        MODE = MODE_DO_NOTHING;
        }
  }
}

void learnLineColors(void) {

    theFlasher = &redBlinkFlasher;

  unsigned Lhigh =  0;
  unsigned Rhigh =  0;
  unsigned Llow =   1024;
  unsigned Rlow =   1024;

  while (!keyPressed()) {
    flash();
    unsigned L_sensor_reading = analogRead(LSENSOR);
    Lhigh = L_sensor_reading > Lhigh ? L_sensor_reading : Lhigh;
    Llow = L_sensor_reading  < Llow  ? L_sensor_reading : Llow;
    
    unsigned R_sensor_reading = analogRead(RSENSOR);
    Rhigh = R_sensor_reading > Rhigh ? R_sensor_reading : Rhigh;
    Rlow = R_sensor_reading  < Rlow  ? R_sensor_reading : Rlow;
        
    Serial.print(L_sensor_reading);
    Serial.print(", ");
    Serial.println(R_sensor_reading);
    delay(50); 
    }

    Lthreshold = (Lhigh+Llow)/2;
    Rthreshold = (Rhigh+Rlow)/2;

    MODE = MODE_DO_NOTHING;
    theFlasher = &noFlasher;
    
}

void followline(void){

  // assuming track is darker, low light reading indicates sensor is on track.
  // sensor states:
  //  0   : both on track
  //  1   : left off, right on track
  //  2   : left on, right off track
  //  3   : both off track
  
  int previousState = 0;
  int sensorState = 0;

  int speed = 80;

 while (!keyPressed()) {
  
  int Lbright = analogRead(LSENSOR) > Lthreshold ? 1 : 0;
  int Rbright = analogRead(RSENSOR) < Rthreshold ? 2 : 0;

  sensorState = Lbright + Rbright;

  // actions
  switch (sensorState) {

    case 0:  

       // both sensors on track: go ahead
       move(speed, speed);
       previousState = 0;
       break;

    case 1: 

       // left off track, right on track: turn right a little bit
       move(speed,speed/2);
       previousState = 1;
       break;

    case 2:

       // left on track, right off track: turn left a little bit
       move(speed/2, speed);
       previousState = 2;
       break;


    case 3: 

       // both sensors off track
       // if left sensor was previously on track, we've gone off to the right: turn left
       if (previousState==2) move(speed/2, speed);
       // else right sensor was previously on track, we've gone off to the left: turn right
       else move(speed,speed/2);
       break;

  }

  delay(10);
  
 } 

  MODE = MODE_DO_NOTHING;
}


void loop() {   
  
  flash();

    // If a key was pressed on the remote,
    //   update global variables keyPressed and lastKeyPressed 
    if (keyPressed()) {
      
      getKeyPressed();
    

    if (keyPress == IRCODE_STAR) {
      Serial.println("Select mode");
      selectMode();
    }
    else switch(MODE) {
 
       case MODE_REMOTE:  // control M0Bot with IR remote
       
          Left_Turn = false;
          Right_Turn = false;
          switch (keyPress){
            case IRCODE_UP:
              Speed += ACCELERATION;
              if (Speed>255) Speed = 255;
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
        break;  // end case MODE_REMOTE
        
        case MODE_LEARN_LINE:   // set detection thresholds for line following                             

          learnLineColors();
          break; // end case MODE_LEARN_LINE

        case MODE_FOLLOW_LINE:

          followline();
          break; // end case MODE_FOLLOW_LINE

        case MODE_DO_NOTHING:

           // do nothing
           break;

          default: // do nothing
          ;
      }
    }
  
  // behaviour
  if (Left_Turn) {
    digitalWrite(LMOTOR, LOW);
    Serial.println("L");
  }
  else if (random(100) < Speed) digitalWrite(LMOTOR, HIGH);
       else digitalWrite(LMOTOR, LOW);
  if (Right_Turn) digitalWrite(RMOTOR, LOW);
  else if (random(100) < Speed) digitalWrite(RMOTOR, HIGH);
       else digitalWrite(RMOTOR, LOW);
  
  delay(1);
  

  
}






    
//    else {
//      if (keyPress == IRCODE_STAR){
//           MODE = MODE_SELECT;
//           Serial.println("Mode select mode");
//      }
//      else switch (MODE) {
//
//       case MODE_REMOTE:  // control M0Bot with IR remote
//       
//          Left_Turn = false;
//          Right_Turn = false;
//          switch (keyPress){
//            case IRCODE_UP:
//              Speed += ACCELERATION;
//              if (Speed>255) Speed = 255;
//              break;
//            case IRCODE_DOWN:
//              // nb Speed is unsigned, can't go <0
//              if (Speed>=ACCELERATION) Speed -= ACCELERATION;
//              else Speed = 0;
//              break;              
//            case IRCODE_LEFT:
//              Left_Turn = true;
//              break;                
//            case IRCODE_RIGHT:
//              Right_Turn = true;
//              break;               
//           }
//        break;  // end case MODE_REMOTE
//        
//        case MODE_LEARN_LINE:   // set detection thresholds for line following 
//                                // (move sensors back and forth across line)
//
//        Serial.print(analogRead(LSENSOR));
//        Serial.print(", ");
//        Serial.println(analogRead(RSENSOR));
//
//        break; // end case MODE_LEARN_LINE
//
//        
//      }
////    Serial.print("Speed = " );  Serial.print(Speed); 
////    Serial.print(", Left_Turn = "); Serial.print(Left_Turn);
////    Serial.print(", Right_Turn = "); Serial.println(Right_Turn);
//    
//    }
//    irrecv.resume(); // Receive the next value
//
//  }
//
