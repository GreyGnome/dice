// From: TrueRandomSeed.ino
//   This example sketch shows how to provide a truly random seed value to the built in 
//   library pseudo random number generator.  This ensures that your sketch will be
//   using a different sequence of random numbers every time it runs.  Unlike the 
//   usually suggested randomSeed(analogRead(0)) this method will provide a much more 
//   uniform and varied seed value.  For more information about the basic technique used
//   here to produce a random number or if you need more than one such number you can 
//   find a library, Entropy from the following web site along with documentation of how
//   the library has been tested to provide TRUE random numbers on a variety of AVR 
//   chips and arduino environments. 
//
//   https://sites.google.com/site/astudyofentropy/project-definition/
//           timer-jitter-entropy-sources/entropy-library
//
//   Copyright 2014 by Walter Anderson, wandrson01 at gmail dot com
//
 
//#include <EnableInterrupt.h>
// Using vim: :set ts=2 sts=2 sw=2 et ff=unix

#include <EEPROM.h>
#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "digitalWriteFast.h"

// === vvv RANDOM vvv ==================================================================
// === vvv RANDOM vvv ==================================================================
// === vvv RANDOM vvv ==================================================================
// The following addresses a problem in version 1.0.5 and earlier of the Arduino IDE 
// that prevents randomSeed from working properly.
//        https://github.com/arduino/Arduino/issues/575
#define randomSeed(s) srandom(s)
 
volatile uint32_t seed;

// Note that the first value of seed is populated in setup().
// The idea is: after the full 32-bit seed is generated, grab one more
// value from TCNT2 and store it in EEPROM.
// The very first time in this program's life, seed will start with the value
// 255 because the EEPROM is filled with all 0xFF's.
void populate_seed(void) {
  seed = seed << 8;     // count=4, seed = 0,0,0,0
                        // count=3, seed=0,0,TCNT2(4),0
                        // count=2, seed=0,TCNT2(4),TCNT(3),0
                        // count=1, seed=TCNT2(4),TCNT2(3),TCNT2(2),0
  seed |= seed | TCNT2; // count=4, seed=0,0,0,TCNT2(4)
                        // count=3, seed=0,0,TCNT2(4),TCNT2(3)
                        // count=2, seed=0,TCNT2(4),TCNT2(3),TCNT2(2)
                        // count=1, TCNT2(4),TCNT2(3),TCNT2(2),TCNT2(1)
}
// === ^^^ RANDOM ^^^ ==================================================================
// === ^^^ RANDOM ^^^ ==================================================================
// === ^^^ RANDOM ^^^ ==================================================================
 
// === DESCRIPTION =====================================================================
// ATMEL ATMEGA8A / ARDUINO
//
//                           +-\/-+
//     RESET   (D 22)  PC6  1|    |28  PC5  (D 19  A5  SCL ADC5)
//             (D  0)  PD0  2|    |27  PC4  (D 18  A4  SDA ADC4)
//             (D  1)  PD1  3|    |26  PC3  (D 17  A3  ADC3)
//       INT0  (D  2)  PD2  4|    |25  PC2  (D 16  A2  ADC2)
//       INT1  (D  3)  PD3  5|    |24  PC1  (D 15  A1  ADC1)
//             (D  4)  PD4  6|    |23  PC0  (D 14  A0  ADC0)
//                     VCC  7|    |22  GND
//                     GND  8|    |21  AREF
//             (D 20)  PB6  9|    |20  AVCC
//             (D 21)  PB7 10|    |19  PB5  (D 13  SCK) 
//             (D  5)  PD5 11|    |18  PB4  (D 12  MISO) 
//  AIN0       (D  6)  PD6 12|    |17  PB3  (D 11  MOSI OC2)
//  AIN1       (D  7)  PD7 13|    |16  PB2  (D 10  SS OC1A)
//             (D  8)  PB0 14|    |15  PB1  (D  9     OC1B)
//                           +----+
//  AIN == Analog Comparator Input

//  O   O  NUMBERING:   1   5     1 and 7 always light together
//  O O O  DICE         2 4 6     2 and 6 always light together
//  O   O  LED's        3   7     3 and 5 always light together


#define SLEEP_AFTER_MILLIS 20000
#define INTR_TOGGLE 0 // D 0
// Use PB instead: Digital Pins 8, 9, 10, 11, 12, 13, 20, 21
#define DIE_1  6 // PORTB, pin 6, Digital pin 20
#define DIE_2  7 // PORTB, pin 7, Digital pin 21
#define DIE_3  0 // PORTB, pin 0, Digital pin  8
#define DIE_4  1 // PORTB, pin 1, Digital pin  9
#define DIE_5  5 // PORTB, pin 5, Digital pin 13
#define DIE_6  4 // PORTB, pin 4, Digital pin 12
#define DIE_7  3 // PORTB, pin 3, Digital pin 11

#define SET_DIE_INPUT  DDRB &= 0b00000100;
#define PULLUP_DIE    PORTB |= 0b11111011;
#define SET_DIE_OUTPUT DDRB |= 0b11111011;
#define OFF_PORT_B    PORTB &= 0b00000100;

volatile uint8_t dot1=0;
volatile uint8_t dot2=0;
volatile uint8_t dot3=0;
volatile uint8_t dot4=0;
volatile uint8_t dot5=0;
volatile uint8_t dot6=0;
volatile uint8_t dot7=0;

inline void off_1(void) { dot1=LOW; }
inline void off_2(void) { dot2=LOW; }
inline void off_3(void) { dot3=LOW; }
inline void off_4(void) { dot4=LOW; }
inline void off_5(void) { dot5=LOW; }
inline void off_6(void) { dot6=LOW; }
inline void off_7(void) { dot7=LOW; }
inline void on_1(void) { dot1=HIGH; }
inline void on_2(void) { dot2=HIGH; }
inline void on_3(void) { dot3=HIGH; }
inline void on_4(void) { dot4=HIGH; }
inline void on_5(void) { dot5=HIGH; }
inline void on_6(void) { dot6=HIGH; }
inline void on_7(void) { dot7=HIGH; }

void die1(void) {
  off_1(); off_2(); off_3();  on_4(); off_5(); off_6(); off_7();
};
void die2(void) {
  off_1(); off_2();  on_3(); off_4();  on_5(); off_6(); off_7();
};
void die3(void) {
  off_1(); off_2();  on_3();  on_4();  on_5(); off_6(); off_7();
};
void die4(void) {
   on_1(); off_2();  on_3(); off_4();  on_5(); off_6();  on_7();
};
void die5(void) {
   on_1(); off_2();  on_3();  on_4();  on_5(); off_6();  on_7();
};
void die6(void) {
   on_1();  on_2();  on_3(); off_4();  on_5();  on_6();  on_7();
};
void dieAll(void) {
   on_1();  on_2();  on_3();  on_4();  on_5();  on_6();  on_7();
};
void dieOff(void) {
  off_1(); off_2(); off_3(); off_4(); off_5(); off_6(); off_7();
};

#define HEADS 6
#define TAILS 7

// These must be these pins because external interrupts are on them.
#define ROLL  2
#define FLIP  3

volatile uint8_t die_value=0;
volatile uint8_t led_sequence=0;

// Port B, pins 6 and 7 are used for the crystal so they're usually not
// available. This makes DigitalWriteFast useless for those pins.
// But we're going to use them in this circuit, so I'll control Port B like this:
#define writeDot1(V) bitWrite(PORTB, 6, V);
#define writeDot2(V) bitWrite(PORTB, 7, V);
#define writeDot3(V) bitWrite(PORTB, 0, V);
#define writeDot4(V) bitWrite(PORTB, 1, V);
#define writeDot5(V) bitWrite(PORTB, 5, V);
#define writeDot6(V) bitWrite(PORTB, 4, V);
#define writeDot7(V) bitWrite(PORTB, 3, V);
//ISR(TIMER2_COMP_vect) {
ISR(TIMER2_OVF_vect) {
  OFF_PORT_B; // NICE!
  switch (led_sequence){
    case 0: writeDot1(dot1); break;
    case 1: writeDot2(dot2); break;
    case 2: writeDot3(dot3); break;
    case 3: writeDot4(dot4); break;
    case 4: writeDot5(dot5); break;
    case 5: writeDot6(dot6); break;
    case 6: writeDot7(dot7); break;
    default: break;
  }
  led_sequence++;
  if (led_sequence == 7) led_sequence = 0;
}

void show_die(uint8_t number) {
  switch (number) {
  case 1: die1(); break;
  case 2: die2(); break;
  case 3: die3(); break;
  case 4: die4(); break;
  case 5: die5(); break;
  case 6: die6(); break;
  }
}
void heads(void) {
  digitalWriteFast(HEADS, HIGH); digitalWriteFast(TAILS, LOW);
}
void tails(void) {
  digitalWriteFast(HEADS, LOW); digitalWriteFast(TAILS, HIGH);
}
// This doesn't work. The LED with the lower forward voltage will light. The other won't
/*
void heads_tails_on(void) {
  digitalWriteFast(HEADS, HIGH); digitalWriteFast(TAILS, HIGH);
}
*/

void heads_tails_1_sec(void) {
  uint8_t i;
  for (i=0; i < 20; i++) {
    digitalWriteFast(HEADS, HIGH); digitalWriteFast(TAILS, LOW);
    delay(50);
    digitalWriteFast(HEADS, LOW); digitalWriteFast(TAILS, HIGH);
    delay(50);
  }
}

void heads_tails_off(void) {
  digitalWriteFast(HEADS, LOW); digitalWriteFast(TAILS, LOW);
}

// NOTE: pinModeFast does not appear to be working at the moment (8/2019).
// Just use this for now.
void set_pin_directions() {
  pinMode(ROLL, INPUT_PULLUP);
  pinMode(FLIP, INPUT_PULLUP);
  SET_DIE_OUTPUT;
  pinMode(HEADS, OUTPUT); pinMode(TAILS, OUTPUT);
}

void set_all_pins_input() {
  pinMode(ROLL, INPUT_PULLUP);
  pinMode(FLIP, INPUT_PULLUP);
  SET_DIE_INPUT;
  pinMode(HEADS, INPUT); pinMode(TAILS, INPUT);
}

// blink the number 4, then show the value of the lowest byte of the seed
// bit by bit, starting with the leftmost bit, by displaying an led
// between led's 2 and 6.
void show_seed() {
  uint8_t lowest_byte = seed & 0x000000FF;
  uint8_t i;
  for (i=0; i < 4; i++) { // blink 4 times
    dieAll(); delay(100); dieOff(); delay(100);
  }
  delay(500);
  i=7;
  while(1) {
    dieOff();
    for (uint8_t j=0; j < 2; j++) { die4(); delay(150); dieOff(); delay(150); }
    on_2(); on_6();
    if (_BV(i) & lowest_byte) on_4();
    delay(500);
    if (i == 0) break;
    i--;
  }
}

volatile uint8_t intr_counter = 0; // Currently unused.
void roll_interrupt() { // Currently only for testing.
  digitalWriteFast(INTR_TOGGLE, HIGH);
  OFF_PORT_B; // NICE!
  intr_counter=1;
  digitalWriteFast(INTR_TOGGLE, LOW);
}

void flip_interrupt() { // Currently only for testing.
  digitalWriteFast(INTR_TOGGLE, HIGH);
  intr_counter=1;
  digitalWriteFast(INTR_TOGGLE, LOW);
}

void go_to_sleep() {
  set_all_pins_input();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  // sleep_bod_disable(); // not on ATmega8. Default is to have it off anyway.
  sei();
  sleep_cpu(); // On wake, the very next instruction will be run.
  sleep_disable();
  sei();
  set_pin_directions();
}

uint8_t seed_bytes_remaining=4;
uint8_t poweron_seed = 1; // we'll preload the seed with each of the first 4 rolls of the dice, once after power on.
void prepare_seed() {
  if (seed_bytes_remaining > 0) {
    populate_seed();
    seed_bytes_remaining--;
    randomSeed(seed);
  } else {
    // Pre-populate a random seed for next time.
    if (poweron_seed) {
      //if (EEPROM.read(0) == 255)
      seed=TCNT2;
      EEPROM.write(0, seed);
      poweron_seed=0;
      show_seed();
    }
  }
}

//
// USBasp Connection
//
// MOSI = 7, MISO= 0, SCK = 9, RST = 4
uint32_t current_millis=0;
void setup()
{
  seed=EEPROM.read(0);   // Saved from previous start from poweron.
  OCR2 = 0xFF;           // NOT REALLY USED HERE. Just set to MAX.
  SFIOR |= (1 << PSR2);  // reset prescalar. Not sure I need to do this.
  TCCR2 = 0;
  TCCR2 |= (0 << COM20); // SHUT OFF OUTPUT PIN
  TCCR2 |= (1 << WGM21 | 1 << WGM20);  /* Fast PWM mode */
  //TIMSK |= (1 << OCIE2); /* enable timer2 output compare match interrupt */
  TIMSK |= (1 << TOIE2); /* enable timer2 overflow interrupt */
  TCCR2 = (1 << CS20);   // No prescaler 
  TIFR |= (1 << TOV2);    /* clear interrupt flag */

  // Get ready for sleep
  ADCSRA &= ~(1<<ADEN); // Disable ADC for better power consumption

  pinModeFast(INTR_TOGGLE, OUTPUT); // For testing.
  set_pin_directions();
  dieAll();             // self-check of the system
  heads_tails_1_sec();	// ditto, but they share a resistor so only the lowest-forward-voltage LED will go on.
  dieOff();
  show_seed();      // self-check of the system
  dieOff();
  heads_tails_off();

  //DDRD &= ~(1 << DDD2); // Clear the PD2 pin
  //PORTD |= (1 << PORTD2);
  //MCUCR &= 0xF0;
  //GICR |=(1<<INT0 | 1 << INT1);
  attachInterrupt(digitalPinToInterrupt(ROLL), roll_interrupt, LOW);
  attachInterrupt(digitalPinToInterrupt(FLIP), flip_interrupt, LOW);
  current_millis=millis();
}
 
#define DELAY_INTERVAL 25
uint16_t delay_interval=DELAY_INTERVAL;

// return 0 when the dice is done rolling.
uint8_t tumble_die(uint8_t last_roll) {
  show_die(last_roll); 
  delay_interval+=10;
  delay(delay_interval);
  if (delay_interval > 200) return 0;
  return 1;
}

// return 0 when the coin is done flipping. Otherwise, it just oscillates the LEDs
// back and forth.
uint8_t side=0;
uint8_t flip_it(uint8_t side) {
  if (side == 1) {
    heads();
  }
  else {
    tails();
  }
  delay_interval+=10;
  delay(delay_interval);
  if (delay_interval > 200) return 0;
  return 1;
}

uint32_t now_millis=0;
uint8_t rolled=false;
uint8_t flipped=false;
uint8_t last_roll=0;
uint8_t last_flip=0;
void loop()
{
  now_millis = millis();
  if (! digitalReadFast(ROLL)){
    // digitalWriteFast(INTR_TOGGLE, HIGH);
    flipped=false;
    rolled=true;
    heads_tails_off();
    current_millis = millis();
    last_roll++; if (last_roll > 6) last_roll=1;
    delay_interval=DELAY_INTERVAL;
    delay(50);
    // digitalWriteFast(INTR_TOGGLE, LOW);
    return;
  }
  if (! digitalReadFast(FLIP)){
    flipped=true;
    rolled=false;
    dieOff();
    current_millis = millis();
    // show the heads/tails LEDs while holding down the flip button
    if (last_flip) == 0 { heads(); last_flip=1; } else { tails(); last_flip=0;}
    delay_interval=DELAY_INTERVAL;
    delay(50);
    return;
  }
  if (flipped) {
    last_flip = (last_flip == 0) ? 1 : 0;
    if (! flip_it(last_flip)) { // LED's will toggle for a short period of time.
      prepare_seed();
      flipped=false;
      current_millis = millis();
      side=random(0,2);
      if (side == 1) heads();
      else tails();
    }
    return;
  }
  if (rolled) {
    last_roll++; if (last_roll > 6) last_roll=1;
    if (! tumble_die(last_roll)) {
      prepare_seed();
      rolled=false;
      current_millis = millis();
      delay_interval=DELAY_INTERVAL;
      show_die(random(1,7));
    }
    return;
  }
  if ((now_millis - current_millis) > SLEEP_AFTER_MILLIS) {
    current_millis = millis();
    go_to_sleep();
  }
}
