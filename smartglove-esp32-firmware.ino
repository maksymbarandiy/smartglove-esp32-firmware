#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const uint8_t MPU1_ADDR = 0x68;
const uint8_t MPU2_ADDR = 0x69;
const uint8_t MPU3_ADDR = 0x68;

const int TOTAL_AXES = 18;
const int WINDOW_SIZE = 1000;
const int SAMPLE_RATE_MS = 10;

int16_t dataBuffer[1000][18];
int bufferIndex = 0;

enum State { IDLE, RECORDING, SENDING };
State currentState = IDLE;

void log(const String &msg, bool toSerial = true, bool toBT = true) {
  if (toSerial) Serial.println(msg);
  if (toBT) SerialBT.println(msg);
}

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
  log("Sending " + String(bufferIndex) + " frames...", true, true);
  for (int i = 0; i < bufferIndex; i++) {
    for (int j = 0; j < TOTAL_AXES; j++) {
      SerialBT.print(dataBuffer[i][j]);
      if (j < TOTAL_AXES - 1) SerialBT.print(",");
    }
    SerialBT.println();
  }
  SerialBT.println("END");
  log("Data sent successfully", true, true);
}

void readAllMPU(int16_t *frame) {
  int16_t ax1, ay1, az1, gx1, gy1, gz1;
  int16_t ax2, ay2, az2, gx2, gy2, gz2;
  int16_t ax3, ay3, az3, gx3, gy3, gz3;
  
  bool ok1 = readMPU6050(Wire, MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
  bool ok2 = readMPU6050(Wire, MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);
  bool ok3 = readMPU6050(Wire1, MPU3_ADDR, ax3, ay3, az3, gx3, gy3, gz3);
  
  if (!ok1) { ax1 = 0; ay1 = 0; az1 = 0; gx1 = 0; gy1 = 0; gz1 = 0; }
  if (!ok2) { ax2 = 0; ay2 = 0; az2 = 0; gx2 = 0; gy2 = 0; gz2 = 0; }
  if (!ok3) { ax3 = 0; ay3 = 0; az3 = 0; gx3 = 0; gy3 = 0; gz3 = 0; }
  
  frame[0] = ax1; frame[1] = ay1; frame[2] = az1;
  frame[3] = gx1; frame[4] = gy1; frame[5] = gz1;
  frame[6] = ax2; frame[7] = ay2; frame[8] = az2;
  frame[9] = gx2; frame[10] = gy2; frame[11] = gz2;
  frame[12] = ax3; frame[13] = ay3; frame[14] = az3;
  frame[15] = gx3; frame[16] = gy3; frame[17] = gz3;
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("SmartGlove");
  
  Wire.begin(21, 22);
  Wire1.begin(18, 19);
  
  initMPU6050(Wire, MPU1_ADDR);
  initMPU6050(Wire, MPU2_ADDR);
  initMPU6050(Wire1, MPU3_ADDR);
  
  log("SmartGlove v1.0 ready", true, true);
  log("Commands: START | STOP | SEND", true, true);
}

void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "START" && currentState == IDLE) {
      bufferIndex = 0;
      currentState = RECORDING;
      log("RECORDING started", true, true);
    } 
    else if (cmd == "STOP" && currentState == RECORDING) {
      currentState = IDLE;
      log("Recording stopped. " + String(bufferIndex) + " frames in buffer", true, true);
    }
    else if (cmd == "SEND" && currentState == IDLE && bufferIndex > 0) {
      currentState = SENDING;
      sendBufferedData();
      currentState = IDLE;
      bufferIndex = 0;
    }
    else if (cmd == "SEND" && bufferIndex == 0) {
      log("Buffer empty. Record something first", true, true);
    }
  }

  if (currentState == RECORDING && bufferIndex < WINDOW_SIZE) {
    int16_t frame[18];
    readAllMPU(frame);
    addToBuffer(frame);
  }
  
  delay(SAMPLE_RATE_MS);
}