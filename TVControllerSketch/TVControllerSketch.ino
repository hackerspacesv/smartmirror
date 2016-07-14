#include <Wire.h>
/*
 * Toshiba Arduino based-Controller for Raspberry Pi.
 * Copyright 2016 Mario Gomez <mario.gomez_at_teubi.co>
 * This code is protected by the GNU/GPL v2.0.
 *
 * Motivation: In the Hackerspace San Salvador Joksan
 * alvarado did a little bit of reverse-ingeneering for
 * the touch panel in our LCD TV. The interface is pretty
 * simple using two banks of voltage divisors that signal
 * the tv for the different control panel functions.
 * However the Raspberry Pi GPIO has really low impedance
 * on its inputs making the voltage division unfeasible.
 * A middle interface is needed between the Raspberry Pi
 * and the TV controller. An ATMega328P at 8MHz@3.3V was
 * selected due to easy to program and low component count.
 * 
 * This sketch simulates the original controller, works as
 * a priority encoder for the voltage divisor and also
 * monitors the TV "power" line to monitor the current status
 * of the TV. It can also be controlled via I2C.
 * 
 * This can be made work with I2C or directly as a buffer
 * between the inputs.
 */

uint8_t bus_config = 0;
uint8_t old_bank = 0;
volatile uint8_t i2c_bank = 0;

void setup() {
  Wire.begin(0x1E); // 1E came from XORing ASCII  hex codes
                    // of first letters of *S*mart *M*irror
  Wire.onReceive(receiveEvent);
  
  // Power LED from TV
  pinMode(A0, INPUT); // Input from Power LED
  pinMode(A1, OUTPUT); // Output to Status LED
  pinMode(A2, OUTPUT); // Output to GPIO
  pinMode(A3, OUTPUT); // Heartbeat
  
  bus_config = 1; // BUS auto-configuration
                  // Sets I2C control on first
                  // message received. To clear
                  // Sent an I2C byte with MSB
                  // Set to 1
  
  // Setup almost everything as input
  // Raspberry GPIO is connected between 0-6
  // Voltage divisors are connected between 7-13
  for(int i=0;i<14;i++) {
    if(i<7) {
      pinMode(i, INPUT_PULLUP);
    } else {
      pinMode(i, INPUT);
    }
  }
}

uint8_t heartbeat = 0;

void loop() {
  // Negate & activate status LED.
  uint8_t pwr_status = (uint8_t)digitalRead(A0)^0x01;
  digitalWrite(A1,pwr_status);
  digitalWrite(A2,pwr_status);

  // Heartbeat monitor
  if((millis()+500)%500 == 0) {
    digitalWrite(A3, heartbeat ^= 0x01);
  }

  // Read input status and store on temp variables
  // This is used lated for priority encoding
  uint8_t bank = 0;
  if(bus_config) {
    for(int i=0;i<7;i++)
      bank |= digitalRead(i)<<i;
  } else {
    bank = i2c_bank;
  }
  
  // Priority selection and grounding for output banks
  // Always do clear first and set latter
  if(bank!=old_bank) {
    // Float pin for old pin
    for(int i=0;i<7;i++) {
      if(old_bank&(1<<(7-i))>0) {
        pinMode(i+7, INPUT);
        break;
      }
    }
    // Ground selected pin
    for(int i=0;i<7;i++) {
      if(bank&(1<<(7-i))>0) {
        pinMode(i+7, OUTPUT);
        digitalWrite(i+7, 0);
        break;
      }
    }
    old_bank = bank;
  }
}

void receiveEvent(int count) {
  while (Wire.available() > 0) {
    i2c_bank = Wire.read();
    bus_config = (i2c_bank>>8)==0;
  }
}

