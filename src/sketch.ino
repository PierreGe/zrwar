#include <Wire.h>
#include <LSM303.h>
#include <ZumoMotors.h>

LSM303 compass;
ZumoMotors motors;

void setup() {

  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();

  // min: {  -777,  -1426,  -4096}    max: {  +163,   -294,  +2043}
  compass.m_min = (LSM303::vector<int16_t>){  -777,  -1426,  -4096};
  compass.m_max = (LSM303::vector<int16_t>){  +163,   -294,  +2043};
}

void loop() {
  move_straight(200, 1000);
  int heading_start = read_heading();
  delay(500);
  rotate_to((heading_start + 60) % 360, 100, 30, 5);
  delay(500);
  move_straight(200, 1000);
  delay(500);
  rotate_to(heading_start, 100, 30, 5);
  delay(500);
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
  float diff = abs(angle - angle_reading);
  while (diff > 20) {
    angle_reading = read_heading();
    diff = abs(angle - angle_reading);
    Serial.println(diff);
    move(speed_left, speed_right, step_duration);
  }
}

void move(int speed_left, int speed_right, int duration) {
  motors.setLeftSpeed(speed_left);
  motors.setRightSpeed(speed_right);
  delay(duration);
}