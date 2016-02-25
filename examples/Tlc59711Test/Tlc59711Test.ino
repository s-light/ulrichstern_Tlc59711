//
// Tests for Tlc59711 library
//
// 22 Feb 2016 by Ulrich Stern
//
// notes:
// * for "flicker" tests, values written at ~30 Hz to make possible flickering
//  visible
//

#include <Tlc59711.h>

const int NUM_TLC = 2;
const int NUM_MODES = 6;   // data transfer modes to test

Tlc59711 tlc(NUM_TLC);

void setup() {
  Serial.begin(9600);
  
  Serial.println("testing Tlc59711 library");
  Serial.println("number chips: " + String(NUM_TLC) + "\n");
}

void loop() {
  timeWrite();
  Serial.println();
  for (int m=0; m<NUM_MODES; m++) {
    setXferMode(m);
    flickerRamp();
    blink();
    flickerConst();
    Serial.println();
  }
  setXferMode(0);
  brightnessRed();
  r0firstB3last();
  Serial.println();
}

void testDone(int delayMillis = 0);

void timeWrite() {
  int n = 100;
  testMessage("timing " + String(n) + " write() calls");
  for (int m=0; m<NUM_MODES; m++)
    if (setXferMode(m)) {
      unsigned long us = micros();
      for (int i=0; i<n; i++)
        tlc.write();
      unsigned long dus = micros()-us;
      Serial.println("  per write(): " + String(dus/float(n), 1) + " us, " +
        "per bit: " + String(dus/float((long)n*NUM_TLC*224), 2) + " us");
    }
    else
      // duration of calls to tlc.write() can be measured correctly using
      // micros() with disabled interrupts if duration is less than one 
      // millisecond; see http://arduino.stackexchange.com/a/4381/14765
      Serial.println("  skipped");
  testDone(5000);
}

void flickerRamp() {
  testMessage("flicker ramp");
  for (uint16_t i = 0; i < 5000; i+=30){
    tlc.setRGB(i, i, i);
    tlc.write();
    delay(33);
  }
  testDone();
}

void flickerConst() {
  testMessage("flicker constant");
  for (int i=0; i<30*5; i++) {
    tlc.setRGB(32768, 6554, 655);
    tlc.write();
    delay(33);
  }
  testDone();
}

void blink() {
  testMessage("blink");
  for (int i=0; i<50; i++) {
    int m = i%2;
    tlc.setRGB(32768*m, 6554*m, 655*m);
    tlc.write();
    delay(100);
  }
  testDone();
}

void brightnessRed() {
  testMessage("brightness red");
  tlc.setRGB(32768, 32768, 32768);
  for (int i = 0; i < 50; i++) {
    tlc.setBrightness(16*(i%2), 0, 0);
    tlc.write();
    delay(200);
  }
  testDone();
}

void r0firstB3last() {
  testMessage("R0 1st chip, B3 last chip");
  tlc.setChannel(0, 6554);
  for (int i=0; i<50; i++) {
    tlc.setChannel(12*NUM_TLC-1, 6554*(i%2));
    tlc.write();
    delay(200);
  }
  testDone();
}

// - - -

void testMessage(String msg) {
  Serial.println("test: " + msg);
}

void testDone(int delayMillis) {
  tlc.reset();
  tlc.write();
  delay(delayMillis);
}

bool setXferMode(int mode) {
  bool timeable = true;
  if (mode == 0) {
    tlc.beginFast();
    Serial.println("xfer: SPI, 10 MHz, buffer...");
  } else if (mode == 1) {
    tlc.beginFast();
    tlc.setTmgrst(false);
    Serial.println("xfer: SPI, 10 MHz, buffer, TMGRST=0...");
  } else if (mode == 2) {
    tlc.beginFast(false);
    Serial.println("xfer: SPI, 10 Mhz, 16 bits at a time...");
  } else if (mode == 3) {
    tlc.beginFast(true, 2000000, 6);
    Serial.println("xfer: SPI, 2 MHz, buffer...");
  } else if (mode == 4) {
    tlc.beginSlow(200, true);
    Serial.println("xfer: no hardware support, interrupts...");
  } else if (mode == 5) {
    tlc.beginSlow();
    Serial.println("xfer: no hardware support, no interrupts...");
    timeable = false;
  }
  return timeable;
}

