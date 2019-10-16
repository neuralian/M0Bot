// M0Bot01
// Trinket M0 Bristlebot
// MGP October 2019

// Load hardware definitions & infrared codes
#include "M0BOT.h"

// RGB LED control
#include <Adafruit_DotStar.h>
Adafruit_DotStar RGBLED = Adafruit_DotStar(
  1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

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

void setup() {

  Serial.begin(115200); // open serial port

  irrecv.enableIRIn(); // Start the IR receiver

  RGBLED.begin();   // start RGB LED controller
  RGBLED.setPixelColor(0, 0, 0, 32); RGBLED.show(); //red
  
  pinMode(LMOTOR, OUTPUT);
  pinMode(RMOTOR, OUTPUT);
  digitalWrite(LMOTOR, LOW);
  digitalWrite(RMOTOR, LOW);

  Tick = 0;

}

void loop() {

 

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
