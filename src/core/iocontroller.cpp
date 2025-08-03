#include "iocontroller.h"

unsigned char* IORegister = new unsigned char[IO_REGISTERS_SIZE];

void _readIORegister(unsigned short address, unsigned char* buffer)
{
    *buffer = IORegister[address];
}

void _writeIORegister(unsigned short address, unsigned char* buffer)
{
    IORegister[address] = *buffer;
}

void doOpIOPort(unsigned short address, unsigned char* buffer, int mode)
{
    switch (mode)
    {
        case 0:
            _readIORegister(address, buffer);
        break;
        case 1:
            _writeIORegister(address, buffer);
        break;
    }
}

unsigned char isBooting()
{
    return !IORegister[BOOT_ROM_LOCK_REGISTER];
}


