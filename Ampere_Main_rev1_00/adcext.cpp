//adcext.cpp A library for handling the external ADC :ADS7041
//EEforEveryone - 2020

#include  "adcext.h"

/**************STRUCT DEFINITION*************************/
struct adcext{

int ADC_CS;
uint16_t NUM_AVERAGE;
uint16_t VREF_MILLIVOLTS;

float ADCEXT_MAXCOUNTS;

uint16_t average_int_counter;
uint32_t int_wip_buffer;
uint32_t counts;

int spi_id;

float custom_factor;

bool rdy;
  
};


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct adcext* adcext__create(int CS_PIN, uint16_t NUM_AVG, uint16_t VREF_MV, float maxcounts, int spi_id){ //constructor
  
  struct adcext* result = (struct adcext*) malloc(sizeof(struct adcext)); //alocate memory, typecast to adcext struct.

  digitalWrite(CS_PIN, HIGH);
  pinMode(CS_PIN, OUTPUT);

  if(NUM_AVG > ADCEXT_AVG_MAX){
    NUM_AVG = ADCEXT_AVG_MAX;
  }

  if(VREF_MV > ADCEXT_VREF_MAX){
    VREF_MV = ADCEXT_VREF_MAX;
  }
  
  adcext__init(result, CS_PIN, NUM_AVG, VREF_MV, maxcounts, spi_id); //initialize to default values
  return(result);
}

void adcext__init(struct adcext *self, int CS_PIN, uint16_t NUM_AVG, uint16_t VREF_MV, float maxcounts, int spi_id){ //modifier (empty)
  self->ADC_CS = CS_PIN;
  self->NUM_AVERAGE = NUM_AVG;
  self->VREF_MILLIVOLTS = VREF_MV;
  self->average_int_counter = 0;
  self->int_wip_buffer = 0;
  self->counts = 0;
  self->custom_factor = 1.00; //custom = mV by default
  self->spi_id = spi_id;
  self->rdy = false; 
  self->ADCEXT_MAXCOUNTS = maxcounts; //used to determine max bits (allows for different resolutions between HW revs)
}

void adcext__destroy(struct adcext *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/
void adcext__run(struct adcext *self){ //gathers samples and handles averaging

//SPI Transaction
uint8_t upperdata = 0;
uint8_t lowerdata = 0;
  if(self->spi_id == 1){ //use SPI1

    digitalWrite(self->ADC_CS, HIGH);
    SPI1.beginTransaction(SPISettings(ADCEXT_SPI_CLK, MSBFIRST, SPI_MODE0)); //Fire up SPI interface, defined in adcext.h (14Max)
    digitalWrite(self->ADC_CS, LOW);
    upperdata = SPI1.transfer((uint8_t) 0x00); //Send 0 while reading a byte
    lowerdata = SPI1.transfer((uint8_t) 0x00); //Send 0 while reading a byte
    digitalWrite(self->ADC_CS, HIGH);
    SPI1.endTransaction();

    
  }else{ //deafult to SPI(0)

    digitalWrite(self->ADC_CS, HIGH);
    SPI.beginTransaction(SPISettings(ADCEXT_SPI_CLK, MSBFIRST, SPI_MODE0)); //Fire up SPI interface, defined in adcext.h (14Max)
    digitalWrite(self->ADC_CS, LOW);
    upperdata = SPI.transfer((uint8_t) 0x00); //Send 0 while reading a byte
    lowerdata = SPI.transfer((uint8_t) 0x00); //Send 0 while reading a byte
    digitalWrite(self->ADC_CS, HIGH);
    SPI.endTransaction();

  }

  Serial.printf("SPIID: %i. Upper: %i. Lower: %i.\n", self->spi_id, upperdata, lowerdata);

  uint16_t data = (upperdata<<8)+lowerdata;

//#TODO: make work for bit counts other than 10!
  //Serial.print("Raw Data: "); Serial.println(data);
  uint16_t thisADCdata = ((data&0x3FFF)>>(14-ADCEXT_NUM_BITS)); // scale 16bit to 10bit

//Averaging

  if(self->average_int_counter > (self->NUM_AVERAGE-1) ){ //if we're done averaging. zero indexed, so subtract one.

    self->counts = self->int_wip_buffer/self->NUM_AVERAGE; //divide down summing buffer.
    self->rdy = true; //set flag that new data is ready!

    self->int_wip_buffer = thisADCdata; //start new sum
    self->average_int_counter = 1; //reset sample counter to one.. because there is already a sample saved here!
    
  }else{ //keep averaging (cumulative)
    self->int_wip_buffer = self->int_wip_buffer + thisADCdata; //add to sum.
    self->average_int_counter = self->average_int_counter+1; //increment counter

      
  }
  
}

bool adcext__sampleReady(struct adcext *self){ //Returns 1 when sample is ready, 0 when not. Acked by reading data.
  return(self->rdy);
}

uint32_t adcext__get_mV(struct adcext *self){// returns measured voltage, in mV.
  self->rdy = false; //reset data ready flag
  return((float)self->VREF_MILLIVOLTS*(float)self->counts/(float)self->ADCEXT_MAXCOUNTS); //return Vref *(counts/maxcounts)
}

void adcext__setCustom(struct adcext *self, float scalingfactor){ //set the custom scaling factor
  self->custom_factor = scalingfactor;
}

float adcext__getCustom(struct adcext *self){//Read the ADC with custom scaling factor applied
  self->rdy = false; //reset data ready flag
  //Serial.print("Factor: "); Serial.println(self->custom_factor);
  return((float)self->VREF_MILLIVOLTS*(float)self->counts/(float)self->ADCEXT_MAXCOUNTS*(float)self->custom_factor); //return Vref *(counts/maxcounts) * customfactor
}
