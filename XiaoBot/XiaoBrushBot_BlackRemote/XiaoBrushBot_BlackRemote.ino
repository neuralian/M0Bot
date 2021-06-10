// XiaoBrushBot_Black.ino
// Brush Bot V1 with Black Remote
// MGP 2021

// Load hardware definitions & infrared codes
#include "XiaoBrushBot.h"
#include "Black_IR_Remote_codes.h"


// Infrared receiver
#include <IRremote.h>
IRrecv irrecv(IRRCV_PIN);
decode_results results;

unsigned Tick;
unsigned MODE = MODE_REMOTE_CONTROL;
unsigned Message = 0;
unsigned LastMessage = 0;
unsigned Speed = 0;
bool Left_Turn = false;
bool Right_Turn = false;

void setup() {

  Serial.begin(115200); // open serial port

  irrecv.enableIRIn(); // Start the IR receiver
  
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

    Serial.println(Message,HEX);

    Left_Turn = false;
    Right_Turn = false;
    switch (Message){
       case IRCODE_STAR: // kick starter
          digitalWrite(RMOTOR, HIGH);
          digitalWrite(LMOTOR, HIGH);
          delay(200);
          Speed = 50;
          break;
       case IRCODE_UP:
          Speed += ACCELERATION;
          if (Speed>100) Speed = 100;
          break;
       case IRCODE_DOWN:
          // Speed is Unsigned, >=0
          if (Speed>=ACCELERATION) Speed -= ACCELERATION;
          else Speed = 0;
          break;              
       case IRCODE_LEFT:
          Left_Turn = true;
          break;                
       case IRCODE_RIGHT:
          Right_Turn = true;
          break;        
       case IRCODE_OK:
          Speed = 0;
          break;       
       }

//    Serial.print("Speed = " );  Serial.print(Speed); 
//    Serial.print(", Left_Turn = "); Serial.print(Left_Turn);
//    Serial.println(", Right_Turn = "); Serial.println(Right_Turn);
    
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
        if (random(100) < Speed ) digitalWrite(RMOTOR, HIGH);
      }
      
  delay(1);
  Tick++;
}
