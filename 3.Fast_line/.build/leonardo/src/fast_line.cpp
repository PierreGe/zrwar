#include <Arduino.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <LSM303.h>
#include <Wire.h>
void setup();
void loop();
#line 1 "src/fast_line.ino"
/*
 * Line following
 */

//#include <QTRSensors.h>
//#include <ZumoReflectanceSensorArray.h>
//#include <ZumoMotors.h>
//#include <ZumoBuzzer.h>
//#include <Pushbutton.h>
//#include <LSM303.h>
//#include <Wire.h>

#define DEVIATE_THRES 100
#define SEARCH_SPEED 150
#define MAX_SPEED 400

#define DELAY_BEFORE_LOST 250
#define DELAY_CONTINUE 150
#define DELAY_BACK 160
#define DELAY_SEARCH 0
#define DELAY_AFTER_SPIN 200

#define DELAY_BEFORE_TURN 50
#define DELAY_TURN 75

#define COMPASS_EPSILON 40

#define DEVIATE_FACTOR 4
#define DEVIATE_EPSILON 10

#define MAX_POSITION 5000
#define POSITION_EPSILON 15
#define MAX_SENSOR 1000
#define SENSOR_EPSILON 10

LSM303 compass;

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

int lastDeviate = 0;

int leftspeed=0, rightspeed=0;

unsigned int sensors[6];

int position, deviate, side, search, seek_delay;
int orientation, spinning;
int turn_left, turn_right;


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
  turn_left = 0;
  turn_right = 0;

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

  if (turn_left == 1) { // turn left!
    turn_left = 0;
    turn_right = 0;
    leftspeed = -SEARCH_SPEED;
    rightspeed = SEARCH_SPEED;
    seek_delay = DELAY_TURN;
  }
  else if (turn_right == 1) { // turn right!
    turn_left = 0;
    turn_right = 0;
    leftspeed = SEARCH_SPEED;
    rightspeed = -SEARCH_SPEED;
    seek_delay = DELAY_TURN;
  }

  else if (position > MAX_POSITION - POSITION_EPSILON) { // I'm lost! -> raw search
    switch (search) {
      case 0: // first, see whether this was just a line break
        search = 1;
        leftspeed = rightspeed = SEARCH_SPEED;
        seek_delay = DELAY_CONTINUE;
        break;
      case 1: // if not, go back
        search = 2;
        leftspeed = rightspeed = -SEARCH_SPEED;
        seek_delay = DELAY_BACK;
        break;
      case 2: // then randomly choose a side
        search = 3;
        side = random(2) * 2 - 1;
        seek_delay = 0;
        break;
      case 3: // and finally spin around to search
        search = 4;
        leftspeed = side * SEARCH_SPEED;
        rightspeed = (-side) * SEARCH_SPEED;
        seek_delay = 0;
        break;
      case 4: 
        break;
          
      default:
        search = 0;
    }
  }

  else if (sensors[0] > MAX_SENSOR - SENSOR_EPSILON) { // turn left?
    turn_left = 1;
    turn_right = 0;
    seek_delay = DELAY_BEFORE_TURN;
  }
  else if (sensors[5] > MAX_SENSOR - SENSOR_EPSILON) { // turn right?
    turn_right = 1;
    turn_left = 0;
    seek_delay = DELAY_BEFORE_TURN;
  }

  else { // normal behavior, follow the line
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
      rightspeed = MAX_SPEED;
    }
    seek_delay = 0;
  }

  motors.setSpeeds(leftspeed, rightspeed);
  delay(seek_delay);

  if (button.isPressed()) {
    motors.setSpeeds(0, 0);
    delay(1000);
  button.waitForButton();
  }
}
