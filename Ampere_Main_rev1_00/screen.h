//Screen.h A library for handling Screens
//EEforEveryone - 2020


// ensure this library description is only included once
#ifndef SRC_SCREEN_H
#define SRC_SCREEN_H

#define OPT_LENGTH 4 //Length of OPT codes, in CHARS

#define LINE_LENGTH 21 //length of one row, in CHARS
#define LINE_HEIGHT 8 //Number of lines of text we can have, in CHARS

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DATA_CHAR_LENGTH 8 //number of chars to store
#define DATA_INT_LENGTH 5 //number of integer places
#define DATA_FLOAT_LENGTH 2 //number of float places

#define I2C_ADDRESS 0x3C //Address for the Display


#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"


struct screen;


/**************CONSTRUCTORS AND DESTRUCTORS**************/
struct screen* screen__create(); //constructor (I2C_ADDRESS)
void screen__init(struct screen *self); //modifier (empty)
void screen__destroy(struct screen *self); //destructor


/**************MEMBER FUNCTIONS**************************/
void screen__erase(struct screen *self, int screentype); //creates a screen framework for a certain type
void screen__update(struct screen *self, int screentype, uint8_t selection, char *title, uint8_t title_len, char *opt1, char *opt2, char *opt3, float voltage, float current, float resistance, float power, float energy); //Draw Blank Screen
bool screen__get_NEED_NUKE(struct screen *self);
void INT_screen__show(struct screen *self);
bool INT_screen__float_to_char(char *numout, float value); //returns true if okay, returns false if truncated

char* screen__getline1(struct screen *self); //returns line1
char* screen__getline2(struct screen *self); //returns line2
char* screen__getline3(struct screen *self); //returns line3
char* screen__getline4(struct screen *self); //returns line4
char* screen__getline5(struct screen *self); //returns line5
char* screen__getline6(struct screen *self); //returns line6
char* screen__getline7(struct screen *self); //returns line7
char* screen__getline8(struct screen *self); //returns line8

#endif
