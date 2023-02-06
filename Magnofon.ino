#include <MIDI.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

float offset = 0;
int lastPotRead = 0;
int lastButtonRead = 0;

float potRead;
float angleRead;
float octaveRead;
float velocity;

MPU6050 mpu(Wire);
MIDI_CREATE_DEFAULT_INSTANCE();


void setup() {
  // put your setup code here, to run once:
  MIDI.begin(MIDI_CHANNEL_OFF);
  Wire.begin();
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  compass.init();

  byte status = mpu.begin();
  while (status != 0) {}
  delay(1000);
  mpu.calcOffsets();  // gyro and accelero


  lastPotRead = map(analogRead(A0), 0, 1023, 0, 24);


  compass.read();
  offset = abs(compass.getZ());
}

void loop() {
  compass.read();
  mpu.update();

  velocity = mpu.getAngleX();
  if (velocity < 0) {
    velocity = 0;
  } else if (velocity > 90) {
    velocity = 90;
  } else {
    velocity = map(velocity * 4, 0, 360, 0, 127);
  }

  // put your main code here, to run repeatedly:

  octaveRead = constrain(map(analogRead(A1), 0, 1023, 0, 3), 0, 2);
  angleRead = round(map(abs(compass.getZ() - offset), 0, 10000, 0, 30) / 6);
  potRead = (angleRead + 36 + octaveRead * 12);


  int buttonRead = digitalRead(2);

  if (buttonRead == HIGH && lastButtonRead == LOW) {
    MIDI.sendNoteOn(potRead, velocity, 1);
  }

  if (buttonRead == HIGH) {
    if (potRead != lastPotRead) {
      MIDI.sendNoteOff(lastPotRead, 0, 1);
      MIDI.sendNoteOn(potRead, velocity, 1);
    }
  }

  if (buttonRead == LOW) {
    MIDI.sendNoteOff(lastPotRead, 0, 1);
    MIDI.sendNoteOff(potRead, 0, 1);
  }

  lastPotRead = potRead;
  lastButtonRead = buttonRead;
}
