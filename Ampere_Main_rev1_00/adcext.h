//adcext.h A library for handling the external ADC :ADS7041
//EEforEveryone - 2020

//Dependent on SPI library for arduino

#include "Arduino.h"
#include <SPI.h>

// ensure this library description is only included once
#ifndef SRC_ADCEXT_H
#define SRC_ADCEXT_H

struct adcext;

#define ADCEXT_AVG_MAX 1024 //maximum number of samples to average
#define ADCEXT_VREF_MAX 5000 //maximum allowed voltage as VREF (5.000V)
#define ADCEXT_SPI_CLK 4000000 //SPI CLK speed (4MHz) 14MHz MAX!
//#define ADCEXT_SPI_CLK 1000000 //SPI CLK speed (1MHz) 14MHz MAX!

#define ADCEXT_DELAY_US_CONST 1 //delay 1us with 10MHz clock speed
#define ADCEXT_NUM_BITS 10


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct adcext* adcext__create(int CS_PIN, uint16_t NUM_AVG, uint16_t VREF_MV, float maxcounts, int spi_id); //constructor
void adcext__init(struct adcext *self, int CS_PIN, uint16_t NUM_AVG, uint16_t VREF_MV, float maxcounts, int spi_id); //modifier (empty)
void adcext__destroy(struct adcext *self); //destructor

/**************MEMBER FUNCTIONS**************************/
void adcext__run(struct adcext *self); //gathers samples and handles averaging
bool adcext__sampleReady(struct adcext *self); //Returns 1 when sample is ready, 0 when not. Acked by reading data.
uint32_t adcext__get_mV(struct adcext *self);// returns measured voltage, in mV.
void adcext__setCustom(struct adcext *self, float scalingfactor); //set the custom scaling factor
float adcext__getCustom(struct adcext *self);//Read the ADC with custom scaling factor applied


#endif
