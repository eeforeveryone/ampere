//Screen.cpp A library for handling Screens
//EEforEveryone - 2020

#include <Arduino.h>

#include "screen.h"
#include "string.h"


/**************STRUCT DEFINITION*************************/
struct screen{
  
SSD1306AsciiWire thisoled; //I2C Display Object

char line1[LINE_LENGTH+1];
char line2[LINE_LENGTH+1];
char line3[LINE_LENGTH+1];
char line4[LINE_LENGTH+1];
char line5[LINE_LENGTH+1];
char line6[LINE_LENGTH+1];
char line7[LINE_LENGTH+1];
char line8[LINE_LENGTH+1];

char volt[DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2]; //+1 for Overflow indicator, +1 for decimal place
char curr[DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2];
char resi[DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2];
char powe[DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2];
char ener[DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2];

bool rdy;
bool need_nuke;
int last_screen_type;
  
};


/**************CONSTRUCTORS AND DESTRUCTORS**************/

bool screen__get_NEED_NUKE(struct screen *self){

  if(self->need_nuke){
    self->need_nuke = false;
    return(true);
  }
  return(false);
}

struct screen* screen__create(){ //constructor ()
  
  struct screen* result = (struct screen*) malloc(sizeof(struct screen)); //alocate memory, typecast to tempsensor struct.
  screen__init(result); //initialize to default values
  return(result);
}
void screen__init(struct screen *self){ //modifier (empty)
  
  //self->thisoled.begin(&Adafruit128x64, I2C_ADDRESS);
  //self->thisoled.clear();

  for(int i = 0; i<(LINE_LENGTH+1); i++){
    self->line1[i] = '\0';
    self->line2[i] = '\0';
    self->line3[i] = '\0';
    self->line4[i] = '\0';
    self->line5[i] = '\0';
    self->line6[i] = '\0';
    self->line7[i] = '\0';
    self->line8[i] = '\0';
  }

  for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){
    self->volt[i] = '\0';
    self->curr[i] = '\0';
    self->resi[i] = '\0';
    self->powe[i] = '\0';
    self->ener[i] = '\0';
    }
  self->last_screen_type = 99999; //invalid
  self->rdy = false;
  self->need_nuke = true; //flag that we must do a clear before write (minimize blinking / traffic)
  
}


void screen__destroy(struct screen *self){ //destructor
  if(self){
    free(self);
  }
}

/**************MEMBER FUNCTIONS**************************/

void screen__erase(struct screen *self, int screentype){

  for(int i = 0; i<(LINE_LENGTH); i++){
    self->line1[i] = ' ';
    self->line2[i] = ' ';
    self->line3[i] = ' ';
    self->line4[i] = ' ';
    self->line5[i] = ' ';
    self->line6[i] = ' ';
    self->line7[i] = ' ';
    self->line8[i] = ' ';
  }
  
  
  switch (screentype){

    default: //(default screen)
      //OPT_LENGTH //defines length of OPT field

      for(int i = 0; i<LINE_LENGTH; i++){
        self->line2[i] = '-'; //fill line 2 with horizontal dashes
      }

      self->line3[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces
      self->line4[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces
      self->line5[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces
      self->line6[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces
      self->line7[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces
      self->line8[(OPT_LENGTH+1)] = '|'; //Pad opt lines with spaces

    break;
  }


  self->last_screen_type = screentype; //save the current screentype
  INT_screen__show(self); //Show the updated buffer
  self->rdy = true; //flag that we're ready to process incoming data
  self->need_nuke = true; //flag that the screen needs to be wiped to redraw lines
}

void screen__update(struct screen *self, int screentype, uint8_t selection, char *title, uint8_t title_len, char *opt1, char *opt2, char *opt3, float voltage, float current, float resistance, float power, float energy){ //Draw Blank Screen

 if(screentype != self->last_screen_type){
    screen__erase(self, screentype);
 }
  
switch (self->last_screen_type) {

default: //(DEFAULT)
   //Fill OPT Screens - consumes CHARS 0- (OPT_LENGTH+1) of left space
  for(int i = 0; i<(OPT_LENGTH+1); i++){ //load data into screen
    if(i < OPT_LENGTH){ 
      self->line3[i] = opt1[i]; //Grab opt content TODO: Input validation in case of opt char being too short!
      self->line4[i] = '-';
      self->line5[i] = opt2[i]; //Grab opt content TODO: Input validation in case of opt char being too short!
      self->line6[i] = '-';
      self->line7[i] = opt3[i]; //Grab opt content TODO: Input validation in case of opt char being too short!
      self->line8[i] = '-';
      
    }else{
      self->line3[i] = ' '; //Pad lines with spaces
      self->line4[i] = ' '; //Pad lines with spaces
      self->line5[i] = ' '; //Pad lines with spaces
      self->line6[i] = ' '; //Pad lines with spaces
      self->line7[i] = ' '; //Pad lines with spaces
      self->line8[i] = ' '; //Pad lines with spaces
    }
  }

  //FILL TITLE AREA
  for(int i = 0; i<LINE_LENGTH; i++){
    if(title_len > (i-1)){
      self->line1[i] = title[i]; //Grab title content
    }else{
      self->line1[i] = ' '; //Pad 1 line with spaces
    }
  }

  //TODO: Fill area in the rest of the screen!
          uint8_t selected_row = selection + 4; //selection: 0 = 4th display row
  
  int startval = (OPT_LENGTH+2+3); //This is the first position for variables
  
    //Voltage on Line 4
    if(selected_row == 4){
      self->line4[startval-3] = '*';
    }else{
      self->line4[startval-3] = ' ';
    }
    self->line4[startval-2] = 'V';
    self->line4[startval-1] = '=';
    
    INT_screen__float_to_char(self->volt,  voltage); //Convert a float into a char string
    for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){ //Use the entire output length
        self->line4[i+startval] = self->volt[i];
    }
    self->line4[startval+(DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2)] = 'V';
  
  
    //Current on Line 5
    if(selected_row == 5){
      self->line5[startval-3] = '*';
    }else{
      self->line5[startval-3] = ' ';
    }
    self->line5[startval-2] = 'I';
    self->line5[startval-1] = '=';
    
    INT_screen__float_to_char(self->curr, current); //Convert a float into a char string
    for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){ //Use the entire output length
        self->line5[i+startval] = self->curr[i];
    }
    self->line5[startval+(DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2)] = 'A';
  
  
    //Resistance on Line 6
    if(selected_row == 6){
      self->line6[startval-3] = '*';
    }else{
      self->line6[startval-3] = ' ';
    }
    self->line6[startval-2] = 'R';
    self->line6[startval-1] = '=';
    
    INT_screen__float_to_char(self->resi, resistance); //Convert a float into a char string
    for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){ //Use the entire output length
        self->line6[i+startval] = self->resi[i];
    }
    self->line6[startval+(DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2)] = 'O';
  
  
  //power on Line 7
  if(selected_row == 7){
      self->line7[startval-3] = '*';
    }else{
      self->line7[startval-3] = ' ';
    }
    self->line7[startval-2] = 'P';
    self->line7[startval-1] = '=';
    
    INT_screen__float_to_char(self->powe, power); //Convert a float into a char string
    for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){ //Use the entire output length
        self->line7[i+startval] = self->powe[i];
    }
    self->line7[startval+(DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2)] = 'W';
  
  
    //energy on Line 8
    if(selected_row == 8){
      self->line8[startval-3] = '*';
    }else{
      self->line8[startval-3] = ' ';
    }
    self->line8[startval-2] = 'E';
    self->line8[startval-1] = '=';
    
    INT_screen__float_to_char(self->ener, energy); //Convert a float into a char string
    for(int i = 0; i< (DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2); i++){ //Use the entire output length
        self->line8[i+startval] = self->ener[i];
    }
    self->line8[startval+(DATA_INT_LENGTH+DATA_FLOAT_LENGTH+2)] = 'J'; //joules

break;
}

  

  INT_screen__show(self); //Show the updated buffer
}


bool INT_screen__float_to_char(char *numout, float value){ //Convert a float into a char string
 bool okay = true; //no truncation
 int val_int = value; //integer
 int val_float = (value-val_int)*1000; //Integer representation of decimal

 char integer_result[DATA_CHAR_LENGTH];
 for(int i = 0; i< DATA_CHAR_LENGTH; i++){ //Clear the char to " "
    integer_result[i] = '\0';
 }
 
 if(val_int == 0){
  integer_result[0] = '0';
 }else{
  sprintf(integer_result, "%d", val_int); //convert int to string
 }


 char float_result[DATA_CHAR_LENGTH];
 for(int i = 0; i< DATA_CHAR_LENGTH; i++){ //Clear the char to " "
    float_result[i] = '\0';
 }
 sprintf(float_result, "%d", val_float); //convert int to string
 
 uint8_t shiftleft_places = 0;
 if(val_float > 99){
  shiftleft_places = 0;
 }else if(val_float > 9){
  shiftleft_places = 1;
 }else{
  shiftleft_places = 2;
 }

 uint8_t intsize = strlen(integer_result); //Get length of int string

 uint8_t integer_index = 0;
 if(intsize > DATA_INT_LENGTH){
  okay = false; //needed to truncate the integer!
 }
 for(int i = 0; i< (DATA_INT_LENGTH); i++){ //Save Integer, padd with leading " "

  if(i < ((DATA_INT_LENGTH)-intsize)){ //not to intsize yet
    numout[i] = ' '; //Pad with leading zeros
  }else{
    numout[i] = integer_result[integer_index];
    integer_index = integer_index + 1;
  }

    
 }
 numout[DATA_INT_LENGTH] = '.'; //add decimal place


uint8_t float_offset = (DATA_INT_LENGTH+1);
 uint8_t floatsize = strlen(float_result); //Get length of float string
 
 for(int i = 0; i< DATA_FLOAT_LENGTH; i++){ //Save Float, padd with trailing " "
    if( i < (floatsize+shiftleft_places) ){ //Wait until string is finished or name length is exceeded

       if(i < shiftleft_places){ //float is less than 0.1xxx
        numout[i+float_offset] = '0'; //add leading zero(s) on the float.
       }else{
        numout[i+float_offset] = float_result[i-shiftleft_places]; //copy over float data!
       }
     
    }else{
      numout[i+float_offset] = '0';
    }
  
 }

 if(okay){
  numout[float_offset+DATA_FLOAT_LENGTH] = ' ';
 }else{
  numout[float_offset+DATA_FLOAT_LENGTH] = '+'; //indicate overflow
  
  for(int i = 0; i< (DATA_INT_LENGTH); i++){ //fill integer with 9's
    numout[i] = '9';
  }
  for(int i = 0; i< DATA_FLOAT_LENGTH; i++){ //fill float with 9's
      numout[i+float_offset] = '9';
  }

 }

  return(okay);
}

void INT_screen__show(struct screen *self){ //This Function causes system reboot! TODO: Fix!
 /*self->thisoled.clear();
 self->thisoled.println(self->line1);
 self->thisoled.println(self->line2);
 self->thisoled.println(self->line3);
 self->thisoled.println(self->line4);
 self->thisoled.println(self->line5);
 self->thisoled.println(self->line6);
 self->thisoled.println(self->line7);
 self->thisoled.println(self->line8);*/
}


/**************TEMP FUNCTIONS**************************/
char* screen__getline1(struct screen *self){
  return self->line1;
}
char* screen__getline2(struct screen *self){
  return self->line2;
}
char* screen__getline3(struct screen *self){
  return self->line3;
}
char* screen__getline4(struct screen *self){
  return self->line4;
}
char* screen__getline5(struct screen *self){
  return self->line5;
}
char* screen__getline6(struct screen *self){
  return self->line6;
}
char* screen__getline7(struct screen *self){
  return self->line7;
}
char* screen__getline8(struct screen *self){
  return self->line8;
}
