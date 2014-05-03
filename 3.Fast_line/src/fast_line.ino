/*
 * Line following
 */

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <LSM303.h>
#include <Wire.h>

#define DEVIATE_THRES 100
#define SEARCH_SPEED 200
#define MAX_SPEED 400

#define DELAY_CONTINUE 150
#define DELAY_BACK 160
#define DELAY_SEARCH 0
#define DELAY_AFTER_SPIN 200

#define COMPASS_EPSILON 40

#define DEVIATE_FACTOR 4
#define DEVIATE_EPSILON 10

#define MAX_POSITION 5000
#define POSITION_EPSILON 20

LSM303 compass;

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

int lastDeviate = 0;

int leftspeed=0, rightspeed=0;

unsigned int sensors[6];

int position, deviate, side, search, seek_delay;
int orientation, spinning;


void setup()
{
  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  Serial.begin(9600);
  Wire.begin();

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  int i;
  for(i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0,0);

  // Turn off LED to indicate we are through with calibration
  digitalWrite(13, LOW);

  randomSeed(analogRead(10));
//  side = 1;
  search = 0;
  spinning = 0;

  // Wait for the user button to be pressed and released
//  button.waitForButton();

  leftspeed = 200;
  rightspeed = 200;
}

void loop()
{
  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
  position = reflectanceSensors.readLine(sensors);

  if ((position < POSITION_EPSILON) || (position > MAX_POSITION - POSITION_EPSILON)) {
    switch (search) {
      case 0: // First, search if this was just a line break
        search = 1;
        leftspeed = rightspeed = SEARCH_SPEED;
        seek_delay = DELAY_CONTINUE;
        break;
      case 1: // If not, go back
        search = 2;
        leftspeed = rightspeed = -SEARCH_SPEED;
        seek_delay = DELAY_BACK;
        break;
      case 2: // then choose a side
        search = 3;
        side = random(2) * 2 - 1;
        seek_delay = 0;
        break;
      case 3: // and finally search
        search = 4;
        leftspeed = side * SEARCH_SPEED;
        rightspeed = (-side) * SEARCH_SPEED;
        seek_delay = 0;
        break;
      case 4: 
/*        compass.read();*/
/*        if (spinning && (abs(int(compass.heading()) - orientation) < COMPASS_EPSILON)) {*/
/*          spinning = 0;*/
/*          search = 5;*/
/*          leftspeed = rightspeed = SEARCH_SPEED;*/
/*          seek_delay = DELAY_AFTER_SPIN;*/
/*        }*/
/*        else if (!spinning && (abs(int(compass.heading()) - orientation) > COMPASS_EPSILON)) {*/
/*          spinning = 1;*/
/*        }*/
/*        else {*/
/*          spinning = 0;*/
/*        }*/
        break;
          
      default:
        search = 0;
    }
  }
  else {
    search = 0;
    deviate = 2500 - position;

    if (deviate > DEVIATE_EPSILON) {
      leftspeed = MAX_SPEED - deviate / DEVIATE_FACTOR;
      rightspeed = MAX_SPEED;
    }
    else if (deviate < -DEVIATE_EPSILON) {
      leftspeed = MAX_SPEED;
      rightspeed = MAX_SPEED + deviate / DEVIATE_FACTOR;
    }
    else {
      leftspeed = MAX_SPEED;
      rightspeed = MAX_SPEED + deviate / DEVIATE_FACTOR;
    }
    seek_delay = 0;
  }

/*  if (deviate > DEVIATE_THRES){*/
/*    leftspeed = SPEED_LOW;*/
/*    rightspeed = SPEED_HIGH;*/
/*  } else if (deviate < -DEVIATE_THRES){*/
/*    leftspeed = SPEED_HIGH;*/
/*    rightspeed = SPEED_LOW;*/
/*  } else {*/
/*    leftspeed = rightspeed = SPEED_HIGH;*/
/*  }*/

  motors.setSpeeds(leftspeed, rightspeed);
  delay(seek_delay);

  if (button.isPressed()) {
    motors.setSpeeds(0, 0);
    delay(1000);
  button.waitForButton();
  }
}
