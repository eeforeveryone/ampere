//ampserial.cpp A library for handling the external Control and Monitoring Interface
//Default: Serial (UART)
//EEforEveryone - 2020

//Dependent on Serial library for arduino

#include  "ampserial.h"

/**************STRUCT DEFINITION*************************/
struct ampserial{

bool new_CMD; //true when a new serial command was recieved

bool CTRL_OVERRIDE; //true when UART trumps UI
bool Print_All; //true when UART requested statistics
uint32_t print_interval_ms; //delay between Reports
uint32_t last_print_ms[AMPSERIAL_MAXPOSSIBLECHANNELS+1]; //last print time, in milliseconds

uint8_t MAX_CHANNEL_NUM;
uint8_t ChannelSelection; //Range = 1:self->MAX_CHANNEL_NUM (inclusive)

char rxbuffer[AMPSERIAL_BUFFER_LENGTH];
uint16_t rx_index;

bool enable_output; //true when time to turn on external interface
uint32_t setpoint_millis; // value of control_loop, in milliunits
int regulation_mode; // mode we are regulating

};


/**************CONSTRUCTORS AND DESTRUCTORS**************/




/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct ampserial* ampserial__create(int maxChannels){ //constructor
  struct ampserial* result = (struct ampserial*) malloc(sizeof(struct ampserial)); //alocate memory, typecast to ampserial struct.

  ampserial__init(result, maxChannels); //initialize to default values
  return(result);
}

void ampserial__init(struct ampserial *self, int maxChannels){ //modifier (empty)
  
  self->setpoint_millis = 0; //setpoint = 0A
  self->regulation_mode = AMPGUI_SEL_I; //defined in ampgui.h

  for (int i = 0; i<(AMPSERIAL_MAXPOSSIBLECHANNELS+1); i++){
    self->last_print_ms[i] = 0; //last print time, in milliseconds
  }
  self->Print_All = true; //print statistics

  if(maxChannels > AMPGUI_MAXPOSSIBLECHANNELS){ //Input validation on number of channels
    self-> MAX_CHANNEL_NUM = AMPGUI_MAXPOSSIBLECHANNELS;
  }else if( maxChannels < 1){
    self-> MAX_CHANNEL_NUM = 1;
  }else{
    self-> MAX_CHANNEL_NUM = maxChannels;
  }
  self-> ChannelSelection = 1; //Range = 1:self->MAX_CHANNEL_NUM (inclusive)
  
  self->print_interval_ms = AMPSERIAL_INTERVAL_MAX;
  self->enable_output = false; //turn off output
  self->CTRL_OVERRIDE = false;
  self->new_CMD = false;

  
  INT_ampserial__flushRX(self); //initialize rxbuffer

  INT_ampserial__SerialBegin(); //start serial control object!
}

void ampserial__destroy(struct ampserial *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/
void INT_ampserial__SerialBegin(){
  Serial1.begin(AMPSERIAL_BAUD); //start up the serial interface
  //print preamble:
  char var[] = "\0Project Ampere Boot! HW_V1.0 FW_V1.0DRAFT";
  INT_ampserial__PrintString(var, 42);
  INT_ampserial__PrintNewLine();
}
void INT_ampserial__PrintFloat(float number){
  Serial1.print(number);
}
void INT_ampserial__PrintInt(uint32_t number){
  Serial1.print(number);
}
void INT_ampserial__PrintString(char *test, int leng){
  for(int i = 0; i<leng; i++){
    Serial1.print(test[i]);
  }
}
void INT_ampserial__PrintNewLine(){
  Serial1.println("\0");
}
uint8_t INT_ampserial__GetByte(){
  return(Serial1.read());
}
int INT_ampserial__SerialAvailable(){ //returns the number of available bytes
  return(Serial1.available());
}
void INT_ampserial__flushRX(struct ampserial *self){ //flushes software RX buffer, not hardware
  for(int i = 0; i<AMPSERIAL_BUFFER_LENGTH; i++){
    self->rxbuffer[i] = ' ';
    self->rx_index = 0; //start at the 0th index
  }
}

uint8_t ampserial__getChannelSelection(struct ampserial *self){ //returns integer of last channel selection (validated)
  return(self->ChannelSelection); //Range = 1:self->MAX_CHANNEL_NUM (inclusive)
}

void ampserial__sendReport(struct ampserial *self, uint32_t time_ms, int OutChannelNum, uint32_t volts, uint32_t amps, uint32_t kiloJoules){

  int this_channelnum = OutChannelNum;
  if(this_channelnum > AMPSERIAL_MAXPOSSIBLECHANNELS){
    this_channelnum = 0;
  }else if(this_channelnum < 1){
    this_channelnum = 0; //Don't send anything if misconfigured
  }

  if(self->Print_All == true && this_channelnum > 0){
    if((time_ms > self->last_print_ms[this_channelnum] + self->print_interval_ms) || (time_ms < self->last_print_ms[this_channelnum])){ //time to print, or ms overflow

      //print preamble:
      char var[] = "L";
      INT_ampserial__PrintString(var, 1);
      INT_ampserial__PrintInt(OutChannelNum);
      INT_ampserial__PrintNewLine();
      
      //print voltage:
      char var1 = 'V';
      INT_ampserial__PrintString(&var1, 1);
      INT_ampserial__PrintInt(volts);
      INT_ampserial__PrintNewLine();

      //print current:
      char var2 = 'I';
      INT_ampserial__PrintString(&var2, 1);
      INT_ampserial__PrintInt(amps);
      INT_ampserial__PrintNewLine();

      //print energy (kJ):
      char var3 = 'J';
      INT_ampserial__PrintString(&var3, 1);
      INT_ampserial__PrintInt(kiloJoules);
      INT_ampserial__PrintNewLine();

      self->last_print_ms[this_channelnum] = time_ms;
    }
  }
  
}

void ampserial__handleRX(struct ampserial *self){ //handles incoming_data
  //Stage 1, load RX buffer... overwriting stale data.
  while(INT_ampserial__SerialAvailable() > 0){
    self->rxbuffer[self->rx_index] = INT_ampserial__GetByte();//load the next byte
    self->rx_index = self->rx_index +1; //increment index

    if(self->rx_index >= AMPSERIAL_BUFFER_LENGTH){
      self->rx_index = 0;
    }
    
  }//AMPSERIAL_BUFFER_LENGTH

  //Stage 2, Flatten RX buffer... (remove circular reference)
  char temp_rx[AMPSERIAL_BUFFER_LENGTH+1];
  for(int i = 0; i< AMPSERIAL_BUFFER_LENGTH; i++){
    
    uint16_t thisindex = self->rx_index+i; //position, assuming no overflow
    if(thisindex > AMPSERIAL_BUFFER_LENGTH-1){ //wrap around buffer if overflow
      thisindex = thisindex-(AMPSERIAL_BUFFER_LENGTH);
    }
    
    temp_rx[i] = self->rxbuffer[thisindex];
  }
  temp_rx[AMPSERIAL_BUFFER_LENGTH] = '\0'; //null terminated char array for string conversion
  String rxbuff = String(temp_rx);

  if(PRINT_DEBUG_SERIAL1){
    Serial1.println(rxbuff);
  }


  //Stage 3, Search flattened string buffer for Header flags.
  int header_index[AMPSERIAL_NUM_HEADERTYPES];

  for(int i = 0; i<AMPSERIAL_NUM_HEADERTYPES ; i++){ //FIND ALL HEADER INDICIES
    header_index[i] = -1;

    switch(i){
      case AMPSERIAL_CMD_SETINTERVAL_INDEX:
          header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_SETINTERVAL);
      break;
      case AMPSERIAL_CMD_SETMODE_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_SETMODE);
      break;
      case AMPSERIAL_CMD_SETVAL_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_SETVAL);
      break;
      case AMPSERIAL_CMD_START_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_START);
      break;
      case AMPSERIAL_CMD_STOP_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_STOP);
      break;
      case AMPSERIAL_CMD_OVERRIDE_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_OVERRIDE);
      break;
      case AMPSERIAL_CMD_RELEASE_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_RELEASE);
      break;
      case AMPSERIAL_CMD_CHANNEL_SELECT_INDEX:
        header_index[i] = rxbuff.indexOf(AMPSERIAL_CMD_CHANNEL_SELECT);
      break;
      default:
        //TODO: ERROR! Do something about it.
      break;
    }
  }

  int last_header_index = -1;
  int last_header_type = -1;
  int last_header_value = -1;

  int previous_header_index = -1;
  int previous_header_type = -1;
  int previous_header_value = -1;


  for(int i = 0; i<AMPSERIAL_NUM_HEADERTYPES ; i++){ //find the two most recent packets
    switch(i){
      case AMPSERIAL_CMD_SETINTERVAL_INDEX: //header type = setinterval
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      case AMPSERIAL_CMD_SETMODE_INDEX: //header type = setmode
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      case AMPSERIAL_CMD_SETVAL_INDEX: //header type = setval
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      case AMPSERIAL_CMD_START_INDEX: //header type = start
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      case AMPSERIAL_CMD_STOP_INDEX: //header type = stop
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;

      case AMPSERIAL_CMD_OVERRIDE_INDEX:
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;

      case AMPSERIAL_CMD_RELEASE_INDEX:
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      case AMPSERIAL_CMD_CHANNEL_SELECT_INDEX:
        if(header_index[i] > last_header_index){ //if this packet type is the last one recieved...
          previous_header_index = last_header_index; //shift the old data back
          previous_header_type = last_header_type;

          last_header_index = header_index[i]; //save this header index
          last_header_type = i; //save the header type
        }
      break;
      
      default:
        //TODO: ERROR! Do something about it.
      break;
    }
    
  }


//Stage 4, Search flattened string buffer for Footer flags. (frame valid packets)
  int this_packet_type = -1;
  int this_packet_start_index = -1;
  int this_packet_stop_index = -1;
  if(last_header_type > -1){ //if we found a valid header!

    this_packet_stop_index = rxbuff.indexOf(AMPSERIAL_DELIMITER, last_header_index); //look for the last footer.
    if(this_packet_stop_index > -1){ //the footer is found, use the last header!
      this_packet_type = last_header_type; //remember the last header type
      this_packet_start_index = last_header_index+AMPSERIAL_CMD_LENGTH; //data start index! (remove header)
      
    }else{ //no footer for last packet!
      this_packet_stop_index = rxbuff.indexOf(AMPSERIAL_DELIMITER, last_header_index); //look for the previous footer.
      if(this_packet_stop_index > -1){ //success!
        this_packet_type = previous_header_type; //remember the last header type
        this_packet_start_index = previous_header_index+AMPSERIAL_CMD_LENGTH; //data start index! (remove header)
      }
      
    }
    
  }

//Stage 5, Extract Data!
  int data = -1;
  if(this_packet_stop_index > -1){ //found valid header and footer
    if(PRINT_DEBUG_SERIAL1){
      Serial1.print("Start Index: ");
      Serial1.println(this_packet_start_index);
      Serial1.print(" Stop Index: ");
      Serial1.println(this_packet_stop_index);
    }
  
    if(this_packet_start_index < this_packet_stop_index){ //if there is data (not empty packet)
      String data_string = rxbuff.substring(this_packet_start_index,this_packet_stop_index);
      data = data_string.toInt();
  
      if(PRINT_DEBUG_SERIAL1){
        Serial1.println(data_string);
        Serial1.println(data);
      }
    }
  
    //Stage 5.1, Save data to appropriate location!
    switch(this_packet_type){ //TODO: check for same packet twice!
        case AMPSERIAL_CMD_SETINTERVAL_INDEX:
            if(data > AMPSERIAL_INTERVAL_MAX){
              data = AMPSERIAL_INTERVAL_MAX;
            }else if( data < AMPSERIAL_INTERVAL_MIN){
              data = AMPSERIAL_INTERVAL_MIN;
            }
            
            self->print_interval_ms = data;
          
        break;
        case AMPSERIAL_CMD_SETMODE_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->regulation_mode = data;
          
        break;
        case AMPSERIAL_CMD_SETVAL_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->setpoint_millis = data;
          
        break;
        case AMPSERIAL_CMD_START_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->enable_output = true;
          
        break;
        case AMPSERIAL_CMD_STOP_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->enable_output = false;
          
        break;
        case AMPSERIAL_CMD_OVERRIDE_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->CTRL_OVERRIDE = true; //set control override flag.
          
        break;
        case AMPSERIAL_CMD_RELEASE_INDEX:
          self->new_CMD = true; //set new CMD flag
          self->CTRL_OVERRIDE = false; //clear control override flag.
          
        break;
        case AMPSERIAL_CMD_CHANNEL_SELECT_INDEX:
          self->new_CMD = true; //set new CMD flag
          uint8_t validated_data = data;
          
          if(validated_data > self->MAX_CHANNEL_NUM){
            validated_data = self->MAX_CHANNEL_NUM;
          }else if(validated_data < 1){
            validated_data = 1;
          }
          self->ChannelSelection = validated_data; //Range = 1:self->MAX_CHANNEL_NUM (inclusive)

        break;
        default:
          //TODO: ERROR! Do something about it.
        break;
      }


    //Stage 5.2, Corrupt valid header after processing
    int changeIndex = this_packet_start_index - (AMPSERIAL_CMD_LENGTH/2);
    
    changeIndex = changeIndex+self->rx_index; //Set position to "n" chars beyond the write index.
    if(changeIndex > AMPSERIAL_BUFFER_LENGTH-1){
      changeIndex = changeIndex - AMPSERIAL_BUFFER_LENGTH;
    }
    
    self->rxbuffer[changeIndex] = '{';// Destroy the header

  }

}
bool ampserial__InControl(struct ampserial *self){ //returns true when a Current setpoint was recieved via Serial.
  return(self->CTRL_OVERRIDE);
}
uint32_t ampserial__getSetpoint(struct ampserial *self){ //returns the last Setpoint recieved via Serial
  return(self->setpoint_millis);
}
int ampserial__getMode(struct ampserial *self){ //returns the last mode recieved via Serial
  return(self->regulation_mode);
}
bool ampserial__getRun(struct ampserial *self){ //returns wether the output should be on or off.
  return(self->enable_output);
}
bool ampserial__newCMD(struct ampserial *self){ //returns true when a new command was recieved (Clear on Read)
  if(self->new_CMD == true){
    self->new_CMD = false;
    return(true);
  }
  return(false);
}
