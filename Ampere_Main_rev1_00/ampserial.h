//ampserial.h A library for handling the external Control and Monitoring Interface
//Default: Serial (UART)
//EEforEveryone - 2020

//Dependent on Serial library for arduino

#include "Arduino.h"
#include  "ampgui.h" //Needed for #define of regulation modes. example: "AMPGUI_SEL_V"
#include "string.h" //needed for parsing incoming data

// ensure this library description is only included once
#ifndef SRC_AMPSERIAL_H
#define SRC_AMPSERIAL_H

struct ampserial;

#define AMPSERIAL_BAUD 115200 //BAUD RATE FOR SERIAL INTERFACE
#define AMPSERIAL_INTERVAL_MIN 100    //MINIMUM DELAY BETWEEN STATUS REPORTS, MIN
#define AMPSERIAL_INTERVAL_MAX 5000  //MAXIMUM DELAY BETWEEN STATUS REPORTS, MAX
#define AMPSERIAL_BUFFER_LENGTH 64 //length of command buffer, in bytes

#define AMPSERIAL_MAXPOSSIBLECHANNELS 10 //allow for changing between up to N ampere channels (memory reservation)... actual number processed dependent on create argument maxChannels.

#define AMPSERIAL_DELIMITER       "<---->" //Delimiter used to mark command end.
#define AMPSERIAL_CMD_SETINTERVAL "<INTm>" //Preamble for SETINTERVAL_ms
#define AMPSERIAL_CMD_SETMODE     "<MODE>" //Preamble for SETMODE (Voltage, Current, etc)
#define AMPSERIAL_CMD_SETVAL      "<VALm>" //Preamble for SET Current (milli-units)
#define AMPSERIAL_CMD_START       "<STRT>" //Enters Run Mode 
#define AMPSERIAL_CMD_STOP        "<STOP>" //Stops Run Mode
#define AMPSERIAL_CMD_OVERRIDE    "<OVER>" //Take control from GUI (Open Session)
#define AMPSERIAL_CMD_RELEASE     "<RELS>" //Release Control (Close Session)
#define AMPSERIAL_CMD_CHANNEL_SELECT "<CHAN>" //pick what channel to use

#define AMPSERIAL_CMD_SETINTERVAL_INDEX 0 //index used for parsing recieved commands
#define AMPSERIAL_CMD_SETMODE_INDEX     1
#define AMPSERIAL_CMD_SETVAL_INDEX      2
#define AMPSERIAL_CMD_START_INDEX       3
#define AMPSERIAL_CMD_STOP_INDEX        4 
#define AMPSERIAL_CMD_OVERRIDE_INDEX    5
#define AMPSERIAL_CMD_RELEASE_INDEX     6
#define AMPSERIAL_CMD_CHANNEL_SELECT_INDEX 7


#define AMPSERIAL_NUM_HEADERTYPES       8 //number of unique header keys!

#define AMPSERIAL_CMD_LENGTH 6 //number of chars in each command

#define PRINT_DEBUG_SERIAL1 false //true = print console stuff to debug string operations... false = don't #TODO: Set to false!


#define AMPSERIAL_BAUD 2000000 //BAUD RATE FOR SERIAL INTERFACE (2Mpps)

/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct ampserial* ampserial__create(int maxChannels); //constructor
void ampserial__init(struct ampserial *self, int maxChannels); //modifier (empty)
void ampserial__destroy(struct ampserial *self); //destructor

/**************MEMBER FUNCTIONS**************************/
void INT_ampserial__SerialBegin();
void INT_ampserial__PrintFloat(float number);
void INT_ampserial__PrintString(char *test, int leng);
void INT_ampserial__PrintNewLine();
uint8_t INT_ampserial__GetByte();
int INT_ampserial__SerialAvailable(); //returns the number of available bytes
void INT_ampserial__flushRX(struct ampserial *self); //flushes software RX buffer, not hardware

void ampserial__sendReport(struct ampserial *self, uint32_t time_ms, int OutChannelNum, uint32_t volts, uint32_t amps, uint32_t kiloJoules); //sends a report to the Serial interface at an appropriate interval
void ampserial__handleRX(struct ampserial *self); //handles incoming_data

bool ampserial__InControl(struct ampserial *self); //returns true when a Current setpoint was recieved via Serial.
uint32_t ampserial__getSetpoint(struct ampserial *self); //returns the last Setpoint recieved via Serial
int ampserial__getMode(struct ampserial *self); //returns the last mode recieved via Serial
bool ampserial__getRun(struct ampserial *self); //returns wether the output should be on or off.
bool ampserial__newCMD(struct ampserial *self); //returns true when a new command was recieved (Clear on Read)

uint8_t ampserial__getChannelSelection(struct ampserial *self); //returns integer of last channel selection (validated)

#endif
