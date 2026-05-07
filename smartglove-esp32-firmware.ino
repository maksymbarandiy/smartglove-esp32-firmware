#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const uint8_t MPU1_ADDR = 0x68;
const uint8_t MPU2_ADDR = 0x69;
const uint8_t MPU3_ADDR = 0x68;

const int TOTAL_AXES = 18;
const int WINDOW_SIZE = 1000;
int16_t dataBuffer[1000][18];
int bufferIndex = 0;

enum State { IDLE, RECORDING, SENDING };
State currentState = IDLE;

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

void addToBuffer(int16_t *data) {
  if (bufferIndex < WINDOW_SIZE) {
    for (int i = 0; i < TOTAL_AXES; i++) {
      dataBuffer[bufferIndex][i] = data[i];
    }
    bufferIndex++;
  }
}

void sendBufferedData() {
  for (int i = 0; i < bufferIndex; i++) {
    for (int j = 0; j < TOTAL_AXES; j++) {
      SerialBT.print(dataBuffer[i][j]);
      if (j < TOTAL_AXES - 1) SerialBT.print(",");
    }
    SerialBT.println();
  }
  SerialBT.println("END");
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("SmartGlove");
  
  Wire.begin(21, 22);
  Wire1.begin(18, 19);
  
  initMPU6050(Wire, MPU1_ADDR);
  initMPU6050(Wire, MPU2_ADDR);
  initMPU6050(Wire1, MPU3_ADDR);
  
  SerialBT.println("SmartGlove ready. Commands: START, STOP, SEND");
  Serial.println("State machine: IDLE/RECORDING/SENDING");
}

void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "START" && currentState == IDLE) {
      bufferIndex = 0;
      currentState = RECORDING;
      SerialBT.println("RECORDING started");
    } 
    else if (cmd == "STOP" && currentState == RECORDING) {
      currentState = IDLE;
      SerialBT.print("Recording stopped. ");
      SerialBT.print(bufferIndex);
      SerialBT.println(" frames captured");
    }
    else if (cmd == "SEND" && currentState == IDLE && bufferIndex > 0) {
      currentState = SENDING;
      SerialBT.println("SENDING data...");
      sendBufferedData();
      currentState = IDLE;
      SerialBT.println("Data sent. Buffer cleared");
      bufferIndex = 0;
    }
  }

  if (currentState == RECORDING && bufferIndex < WINDOW_SIZE) {
    int16_t ax1, ay1, az1, gx1, gy1, gz1;
    int16_t ax2, ay2, az2, gx2, gy2, gz2;
    int16_t ax3, ay3, az3, gx3, gy3, gz3;

    bool ok1 = readMPU6050(Wire, MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
    bool ok2 = readMPU6050(Wire, MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);
    bool ok3 = readMPU6050(Wire1, MPU3_ADDR, ax3, ay3, az3, gx3, gy3, gz3);

    if (ok1 && ok2 && ok3) {
      int16_t frame[18] = {ax1, ay1, az1, gx1, gy1, gz1,
                           ax2, ay2, az2, gx2, gy2, gz2,
                           ax3, ay3, az3, gx3, gy3, gz3};
      addToBuffer(frame);
    }
  }
  
  delay(10);
}