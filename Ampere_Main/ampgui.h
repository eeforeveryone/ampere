//ampgui.h A library for handling I/O functions for the Project Ampere GUI
//EEforEveryone - 2020

//This library organizes data for display in screens.
//dependent on "screen.h"

#include "Arduino.h"
#include "screen.h"
#include <Encoder.h>

// ensure this library description is only included once
#ifndef SRC_AMPGUI_H
#define SRC_AMPGUI_H

#define AMPGUI_BUTTON_ACTIVE_STATE false //logical button signal state when button is pressed
#define AMPGUI_BUTTON_PUSH_TIME 10 //time state must be held, to be considered "pushed" - ms
#define AMPGUI_BUTTON_HOLD_TIME 500 //time state must be held, to be considered "held" - ms
#define AMPGUI_NUM_BUTTONS 7

#define AMPGUI_OPT1     0 //map to array position
#define AMPGUI_OPT2     1 //map to array position
#define AMPGUI_OPT3     2 //map to array position
#define AMPGUI_UP       3 //map to array position
#define AMPGUI_DOWN     4 //map to array position
#define AMPGUI_ENTER    5 //map to array position
#define AMPGUI_ENC      6 //map to array position

#define AMPGUI_SEL_V 0 //map row 0 to volts
#define AMPGUI_SEL_I 1 //map row 1 to amps
#define AMPGUI_SEL_R 2 //map row 2 to resistance
#define AMPGUI_SEL_P 3 //map row 3 to power
#define AMPGUI_SEL_C 4 //map row 4 to energy (C)

#define AMPGUI_BUTTON_NORMAL 0 // normal button mode
#define AMPGUI_BUTTON_SET 1 //Change Setpoint mode
#define AMPGUI_BUTTON_RUN 2 //button behavior while running

#define AMPGUI_OUTPUT_MAX 100000

#define AMPGUI_MAXPOSSIBLECHANNELS 10 //allow for changing between up to N ampere channels (memory reservation)... actual number processed dependent on create argument maxChannels.



struct ampgui;
struct gui_button;

/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct ampgui* ampgui__create(struct screen* screen_obj, int maxChannels, int upPin, int downPin, int enterPin, int opt1Pin, int opt2Pin, int opt3Pin); //constructor
void ampgui__init(struct ampgui *self, struct screen* screen_obj, int maxChannels, int upPin, int downPin, int enterPin, int opt1Pin, int opt2Pin, int opt3Pin); //modifier (empty)
void ampgui__startEncoder(struct ampgui *self, Encoder *enc, int button); //start encoder
void ampgui__destroy(struct ampgui *self); //destructor

/**************MEMBER FUNCTIONS**************************/

void ampgui__update(struct ampgui *self, uint32_t ms, bool passive); //Reacts to button presses, GUI in control when passive == false
void ampgui__button_update(struct ampgui *self, uint8_t button_index, bool newstate);
bool ampgui__button_getPush(struct ampgui *self, uint8_t button_index); //returns true once when pushed.
uint16_t ampgui__button_getHold(struct ampgui *self, uint8_t button_index); //returns number of hold intervals the button is held for

void ampgui__set(struct ampgui *self, float mV, float mA, float mOhm, float mW, float mC); //update live variables

uint16_t ampgui__getMode(struct ampgui *self); //Returns mode (AMPGUI_SEL_x) value
uint32_t ampgui__getSetValue(struct ampgui *self); //Returns current set value (milli_units)
bool ampgui__getRunLoop(struct ampgui *self); //Returns true when the controlloop should start

uint8_t ampgui__getChannelSelection(struct ampgui *self); //returns currently processed channel

#endif
