// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//
#include "helper_functions.ino" // TODO: MIGHT BE UNNECESSARY?

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
//
//
//


//
// GLOBAL VARIABLES
//
enum State
{
    idle,
    error,
    disabled,
    running
};

State state = idle;
bool fanOn = false;
//
//
//


//
// SETUP CODE
//
void setup() 
{
    write(ddr_b, 7, 0); // PB7 is one vent control button.     INPUT
    write(ddr_b, 6, 0); // PB6 is another vent control button. INPUT
    write(ddr_f, 6, 0); // PF6 is the water sensor signal.     INPUT
    write(ddr_k, 6, 0); // PK6 is the humidity sensor signal.  INPUT
    
}
//
//
//


//
// LOOP CODE
//
void loop() 
{
    if (state == idle)
    {
        idle_process();
    }
    else if (state == error)
    {
        error_process();
    }
    else if (state == disabled)
    {
        disabled_process();
    }
    else if (state == running)
    {
        running_process();
    }
}
//
//
//


//
// STATE FUNCTIONS
//
void disabled_process()
{
    
}

void idle_process()
{
    HandleVentButtons();
}

void running_process()
{
    HandleVentButtons();
}

void error_process()
{
    HandleVentButtons();
}
//
//
//


//
// Additional Functions
//
void HandleVentButtons()
{
    bool pb7 = read(port_b, 7);
    bool pb6 = read(port_b, 6);
    if(!(pb7 && pb6))
    {
        if(pb7)
        {
            // adjust vent 1 way (stepper motor)
        }
        else if (pb6)
        {
            // adjust vent the other way (stepper motor)
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
//
//
//