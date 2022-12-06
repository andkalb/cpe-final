// helper_functions.ino
// This file contains various
// helper functions defined for the
// convenience of the overall program.


void write(unsigned char* port, unsigned int pinNumber, bool high_nLow)
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

bool read(unsigned char* port, unsigned int pinNumber)
{
  return !((*port & (0x01 << pinNumber)) == 0x00);
}
