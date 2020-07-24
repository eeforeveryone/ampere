//dacext.cpp A library for handling the external DAC :DAC081S101
//EEforEveryone - 2020

//Dependent on SPI library for arduino

#include  "dacext.h"

/**************STRUCT DEFINITION*************************/
struct dacext{

int DAC_CS;
uint16_t VREF_MILLIVOLTS;
uint16_t DACEXT_MAXCOUNTS;
uint16_t shift_upper;
uint16_t shift_lower;

int spi_id; //used for storing which SPI to use

float scalingfactor;
  
};

/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct dacext* dacext__create(int CS_PIN, uint16_t VREF_MV, uint16_t dac_bits, int spi_id){ //constructor
  struct dacext* result = (struct dacext*) malloc(sizeof(struct dacext)); //alocate memory, typecast to adcext struct.

  digitalWrite(CS_PIN, HIGH);
  pinMode(CS_PIN, OUTPUT);

  if(VREF_MV > DACEXT_VREF_MAX){
    VREF_MV = DACEXT_VREF_MAX;
  }
  
  dacext__init(result, CS_PIN, VREF_MV, dac_bits, spi_id); //initialize to default values
  return(result);
}

void dacext__init(struct dacext *self, int CS_PIN, uint16_t VREF_MV, uint16_t dac_bits, int spi_id){ //modifier (empty)
  self->DAC_CS = CS_PIN;
  self->VREF_MILLIVOLTS = VREF_MV;
  self->DACEXT_MAXCOUNTS = pow(2,dac_bits); //calculate 2^nth bits

  self->spi_id = spi_id;
  
  self->shift_upper = dac_bits-4;  //8-bit: 4; //10-bit: 6; 12-bit: 8 //used for splitting data into two bytes
  self->shift_lower = 12-dac_bits; //8-bit: 4; //10-bit: 2; 12-bit: 0 //used for splitting data into two bytes

  dacext__set_mV(self,0); //set the output to 0A
  
}

void dacext__destroy(struct dacext *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/
void dacext__set_mV(struct dacext *self, uint16_t millivolts){ // sets DAC to voltage, in mV.
  
  uint16_t data = (self->DACEXT_MAXCOUNTS*millivolts)/self->VREF_MILLIVOLTS; // convert requested mV into counts

  if(data > self->DACEXT_MAXCOUNTS){
    data = self->DACEXT_MAXCOUNTS;
  }
  uint8_t upperdata = (data>>self->shift_upper)&0b00001111; //Force into normal mode, then upper 4 bits of data
  uint8_t lowerdata = (data<<self->shift_lower)&0b11111111; //retain lower 4-8 bits of data

  if(self->spi_id == 1){ //use SPI1!
    SPI1.beginTransaction(SPISettings(DACEXT_SPI_CLK, MSBFIRST, SPI_MODE1)); //Fire up SPI interface, 25MHz (30Max)
      digitalWrite(self->DAC_CS, HIGH);
      SPI1.transfer(0); //dummy byte
      digitalWrite(self->DAC_CS, LOW);

      SPI1.transfer(upperdata); //upper byte = mode select + upper 6 bits of data
      SPI1.transfer(lowerdata); //lower byte = lower 2 bits of data + zeros
      digitalWrite(self->DAC_CS, HIGH);
    SPI1.endTransaction();
    
  }else{ //Default to SPI(0)
    SPI.beginTransaction(SPISettings(DACEXT_SPI_CLK, MSBFIRST, SPI_MODE1)); //Fire up SPI interface, 25MHz (30Max)
      digitalWrite(self->DAC_CS, HIGH);
      SPI.transfer(0); //dummy byte
      digitalWrite(self->DAC_CS, LOW);
      SPI.transfer(upperdata); //upper byte = mode select + upper 6 bits of data
      SPI.transfer(lowerdata); //lower byte = lower 2 bits of data + zeros
      digitalWrite(self->DAC_CS, HIGH);
    SPI.endTransaction();

  }
}

void dacext__setCustomFactor(struct dacext *self, float scalingfactor){ //set the custom scaling factor
  self->scalingfactor = scalingfactor;
}

void dacext__setCustom(struct dacext *self, float setpoint){ //Set the DAC with custom scaling factor applied
  dacext__set_mV(self, setpoint*self->scalingfactor);
}
