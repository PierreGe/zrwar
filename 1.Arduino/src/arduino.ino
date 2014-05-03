#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <Wire.h>
#include <LSM303.h>


#define LED_PIN 13


LSM303 compass;

ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);


#define FULL_SPEED 100
#define HALF_SPEED 50

#define EPSILON 10


void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-1226, -4096, -4096};
  compass.m_max = (LSM303::vector<int16_t>){-474, -1504, -4096};


  // Wait for the user button to be pressed and released
  button.waitForButton();
  delay(1000);
}

void loop()
{
  digitalWrite(LED_PIN, HIGH);

  int origin, current, final;

/*  // DECALAGE*/
/*  compass.read();*/
/*  origin = int(compass.heading());*/
/*  final = (origin + 305) % 360;*/
/*  motors.setLeftSpeed(-HALF_SPEED);*/
/*  motors.setRightSpeed(HALF_SPEED);*/
/*  do {*/
/*    compass.read();*/
/*    current = int(compass.heading());*/
/*    Serial.println(current);*/
/*  } while (abs(current - final) > EPSILON);*/


  // PLUS_LINE
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(100);

  // PLUS_LOOP
  compass.read();
  origin = int(compass.heading());
  final = (origin + 280) % 360;
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(HALF_SPEED);
  do {
    compass.read();
    current = int(compass.heading());
    Serial.println(current);
  } while (abs(current - final) > EPSILON);

  // PLUS_LINE
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(200);

  // MINUS_LOOP
  compass.read();
  origin = int(compass.heading());
  final = (origin + 80) % 360;
  motors.setLeftSpeed(HALF_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  do {
    compass.read();
    current = int(compass.heading());
    Serial.println(current);
  } while (abs(current - final) > EPSILON);

  // MINUS_LINE
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(100);

/*  // DECALAGE*/
/*  compass.read();*/
/*  origin = int(compass.heading());*/
/*  final = (origin + 35) % 360;*/
/*  motors.setLeftSpeed(HALF_SPEED);*/
/*  motors.setRightSpeed(-HALF_SPEED);*/
/*  do {*/
/*    compass.read();*/
/*    current = int(compass.heading());*/
/*    Serial.println(current);*/
/*  } while (abs(current - final) > EPSILON);*/

  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);

  delay(5000);
}

