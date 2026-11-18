#include <BLEDevice.h>

int led = 0;
int ledPlaca = 8;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(ledPlaca, OUTPUT);

  BLEDevice::init("SempreAqui");
  BLEDevice::startAdvertising();
  Serial.println("Pulseira emitindo BLE");
}

void loop() {
  digitalWrite(led, HIGH);
  digitalWrite(ledPlaca, LOW);
  delay(100);
  digitalWrite(led, LOW);
  digitalWrite(ledPlaca, HIGH);
  delay(1000);
}
