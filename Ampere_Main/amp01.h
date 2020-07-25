//amp01.h A library for controlling one SPI-based Project Ampere Electronic Load Front-End
//EEforEveryone - 2020

//Dependent on SPI library for arduino

#include "Arduino.h"
#include <SPI.h>
#include "adcext.h" //external ADC driver
#include "dacext.h" //external DAC driver
#include "ampgui.h" //includes #defines for "AMPGUI_SEL_P"... etc


// ensure this library description is only included once
#ifndef SRC_AMP01_H
#define SRC_AMP01_H

#define AMP01_ADC_OVERSAMPLE 10 //number of samples to average
#define AMP01_LIMIT_MARGIN 1.1 //allow 10% violation of margin to prevent glitches causing problems.

#define AMP01_ENFORCE_LIMITS true //set false to allow UNLIMITED POWER!

struct amp01;

/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct amp01* amp01__create(struct adcext* VADC, struct adcext* IADC, struct dacext* DAC, float Imax, float Vmax, float Pmax); //constructor
void amp01__init(struct amp01 *self, struct adcext* VADC, struct adcext* IADC, struct dacext* DAC, float Imax, float Vmax, float Pmax); //modifier (empty)
void amp01__destroy(struct amp01 *self); //destructor

/**************MEMBER FUNCTIONS**************************/
void amp01__Sample(struct amp01 *self); //Gets Data from ADCs!
bool amp01__SetCurrent(struct amp01 *self, int setpoint_milli); //Change mode to current and adjust setpoint
bool amp01__SetResistance(struct amp01 *self, int setpoint_milli); //Change mode to resistance and adjust setpoint
bool amp01__SetPower(struct amp01 *self, int setpoint_milli); //Change mode to power and adjust setpoint
void amp01__ISR_routine(struct amp01 *self); //executes things that must have precise timing -- call from ISR with 1ms period

void amp01__EnableOutput(struct amp01 *self); //turns on output with last settings
void amp01__DisableOutput(struct amp01 *self); //turns off output

uint32_t amp01__GetVout(struct amp01 *self);
uint32_t amp01__GetIout(struct amp01 *self);
uint32_t amp01__GetRout(struct amp01 *self);
uint32_t amp01__GetPout(struct amp01 *self);
uint32_t amp01__GetEout(struct amp01 *self);

int INT__amp01__checkLIMITS(struct amp01 *self, int current_milli); //check a new current, return a current clipped to the max thermal design power



#endif
