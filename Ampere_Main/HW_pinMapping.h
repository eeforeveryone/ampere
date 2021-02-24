//HW_pinMapping.h
//This file configures the GPIO of the Ampere main controller
//This file determines the HW limits of the daughtercards (Cooling, Resultion of A/D converters)

#define MAIN_ADC_OVERSAMPLE 10


#ifdef HW_REV__AMP01_A //Dev Kit
  #define MAIN_LED_PIN 2 //pin for debug led
  #define NUM_LOADS 1 //number of electronic loads

  //====== AMP1 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC_CS 7 //CS for Voltage ADC is on pin 7
    #define MAIN_VADC_VREF_mV 2500 //reference voltage for Vadc
    #define MAIN_VADC_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC_CS 8 //CS for Current ADC is on pin 8
    #define MAIN_IADC_VREF_mV 1666 //reference voltage for Vadc
    #define MAIN_IADC_CUSTOM_SCALE 20.0 //custom scaliing factor to translate voltage to amps in
  
    #define MAIN_ADC_BITS 10 //max counts for the ADCs on this hardware
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC_CS 9 //CS for DAC is on pin 9
    #define MAIN_DAC_VREF_mV 3300 //reference voltage for DAC
    #define MAIN_DAC_CUSTOM_SCALE 0.15 //scaling factor to translate Vout into Iout //255 = 11,000mA ... 3300/11,000 = 0.3 //....setCustom is now in mA ... #TODO: figure out why I needed to set 0.15 instead of 0.3
    
    #define MAIN_DAC_BITS 8
    //====== DAC SETUP =======================//
  //====== AMP1 SETUP =======================//
  
  //====== UI SETUP ========================//
  #define MAIN_OPT1_PIN 14 //digital pin definition
  #define MAIN_OPT2_PIN 15 //digital pin definition
  #define MAIN_OPT3_PIN 16 //digital pin definition

  #define MAIN_USBDET_N 6 //low = USB Control Plugged in... high = unplugged
  
  #define MAIN_DOWN_PIN 3 //digital pin definition
  #define MAIN_ENTER_PIN 4 //digital pin definition
  #define MAIN_UP_PIN 5 //digital pin definition
  //====== UI SETUP ========================//
  //====== ELECT. LIMITS====================//
  #define MAIN_IMAX 10000   //10A
  #define MAIN_VMAX 100000  //100V
  #define MAIN_PMAX 53000   //53,000 mW (53W) --hits 64C on heatsink near part @ Steady-State
  //====== ELECT. LIMITS====================//

#endif


#ifdef HW_REV__AMP02_A //4-up Motherboard
  #define MAIN_LED_PIN 2 //pin for debug led
  #define NUM_LOADS 4 //number of electronic loads


  //====== AMP1 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC1_CS           28 //CS for Voltage ADC is on pin x //TODO: SCHEMATIC SHOWS 28!
    #define MAIN_VADC1_VREF_mV      2500 //reference voltage for Vadc
    #define MAIN_VADC1_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC1_CS           30 //CS for Current ADC is on pin x //TODO: SCHEMATIC SHOWS 30!
    #define MAIN_IADC1_VREF_mV      3125 //reference voltage for Iadc
    #define MAIN_IADC1_CUSTOM_SCALE 3.333 //custom scaliing factor to translate voltage to amps in
  
    #define MAIN_ADC1_BITS          10 //max counts for the ADCs on this hardware
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC1_CS            29 //CS for DAC is on pin x
    #define MAIN_DAC1_VREF_mV       3300 //reference voltage for DAC
    #define MAIN_DAC1_CUSTOM_SCALE  0.3 //scaling factor to translate Vout into Iout //1A = 0.3V
    
    #define MAIN_DAC1_BITS          12
    #define MAIN_AMP1_SPIID         0 //use SPI
    //====== DAC SETUP =======================//
  //====== AMP1 SETUP =======================//
  //====== AMP2 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC2_CS           31 //CS for Voltage ADC is on pin x //TODO: SCHEMATIC SHOWS 31!
    #define MAIN_VADC2_VREF_mV      2500 //reference voltage for Vadc
    #define MAIN_VADC2_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC2_CS           33 //CS for Current ADC is on pin x //TODO: SCHEMATIC SHOWS 33!
    #define MAIN_IADC2_VREF_mV      3125 //reference voltage for Iadc
    #define MAIN_IADC2_CUSTOM_SCALE 3.33333 //custom scaliing factor to translate voltage to amps in
  
    #define MAIN_ADC2_BITS          10 //max counts for the ADCs on this hardware
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC2_CS            32 //CS for DAC is on pin x 
    #define MAIN_DAC2_VREF_mV       3300 //reference voltage for DAC
    #define MAIN_DAC2_CUSTOM_SCALE  0.3 //scaling factor to translate Vout into Iout //1A = 0.3V
    
    #define MAIN_DAC2_BITS          12
    #define MAIN_AMP2_SPIID         1 //use SPI1
    //====== DAC SETUP =======================//
  //====== AMP2 SETUP =======================//
  //====== AMP3 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC3_CS           36 //CS for Voltage ADC is on pin x //TODO: SCHEMATIC SHOWS 36!
    #define MAIN_VADC3_VREF_mV      2500 //reference voltage for Vadc
    #define MAIN_VADC3_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC3_CS           34 //CS for Current ADC is on pin x //TODO: SCHEMATIC SHOWS 34!
    #define MAIN_IADC3_VREF_mV      3125 //reference voltage for Iadc
    #define MAIN_IADC3_CUSTOM_SCALE 3.33333 //custom scaliing factor to translate voltage to amps in
  
    #define MAIN_ADC3_BITS          10 //max counts for the ADCs on this hardware
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC3_CS            35 //CS for DAC is on pin x
    #define MAIN_DAC3_VREF_mV       3300 //reference voltage for DAC
    #define MAIN_DAC3_CUSTOM_SCALE  0.3 //scaling factor to translate Vout into Iout //1A = 0.3V
    
    #define MAIN_DAC3_BITS          12
    #define MAIN_AMP3_SPIID         0 //use SPI
    //====== DAC SETUP =======================//
  //====== AMP3 SETUP =======================//
  //====== AMP4 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC4_CS           41 //CS for Voltage ADC is on pin x //TODO: SCHEMATIC SHOWS 41!
    #define MAIN_VADC4_VREF_mV      2500 //reference voltage for Vadc
    #define MAIN_VADC4_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC4_CS           37 //CS for Current ADC is on pin x //TODO: SCHEMATIC SHOWS 37!
    #define MAIN_IADC4_VREF_mV      3125 //reference voltage for Iadc
    #define MAIN_IADC4_CUSTOM_SCALE 3.33333 //custom scaling factor to translate voltage to amps in
  
    #define MAIN_ADC4_BITS          10 //max counts for the ADCs on this hardware
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC4_CS            38 //CS for DAC is on pin x
    #define MAIN_DAC4_VREF_mV       3300 //reference voltage for DAC
    #define MAIN_DAC4_CUSTOM_SCALE  0.3 //scaling factor to translate Vout into Iout //1A = 0.3V
    
    #define MAIN_DAC4_BITS          12
    #define MAIN_AMP4_SPIID         1 //use SPI1
    //====== DAC SETUP =======================//
  //====== AMP4 SETUP =======================//

  //====== UI SETUP ========================//
  #define MAIN_OPT1_PIN 6 //digital pin definition
  #define MAIN_OPT2_PIN 7 //digital pin definition
  #define MAIN_OPT3_PIN 8 //digital pin definition

  #define MAIN_USBDET_N 9 //low = USB Control Plugged in... high = unplugged

  #define NUM_STRIPS 2 //number of RGB strips //used by fastled
  #define MAIN_RGB_D0 24
  #define MAIN_RGB_D1 25
  
  #define MAIN_DOWN_PIN 3 //digital pin definition
  #define MAIN_ENTER_PIN 4 //digital pin definition
  #define MAIN_UP_PIN 5 //digital pin definition
  //====== UI SETUP ========================//
  //====== ELECT. LIMITS====================//
  #define MAIN_IMAX 10000   //10,000 mA (10A)
  #define MAIN_VMAX 100000  //100,000 mV (100V)
  #define MAIN_PMAX 275000 //275,000 mW (275W)
  //====== ELECT. LIMITS====================//
#endif

#ifdef HW_REV__AMP03_A //Community Edition
  #define MAIN_LED_PIN 2 //pin for debug led
  #define NUM_LOADS 1 //number of electronic loads
  
  //====== AMP1 SETUP =======================//
    //====== ADC SETUP =======================//
    #define MAIN_VADC1_CS 21 //ALG pin for Voltage ADC is pin 21
    #define MAIN_VADC1_VREF_mV 3300 //reference voltage for Vadc
    #define MAIN_VADC1_CUSTOM_SCALE 41.0 //custom scaliing factor to translate voltage to volts in
    
    #define MAIN_IADC1_CS 20 // ALG pin for Current ADC is pin 20 
    #define MAIN_IADC1_VREF_mV 3300 //reference voltage for Iadc
    #define MAIN_IADC1_CUSTOM_SCALE 4.545454 //custom scaliing factor to translate voltage to amps in
  
    #define MAIN_ADC1_BITS 10 //max counts for the ADCs on this hardware

    #define MAIN_ADC1_SPIID -1 //-1 = analog input
    //====== ADC SETUP =======================//
    //====== DAC SETUP =======================//
    #define MAIN_DAC1_CS 10 //CS for DAC is on pin 10
    #define MAIN_DAC1_VREF_mV 3300 //reference voltage for DAC
    #define MAIN_DAC1_CUSTOM_SCALE 0.22059 //scaling factor to translate Vout into Iout //255 = 11,000mA ... 3300/11,000 = 0.3 //....setCustom is now in mA
    
    #define MAIN_DAC1_BITS 12
    //====== DAC SETUP =======================//
  //====== AMP1 SETUP =======================//
  
  //====== UI SETUP ========================//
  #define MAIN_OPT1_PIN 9 //digital pin definition
  #define MAIN_OPT2_PIN 1 //digital pin definition
  #define MAIN_OPT3_PIN 0 //digital pin definition

  #define MAIN_USBDET_N 14 //low = USB Control Plugged in... high = unplugged
  
  #define MAIN_DOWN_PIN 3 //digital pin definition
  #define MAIN_ENTER_PIN 5 //digital pin definition
  #define MAIN_UP_PIN 8 //digital pin definition

  #define MAIN_ENC_SW_PIN 4 //digital pin
  #define MAIN_ENC_A_PIN 7 //digital pin
  #define MAIN_ENC_B_PIN 6 //digital pin
  
  #define MAIN_AMP1_SPIID         0 //use SPI
  //====== UI SETUP ========================//
  //====== ELECT. LIMITS====================//
  #define MAIN_IMAX 10000   //10,000 mA (10A)
  #define MAIN_VMAX 60000   //60,000 mV (60V)
  #define MAIN_PMAX 300000 //300,000 mW (300W)
  //====== ELECT. LIMITS====================//
#endif
