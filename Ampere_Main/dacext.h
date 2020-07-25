//dacext.h A library for handling the external DAC :DAC081S101
//EEforEveryone - 2020

//Dependent on SPI library for arduino

#include "Arduino.h"
#include <SPI.h>

// ensure this library description is only included once
#ifndef SRC_DACEXT_H
#define SRC_DACEXT_H

struct dacext;

#define DACEXT_VREF_MAX 5000 //maximum allowed voltage as VREF (5.000V)
//#define DACEXT_SPI_CLK 25000000 //SPI CLK speed (25MHz) 30MHz MAX!
#define DACEXT_SPI_CLK 4000000 //SPI CLK speed (8MHz) 30MHz MAX!

#define DACEXT_DELAY_US_CONST 100000000 //delay 1us with 10MHz clock speed


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct dacext* dacext__create(int CS_PIN, uint16_t VREF_MV, uint16_t dac_bits, int spi_id); //constructor
void dacext__init(struct dacext *self, int CS_PIN, uint16_t VREF_MV, uint16_t dac_bits, int spi_id); //modifier (empty)
void dacext__destroy(struct dacext *self); //destructor

/**************MEMBER FUNCTIONS**************************/
void dacext__set_mV(struct dacext *self, uint16_t millivolts);// sets DAC to voltage, in mV.
void dacext__setCustomFactor(struct dacext *self, float scalingfactor); //set the custom scaling factor
void dacext__setCustom(struct dacext *self, float setpoint); //Set the DAC with custom scaling factor applied


#endif
