#include <Wire.h>

const uint8_t MPU1_ADDR = 0x68;
const uint8_t MPU2_ADDR = 0x69;

bool readMPU6050(uint8_t addr, int16_t &ax, int16_t &ay, int16_t &az,
                                int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(addr, (uint8_t)14);
  if (Wire.available() < 14) return false;

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();
  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();
  return true;
}

void initMPU6050(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  initMPU6050(MPU1_ADDR);
  initMPU6050(MPU2_ADDR);
  Serial.println("Two MPU6050 initialized");
}

void loop() {
  int16_t ax1, ay1, az1, gx1, gy1, gz1;
  int16_t ax2, ay2, az2, gx2, gy2, gz2;

  bool ok1 = readMPU6050(MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
  bool ok2 = readMPU6050(MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);

  if (ok1) {
    Serial.print("MPU1: ax="); Serial.print(ax1); Serial.print(" ay="); Serial.print(ay1); Serial.print(" az="); Serial.println(az1);
  } else Serial.println("MPU1 read failed");

  if (ok2) {
    Serial.print("MPU2: ax="); Serial.print(ax2); Serial.print(" ay="); Serial.print(ay2); Serial.print(" az="); Serial.println(az2);
  } else Serial.println("MPU2 read failed");

  delay(100);
}