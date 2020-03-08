// Trinket M0 Bristlebot header

#include <Adafruit_DotStar.h>
#include "M0BOT_IRCODES.h"

// M0Bot PINS
#define IRRCV_PIN 0
#define LMOTOR 1
#define RMOTOR 4
#define LSENSOR 1
#define RSENSOR 3

#define ACCELERATION 10

// Behaviour modes
#define MODE_NOT_SET 0
#define MODE_SELECT  1
#define MODE_REMOTE  2
#define MODE_LEARN_LINE 3
#define MODE_LINEFOLLOW 4
