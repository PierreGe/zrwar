#include <Arduino.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <LSM303.h>
#include <Wire.h>
void calibrate_compass();
void calibrate_sensors();
void setup();
void stop();
void search_start();
void start_turn();
void mid_turn();
void search_finish();
void loop();
float relativeHeading(float heading_from, float heading_to);
float averageHeading();
#line 1 "src/petanque.ino"
//#include <QTRSensors.h>
//#include <ZumoReflectanceSensorArray.h>
//#include <ZumoMotors.h>
//#include <ZumoBuzzer.h>
//#include <Pushbutton.h>
//#include <LSM303.h>
//#include <Wire.h>

#define SPEED_RATIO 0.82 // Ratio for right compared to left motor


#define CALIBRATE_SPEED 200
#define MAX_SPEED       400 // Maximum motor speed when going straight; variable speed when turning
#define TURN_BASE_SPEED 100 // Base speed when turning (added to variable speed)


#define SEARCH_SPEED 100
#define FORWARD_SPEED 400
#define TURN_SPEED 200


#define DELAY_START 1000
#define DELAY_PAUSE 1000

#define DELAY_FIRST 5000
#define DELAY_SECOND 2000

#define DELAY_FINISH 1000
#define DELAY_SEARCH_FINISH 100

#define DELAY_START_TURN 200
#define DELAY_MID_TURN 200


#define DEVIATE_FACTOR 4
#define DEVIATE_EPSILON 40
#define MAX_POSITION 5000
#define POSITION_EPSILON 200


#define CALIBRATION_SAMPLES 70  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate

// Allowed deviation (in degrees) relative to target angle that must be achieved before driving straight
#define DEVIATION_THRESHOLD 5

#define HEADING_FACTOR 1.5

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
LSM303 compass;


typedef enum {SEARCH_START, START_TURN, FIRST_FORWARD, MID_TURN, SECOND_FORWARD, SEARCH_FINISH, FINISH} states_t;

states_t state;


int position;
unsigned int sensors[6];

int leftspeed, rightspeed;

int deviate, angle, speed;
float current_heading, relative_heading, target_heading;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void calibrate_compass() {
  // The highest possible magnetic value to read in any direction is 2047
  // The lowest possible magnetic value to read in any direction is -2047
  LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32767, -32767, -32767};
  unsigned char index;

  // Initiate LSM303
  compass.init();

  // Enables accelerometer and magnetometer
  compass.enableDefault();

  compass.writeReg(LSM303::CRB_REG_M, CRB_REG_M_2_5GAUSS); // +/- 2.5 gauss sensitivity to hopefully avoid overflow problems
  compass.writeReg(LSM303::CRA_REG_M, CRA_REG_M_220HZ);    // 220 Hz compass update rate

  delay(1000);

  // To calibrate the magnetometer, the Zumo spins to find the max/min
  // magnetic vectors. This information is used to correct for offsets
  // in the magnetometer data.
  motors.setLeftSpeed(CALIBRATE_SPEED);
  motors.setRightSpeed(-CALIBRATE_SPEED * SPEED_RATIO);

  for(index = 0; index < CALIBRATION_SAMPLES; index ++)
  {
    // Take a reading of the magnetic vector and store it in compass.m
    compass.read();

    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);

    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);

    delay(50);
  }

  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);

  // Set calibrated values to compass.m_max and compass.m_min
  compass.m_max.x = running_max.x;
  compass.m_max.y = running_max.y;
  compass.m_min.x = running_min.x;
  compass.m_min.y = running_min.y;
}


////////////////////////////////////////////////////////////////////////////////
void calibrate_sensors() {
  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

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
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Setup will calibrate our compass by finding maximum/minimum magnetic readings
void setup()
{
  button.waitForButton();
  
//  delay(500);
  
//  calibrate_compass();
  
//  button.waitForButton();
  
//  delay(500);
//  calibrate_sensors();
  
  button.waitForButton();
  
  state = FIRST_FORWARD;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void stop() {
  leftspeed = 0;
  rightspeed = 0;
  motors.setSpeeds(leftspeed, rightspeed);
}

////////////////////////////////////////////////////////////////////////////////
void search_start() {
  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
  position = reflectanceSensors.readLine(sensors);

  deviate = 2500 - position;

  if ((position > MAX_POSITION - POSITION_EPSILON) || (position < POSITION_EPSILON)) {
    leftspeed = SEARCH_SPEED;
    rightspeed = -SEARCH_SPEED;
  }
  else if (deviate > DEVIATE_EPSILON + POSITION_EPSILON) {
    state = START_TURN;
    stop();
    angle = 90;
  }
  else if (deviate < -DEVIATE_EPSILON - POSITION_EPSILON) {
    state = START_TURN;
    stop();
    angle = -90;
  }
  else if (deviate > DEVIATE_EPSILON) {
    leftspeed = SEARCH_SPEED - deviate / DEVIATE_FACTOR;
    rightspeed = SEARCH_SPEED;
  }
  else if (deviate < -DEVIATE_EPSILON) {
    leftspeed = SEARCH_SPEED;
    rightspeed = SEARCH_SPEED + deviate / DEVIATE_FACTOR;
  }
  else {
    leftspeed = SEARCH_SPEED;
    rightspeed = SEARCH_SPEED;
    motors.setSpeeds(leftspeed, rightspeed);
  }
  motors.setSpeeds(leftspeed, rightspeed);
}



////////////////////////////////////////////////////////////////////////////////
void start_turn() {
  leftspeed = (angle / abs(angle)) * TURN_SPEED;
  rightspeed = ((-angle) / abs(angle)) * TURN_SPEED * SPEED_RATIO;
  
  motors.setSpeeds(leftspeed, rightspeed);
  
  delay(DELAY_START_TURN);

  state = FIRST_FORWARD;
}

////////////////////////////////////////////////////////////////////////////////
void mid_turn() {
  leftspeed = TURN_SPEED;
  rightspeed = (-TURN_SPEED) * SPEED_RATIO;
  
  motors.setSpeeds(leftspeed, rightspeed);
  
  delay(DELAY_MID_TURN);

  state = SECOND_FORWARD;
}



////////////////////////////////////////////////////////////////////////////////
void search_finish() {

  position = reflectanceSensors.readLine(sensors);

  if ((position < POSITION_EPSILON) || (position > MAX_POSITION - POSITION_EPSILON)) { // found
    state = FINISH;
  }

  leftspeed = SEARCH_SPEED;
  for (int i = 0; i < 20; i++) {
    rightspeed = sqrt(i);
    motors.setSpeeds(leftspeed, rightspeed);
    delay(DELAY_SEARCH_FINISH);
  }

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void loop()
{
  switch (state) {
    case SEARCH_START:
      delay(DELAY_START);
      search_start();
      break;
    case START_TURN:
      start_turn();
      break;
    case FIRST_FORWARD:
      delay(DELAY_PAUSE);
      leftspeed = FORWARD_SPEED;
      rightspeed = FORWARD_SPEED * SPEED_RATIO;
      motors.setSpeeds(leftspeed, rightspeed);
      delay(DELAY_FIRST);
      stop();
      delay(DELAY_PAUSE);
      state = MID_TURN;
      break;
    case MID_TURN:
      mid_turn();
      break;
    case SECOND_FORWARD:
      delay(DELAY_PAUSE);
      leftspeed = FORWARD_SPEED;
      rightspeed = FORWARD_SPEED * SPEED_RATIO;
      motors.setSpeeds(leftspeed, rightspeed);
      delay(DELAY_SECOND);
      stop();
      delay(DELAY_PAUSE);
      state = SEARCH_FINISH;
      break;
    case SEARCH_FINISH:
      delay(DELAY_FINISH);
      search_finish();
      break;
    case FINISH:
    default:
      stop();
  }


}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// Converts x and y components of a vector to a heading in degrees.
// This function is used instead of LSM303::heading() because we don't
// want the acceleration of the Zumo to factor spuriously into the
// tilt compensation that LSM303::heading() performs. This calculation
// assumes that the Zumo is always level.
template <typename T> float heading(LSM303::vector<T> v)
{
  float x_scaled =  2.0*(float)(v.x - compass.m_min.x) / ( compass.m_max.x - compass.m_min.x) - 1.0;
  float y_scaled =  2.0*(float)(v.y - compass.m_min.y) / (compass.m_max.y - compass.m_min.y) - 1.0;

  float angle = atan2(y_scaled, x_scaled)*180 / M_PI;
  if (angle < 0)
    angle += 360;
  return angle;
}

////////////////////////////////////////////////////////////////////////////////
// Yields the angle difference in degrees between two headings
float relativeHeading(float heading_from, float heading_to)
{
  float relative_heading = heading_to - heading_from;

  // constrain to -180 to 180 degree range
  if (relative_heading > 180)
    relative_heading -= 360;
  if (relative_heading < -180)
    relative_heading += 360;

  return relative_heading;
}

////////////////////////////////////////////////////////////////////////////////
// Average 10 vectors to get a better measurement and help smooth out
// the motors' magnetic interference.
float averageHeading()
{
  LSM303::vector<int32_t> avg = {0, 0, 0};

  for(int i = 0; i < 10; i ++)
  {
    compass.read();
    avg.x += compass.m.x;
    avg.y += compass.m.y;
  }
  avg.x /= 10.0;
  avg.y /= 10.0;

  // avg is the average measure of the magnetic vector.
  return heading(avg);
}

