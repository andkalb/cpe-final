// helper_functions.ino
// This file contains various
// helper functions defined for the
// convenience of the overall program.

void init(unsigned int* port, bool output_nInput)
{
    write(port, 8, output_nInput);
}

void write(unsigned int* port, unsigned int pinNumber, bool high_nLow)
{
    if(high_nLow)
    {
        *port |= (0x01 << pinNumber);
    }
    else
    {
        *port &= ~(0x01 << pinNumber);
    }
}

bool read(unsigned int* port, unsigned int pinNumber)
{
    
}