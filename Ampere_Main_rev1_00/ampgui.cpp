//ampgui.h A library for handling I/O functions for the Project Ampere GUI
//EEforEveryone - 2020

//This library organizes data for display in screens.
//dependent on "screen.h"

#include  "ampgui.h"

/**************STRUCT DEFINITION*************************/

struct gui_button{

  bool OldState;
  uint32_t OldTime;
  bool pushed; 
  bool acked;
  uint16_t holdcount;

};

struct ampgui{

  uint32_t time_ms;
  struct gui_button buttons[AMPGUI_NUM_BUTTONS];
  struct screen* myscreen; 

  int screentype;
  int screen_selection; //stores current selection
  int opt_index; //stores opt index (opt Array)
  
  float ext_mV;
  float ext_mA;
  float ext_mOhm;
  float ext_mW;
  float ext_mC;

  char opt1_show [OPT_LENGTH+1];
  char opt2_show [OPT_LENGTH+1];
  char opt3_show [OPT_LENGTH+1];
  
  char title[21];
  int title_length;

  uint32_t selection_value_milli;
  uint16_t button_input_mode[AMPGUI_MAXPOSSIBLECHANNELS+1]; //AMPGUI_BUTTON_NORMAL || AMPGUI_BUTTON_SET || AMPGUI_BUTTON_RUN //one per channel, zero index not used
  
  uint8_t MAX_CHANNEL_NUM;
  uint8_t ChannelSelection; //Range = 1:self->MAX_CHANNEL_NUM (inclusive) ##Aligned with ampserial
  int TITLE_DISP_CHANNELNUM_INDEX;

  int OPT1_PIN;
  int OPT2_PIN;
  int OPT3_PIN;
  
  int DOWN_PIN;
  int ENTER_PIN;
  int UP_PIN;

};


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct ampgui* ampgui__create(struct screen* screen_obj, int maxChannels, int upPin, int downPin, int enterPin, int opt1Pin, int opt2Pin, int opt3Pin){ //constructor
  
  struct ampgui* result = (struct ampgui*) malloc(sizeof(struct ampgui)); //alocate memory, typecast to adcext struct.
  
  ampgui__init(result, screen_obj, maxChannels, upPin, downPin, enterPin, opt1Pin, opt2Pin, opt3Pin); //initialize to default values
  return(result);
}

void ampgui__init(struct ampgui *self, struct screen* screen_obj, int maxChannels, int upPin, int downPin, int enterPin, int opt1Pin, int opt2Pin, int opt3Pin){ //modifier (empty)

for(int i = 0; i<AMPGUI_NUM_BUTTONS; i++){ //reset all buttons
    self->buttons[i].OldState = !AMPGUI_BUTTON_ACTIVE_STATE;
    self->buttons[i].OldTime = 0;
    self->buttons[i].pushed = false;
    self->buttons[i].acked = false;
    self->buttons[i].holdcount = 0;
  }

  self->myscreen = screen_obj; //save pointer to screen object
  self->time_ms = 0;

  self->screentype = 0;
  self->screen_selection = 0; //stores current selection
  self->opt_index = 0; //stores opt index (opt Array)
  
  self->ext_mV = 0;
  self->ext_mA = 0;
  self->ext_mOhm = 0;
  self->ext_mW = 0;
  self->ext_mC = 0;

  //Validate maxChannels to prevent memory errors
  if(maxChannels > AMPGUI_MAXPOSSIBLECHANNELS){ //too big!
    self->MAX_CHANNEL_NUM = AMPGUI_MAXPOSSIBLECHANNELS; //clip
  }else if(maxChannels < 1){ //too small!
    self->MAX_CHANNEL_NUM = 1; //clip
  }else{ //just right
    self->MAX_CHANNEL_NUM = maxChannels;
  }
  
  self->ChannelSelection = 1; //Range = 1:self->MAX_CHANNEL_NUM (inclusive)

  self->OPT1_PIN = opt1Pin;
  self->OPT2_PIN = opt2Pin;
  self->OPT3_PIN = opt3Pin;
  self->DOWN_PIN = downPin;
  self->ENTER_PIN = enterPin;
  self->UP_PIN = upPin;

  pinMode(self->OPT1_PIN, INPUT_PULLUP);
  pinMode(self->OPT2_PIN, INPUT_PULLUP);
  pinMode(self->OPT3_PIN, INPUT_PULLUP);
  pinMode(self->ENTER_PIN, INPUT_PULLUP);
  pinMode(self->DOWN_PIN, INPUT_PULLUP);
  pinMode(self->UP_PIN, INPUT_PULLUP);

  self->opt1_show[0] = 'C';
  self->opt1_show[1] = 'H';
  self->opt1_show[2] = '+';
  self->opt1_show[3] = ' ';
  self->opt1_show[4] = '\0';
  
  self->opt2_show[0] = ' ';
  self->opt2_show[1] = ' ';
  self->opt2_show[2] = ' ';
  self->opt2_show[3] = ' ';
  self->opt2_show[4] = '\0';

  
  
  self->opt3_show[0] = ' ';
  self->opt3_show[1] = ' ';
  self->opt3_show[2] = ' ';
  self->opt3_show[3] = ' ';
  self->opt3_show[4] = '\0';
  

  self->title_length = 20;

  for(int i = 0; i<self->title_length; i++){
    self->title[i] = ' ';
  }

  self->title[0] = 'C';
  self->title[1] = 'h';
  self->title[2] = 'a';
  self->title[3] = 'n';
  self->title[4] = 'n';
  self->title[5] = 'e';
  self->title[6] = 'l';
  self->title[7] = '(';
  self->title[8] = ' '; //PUT CHANNELNUM here!
  self->title[9] = ')';
  self->title[10]= ' ';
  self->title[11]= ' ';

  self->TITLE_DISP_CHANNELNUM_INDEX = 8;
  

  self->selection_value_milli = 0;

  for(int i = 0; i<(AMPGUI_MAXPOSSIBLECHANNELS+1); i++){
    self->button_input_mode[i] = AMPGUI_BUTTON_NORMAL; //AMPGUI_BUTTON_NORMAL || AMPGUI_BUTTON_SET || AMPGUI_BUTTON_RUN
  }
 
}

void ampgui__destroy(struct ampgui *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/

void ampgui__update(struct ampgui *self, uint32_t ms, bool passive){ //Reacts to button presses
  self->time_ms = ms; //update time

  if(passive){ //Button Control is being overridden by the pc control method

    //donothing.. //TODO: make new screen and react to a specific button... so that we can get back out of a locked up state.
    self->title[self->title_length-2] = 'S'; //show SER in top right corner
    self->title[self->title_length-1] = 'E';
    self->title[self->title_length]   = 'R';

    self->title[self->TITLE_DISP_CHANNELNUM_INDEX] = 'x'; //Show the channel selection in the correct spot TODO: get this from other class


  }else{ //we need to process buttons!
  
    //update button states
    ampgui__button_update(self, AMPGUI_OPT1   , digitalRead(self->OPT1_PIN)); 
    ampgui__button_update(self, AMPGUI_OPT2   , digitalRead(self->OPT2_PIN)); 
    ampgui__button_update(self, AMPGUI_OPT3   , digitalRead(self->OPT3_PIN)); 
    ampgui__button_update(self, AMPGUI_UP     , digitalRead(self->UP_PIN)); 
    ampgui__button_update(self, AMPGUI_DOWN   , digitalRead(self->DOWN_PIN)); 
    ampgui__button_update(self, AMPGUI_ENTER  , digitalRead(self->ENTER_PIN)); 


    //TODO: Input Validation?
    char index_char = self->ChannelSelection + 48; //add 48 to shift 0... to ascii '0'
    self->title[self->TITLE_DISP_CHANNELNUM_INDEX] = index_char; //Show the channel selection in the correct spot
  
    //do things! (respond to buttons

    //ALWAYS do OPT1 action (change channel)
    if(ampgui__button_getPush(self,AMPGUI_OPT1)){
      self->ChannelSelection = self->ChannelSelection +1;

      if(self->ChannelSelection > self->MAX_CHANNEL_NUM){ //overflow.. roll over to 1
        self->ChannelSelection = 1;
      }else if(self->ChannelSelection < 1){
        self->ChannelSelection > self->MAX_CHANNEL_NUM; //underflow.. roll over to max
      }

    }
  
    switch (self->button_input_mode[self->ChannelSelection]){
  
      case AMPGUI_BUTTON_SET:
  
        self->title[self->title_length-2] = 'S';
        self->title[self->title_length-1] = 'e';
        self->title[self->title_length] = 't';
  
        if(ampgui__button_getPush(self, AMPGUI_UP)){            //UP BUTTON PRESSED
          self->selection_value_milli = self->selection_value_milli +10;   //INCREMENT SETPOINT
          
          if(self->selection_value_milli > AMPGUI_OUTPUT_MAX){ //reset on overflow
            self->selection_value_milli = AMPGUI_OUTPUT_MAX;
          }
        }
        if(ampgui__button_getHold(self, AMPGUI_UP)){            //UP BUTTON HELD
          uint16_t tempval = ampgui__button_getHold(self, AMPGUI_UP);
  
          if(tempval < 50){
            self->selection_value_milli = self->selection_value_milli +100;   //INCREMENT SETPOINT
          }else{
            self->selection_value_milli = self->selection_value_milli +1000;   //INCREMENT SETPOINT
          }
          
          if(self->selection_value_milli > AMPGUI_OUTPUT_MAX){ //reset on overflow
            self->selection_value_milli = AMPGUI_OUTPUT_MAX;
          }
        }
        
        
        if(ampgui__button_getPush(self, AMPGUI_DOWN)){          //DOWN BUTTON PRESSESD
          self->selection_value_milli = self->selection_value_milli -10;   //DECREMNT SETPOINT
          
          if(self->selection_value_milli > AMPGUI_OUTPUT_MAX){ //reset on underflow (unsigned)
            self->selection_value_milli = 0;
          }
        }
        if(ampgui__button_getHold(self, AMPGUI_DOWN)){          //DOWN BUTTON HELD
          uint16_t tempval = ampgui__button_getHold(self, AMPGUI_DOWN);
  
          if(tempval < 50){
            self->selection_value_milli = self->selection_value_milli -100;   //DECREMNT SETPOINT
          }else{
            self->selection_value_milli = self->selection_value_milli -1000;   //DECREMNT SETPOINT
          }
          
          if(self->selection_value_milli > AMPGUI_OUTPUT_MAX){ //reset on underflow (unsigned)
            self->selection_value_milli = 0;
          }
        }
  
        if(ampgui__button_getPush(self, AMPGUI_ENTER)){           //ENTER BUTTON PRESSESD
          self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_NORMAL;            //CHANGE INPUT MODE to  "AMPGUI_BUTTON_NORMAL"
        }
  
        if(ampgui__button_getHold(self, AMPGUI_ENTER)){         //ENTER BUTTON HELD #TODO: This may flicker between set and normal for a while. Fix!
          self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_RUN;          //CHANGE INPUT MODE to  "AMPGUI_BUTTON_RUN"
        }
  
      break;
      case AMPGUI_BUTTON_RUN: //Signal main loop to regulate setpoint. lock out changing variables.
  
        self->title[self->title_length-2] = 'R';
        self->title[self->title_length-1] = 'U';
        self->title[self->title_length] = 'N';
  
        if(ampgui__button_getPush(self, AMPGUI_ENTER)){           //ENTER BUTTON PRESSESD
          self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_NORMAL;            //CHANGE INPUT MODE to  "AMPGUI_BUTTON_NORMAL"
        }
  
      break;
  
      default:
        self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_NORMAL; //Resolve invalid states
        self->title[self->title_length-2] = 'C';
        self->title[self->title_length-1] = 'f';
        self->title[self->title_length] = 'g';
        
        if(ampgui__button_getPush(self, AMPGUI_UP)){            //UP BUTTON PRESSED
          self->screen_selection = self->screen_selection -1;   //Up one row (Selection -1)
  
          if(self->screen_selection < AMPGUI_SEL_V){ //reset on underflow
            self->screen_selection = AMPGUI_SEL_P;
          }
          
        }
        
        if(ampgui__button_getPush(self, AMPGUI_DOWN)){          //DOWN BUTTON PRESSESD
          self->screen_selection = self->screen_selection +1;   //Down one row (Selection +1)
          
          if(self->screen_selection > AMPGUI_SEL_P){ //reset on overflow
            self->screen_selection = AMPGUI_SEL_V;
          }
        }
      
          //self->selection_value_milli = 0;
        //self->button_input_mode = AMPGUI_BUTTON_NORMAL;
      
        if(ampgui__button_getPush(self, AMPGUI_ENTER)){           //ENTER BUTTON PRESSESD
          self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_SET;            //CHANGE INPUT MODE to  "AMPGUI_BUTTON_SET"
        }
  
        if(ampgui__button_getHold(self, AMPGUI_ENTER)){         //ENTER BUTTON HELD #TODO: This will flicker between set and normal for a while. Fix!
          self->button_input_mode[self->ChannelSelection] = AMPGUI_BUTTON_RUN;          //CHANGE INPUT MODE to  "AMPGUI_BUTTON_RUN"
        }
        
      
        //#TODO: OPT1 Pressed - perform action
        //#TODO: OPT2 Pressed - perform action
        //#TODO: OPT3 Pressed - perform action
  
      break;
    }

  
}
  

  screen__update(self->myscreen, self->screentype, self->screen_selection, self->title, self->title_length, self->opt1_show, self->opt2_show, self->opt3_show, self->ext_mV, self->ext_mA, self->ext_mOhm, self->ext_mW, self->ext_mC); //Update SCREEN!
  
}

//AMPGUI_BUTTON_ACTIVE_STATE
//AMPGUI_BUTTON_PUSH_TIME
//AMPGUI_BUTTON_HOLD_TIME

void ampgui__button_update(struct ampgui *self, uint8_t button_index, bool newstate){

    if(self->buttons[button_index].OldState != newstate){ //BUTTON EDGE DETECTED - debounce
      self->buttons[button_index].OldTime = self->time_ms; //save current time
      self->buttons[button_index].OldState = newstate; //save current state
      self->buttons[button_index].pushed = false; //reset pushed
      self->buttons[button_index].acked = false; //reset acked
      self->buttons[button_index].holdcount = 0; //reset holdcount
      
    }else{

      if(self->buttons[button_index].OldState == AMPGUI_BUTTON_ACTIVE_STATE){

        if( (self->time_ms - self->buttons[button_index].OldTime) < 0){ //handle overflow gracefully
          self->buttons[button_index].OldTime = self->time_ms; //reset oldtime
        }

        if( (self->time_ms - self->buttons[button_index].OldTime) > AMPGUI_BUTTON_PUSH_TIME){ //push time elapsed!
          self->buttons[button_index].pushed = true; //set pushed
        }

        if( (self->time_ms - self->buttons[button_index].OldTime) > AMPGUI_BUTTON_HOLD_TIME){ //hold time elapsed!
          self->buttons[button_index].holdcount = (self->time_ms - self->buttons[button_index].OldTime) / AMPGUI_BUTTON_HOLD_TIME; //save count intervals
        }
        
      }else{
        self->buttons[button_index].pushed = false; //reset pushed
        self->buttons[button_index].acked = false; //reset acked
        self->buttons[button_index].holdcount = 0; //reset holdcount
      }
      
    }
  
}

bool ampgui__button_getPush(struct ampgui *self, uint8_t button_index){ //returns true once when pushed.
  if( self->buttons[button_index].pushed & (!self->buttons[button_index].acked) ){
    self->buttons[button_index].acked = true;
    return(true);
  }
  
  return(false);
}

uint16_t ampgui__button_getHold(struct ampgui *self, uint8_t button_index){ //returns number of hold intervals the button is held for
  return(self->buttons[button_index].holdcount);
}

void ampgui__set(struct ampgui *self, float mV, float mA, float mOhm, float mW, float mC){ //update live variables
  
  self->ext_mV = mV;
  self->ext_mA = mA;
  self->ext_mOhm = mOhm;
  self->ext_mW = mW;
  self->ext_mC = mC;

  screen__update(self->myscreen, self->screentype, self->screen_selection, self->title, self->title_length, self->opt1_show, self->opt2_show, self->opt3_show, self->ext_mV, self->ext_mA, self->ext_mOhm, self->ext_mW, self->ext_mC); //Update SCREEN!
  
}

uint16_t ampgui__getMode(struct ampgui *self){ //Returns mode (AMPGUI_SEL_x) value
  return(self->screen_selection);
}

uint32_t ampgui__getSetValue(struct ampgui *self){ //Returns current set value (milli_units)
  return(self->selection_value_milli);
}

bool ampgui__getRunLoop(struct ampgui *self){ //Returns true when the controlloop should start
  if(self->button_input_mode[self->ChannelSelection] == AMPGUI_BUTTON_RUN){
    return(true);
  }
  return(false);
}

uint8_t ampgui__getChannelSelection(struct ampgui *self){ //returns currently processed channel
  return(self->ChannelSelection);
}
