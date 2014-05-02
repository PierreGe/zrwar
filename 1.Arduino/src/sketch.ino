#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <Wire.h>
#include <LSM303.h>


#define LED_PIN 13


LSM303 compass;

ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);


#define FULL_SPEED 200
#define HALF_SPEED 100

#define EPSILON 5


void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-1391, -705, -4096};
  compass.m_max = (LSM303::vector<int16_t>){-675, +223, -4096};

//in: { -1391,   -705,  -4096}    max: {  -675,   +223,  -4096}

  // Wait for the user button to be pressed and released
  button.waitForButton();
  delay(1000);
}

void loop()
{
  digitalWrite(LED_PIN, HIGH);

  float origin, current, final;

  // PLUS_LINE
  Serial.println("Tout droit");
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(100);

  // PLUS_LOOP
  Serial.println("A droite");
  compass.read();
  Serial.println("1");
  origin = compass.heading();
  Serial.println("2");
  final = (int(origin) + 270) % 360;
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(HALF_SPEED);
  do {
    compass.read();
    current = compass.heading();
    Serial.println(current);
  } while (abs(current - final) > EPSILON);

  // PLUS_LINE
  Serial.println("Tout droit");
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(200);

  // MINUS_LOOP
  Serial.println("A gauche");
  compass.read();
  origin = compass.heading();
  final = (int(origin) + 90) % 360;
  motors.setLeftSpeed(HALF_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  do {
    compass.read();
    current = compass.heading();
  } while (abs(current - final) > EPSILON);

  // MINUS_LINE
  Serial.println("Tout droit");
  motors.setLeftSpeed(FULL_SPEED);
  motors.setRightSpeed(FULL_SPEED);
  delay(100);

    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);

    delay(5000);
}

