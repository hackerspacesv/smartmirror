/*
 * Toshiba Arduino-Controller for Raspberry Pi.
 * Copyright 2016 Mario Gómez <mario.gomez_at_teubi.co>
 * This code is protected by the GNU/GPL v2.0.
 *
 * Motivation: In the Hackerspace San Salvador we Joksan
 * alvarado did a little bit of reverse-ingeneering for
 * the touch panel in our LCD TV. The interface is pretty
 * simple using two banks of voltage divisors that signal
 * the tv for the different control panel functions.
 * However the Raspberry Pi GPIO have really low impedance
 * on its inputs making the voltage division impossible.
 * A middle interface is needed between the Raspberry Pi
 * and the TV controller. An ATMega328P at 8MHz@3.3V was
 * selected due to easy to program and low component setup.
 * 
 * This sketch simulates the original controller, works as
 * a priority encoder for the voltage divisor and also
 * monitors the TV "power" line to monitor the current status
 * of the TV.
 */

void setup() {
  // Power LED from TV
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  // Raspberry Pi inputs
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  // Voltage divisors
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
}

uint8_t old_bank1 = 0;
uint8_t old_bank2 = 0;

void loop() {
  // Negate & activate status LED.
  uint8_t pwr_status = (uint8_t)digitalRead(A0)^0x01;
  digitalWrite(A1,pwr_status);
  digitalWrite(A2,pwr_status);

  // Read input status and store on temp variables
  // This is used lated for priority encoding
  uint8_t bank1 = 0;
  for(int i=0;i<4;i++)
    bank1 |= digitalRead(i)<<i;

  uint8_t bank2 = 0;
  for(int i=0;i<3;i++)
    bank2 |= digitalRead(i+4)<<i;

  // Priority selection and grounding for bank1
  if(bank1!=old_bank1) {
    // Float pin for old pin
    for(int i=0;i<4;i++) {
      if(old_bank1&(1<<(4-i))>0) {
        pinMode(i+7, INPUT);
        break;
      }
    }
    // Ground selected pin
    for(int i=0;i<4;i++) {
      if(bank1&(1<<(4-i))>0) {
        pinMode(i+7, OUTPUT);
        digitalWrite(i+7, 0);
        break;
      }
    }
    old_bank1 = bank1;
  }

  // Priority selection and grounding for bank2
  if(bank2!=old_bank2) {
    // Float pin for old selected
    for(int i=0;i<3;i++) {
      if(old_bank1&(1<<(3-i))>0) {
        pinMode(i+11, INPUT);
        break;
      }
    }
    // Ground selected pin
    for(int i=0;i<3;i++) {
      if(bank1&(1<<(3-i))>0) {
        pinMode(i+11, OUTPUT);
        digitalWrite(i+11, 0);
        break;
      }
    }
    old_bank2 = bank2;
  }
}
