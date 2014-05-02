#include <Wire.h>
#include <LSM303.h>
#include <ZumoMotors.h>

LSM303 compass;
ZumoMotors motors;

void setup() {

  Wire.begin();
  compass.init();
  compass.enableDefault();

  compass.m_min = (LSM303::vector<int16_t>){-273, -658, +1994};
  compass.m_max = (LSM303::vector<int16_t>){-266, -646, +2005};
}

void loop() {
  int heading_start = read_heading();
  move_straight(200, 1000);
  rotate_to(heading_start + 60, 300, 100, 2);
  move_straight(200, 1000);
  rotate_to(heading_start, 100, 300, 2);
}

/* ---------- ---------- ---------- ---------- ---------- */

float read_heading() {
  compass.read();
  float heading = compass.heading((LSM303::vector<int>){0, 1, 0});
  return heading;
}

void move_straight(int speed, int duration) {
  move(speed, speed, duration);
}

void rotate_to(float angle, int speed_left, int speed_right, int step_duration) {
  float angle_reading = read_heading();
  while (angle - angle_reading > 10 && angle_reading - angle < 10) {
    angle_reading = read_heading();
    move(speed_left, speed_right, step_duration);
  }
}

void move(int speed_left, int speed_right, int duration) {
  motors.setLeftSpeed(speed_left);
  motors.setRightSpeed(speed_right);
  delay(duration);
}