#include <Wire.h>
#include "BluetoothSerial.h"

const uint8_t MPU1_ADDR = 0x68;
const uint8_t MPU2_ADDR = 0x69;
const uint8_t MPU3_ADDR = 0x69;
const int TOTAL_AXES = 18;
const int WINDOW_SIZE = 1000;
const int SAMPLE_RATE_MS = 10;
const int AXES_PER_MPU = 6;

enum State { IDLE, RECORDING, SENDING };
State currentState = IDLE;

float dataBuffer[WINDOW_SIZE][TOTAL_AXES];
int sampleCount = 0;
unsigned long lastSampleTime = 0;

BluetoothSerial SerialBT;

// ── Лог хелпер ──────────────────────────────────────────────
void log(const char* tag, const String& msg) {
  Serial.print("["); Serial.print(tag); Serial.print("] ");
  Serial.println(msg);
}

bool readMPU6050(TwoWire &bus, uint8_t addr,
                 int16_t &ax, int16_t &ay, int16_t &az,
                 int16_t &gx, int16_t &gy, int16_t &gz) {
  bus.beginTransmission(addr);
  bus.write(0x3B);
  if (bus.endTransmission(false) != 0) {
    log("MPU", "endTransmission failed, addr=0x" + String(addr, HEX));
    return false;
  }
  bus.requestFrom(addr, (uint8_t)14);
  if (bus.available() < 14) {
    log("MPU", "not enough bytes, addr=0x" + String(addr, HEX));
    return false;
  }
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
  uint8_t err = bus.endTransmission();
  if (err == 0) {
    log("INIT", "MPU OK addr=0x" + String(addr, HEX));
  } else {
    log("INIT", "MPU FAIL addr=0x" + String(addr, HEX) + " err=" + String(err));
  }
  delay(10);
}

void readAllMpu(float dataRow[]) {
  int16_t ax, ay, az, gx, gy, gz;
  int idx = 0;

  if (readMPU6050(Wire, MPU1_ADDR, ax, ay, az, gx, gy, gz)) {
    dataRow[idx++]=ax; dataRow[idx++]=ay; dataRow[idx++]=az;
    dataRow[idx++]=gx; dataRow[idx++]=gy; dataRow[idx++]=gz;
  } else {
    log("READ", "MPU1 read failed — filling zeros");
    for (int i=0;i<AXES_PER_MPU;i++) dataRow[idx++]=0;
  }

  if (readMPU6050(Wire, MPU2_ADDR, ax, ay, az, gx, gy, gz)) {
    dataRow[idx++]=ax; dataRow[idx++]=ay; dataRow[idx++]=az;
    dataRow[idx++]=gx; dataRow[idx++]=gy; dataRow[idx++]=gz;
  } else {
    log("READ", "MPU2 read failed — filling zeros");
    for (int i=0;i<AXES_PER_MPU;i++) dataRow[idx++]=0;
  }

  if (readMPU6050(Wire1, MPU3_ADDR, ax, ay, az, gx, gy, gz)) {
    dataRow[idx++]=ax; dataRow[idx++]=ay; dataRow[idx++]=az;
    dataRow[idx++]=gx; dataRow[idx++]=gy; dataRow[idx++]=gz;
  } else {
    log("READ", "MPU3 read failed — filling zeros");
    for (int i=0;i<AXES_PER_MPU;i++) dataRow[idx++]=0;
  }
}

void sendBufferedData() {
  log("SEND", "Starting transfer, frames=" + String(sampleCount));
  for (int i = 0; i < sampleCount; i++) {
    for (int j = 0; j < TOTAL_AXES; j++) {
      SerialBT.print(dataBuffer[i][j]);
      if (j < TOTAL_AXES - 1) SerialBT.print(',');
    }
    SerialBT.println();

    if (i % 20 == 0) {
      log("SEND", "Progress: " + String(i) + "/" + String(sampleCount));
      delay(10);
    }
  }
  SerialBT.println("END");
  log("SEND", "Transfer complete, sent END");
}

void handleBluetoothCommand() {
  if (!SerialBT.available()) return;

  String command = SerialBT.readStringUntil('\n');
  command.trim();
  log("CMD", "Received: '" + command + "'");

  if (command == "START") {
    if (currentState != IDLE) {
      log("CMD", "Ignoring START — not in IDLE, state=" + String(currentState));
      return;
    }
    currentState = RECORDING;
    sampleCount = 0;
    log("CMD", "→ RECORDING");

  } else if (command == "STOP") {
    if (currentState != RECORDING) {
      log("CMD", "Ignoring STOP — not in RECORDING, state=" + String(currentState));
      return;
    }
    currentState = SENDING;
    log("CMD", "→ SENDING");

  } else if (command == "RESET") {
    currentState = IDLE;
    sampleCount = 0;
    log("CMD", "→ IDLE (reset)");

  } else {
    log("CMD", "Unknown command: '" + command + "'");
  }
}

void handleRecording() {
  unsigned long now = millis();
  if ((now - lastSampleTime) < SAMPLE_RATE_MS) return;
  lastSampleTime = now;

  if (sampleCount < WINDOW_SIZE) {
    readAllMpu(dataBuffer[sampleCount]);
    sampleCount++;
    // Лог кожні 100 фреймів щоб не спамити
    if (sampleCount % 100 == 0) {
      log("REC", "Frames collected: " + String(sampleCount));
    }
  } else {
    log("REC", "Buffer full (1000 frames) → auto SENDING");
    currentState = SENDING;
  }
}

void handleSending() {
  sendBufferedData();
  currentState = IDLE;
  sampleCount = 0;
  log("SEND", "Done → IDLE");
}

void setup() {
  Serial.begin(115200);
  log("BOOT", "Starting...");

  SerialBT.begin("SmartGlove_ESP32");
  log("BOOT", "Bluetooth started: SmartGlove_ESP32");

  Wire.begin(21, 22);
  Wire1.begin(18, 19);
  log("BOOT", "I2C buses initialized");

  initMPU6050(Wire,  MPU1_ADDR);
  initMPU6050(Wire,  MPU2_ADDR);
  initMPU6050(Wire1, MPU3_ADDR);

  log("BOOT", "Setup complete, waiting for BT connection...");
}

void loop() {
  handleBluetoothCommand();

  switch (currentState) {
    case RECORDING: handleRecording(); break;
    case SENDING:   handleSending();   break;
    default: break;
  }
}