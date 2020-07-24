//dbgled.cpp A library for handling the debug / status led
//EEforEveryone - 2020

#include  "dbgled.h"

/**************STRUCT DEFINITION*************************/
struct dbgled{

int LED_PIN;
uint8_t blinkmode;
uint8_t oldstate;
uint32_t oldms;
uint32_t newms;
int lastPinSetting;
};


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct dbgled* dbgled__create(int PIN){ //constructor
  struct dbgled* result = (struct dbgled*) malloc(sizeof(struct dbgled)); //alocate memory, typecast to adcext struct.
  
  dbgled__init(result, PIN); //initialize to default values
  return(result);
}

void dbgled__init(struct dbgled *self, int PIN){ //modifier (empty)

    pinMode(PIN, INPUT); //default to dim, on.
  
  self->LED_PIN = PIN;
  self->blinkmode = DBGLED_MODE_IDLE;
  self->oldstate = 0; //state = step through blink sequence
  self->lastPinSetting = DBGLED_DIM;
  self->oldms = 0; //old ms = 0;
  self->newms = 0; //newms = 0;
}

void dbgled__destroy(struct dbgled *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/
void dbgled__refresh(struct dbgled *self, uint32_t newms){ //make blinking happen
  self->newms = newms;
  switch(self->blinkmode){
    case DBGLED_MODE_IDLE:
      INT_dbgled__doblink(self, DBGLED_ON, DBGLED_DIM, DBGLED_PER_NORMAL, DBGLED_PER_NORMAL); //dim slow flash
    break;
    case DBGLED_MODE_ERROR:
      INT_dbgled__doDblBlink(self, DBGLED_ON, DBGLED_OFF, DBGLED_DIM, DBGLED_PER_DOUBLE, DBGLED_PER_FAST); //double bright flash, then dim
    break;
    case DBGLED_MODE_RUN:
      INT_dbgled__doblink(self, DBGLED_ON, DBGLED_OFF, DBGLED_PER_FAST, DBGLED_PER_FAST); //bright fast flash
    break;
    case DBGLED_MODE_OFF:
      INT_dbgled__setpin(self, DBGLED_OFF); //turn off the led
      self->oldms = self->newms;
    break;
    default:
      self->blinkmode = DBGLED_MODE_ERROR;
      self->oldstate = 0; //state = step through blink sequence
      self->oldms = self->newms;
    break;
  }
}
void dbgled__off(struct dbgled *self){ //change mode
  if(self->blinkmode != DBGLED_MODE_OFF){
    self->blinkmode = DBGLED_MODE_OFF;
    self->oldstate = 0; //state = step through blink sequence
    self->oldms = self->newms;
  }
}
void dbgled__run(struct dbgled *self){ //change mode
  if(self->blinkmode != DBGLED_MODE_RUN){
    self->blinkmode = DBGLED_MODE_RUN;
    self->oldstate = 0; //state = step through blink sequence
    self->oldms = self->newms;
  }
}
void dbgled__idle(struct dbgled *self){ //change mode
  if(self->blinkmode != DBGLED_MODE_IDLE){
    self->blinkmode = DBGLED_MODE_IDLE;
    self->oldstate = 0; //state = step through blink sequence
    self->oldms = self->newms;
  }
}
void dbgled__error(struct dbgled *self){ //change mode
  if(self->blinkmode != DBGLED_MODE_ERROR){
    self->blinkmode = DBGLED_MODE_ERROR;
    self->oldstate = 0; //state = step through blink sequence
    self->oldms = self->newms;
  }
}


void INT_dbgled__doblink(struct dbgled *self, uint8_t firstPinState, uint8_t secondPinState, uint16_t firstdelay, uint16_t seconddelay){ //do a blink
    if(  self->oldms > self->newms){//overflow
      self->oldms = self->newms;
    }
    
    switch(self->oldstate){
      case 0:
      INT_dbgled__setpin(self, firstPinState);
      
      if(self->newms > self->oldms+firstdelay){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      case 1:
      INT_dbgled__setpin(self, secondPinState);
      
      if(self->newms > self->oldms+seconddelay){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      
      default:
        self->oldstate = 0; //reset
        self->oldms = self->newms;
      break;
    }

}

void INT_dbgled__doDblBlink(struct dbgled *self, uint8_t firstPinState, uint8_t secondPinState, uint8_t thirdPinState, uint16_t blinkdelay, uint16_t longdelay){ //do a blink
    if(  self->oldms > self->newms){//overflow
      self->oldms = self->newms;
    }
    
    switch(self->oldstate){
      case 0:
      INT_dbgled__setpin(self, firstPinState);
      
      if(self->newms - blinkdelay > self->oldms){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      case 1:
      INT_dbgled__setpin(self, secondPinState);
      
      if(self->newms - blinkdelay > self->oldms){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      case 2:
      INT_dbgled__setpin(self, firstPinState);
      
      if(self->newms - blinkdelay > self->oldms){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      case 3:
      INT_dbgled__setpin(self, thirdPinState);
      
      if(self->newms - longdelay > self->oldms){ //wait for state transition
        self->oldstate = self->oldstate + 1;
        self->oldms = self->newms;
      }
      break;
      
      default:
        self->oldstate = 0; //reset
      break;
    }

}

void INT_dbgled__setpin(struct dbgled *self, int pinSetting){ //change pin state
  if(self->lastPinSetting != pinSetting){
    switch(pinSetting){
      case DBGLED_ON:
        pinMode(self->LED_PIN, OUTPUT);
        digitalWrite(self->LED_PIN, DBGLED_ON);
      break;
      case DBGLED_OFF:
        pinMode(self->LED_PIN, OUTPUT);
        digitalWrite(self->LED_PIN, DBGLED_OFF);
      break;
      default:
        pinMode(self->LED_PIN, INPUT);
      break;
    }
    self->lastPinSetting = pinSetting;
  }
}
