// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//
#include "helper_functions.ino" // TODO: MIGHT BE UNNECESSARY?
#include <LiquidCrystal.h>
#include <Stepper.h>
#include <dht11.h>

//
// ADDRESSES/PORT SETUP
//
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23;

volatile unsigned char* port_f = (unsigned char*) 0x31; 
volatile unsigned char* ddr_f  = (unsigned char*) 0x30; 
volatile unsigned char* pin_f  = (unsigned char*) 0x2F;

volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106;

volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
//
//
//


//
// GLOBAL VARIABLES
//
const int rs = A0, en = A2, d4 = A3, d5 = A4, d6 = A5, d7 = A6; // Where the LCD is plugged in
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int stepsPerRevolution = 2048;
Stepper stepper = Stepper(stepsPerRevolution, 2, 4, 3, 5); // numbers are digital ports

dht11 dht; // Temp Sensor

enum State
{
    idle,
    error,
    disabled,
    running
};

State state = disabled;

bool fanOn = false;

unsigned int waterThreshold = 100;       // DETERMINE THESE CONSTANTS
unsigned int errorWaterThreshold = 50;   // " " " " " " " " " " " "
unsigned int temperatureThreshold = 75;  // " " " " " " " " " " " "
//
//
//


//
// SETUP CODE
//
void setup() 
{
    Write(ddr_b, 7, 0); // PB7 is one vent control button.     INPUT
    Write(ddr_b, 6, 0); // PB6 is another vent control button. INPUT
    //Write(ddr_f, 6, 0); // PF6 is the water sensor signal.     INPUT
    //Write(ddr_k, 6, 0); // PK6 is the humidity sensor signal.  INPUT  TODO: unnecessary, use 0 channel adc
    
   
    // WRITE YELLOW LED ON HERE, since we start in Disabled state

    stepper.setSpeed(5); // Initialize step motor
    lcd.begin(16, 2);    // Initialize LCD

    ADCInit();           // Initialize ADC
}
//
//
//


//
// LOOP CODE
//
void loop() 
{
    unsigned int waterLevel = ADCRead(0); // ADC signal wire is in A0
    dht11.read(4); // TODO: Put dht signal DIGITAL port we are reading here
    if (state == idle)
    {
        IdleProcess(waterLevel);
    }
    else if (state == error)
    {
        ErrorProcess(waterLevel);
    }
    else if (state == disabled)
    {
        DisabledProcess();
    }
    else if (state == running)
    {
        RunningProcess(waterLevel);
    }
}
//
//
//


//
// STATE FUNCTIONS
//
void DisabledProcess()
{
    // read start button, switch to running process, USE ISR
}

void IdleProcess(unsigned int waterLevel)
{
    HandleVentButtons();
    if(dht11.temperature > temperatureThreshold)
    {

    }
}

void RunningProcess(unsigned int waterLevel)
{
    HandleVentButtons();
    if(waterLevel < errorWaterThreshold)
    {
        state = error;
        // turn off all LEDS
        // turn on red led
        // error message on LCD
        // motor off

    }

}

void ErrorProcess(unsigned int waterLevel)
{
    HandleVentButtons();
    //if(read()) read reset button
    //{
    if(waterLevel > errorWaterThreshold)
    {

    }
    // }
}
//
//
//


//
// ADDITIONAL FUNCTIONS
//
void HandleVentButtons()
{
    bool pb7 = Read(port_b, 7);
    bool pb6 = Read(port_b, 6);
    if(!(pb7 && pb6))
    {
        if(pb7)
        {

            // adjust vent 1 way (stepper motor)
            stepper.step(1);
        }
        else if (pb6)
        {
            // adjust vent the other way (stepper motor)
            stepper.step(1);
        }
    }
}

void CheckAndOutputLCD()
{

}

void SetFanOn(bool on)
{
    // Break/Restore connection to fan
    fanOn = on;
}

void ADCInit()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int ADCRead(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00100000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}

//
//
//