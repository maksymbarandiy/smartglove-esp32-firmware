#include <Wire.h>

const uint8_t MPU1_ADDR = 0x68;  // на Wire (піни 21,22)
const uint8_t MPU2_ADDR = 0x69;  // на Wire (піни 21,22)
const uint8_t MPU3_ADDR = 0x68;  // на Wire1 (піни 18,19)

bool readMPU6050(TwoWire &bus, uint8_t addr, int16_t &ax, int16_t &ay, int16_t &az,
                 int16_t &gx, int16_t &gy, int16_t &gz) {
  bus.beginTransmission(addr);
  bus.write(0x3B);
  if (bus.endTransmission(false) != 0) return false;
  bus.requestFrom(addr, (uint8_t)14);
  if (bus.available() < 14) return false;

  ax = (bus.read() << 8) | bus.read();
  ay = (bus.read() << 8) | bus.read();
  az = (bus.read() << 8) | bus.read();
  bus.read(); bus.read();
  gx = (bus.read() << 8) | bus.read();
  gy = (bus.read() << 8) | bus.read();
  gz = (bus.read() << 8) | bus.read();
  return true;
}

void initMPU6050(TwoWire &bus, uint8_t addr) {
  bus.beginTransmission(addr);
  bus.write(0x6B);
  bus.write(0x00);
  bus.endTransmission();
  delay(10);
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(21, 22);      // перша I2C шина
  Wire1.begin(18, 19);     // друга I2C шина
  
  initMPU6050(Wire, MPU1_ADDR);
  initMPU6050(Wire, MPU2_ADDR);
  initMPU6050(Wire1, MPU3_ADDR);
  
  Serial.println("Three MPU6050 initialized (Wire + Wire1)");
}

void loop() {
  int16_t ax1, ay1, az1, gx1, gy1, gz1;
  int16_t ax2, ay2, az2, gx2, gy2, gz2;
  int16_t ax3, ay3, az3, gx3, gy3, gz3;

  bool ok1 = readMPU6050(Wire, MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
  bool ok2 = readMPU6050(Wire, MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);
  bool ok3 = readMPU6050(Wire1, MPU3_ADDR, ax3, ay3, az3, gx3, gy3, gz3);

  if (ok1) {
    Serial.print("MPU1: ax="); Serial.print(ax1); Serial.print(" ay="); Serial.print(ay1); Serial.print(" az="); Serial.println(az1);
  }
  if (ok2) {
    Serial.print("MPU2: ax="); Serial.print(ax2); Serial.print(" ay="); Serial.print(ay2); Serial.print(" az="); Serial.println(az2);
  }
  if (ok3) {
    Serial.print("MPU3: ax="); Serial.print(ax3); Serial.print(" ay="); Serial.print(ay3); Serial.print(" az="); Serial.println(az3);
  }

  delay(100);
}