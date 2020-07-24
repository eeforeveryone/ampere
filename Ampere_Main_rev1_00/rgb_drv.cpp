
/*
 * rgb_drv.cpp
 *
 * This class handles enumeration, configuration, and communication with RGB strips using the WS2812B protocol.
 * ALL Numbers Reported in milli-units unless otherwise noted! (Integer carrying 3 decimal places)
 *
 *
 *  Created on:     Jan 16, 2020
 *  Last Modified:  Jan 16, 2020
 *      Author: EEforEveryone
 */

/*
class rgb_drv{
  private:
    int DATA_pin;

    uint8_t hue;

  uint32_t RGB_mode;      //Current RGB mode
  uint32_t oldmillis;

  public:
    rgb_drv(int RGB_pin, uint32_t setmode);

    void run(uint32_t systick); //Update variables, run things, etc.
   

};

*/


#include "rgb_drv.h"
#include <Arduino.h>

//FASTLED_USING_NAMESPACE

using namespace std;

rgb_drv::rgb_drv(int RGB_pin, uint32_t setmode){
  DATA_pin = RGB_pin;
  RGB_mode = setmode;
  hue = random(255); //pick a random starting color
  oldmillis = 0; 


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
strip = Adafruit_NeoPixel(RGB_DRV_NUM_LEDS, DATA_pin, NEO_GRB + NEO_KHZ800);

strip.begin();
strip.setBrightness(255); //full brightness
strip.show(); // Initialize all pixels to 'off'

}



void rgb_drv::run(uint32_t systick){ //Update variables, run things, etc.

  if((systick > (oldmillis + REFRESH_INTERVAL)) || (systick < oldmillis) ){ //if it's time to refresh
     oldmillis = systick; //reset the counter

    
    rainbowCycle();
    // send the 'leds' array out to the actual LED strip
    strip.show();  
    
     
  }
  
}

void rgb_drv::rainbowCycle() {
  uint16_t i, j;

  hue = hue + 128;

  for(i=0; i< RGB_DRV_NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue+(i*(16384/RGB_DRV_NUM_LEDS)),255,255));
  }
  strip.show();
  
}
