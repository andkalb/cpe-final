// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//
#include "helper_functions.ino" // TODO: MIGHT BE UNNECESSARY?
#include <LiquidCrystal.h>
#include <Stepper.h>
#include <dht11.h>
#include <Wire.h>
#include "RTClib.h" // the adafruit library

//
// ADDRESSES/PORT SETUP
//
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23;

volatile unsigned char* port_f = (unsigned char*) 0x31; 
volatile unsigned char* ddr_f  = (unsigned char*) 0x30; 
volatile unsigned char* pin_f  = (unsigned char*) 0x2F;

volatile unsigned char* port_j = (unsigned char*) 0x105; 
volatile unsigned char* ddr_j  = (unsigned char*) 0x104; 
volatile unsigned char* pin_j  = (unsigned char*) 0x103;

volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106;

volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;

volatile unsigned char* my_PCMSK1 = (unsigned char*) 0x6C; 
volatile unsigned char* my_PCICR  = (unsigned char*) 0x68; 

 volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
 volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
 volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
 volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
 volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
#define RDA 0x80 // FOR USART stuff
#define TBE 0x20 // USART STUFF
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
unsigned int waterLevel; // variable for water sensor

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

enum State
{
    idle,
    error,
    disabled,
    running
};

State state = disabled;

bool fanOn = false;
bool startButtonPressed = false;

unsigned int waterThreshold = 250;       // DETERMINE THESE CONSTANTS
unsigned int temperatureThreshold = 20;  // " " " " " " " " " " " " Celsius
//
//
//


//
// SETUP CODE
//
void setup() 
{
    while (! rtc.begin()) 
    {
        Serial.println("Couldn't find RTC");
    }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    Write(ddr_j, 1, 0); // PJ1 is start button                 INPUT
    Write(port_j, 1, 1); // PJ1 needs pullup resistor enabled
    Write(my_PCMSK1, 2, 1); // PCMSK1 14 set
    Write(my_PCICR, 1, 1); // PCIE1 set

    Write(ddr_b, 7, 0); // PB7 is one vent control button.     INPUT
    Write(ddr_b, 6, 0); // PB6 is another vent control button. INPUT
    //Write(ddr_f, 6, 0); // PF6 is the water sensor signal.     INPUT TODO: unnecessary, use adc channel num 0
    //Write(ddr_k, 6, 0); // PK6 is the humidity sensor signal.  INPUT
    
   
    // WRITE YELLOW LED ON HERE, since we start in Disabled state

    stepper.setSpeed(5); // Initialize step motor
    lcd.begin(16, 2);    // Initialize LCD

    ADCInit();           // Initialize ADC
    SetupTimer();
    *myTCCR1B |= 0x01; // start 1 minute timer
    
}
//
//
//


//
// LOOP CODE
//
void loop() 
{
    DateTime now = rtc.now();
    waterLevel = ADCRead(0); // ADC signal wire is in A0
    dht.read(4); // TODO: Put dht signal DIGITAL port we are reading here
    if (state == idle)
    {
        IdleProcess();
    }
    else if (state == error)
    {
        ErrorProcess();
    }
    else if (state == disabled)
    {
        DisabledProcess();
    }
    else if (state == running)
    {
        RunningProcess();
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
    // turn off yellow led when ISR hits
    // goto idle state when ISR hits, turn on green led
    // nothing to do in here, ISR handles everything
}

void IdleProcess()
{
    HandleVentButtons();
    if(dht.temperature > temperatureThreshold)
    {
        state = running;
        SetFanOn(true); // start fan motor
        // set blue LED on
        // turn green LED off
    }

    if(waterLevel <= waterThreshold)
    {
        state = error;
        // turn red LED on
        // turn green LED off
        // error message on LCd
        SetFanOn(false); // motor off
    }
}

void RunningProcess()
{
    HandleVentButtons();
    if(waterLevel <= waterThreshold)
    {
        state = error;
        // turn off blue led
        // turn on red led
        // error message on LCD
        SetFanOn(false); // motor off
    }
    else if (dht.temperature <= temperatureThreshold)
    {
        state = idle;
        // blue led off
        // green led on
        SetFanOn(false);
    }

}

void ErrorProcess()
{
    HandleVentButtons();
    //if(read()) read reset button
    //{
    if(waterLevel > waterThreshold)
    {
        state = idle;
        // red LED off
        // green LED on
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

void SetupTimer()
{
    // setup timer control registers
    *myTCCR1A= 0x00;
    *myTCCR1B= 0X00;
    *myTCCR1C= 0x00; 
  
    // 1024 prescalar
    *myTCCR1B= 0X00;
    
    // reset TOV flag
    *myTIFR1 |= 0x01;
  
    // enable TOV interrupt
    *myTIMSK1 |= 0x01;
}

void U0Init(int U0baud)
{
    unsigned long FCPU = 16000000;
    unsigned int tbaud;
    tbaud = (FCPU / 16 / U0baud - 1);
    // Same as (FCPU / (16 * U0baud)) - 1;
    *myUCSR0A = 0x20;
    *myUCSR0B = 0x18;
    *myUCSR0C = 0x06;
    *myUBRR0  = tbaud;
}

void PutChar(unsigned char U0pdata)
{
    while((*myUCSR0A & TBE)==0);
    *myUDR0 = U0pdata;
}

void Print(String s)
{
    for(int i = 0; i < s.length(); i++)
    {
        PutChar(s[i]);
    }
}
//
//
//


//
// INTERRUPTS
//
ISR (PCINT1_vect) // PCINT1 for PJ1
{
    if(*pin_j == 1)
    {
        //button released
        startButtonReleased = true;
        if(state != disabled) // act as stop button
        {
            state = disabled;
            SetFanOn(false); // turn off fan motor
            // turn yellow LED on
            // turn all other LEDs off
        }
        else
        {
            state = idle;
            // turn green LED on
            // turn yellow LED off
        }
    }

    if(*pin_j == 3)
    {
        if(startButtonReleased)
        {
            //button pressed
            startButtonReleased = false;
        }
    }
}

ISR(TIMER1_OVF_vect)
{
    *myTCCR1B &= 0xF8; // stop timer
    *myTCNT1 =  (unsigned int) (49911);   // load count (1 min hopefully)
    *myTCCR1B |=   0x01; // restart timer
    if(state != disabled && state != error)
    {
        // lcd print humidity percent and temp val
    }

}
//
//
//