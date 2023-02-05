#include <MIDI.h>
#include "Wire.h"
#include <MPU6050_light.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

float offset = 0;
int lastPotRead = 0;
int lastButtonRead = 0;

const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
char* noteName;
int note;
int noteM;

float potRead;
float angleRead;
float octaveRead;
float velocity;

MPU6050 mpu(Wire);


#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32




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
