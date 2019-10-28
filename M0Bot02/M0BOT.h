// Trinket M0 Bristlebot header

#include <Adafruit_DotStar.h>
#include "M0BOT_IRCODES.h"

#define IRRCV_PIN 0
#define RMOTOR 4
#define LMOTOR 1
#define ACCELERATION 10

// Behaviour modes
#define MODE_NOT_SET 0
#define MODE_MODE_SELECT  1
#define MODE_REMOTE_CONTROL  2
#define MODE_LINEFOLLOW 3
