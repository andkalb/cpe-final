// cpe-final.ino
// CPE 301 Final Project - Fall 2022
// Written by Andrew Kalb and Preston Peterson
//


//
// ADDRESSES/PORT SETUP
//

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
