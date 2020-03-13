
/**
   Copyright @2020 by Alexzander Purwoko Widiantoro <purwoko908@gmail.com>
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimedAction.h>
#include "module_runningled.h"
#define TEMP_PORT A0
#define INFRARED_PORT A1
#define PORT_LED A5
#define LDR_PORT A2

typedef struct __disps_ {
  int *pointToDisp;
  bool hasDotted;
  bool isDisplayOff;
} DisplayPoint;

double onTemp = 0.0;

bool hasFire = false;
bool lampOff = false;
DisplayPoint displayTemp[4];

void dispTemps();
void collectTemps();
void collectInfrared();
void isLampOff();
OneWire oneWire(TEMP_PORT);
DallasTemperature sensors(&oneWire);
TimedAction displayAction = TimedAction(1, dispTemps);
TimedAction collectFireAction = TimedAction(500, collectInfrared);
TimedAction collectTempAction = TimedAction(1000, collectTemps);
TimedAction lampOffAction = TimedAction(2000, isLampOff);
int *firstDisp[] = {L, O, A, d};
int *fire[] = {f, i, r, E};
int *dispLampOff[] = {
  L, A, n, P,
  empty,
  O, f, f,
  empty
};
int *dispLampOn[] = {
  L, A, n, P,
  empty,
  O, n,
  empty
};
void setup() {
  // put your setup code here, to run once:
  for (int nx = 0; nx < 4; nx++) {
    DisplayPoint *tmp = &displayTemp[nx];
    tmp -> hasDotted = false;
    tmp -> pointToDisp = firstDisp[nx];
    tmp -> isDisplayOff = false;
  }
  setupRunning4x();
  sensors.begin();
  sensors.setWaitForConversion(false);
  Serial.begin(9600);
  Serial.println("SETUP");
  pinMode(PORT_LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  collectTempAction.check();
  displayAction.check();
  collectFireAction.check();
  lampOffAction.check();
}

int *selectNum(int num) {
  switch (num) {
    case 0:
      return num_0;
    case 1:
      return num_1;
    case 2:
      return num_2;
    case 3:
      return num_3;
    case 4:
      return num_4;
    case 5:
      return num_5;
    case 6:
      return num_6;
    case 7:
      return num_7;
    case 8:
      return num_8;
    case 9:
      return num_9;
    default:
      return NULL;
  }
}

void setDispPoint(DisplayPoint *point, int *decNum, bool hasDotted, bool isDisplayOff = false) {
  point -> pointToDisp = decNum;
  point -> hasDotted = hasDotted;
  point -> isDisplayOff = isDisplayOff;
}

void applyDisp() {
  for (int hx = 0; hx < 4; hx++) {
    DisplayPoint *point = &displayTemp[hx];
    if (!(point -> isDisplayOff))
      show(hx + 1, point -> pointToDisp, point -> hasDotted);
  }
}

void dispTemps() {
  applyDisp();
}

void collectInfrared() {
  int vout = analogRead(INFRARED_PORT);
  hasFire = vout < 600;
  if (hasFire) {
    for (int nx = 0; nx < 4; nx++) {
      DisplayPoint *tmp = &displayTemp[nx];
      tmp -> hasDotted = false;
      tmp -> pointToDisp = fire[nx];
    }
    digitalWrite(PORT_LED, LOW);
  }
}

void isLampOff() {
  double vIn = analogRead(LDR_PORT);
  bool isOff = (vIn == 0.0);
  Serial.println(vIn);
  if (lampOff != isOff) {
      digitalWrite(PORT_LED, LOW);
      lampOff = isOff;
      animateFromLeft4(lampOff ? dispLampOff : dispLampOn, 9);
      digitalWrite(PORT_LED, HIGH);
  }
}
void collectTemps() {
  uint8_t addr;
  if (sensors.getAddress(&addr, 0)) {
    sensors.requestTemperaturesByAddress(&addr);
    onTemp = sensors.getTempC(&addr);
    //onTemp = *currentTemp;
    bool hasSigns = (onTemp < 1);
    bool is3digits = (onTemp > 99);
    setDispPoint(&displayTemp[3], c, false);
    if (hasSigns) {
      setDispPoint(&displayTemp[0], strips, false);
      int allDigit = (int) onTemp;
      setDispPoint(&displayTemp[2], selectNum(allDigit % 10), false);
      allDigit /= 10;
      setDispPoint(&displayTemp[1], selectNum(allDigit % 10), false);
    }
    else if (is3digits) {
      int allDigit = (int) onTemp;
      for (int hx = 3; hx > 0; hx--) {
        setDispPoint(&displayTemp[hx - 1], selectNum(allDigit % 10), false);
        allDigit /= 10;
      }
    } else {
      int allDigit = (int) (onTemp * 10);
      for (int hx = 3; hx > 0; hx--) {
        int digits = allDigit % 10;
        setDispPoint(&displayTemp[hx - 1], selectNum(digits), false, (hx == 1 && digits == 0));
        allDigit /= 10;
      }
      setDispPoint(&displayTemp[1], displayTemp[1].pointToDisp, true);
    }
    digitalWrite(PORT_LED, (onTemp > 40) ? LOW : HIGH);
  }
}
