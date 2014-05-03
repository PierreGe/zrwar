#include <Arduino.h>
#include <ZumoMotors.h>
void setup();
void loop();
#line 1 "src/sketch.ino"
//#include <ZumoMotors.h>

/*
 * This example uses the ZumoMotors library to drive each motor on the Zumo
 * forward, then backward. The yellow user LED is on when a motor should be
 * running forward and off when a motor should be running backward. If a
 * motor on your Zumo has been flipped, you can correct its direction by
 * uncommenting the call to flipLeftMotor() or flipRightMotor() in the setup()
 * function.
 */

#define LED_PIN 13

ZumoMotors motors;

#define PLUS_LOOP 1
#define MINUS_LOOP 2
#define PLUS_LINE 3
#define MINUS_LINE 4

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  // uncomment one or both of the following lines if your motors' directions need to be flipped
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
}

void loop()
{
  // run left motor forward

  digitalWrite(LED_PIN, HIGH);

  // PLUS_LOOP
  motors.setLeftSpeed(300);
  motors.setRightSpeed(150);
  delay(5000);

  // PLUS_LINE
  motors.setLeftSpeed(200);
  motors.setRightSpeed(200);
  delay(1500);

  // MINUS_LOOP
  motors.setLeftSpeed(150);
  motors.setRightSpeed(300);
  delay(5000);

  // MINUS_LINE
  motors.setLeftSpeed(200);
  motors.setRightSpeed(200);
  delay(1500);

  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);

  delay(500);
}