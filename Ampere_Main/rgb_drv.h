/*
 * rgb_drv.h
 *
 * This class handles enumeration, configuration, and communication with RGB strips using the WS2812B protocol.
 * ALL Numbers Reported in milli-units unless otherwise noted! (Integer carrying 3 decimal places)
 *
 *
 *usng FastLED version 3.3.2
 *
 *
 *  Created on:     Jan 16, 2020
 *  Last Modified:  Jan 16, 2020
 *      Author: EEforEveryone
 */

 #include <Arduino.h>
#include <Adafruit_NeoPixel.h>


#ifndef RGB_DRV_H
#define RGB_DRV_H

#define RGB_DRV_NUM_LEDS    10 //Number of LEDs per strip.
#define REFRESH_INTERVAL 10 //number of ms between frames



//Class Definition

class rgb_drv{
  private:
    
    uint16_t hue;

    Adafruit_NeoPixel strip;

  uint32_t RGB_mode;      //Current RGB mode
  uint32_t oldmillis;


   uint32_t DATA_pin; //TODO: make private!

   void rainbowCycle(); //runs rainbow cycle
  

  public:
    rgb_drv(int RGB_pin, uint32_t setmode);

    void run(uint32_t systick); //Update variables, run things, etc.
    
   

};









 #endif
