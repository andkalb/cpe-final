// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//

#include <LiquidCrystal.h>
#include <Stepper.h>
#include "DHT.h"
#include <Wire.h>
#include "RTClib.h" // the adafruit library

//
// ADDRESSES/PORT SETUP
//
volatile unsigned char* port_a = (unsigned char*) 0x22; 
volatile unsigned char* ddr_a  = (unsigned char*) 0x21; 
volatile unsigned char* pin_a  = (unsigned char*) 0x20;

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

volatile unsigned char* my_PCMSK1 = (unsigned char*) 0x6C; 
volatile unsigned char* my_PCICR  = (unsigned char*) 0x68; 

 volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
 volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
 volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
 volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
 volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
#define RDA 0x80 // FOR USART stuff
#define TBE 0x20 // USART STUFF

#define GREEN 0
#define BLUE 1
#define YELLOW 2
#define RED 3
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
bool ventClockwise = false;

DHT dht(11, DHT11); // Temp Sensor
unsigned int waterLevel; // variable for water sensor

RTC_DS1307 rtc;
DateTime now;
DateTime now;

enum State
{
    idle,
    error,
    disabled,
    running
};

State state = disabled;





bool fanOn = false;
bool startButtonReleased = false;

unsigned int waterThreshold = 250;       // DETERMINE THESE CONSTANTS
unsigned int temperatureThreshold = 25;  // " " " " " " " " " " " " Celsius

int previousMin = 0;
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
        Print("Couldn't find RTC");
        Print("Couldn't find RTC");
    }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    previousMin = rtc.now().minute();

    Write(ddr_j, 1, 0); // PJ1 is start button                 INPUT
    Write(port_j, 1, 1); // PJ1 needs pullup resistor enabled
    Write(my_PCMSK1, 2, 1); // PCMSK1 14 set
    Write(my_PCICR, 1, 1); // PCIE1 set

    Write(ddr_j, 0, 0); //PJ0 is the RESET button
    Write(port_j, 0, 1); //PJ0 needs pullup resistor enabled
 
    Write(ddr_b, 7, 0); // PB7 is one vent control button.     INPUT
    Write(ddr_b, 6, 0); // PB6 is another vent control button. INPUT
    Write(ddr_f, 1, 0); // PF1 is the water sensor signal.     INPUT
    Write(ddr_b, 5, 0); // PB5 is the humidity sensor signal.  INPUT
    
    Write(ddr_a, 0, 1); //PA0 is the GREEN LED  OUTPUT
    Write(ddr_a, 1, 1); //PA1 is the BLUE LED   OUTPUT
    Write(ddr_a, 2, 1); //PA2 is the YELLOW LED OUTPUT
    Write(ddr_a, 3, 1); //PA3 is the RED LED    OUTPUT

    Write(ddr_f, 7, 1); // PF7 is Analog 5, the fan connection OUTPUT
   
    Write(ddr_a, YELLOW, 1); // WRITE YELLOW LED ON HERE, since we start in Disabled state

    //stepper.setSpeed(5); // Initialize step motor TODO: may need to uncomment
    lcd.begin(16, 2);    // Initialize LCD

    ADCInit();           // Initialize ADC
    SetupTimer();
    *myTCCR1B |= 0x01; // start 1 minute timer
     Print("Disabled state entered");
    
}
//
//
//


//
// LOOP CODE
//
void loop() 
{
    now = rtc.now();
    now = rtc.now();
    waterLevel = ADCRead(1); // ADC signal wire is in A1
    temp = dht.readTemperature();
    humidity = dht.readHumidity();
 
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
    CheckAndOutputLCD();
    if(dht.temperature > temperatureThreshold)
    {
        Print("Running state entered");
        state = running;
        SetFanOn(true); // start fan motor
        Write(port_a, BLUE, 1);  // set blue LED on
        Write(port_a, GREEN, 0); // turn green LED off
    }

    if(waterLevel <= waterThreshold)
    {
        Print("Error state entered");
        state = error;
        Write(port_a, RED, 1);
        Write(port_a, GREEN, 0);

        // error message on LCD
        lcd.setCursor(0, 0);
        lcd.print("ERROR");
        lcd.setCursor(0, 1);
        lcd.print("Water lvl low");

        SetFanOn(false); // motor off
    }
}

void RunningProcess()
{
    HandleVentButtons();
    CheckAndOutputLCD();
    if(waterLevel <= waterThreshold)
    {
        Print("Error state entered");
        state = error;
        Write(port_a, BLUE, 0);
        Write(port_a, RED, 1);
        
        // error message on LCD
        lcd.setCursor(0, 0);
        lcd.print("ERROR");
        lcd.setCursor(0, 1);
        lcd.print("Water lvl low");
        
        SetFanOn(false); // motor off
    }
    else if (dht.readTemperature() <= temperatureThreshold)
    {
        Print("Idle state entered");
        state = idle;
        Write(port_a, BLUE, 0);
        Write(port_a, GREEN, 1);
        SetFanOn(false);
    }

}

void ErrorProcess()
{
    HandleVentButtons();
    CheckAndOutputLCD();
    if(Read(port_j, 0)) // reset button pressed
    {
        if(waterLevel > waterThreshold)
        {
            Print("Idle state entered");
            state = idle;
            Write(port_a, RED, 0);
            Write(port_a, GREEN, 1);
        }
    }
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
            if(!ventClockwise)
            {
                Print("Vent Adjusted Clockwise");
            }
            ventClockwise = true;
        }
        else if (pb6)
        {
            // adjust vent the other way (stepper motor)
            stepper.step(-1);
            if(ventClockwise)
            {
                Print("Vent Adjusted Counter-Clockwise");
            }
            ventClockwise = false;
        }
    }
}

void SetFanOn(bool on)
{
    // Break/Restore connection to fan
    Write(port_f, 7, on); // PF7 is analog 7
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
    String toPrint = String(now.year());
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar('/');
    toPrint = now.month();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar('/');
    toPrint = now.day();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(' ');
    toPrint = now.hour();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(':');
    toPrint = now.minute();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(':');
    toPrint = now.second();
    String toPrint = String(now.year());
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar('/');
    toPrint = now.month();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar('/');
    toPrint = now.day();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(' ');
    toPrint = now.hour();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(':');
    toPrint = now.minute();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
    }
    PutChar(':');
    toPrint = now.second();
    for(int i = 0; i < toPrint.length(); i++)
    {
      PutChar(toPrint[i]);
      PutChar(toPrint[i]);
    }
    PutChar(' ');

    s += "\n";    
    for(int i = 0; i < s.length(); i++)
    {
        PutChar(s[i]);
    }
}

void CheckAndOutputLCD()
{
    if((now.minute() - previousMin) > 0)
    {
        previousMin = now.minute();
        if(state != disabled && state != error)
        {
            // print humidity percent and temp val
            lcd.setCursor(0, 0);
            lcd.print("Humidity: ");
            lcd.print((float)dht.humidity, 2);
            lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print("Temperature: ");
            lcd.print((float)dht.temperature, 2);
            lcd.print(" C");
        }
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
            Print("Disabled state entered");
            state = disabled;
            SetFanOn(false); // turn off fan motor
            Write(port_a, YELLOW, 1);
            Write(port_a, GREEN, 0);
            Write(port_a, RED, 0);
            Write(port_a, BLUE, 0);
        }
        else     // start system
        {
            Print("Idle state entered");
            state = idle;
            Write(port_a, GREEN, 1);
            Write(port_a, YELLOW, 0);
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
