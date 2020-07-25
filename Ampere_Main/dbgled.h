//dbgled.h A library for handling the debug / status led
//EEforEveryone - 2020

//Dependent on GPIO library for arduino

#include "Arduino.h"


// ensure this library description is only included once
#ifndef SRC_DBGLED_H
#define SRC_DBGLED_H

struct dbgled;

#define DBGLED_PER_NORMAL 1000 //normal blink period
#define DBGLED_PER_FAST   500 //fast blink period
#define DBGLED_PER_DOUBLE 100 //double blink period

#define DBGLED_MODE_IDLE 0 //id for normal blink
#define DBGLED_MODE_RUN 1 //id for run blink
#define DBGLED_MODE_ERROR 2 //id for error blink
#define DBGLED_MODE_OFF 3

#define DBGLED_ON 1 //output, high
#define DBGLED_OFF 0 //output, low
#define DBGLED_DIM 2 //input


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct dbgled* dbgled__create(int PIN); //constructor
void dbgled__init(struct dbgled *self, int PIN); //modifier (empty)
void dbgled__destroy(struct dbgled *self); //destructor

/**************MEMBER FUNCTIONS**************************/
void dbgled__refresh(struct dbgled *self, uint32_t newms); //make blinking happen
void dbgled__off(struct dbgled *self); //change mode
void dbgled__run(struct dbgled *self); //change mode
void dbgled__idle(struct dbgled *self); //change mode
void dbgled__error(struct dbgled *self); //change mode

void INT_dbgled__doblink(struct dbgled *self, uint8_t firstPinState, uint8_t secondPinState, uint16_t firstdelay, uint16_t seconddelay); //do a blink
void INT_dbgled__doDblBlink(struct dbgled *self, uint8_t firstPinState, uint8_t secondPinState, uint8_t thirdPinState, uint16_t blinkdelay, uint16_t longdelay); //do a double blink
void INT_dbgled__setpin(struct dbgled *self, int pinSetting); //change pin state



#endif
