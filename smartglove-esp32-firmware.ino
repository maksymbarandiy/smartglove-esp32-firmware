#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const uint8_t MPU1_ADDR = 0x68;
const uint8_t MPU2_ADDR = 0x69;
const uint8_t MPU3_ADDR = 0x68;

bool recording = false;

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
  SerialBT.begin("SmartGlove");
  
  Wire.begin(21, 22);
  Wire1.begin(18, 19);
  
  initMPU6050(Wire, MPU1_ADDR);
  initMPU6050(Wire, MPU2_ADDR);
  initMPU6050(Wire1, MPU3_ADDR);
  
  SerialBT.println("SmartGlove ready. Send START or STOP");
  Serial.println("Three MPU6050 initialized");
}

void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    if (cmd == "START") {
      recording = true;
      SerialBT.println("Recording started");
    } else if (cmd == "STOP") {
      recording = false;
      SerialBT.println("Recording stopped");
    }
  }

  if (recording) {
    int16_t ax1, ay1, az1, gx1, gy1, gz1;
    int16_t ax2, ay2, az2, gx2, gy2, gz2;
    int16_t ax3, ay3, az3, gx3, gy3, gz3;

    bool ok1 = readMPU6050(Wire, MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
    bool ok2 = readMPU6050(Wire, MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);
    bool ok3 = readMPU6050(Wire1, MPU3_ADDR, ax3, ay3, az3, gx3, gy3, gz3);

    if (ok1 && ok2 && ok3) {
      SerialBT.print(ax1); SerialBT.print(","); SerialBT.print(ay1); SerialBT.print(","); SerialBT.print(az1); SerialBT.print(",");
      SerialBT.print(gx1); SerialBT.print(","); SerialBT.print(gy1); SerialBT.print(","); SerialBT.print(gz1); SerialBT.print(",");
      SerialBT.print(ax2); SerialBT.print(","); SerialBT.print(ay2); SerialBT.print(","); SerialBT.print(az2); SerialBT.print(",");
      SerialBT.print(gx2); SerialBT.print(","); SerialBT.print(gy2); SerialBT.print(","); SerialBT.print(gz2); SerialBT.print(",");
      SerialBT.print(ax3); SerialBT.print(","); SerialBT.print(ay3); SerialBT.print(","); SerialBT.print(az3); SerialBT.print(",");
      SerialBT.print(gx3); SerialBT.print(","); SerialBT.print(gy3); SerialBT.print(","); SerialBT.println(gz3);
    }
  }
  
  delay(100);
}