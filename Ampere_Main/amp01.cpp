//amp01.cpp A library for controlling one SPI-based Project Ampere Electronic Load Front-End
//EEforEveryone - 2020

//Dependent on SPI library for arduino

#include "amp01.h"

/**************STRUCT DEFINITION*************************/
struct amp01{

  struct adcext* vadc; //external adc object for Voltage ADC.
  struct adcext* iadc; //external adc object for Current ADC.
  struct dacext* idac; //external dac object for Current DAC. (sets output current)

  uint32_t volt_in_milli;
  uint32_t amp_in_milli;
  uint32_t ohm_in_milli;
  uint32_t watt_in_milli;
  uint32_t energy_count_milli;

  bool enabled; // true (1) when output enabled, false (0) when output disabled
  uint32_t this_reg_Mode; //stores what mode is active
  uint32_t amp_set_milli;
  uint32_t ohm_set_milli;
  uint32_t watt_set_milli;

  float IMAX;
  float VMAX;
  float PMAX;
  
};

/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct amp01* amp01__create(struct adcext* VADC, struct adcext* IADC, struct dacext* DAC, float Imax, float Vmax, float Pmax){ //constructor
  struct amp01* result = (struct amp01*) malloc(sizeof(struct amp01)); //alocate memory, typecast to amp01 struct.

  amp01__init(result, VADC, IADC, DAC, Imax, Vmax, Pmax); //initialize to default values, use SPI0
  return(result);
}

void amp01__init(struct amp01 *self, struct adcext* VADC, struct adcext* IADC, struct dacext* DAC, float Imax, float Vmax, float Pmax){ //modifier (lots)
  self->vadc = VADC; //save the pointer
  self->iadc = IADC; //save the pointer
  self->idac = DAC; //save the pointer

  self->IMAX = Imax;
  self->VMAX = Vmax;
  self->PMAX = Pmax;

  self->volt_in_milli = 0;
  self->amp_in_milli = 0;
  self->ohm_in_milli = 0;
  self->watt_in_milli = 0;
  self->energy_count_milli = 0;

  self->enabled = false; // true (1) when output enabled, false (0) when output disabled
  self->this_reg_Mode = 0; //stores what mode is active
  self->amp_set_milli = 0;
  self->ohm_set_milli = 0;
  self->watt_set_milli = 0;
}

void amp01__destroy(struct amp01 *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/
void amp01__Sample(struct amp01 *self){ //Gets Data from ADCs!
  adcext__run(self->vadc); //sample the voltage ADC
  adcext__run(self->iadc); //sample the current ADC

  if(adcext__sampleReady(self->vadc)){ //new sample ready for voltage ADC
    self->volt_in_milli = adcext__getCustom(self->vadc); //getCustom set up with passed-in scale value

    //Serial.print("V = "); Serial.println(adcext__get_mV(self->vadc));
    //Serial.print("Scaled: "); Serial.println(self->volt_in_milli);


    if(self->amp_in_milli > 0){
      self->ohm_in_milli = self->volt_in_milli*1000/self->amp_in_milli;
    }else{
      self->ohm_in_milli = 1000000000;
    }
    self->watt_in_milli = self->volt_in_milli*self->amp_in_milli/1000;
  }
  if(adcext__sampleReady(self->iadc)){ //new sample ready for current ADC
    self->amp_in_milli = adcext__getCustom(self->iadc); //getCustom set up so that 1000 = 1A through current shunt

    if(self->amp_in_milli > 0){
      self->ohm_in_milli = self->volt_in_milli*1000/self->amp_in_milli;
    }else{
      self->ohm_in_milli = 1000000000;
    }
    self->watt_in_milli = self->volt_in_milli*self->amp_in_milli/1000;
  }
}

bool amp01__SetCurrent(struct amp01 *self, int setpoint_milli){ //Change mode to current and adjust setpoint

  //if( (AMPGUI_SEL_I != self->this_reg_Mode) /*|| (setpoint_milli != self->amp_set_milli)*/ ){ //need to set the new value!
    self->amp_set_milli = setpoint_milli;
    self->this_reg_Mode = AMPGUI_SEL_I;
    
    if(self->enabled == true){ //only change the output value when enabled
      uint32_t setpoint = self->amp_set_milli;
        
      if(AMP01_ENFORCE_LIMITS == true){
        dacext__setCustom(self->idac, INT__amp01__checkLIMITS(self,setpoint) );//set the output current (limits enforced)
      }else{
        dacext__setCustom(self->idac, setpoint);//set the output current (unlimited)
      }
      return(true); //we set the new value!
      
    }
    return(false); //we didn't set the new value
  /*}else{
    return(true);
  }*/
  return(false); //this should never be reached
}

bool amp01__SetResistance(struct amp01 *self, int setpoint_milli){ //Change mode to resistance and adjust setpoint

    //if( (AMPGUI_SEL_R != self->this_reg_Mode) /*|| (setpoint_milli != self->ohm_set_milli)*/ ){ //need to set the new value!
    self->ohm_set_milli = setpoint_milli;
    self->this_reg_Mode = AMPGUI_SEL_R;
    
    if(self->enabled == true){ //only change the output value when enabled
      if(self->ohm_set_milli > 0){ //if the ohm setpoint is valid.
        uint32_t setpoint = self->volt_in_milli/((float)self->ohm_set_milli/1000.0); //ohms law
        
        if(AMP01_ENFORCE_LIMITS == true){
          dacext__setCustom(self->idac, INT__amp01__checkLIMITS(self,setpoint) );//set the output current (limits enforced)
        }else{
          if (setpoint > self->IMAX){
            setpoint = self->IMAX;
          }
          dacext__setCustom(self->idac, setpoint);//set the output current (unlimited)
        }
        return(true); //we set the new value
        
      }else{ //divide by zero error
        dacext__setCustom(self->idac, 0);//turn off the output
      }
    }
    return(false); //we didn't set the new value
  /*}else{
    return(true); //already in the correct mode
  }*/
  return(false); //this should never be reached
}

bool amp01__SetPower(struct amp01 *self, int setpoint_milli){ //Change mode to power and adjust setpoint

  //if( (AMPGUI_SEL_P != self->this_reg_Mode) /*|| (setpoint_milli != self->watt_set_milli)*/ ){ //need to set the new value!
    self->watt_set_milli = setpoint_milli;
    self->this_reg_Mode = AMPGUI_SEL_P;
    
    if(self->enabled == true){ //only set the output value when enabled
      if(self->volt_in_milli > 0){ //no divide by zero error
      
        uint32_t setpoint = (self->watt_set_milli*1000)/self->volt_in_milli; //I = Pwr/Volt (milli)
        
        if(AMP01_ENFORCE_LIMITS == true){
          dacext__setCustom(self->idac, INT__amp01__checkLIMITS(self,setpoint) );//set the output current (limits enforced)
        }else{
          if (setpoint > self->IMAX){
            setpoint = self->IMAX;
          }
          dacext__setCustom(self->idac, setpoint);//set the output current (unlimited)
        }
        return(true); //we set the new value
        
      }else{
        dacext__setCustom(self->idac, 0);//turn off the output
      }
    }
    return(false); //we didn't set the new value
  /*}else{
    return(true); //already in the correct mode
  }*/
  return(false); //this should never be reached
}

void amp01__ISR_routine(struct amp01 *self){ //executes things that must have precise timing -- call from ISR with 1ms period
  if(self->enabled == true){ //this prevents the ISR from writing to energy_count while being reset by the main loop

    //SAMPLE ADC - Currently Free-Running in "amp01__Sample" -- called from main loop

    //Calculate Energy
    self->energy_count_milli = self->energy_count_milli + (self->volt_in_milli*self->amp_in_milli/1000)/1000; //energy + power(mW) * ms

    //RUN CTRL LOOP

    //UPDATE DAC
    
  }
}

void amp01__EnableOutput(struct amp01 *self){ //turns on output with last settings, resets energy count on fresh enable
  if(self->enabled == false){
    self->energy_count_milli = 0;
  }
  self->enabled = true;
  
}
void amp01__DisableOutput(struct amp01 *self){ //turns off output
  if(self->enabled == true){
    dacext__setCustom(self->idac, 0);//turn off the output
  }
  self->enabled = false;  
  self->amp_set_milli = 0; //reset setpoints to 0
  self->ohm_set_milli = 0; //reset setpoints to 0
  self->watt_set_milli = 0; //reset setpoints to 0
}

uint32_t amp01__GetVout(struct amp01 *self){
  return(self->volt_in_milli);
}
uint32_t amp01__GetIout(struct amp01 *self){
  return(self->amp_in_milli);
}
uint32_t amp01__GetRout(struct amp01 *self){
  return(self->ohm_in_milli);
}
uint32_t amp01__GetPout(struct amp01 *self){
  return(self->watt_in_milli);
}
uint32_t amp01__GetEout(struct amp01 *self){
  return(self->energy_count_milli);
}

int INT__amp01__checkLIMITS(struct amp01 *self, int current_milli){ //check a new current, return a current clipped to the max thermal design power
  uint32_t current_out = current_milli;

  if (current_out < 0){ //fix negative numbers
    current_out = 0;
    
  }
  
  if(self->volt_in_milli > (self->VMAX*AMP01_LIMIT_MARGIN)){                      //enforce voltage limit
    current_out = 0;
    
  }
  
  if((((uint32_t)current_out * self->volt_in_milli)/1000) > (self->PMAX*AMP01_LIMIT_MARGIN)){ //enforce power limit. no div/0 possible, because I=0 means power limit not reached

      if(self->volt_in_milli > 0){
        current_out = (self->PMAX*1000)/self->volt_in_milli; //I = Pwr/Volt (milli) {Set to MAX pwr}
      }else{
        current_out = 0; //div/0 error
      }
    
  }

  if(current_out > (self->IMAX*AMP01_LIMIT_MARGIN)){                              //enforce current limit
    current_out = self->IMAX;
    
  }

  return(current_out);
}
