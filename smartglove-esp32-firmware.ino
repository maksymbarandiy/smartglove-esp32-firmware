#include <Wire.h>

const uint8_t MPU_ADDR = 0x68;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("MPU6050 initialized");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read(); // skip temperature
  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();

  Serial.print("ax="); Serial.print(ax);
  Serial.print(" ay="); Serial.print(ay);
  Serial.print(" az="); Serial.print(az);
  Serial.print(" gx="); Serial.print(gx);
  Serial.print(" gy="); Serial.print(gy);
  Serial.print(" gz="); Serial.println(gz);

  delay(100);
}
