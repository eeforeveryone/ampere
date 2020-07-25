// Ampere Main Project
// Electronic Load Firmware v1.01
// EEforEveryone - 2020
//This software performs the functions defined in the release notes below:

//////////////////////////////////////////////////////////////////////////
// RELEASE NOTES:                                                       //
//////////////////////////////////////////////////////////////////////////
// V1.00 - Initial Release:                                             //
// Testing Working:                                                     //
//  Current Regulation (Programmable)                                   //
//  Change Settings via Button Interface                                //
//  Coloumb Counting                                                    //
//  Resistance Measurement                                              //
//  Voltage Measurement                                                 //
//                                                                      //
//                                                                      //
// V1.01 - Regulation Update:                                           //
// Tested Working:                                                      //
//  Power Regulation                                                    //
//  Ohm Regulation                                                      //
//  Added Splash Screen to show Version on Boot                         //
//  Control via. Serial Protocol                                        //
//                                                                      //
//                                                                      //
//                                                                      //
//                                                                      //
// Planned for a Future Release:                                        //
//  Slew Rate Control                                                   //
//  Support for Community Edition Ampere Hardware                       //
//  Detection of Plugged status for daughtercards!                      //
//  Detection of Instability, and Faults (Measured != Setpoint)         //
//                                                                      //
// Known Issues:                                                        //
//                                                                      //
//                                                                      //
//                                                                      //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//WIP TODO:
//  Debug & Prototype Future Release Functions!
// Implement MAIN_USBDET_N COM Release!

#define MAIN_VERSION_STRING "Ampere V1.01_Develop"


///////////////////////////////////////
// INCLUDES......................... //
///////////////////////////////////////
#include "screen.h"
#include <Wire.h>
#include <SPI.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include "amp01.h" //Ampere Front-End Object
#include "ampgui.h" //high level GUI object
#include "dbgled.h" //debug led object

#include "rgb_drv.h"

#include "ampserial.h" //Serial Control object

//#define HW_REV__AMP01_A   //This is the dev kit
#define HW_REV__AMP02_A     //This is the 4-up Controller
//#define HW_REV__AMP03_A   //This is the Community Edition Controller!
#include "HW_pinMapping.h" //Configure Pins

//#define PRINT_DEBUG true //set true when debug com port active


///////////////////////////////////////
// OOP OBJECT DEFINITION............ //
///////////////////////////////////////
struct screen* myscreen; //object for GUI screen
SSD1306AsciiWire oled; //object for oled comms
IntervalTimer ctrltimer; //object for control timer

struct ampgui* mygui; //object for GUI controller
struct ampserial* pc; //object for serial control object
struct dbgled* led; //object for debug led


struct amp01* myLoad[NUM_LOADS]; //object for front ends

struct adcext* vadc[NUM_LOADS]; //objects for front end child objects (testing memory weirdness
struct adcext* iadc[NUM_LOADS];
struct dacext* dac[NUM_LOADS];




///////////////////////////////////////
// GLOBAL VARIABLE DEFINITION....... //
///////////////////////////////////////
uint32_t millisecond = 0; //This varible represents ms



//**************************//
//CONTROL LOOP ISR BEGIN----//
void CONTROLLOOP_INTERRUPT_HANDLER(){
  noInterrupts(); //disable interrupts

      for (int i = 0; i < NUM_LOADS; i++){
        amp01__ISR_routine(myLoad[i]); //run all daughtercard ISR
      }
      

    millisecond = millisecond + 1;
    
  interrupts(); //enable interrupts
}

//CONTROL LOOP ISR END------//
//**************************//

//******************************//
//MAIN LOOP UPDATE GUI START----//
void MAIN_oled_update(struct screen* myscreen){
  //#TODO: FIX I2C Layer of function
   if(screen__get_NEED_NUKE(myscreen)){
    oled.clear();
   }
   oled.setCursor(0,0);
   oled.println(screen__getline1(myscreen));
   oled.println(screen__getline2(myscreen));
   oled.println(screen__getline3(myscreen));
   oled.println(screen__getline4(myscreen));
   oled.println(screen__getline5(myscreen));
   oled.println(screen__getline6(myscreen));
   oled.println(screen__getline7(myscreen));
   oled.println(screen__getline8(myscreen));

}



//******************************//
//MAIN LOOP UPDATE GUI END------//

void setup() {
  ///////////////////////////////////////
  // START HAL LAYERS & OBJECTS....... //
  ///////////////////////////////////////

  Wire.begin();
  Wire.setClock(800000L);

  myscreen = screen__create();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.println(MAIN_VERSION_STRING);
  oled.println("------------------------");
  delay(200);
  oled.println("Electronic Load\n");
  oled.println("EEforEveryone\n");
  delay(200);
  oled.print("Boot.");
  delay(50); oled.print('.');
  delay(50); oled.print('.');
  delay(50); oled.print('.');
  delay(50); oled.print('.');
  delay(50); oled.print('.');
  delay(200);

  mygui = ampgui__create(myscreen, NUM_LOADS, MAIN_UP_PIN, MAIN_DOWN_PIN, MAIN_ENTER_PIN, MAIN_OPT1_PIN, MAIN_OPT2_PIN, MAIN_OPT3_PIN); //constructor

    Serial.begin(115200);
    delay(50); oled.print('.');



  #ifdef HW_REV__AMP01_A
//    SPI.setCS(10);
//    SPI.setSCK(13);
//    SPI.setMISO(12);
//    SPI.setMOSI(11);
    SPI.begin(); //Start up SPI for adc and dac!
    
    vadc[0] = adcext__create(MAIN_VADC_CS, MAIN_ADC_OVERSAMPLE, MAIN_VADC_VREF_mV, 1024, 0); //constructor
    adcext__setCustom(vadc[0], MAIN_VADC_CUSTOM_SCALE); //set custom scaling factor to 41.0 (vin - milliVolts)
    
    iadc[0] = adcext__create(MAIN_IADC_CS, MAIN_ADC_OVERSAMPLE, MAIN_IADC_VREF_mV, 1024, 0); //constructor
    adcext__setCustom(iadc[0], MAIN_IADC_CUSTOM_SCALE); //set custom scaling factor to translate mv into milliamps)
    
    dac[0] = dacext__create(MAIN_DAC_CS, MAIN_DAC_VREF_mV, MAIN_DAC_BITS, 0); //(CS, Vref, number_bits, SPI_HW id);
    dacext__setCustomFactor(dac[0], MAIN_DAC_CUSTOM_SCALE); 
    myLoad[0] = amp01__create(vadc[0], iadc[0], dac[0], MAIN_IMAX, MAIN_VMAX, MAIN_PMAX); //constructor

    delay(50); oled.print('.');

  #endif
  #ifdef HW_REV__AMP02_A
    //SPI.setCS(-1);
    //SPI.setSCK(13);
    //SPI.setMISO(12);
    //SPI.setMOSI(11);
    SPI.begin(); //Start up SPI for adc and dac!
    
    //SPI1.setCS(-1); //32 reserved... 38 is default
    SPI1.setMOSI(26);
    SPI1.setSCK(27);
    SPI1.setMISO(39);
    
    SPI1.begin(); //Start up SPI1 for adc and dac!

      #ifdef PRINT_DEBUG
        Serial.println("Start Load 1!");
      #endif
    
      vadc[0] = adcext__create(MAIN_VADC1_CS, MAIN_ADC_OVERSAMPLE, MAIN_VADC1_VREF_mV, 1024, MAIN_AMP1_SPIID); //constructor
      adcext__setCustom(vadc[0], MAIN_VADC1_CUSTOM_SCALE); //set custom scaling factor to 41.0 (vin - milliVolts)
      iadc[0] = adcext__create(MAIN_IADC1_CS, MAIN_ADC_OVERSAMPLE, MAIN_IADC1_VREF_mV, 1024, MAIN_AMP1_SPIID); //constructor
      adcext__setCustom(iadc[0], MAIN_IADC1_CUSTOM_SCALE); //set custom scaling factor to translate mv into milliamps)
      dac[0] = dacext__create(MAIN_DAC1_CS, MAIN_DAC1_VREF_mV, MAIN_DAC1_BITS, MAIN_AMP1_SPIID); //(CS, Vref, number_bits, SPI_HW id);
      dacext__setCustomFactor(dac[0], MAIN_DAC1_CUSTOM_SCALE); 
      myLoad[0] = amp01__create(vadc[0], iadc[0], dac[0], MAIN_IMAX, MAIN_VMAX, MAIN_PMAX); //constructor

      
      #ifdef PRINT_DEBUG
        Serial.println("Start Load 2!");
      #endif

      vadc[1] = adcext__create(MAIN_VADC2_CS, MAIN_ADC_OVERSAMPLE, MAIN_VADC2_VREF_mV, 1024, MAIN_AMP2_SPIID); //constructor
      adcext__setCustom(vadc[1], MAIN_VADC2_CUSTOM_SCALE); //set custom scaling factor to 41.0 (vin - milliVolts)
      iadc[1] = adcext__create(MAIN_IADC2_CS, MAIN_ADC_OVERSAMPLE, MAIN_IADC2_VREF_mV, 1024, MAIN_AMP2_SPIID); //constructor
      adcext__setCustom(iadc[1], MAIN_IADC2_CUSTOM_SCALE); //set custom scaling factor to translate mv into milliamps)
      dac[1] = dacext__create(MAIN_DAC2_CS, MAIN_DAC2_VREF_mV, MAIN_DAC2_BITS, MAIN_AMP2_SPIID); //(CS, Vref, number_bits, SPI_HW id);
      dacext__setCustomFactor(dac[1], MAIN_DAC2_CUSTOM_SCALE); 
      myLoad[1] = amp01__create(vadc[1], iadc[1], dac[1], MAIN_IMAX, MAIN_VMAX, MAIN_PMAX); //constructor

      #ifdef PRINT_DEBUG
        Serial.println("Start Load 3!");
      #endif

      vadc[2] = adcext__create(MAIN_VADC3_CS, MAIN_ADC_OVERSAMPLE, MAIN_VADC3_VREF_mV, 1024, MAIN_AMP3_SPIID); //constructor
      adcext__setCustom(vadc[2], MAIN_VADC3_CUSTOM_SCALE); //set custom scaling factor to 41.0 (vin - milliVolts)
      iadc[2] = adcext__create(MAIN_IADC3_CS, MAIN_ADC_OVERSAMPLE, MAIN_IADC3_VREF_mV, 1024, MAIN_AMP3_SPIID); //constructor
      adcext__setCustom(iadc[2], MAIN_IADC3_CUSTOM_SCALE); //set custom scaling factor to translate mv into milliamps)
      dac[2] = dacext__create(MAIN_DAC3_CS, MAIN_DAC3_VREF_mV, MAIN_DAC3_BITS, MAIN_AMP3_SPIID); //(CS, Vref, number_bits, SPI_HW id);
      dacext__setCustomFactor(dac[2], MAIN_DAC3_CUSTOM_SCALE); 
      myLoad[2] = amp01__create(vadc[2], iadc[2], dac[2], MAIN_IMAX, MAIN_VMAX, MAIN_PMAX); //constructor

      #ifdef PRINT_DEBUG
        Serial.println("Start Load 4!");
      #endif

      vadc[3] = adcext__create(MAIN_VADC4_CS, MAIN_ADC_OVERSAMPLE, MAIN_VADC4_VREF_mV, 1024, MAIN_AMP4_SPIID); //constructor
      adcext__setCustom(vadc[3], MAIN_VADC4_CUSTOM_SCALE); //set custom scaling factor to 41.0 (vin - milliVolts)
      iadc[3] = adcext__create(MAIN_IADC4_CS, MAIN_ADC_OVERSAMPLE, MAIN_IADC4_VREF_mV, 1024, MAIN_AMP4_SPIID); //constructor
      adcext__setCustom(iadc[3], MAIN_IADC4_CUSTOM_SCALE); //set custom scaling factor to translate mv into milliamps)
      dac[3] = dacext__create(MAIN_DAC4_CS, MAIN_DAC4_VREF_mV, MAIN_DAC4_BITS, MAIN_AMP4_SPIID); //(CS, Vref, number_bits, SPI_HW id);
      dacext__setCustomFactor(dac[3], MAIN_DAC4_CUSTOM_SCALE); 
      myLoad[3] = amp01__create(vadc[3], iadc[3], dac[3], MAIN_IMAX, MAIN_VMAX, MAIN_PMAX); //constructor
      
      

      
      delay(50); oled.print('.');
  #endif
    

  pc = ampserial__create(NUM_LOADS); //Create serial control object!
  led = dbgled__create(MAIN_LED_PIN); //create debug led object and link to hw pin!
  delay(50); oled.print('.');

  

  ctrltimer.begin(CONTROLLOOP_INTERRUPT_HANDLER, 1000);  // run control loop every millisecond
}

void loop() {
  // define main loop variables here

  char opt [OPT_LENGTH+1] = "1234";
  char title[] = "Ampere: Main Screen";
  int title_length = 18;

  uint32_t set_amp_milli[NUM_LOADS];
  uint32_t set_volt_milli[NUM_LOADS];
  uint32_t set_ohm_milli[NUM_LOADS];
  uint32_t set_watt_milli[NUM_LOADS];

  for (uint8_t i = 0; i<NUM_LOADS; i++){ //set all setpoints to zero!
    set_amp_milli[i] = 0;
    set_volt_milli[i] = 0;
    set_ohm_milli[i] = 0;
    set_watt_milli[i] = 0;
  }

  pinMode(MAIN_USBDET_N, INPUT);

  int oldseconds = 0;


  #ifdef HW_REV__AMP02_A //start RGB objects
    pinMode(MAIN_RGB_D0, OUTPUT); //just in case
    pinMode(MAIN_RGB_D1, OUTPUT); //just in case
  
    rgb_drv rgb0(MAIN_RGB_D0,0); //RGB LED objects;
    rgb_drv rgb1(MAIN_RGB_D1,0); //RGB LED objects;

  #endif

  while(1){ //Infinate loop

    #ifdef HW_REV__AMP02_A //run RGB objects
      rgb0.run(millisecond);
      rgb1.run(millisecond);
    #endif

      
      
      
    for(int i = 0; i< NUM_LOADS; i++){
        #ifdef PRINT_DEBUG
          Serial.print("Sample Load ");
          Serial.println(i);
        #endif
        amp01__Sample(myLoad[i]); //sample all the ADCs!
    }


    if(ampserial__InControl(pc) && digitalRead(MAIN_USBDET_N) == 0){ //if the PC is in control & the cable wasn't ripped out ungracefully
      ampgui__update(mygui, millis(),true); //change button control method to override mode (don't allow setting via buttons)

      int thisCH_sel = ampserial__getChannelSelection(pc) -1; 
      struct amp01* thisload = myLoad[thisCH_sel]; //temp pointer for front-ends
      
      if(ampserial__getRun(pc)){ //Time to run the controlloop!
          amp01__EnableOutput(thisload); //turn on the output!
    
          switch(ampserial__getMode(pc)){ //what are we regulating
            case AMPGUI_SEL_I: //regulate Amps
              if(amp01__SetCurrent(thisload, ampserial__getSetpoint(pc))){//set the output current, in mA.
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
              
            break;
            case AMPGUI_SEL_R:
              if(amp01__SetResistance(thisload, ampserial__getSetpoint(pc))){
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
              
            break;
            case AMPGUI_SEL_P:
              if(amp01__SetPower(thisload, ampserial__getSetpoint(pc))){ //if we successfully set the value
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
            
            break;
            default:
            //ERROR! Turn off the output
              amp01__DisableOutput(thisload); //turns off output
              dbgled__error(led); //change debug led mode to error
            break;
          }
          
      }else{
        amp01__DisableOutput(thisload); //turns off output
        
        dbgled__idle(led); //change debug led mode to idle
      }
      
      ampgui__set(mygui, amp01__GetVout(thisload)/1000.0, amp01__GetIout(thisload)/1000.0, amp01__GetRout(thisload)/1000.0, amp01__GetPout(thisload)/1000.0, amp01__GetEout(thisload)/1000000.0); //Display Live Readouts
      
    }else{ //the GUI is in control
      ampgui__update(mygui, millis(),false); //Update GUI buttons!
      
      int thisCH_sel = ampgui__getChannelSelection(mygui) -1; 
      struct amp01* thisload = myLoad[thisCH_sel]; //temp pointer for front-ends
      
  
      if(ampgui__getRunLoop(mygui)){ //Time to run the controllop!
        amp01__EnableOutput(thisload); //turn on the output!
  
        switch(ampgui__getMode(mygui)){ //what are we regulating #todo: input validation
          case AMPGUI_SEL_I: //regulate Amps
             if(amp01__SetCurrent(thisload, set_amp_milli[thisCH_sel])){//set the output current, in mA.
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
              
          break;
          case AMPGUI_SEL_R:
            if(amp01__SetResistance(thisload, set_ohm_milli[thisCH_sel])){//set the output current, in mA.
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
            
          break;
          case AMPGUI_SEL_P:
            if(amp01__SetPower(thisload, set_watt_milli[thisCH_sel])){//set the output current, in mA.
                dbgled__run(led); //change debug led mode to run
              }else{ //error or disabled
                dbgled__error(led); //change debug led mode to error
              }
            
          break;
          default:
          //ERROR! Turn off the output
              amp01__DisableOutput(thisload); //turns off output
              dbgled__error(led); //change debug led mode to error
          break;
        }
  
        ampgui__set(mygui, amp01__GetVout(thisload)/1000.0, amp01__GetIout(thisload)/1000.0, amp01__GetRout(thisload)/1000.0, amp01__GetPout(thisload)/1000.0, amp01__GetEout(thisload)/1000000.0); //Display Live Readouts
       
          
      }else{ //allow change settings
        
        amp01__DisableOutput(thisload); //turns off output
  
        switch(ampgui__getMode(mygui)){              //Change Regulation Setpoints
        case AMPGUI_SEL_V: //regulate Volts
          set_volt_milli[thisCH_sel] = ampgui__getSetValue(mygui);
        break;
        case AMPGUI_SEL_I: //regulate Amps
          set_amp_milli[thisCH_sel] = ampgui__getSetValue(mygui);
        break;
        case AMPGUI_SEL_R:
          set_ohm_milli[thisCH_sel] = ampgui__getSetValue(mygui);
        break;
        case AMPGUI_SEL_P:
          set_watt_milli[thisCH_sel] = ampgui__getSetValue(mygui);
        break;
        default:
        //ERROR! #todo: something
        dbgled__error(led); //change debug led mode to error
  
        break;
      }
        dbgled__idle(led); //change debug led mode to idle
        ampgui__set(mygui, set_volt_milli[thisCH_sel]/1000.0, set_amp_milli[thisCH_sel]/1000.0, set_ohm_milli[thisCH_sel]/1000.0, set_watt_milli[thisCH_sel]/1000.0, amp01__GetEout(thisload)/1000000.0); //Display Current Settings
      }
  
    }
    
    dbgled__refresh(led, millisecond); //make blinking happen
    ampserial__handleRX(pc); //Read incoming serial data.


    for(int i = 0; i< NUM_LOADS; i++){
      ampserial__sendReport(pc, millisecond, i+1, amp01__GetVout(myLoad[i]), amp01__GetIout(myLoad[i]), amp01__GetEout(myLoad[i])); //Always Send UART report if we need to
    }

    MAIN_oled_update(myscreen); //Always refresh display

  }
}
