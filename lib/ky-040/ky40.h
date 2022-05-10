#pragma once
#include <Arduino.h>

int pinA;       //ky-040 clk pin, interrupt & add 100nF/0.1uF capacitors between pin & ground!!!
int pinB;       //ky-040 dt  pin,             add 100nF/0.1uF capacitors between pin & ground!!!
int pinButton;  //ky-040 sw  pin, interrupt & add 100nF/0.1uF capacitors between pin & ground!!!

volatile int16_t kyPosition = 0;
volatile int16_t kyLastChange = 0;
volatile uint32_t kyPressReleaseTime = 0;
volatile uint32_t kyPressTime = 0;
volatile bool kyPressed = false;
volatile bool kyLastRead = false;

void ICACHE_RAM_ATTR encoderISR() {
  noInterrupts();
  bool reading = digitalRead(pinB);
  if(millis() - kyLastChange < ((reading == kyLastRead) ? 30 : 200) || kyPressed || millis() - kyPressReleaseTime < 100) return;
  kyLastChange = millis();
  if(reading) {
    kyPosition--;
  } else {
    kyPosition++;
  }
  kyLastRead = reading;
  interrupts();
}

void ICACHE_RAM_ATTR encoderButtonISR() {
  noInterrupts();
  kyPressed = !digitalRead(pinButton);
  if(kyPressed) {
      kyPressTime = millis();
  }
  kyPressReleaseTime = millis();
  interrupts();
}

void beginKY40(int pinA1, int pinB1, int pinButton1) {
    pinA = pinA1;
    pinB = pinB1;
    pinButton = pinButton1;
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB1, INPUT_PULLUP);
    pinMode(pinButton1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinA),  encoderISR,       RISING);
    attachInterrupt(digitalPinToInterrupt(pinButton), encoderButtonISR, CHANGE);
}