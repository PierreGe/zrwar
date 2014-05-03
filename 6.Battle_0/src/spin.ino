#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <Wire.h>
#include <LSM303.h>

#define SPEED           100 // Maximum motor speed when going straight; variable speed when turning
#define TURN_BASE_SPEED 100 // Base speed when turning (added to variable speed)


#define CALIBRATION_SAMPLES 70  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate

// Allowed deviation (in degrees) relative to target angle that must be achieved before driving straight
#define DEVIATION_THRESHOLD 5

#define HEADING_FACTOR 1.5

ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
LSM303 compass;

// Setup will calibrate our compass by finding maximum/minimum magnetic readings
void setup()
{
  button.waitForButton();

  Serial.println("starting calibration");
}

void loop()
{
  int leftSpeed = 400;
  int rightSpeed = -380;
  motors.setSpeeds(leftSpeed, rightSpeed);
  delay(5000);
  motors.setSpeeds(0, 0);
}

