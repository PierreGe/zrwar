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
#define SEARCH_SPEED 300
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
#define DEVIATE_EPSILON 50

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

  leftspeed = 400;
  rightspeed = 400;

  // Wait for the user button to be pressed and released
  button.waitForButton();
}

void loop()
{
  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
  position = reflectanceSensors.readLine(sensors);

/*  if (turn_left == 1) { // turn left!*/
/*    turn_left = 0;*/
/*    turn_right = 0;*/
/*    leftspeed = -SEARCH_SPEED;*/
/*    rightspeed = SEARCH_SPEED;*/
/*    seek_delay = DELAY_TURN;*/
/*  }*/
/*  else if (turn_right == 1) { // turn right!*/
/*    turn_left = 0;*/
/*    turn_right = 0;*/
/*    leftspeed = SEARCH_SPEED;*/
/*    rightspeed = -SEARCH_SPEED;*/
/*    seek_delay = DELAY_TURN;*/
/*  }*/

 
  deviate = 2500 - position;

  if ((position > MAX_POSITION - POSITION_EPSILON) || (position < POSITION_EPSILON)) {
    leftspeed = rightspeed = -SEARCH_SPEED;
  }
  else if (deviate > DEVIATE_EPSILON) {
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

  motors.setSpeeds(leftspeed, rightspeed);
}
