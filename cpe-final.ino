// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//


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
//
//
//


//
// SETUP CODE
//
void setup() 
{
    
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

}

void running_process()
{

}

void error_process()
{
    
}
//
//
//

