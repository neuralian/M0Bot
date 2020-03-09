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

void setup() {

  Serial.begin(115200); // open serial port

  irrecv.enableIRIn(); // Start the IR receiver
  
  pinMode(LMOTOR, OUTPUT);
  pinMode(RMOTOR, OUTPUT);
  pinMode(LSENSOR, INPUT);
  pinMode(RSENSOR, INPUT);

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
        Serial.println("Unrecognized/unassigned keypress");
        MODE = MODE_DO_NOTHING;
        }
  }
}

void learnLineColors(void) {

  unsigned Lhigh =  0;
  unsigned Rhigh =  0;
  unsigned Llow =   1024;
  unsigned Rlow =   1024;

  while (!keyPressed()) {
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
}

void followline(void){


  Serial.println(Lthreshold);
  Serial.println(", ");
  Serial.println(Rthreshold);

}


void loop() {   

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
