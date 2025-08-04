#include "cpuiocontroller.h"
#include <iostream>

unsigned char* IORegisterFile = new unsigned char[IO_REGISTER_FILE_SIZE];

void _readIORegisterFile(unsigned short address, unsigned char* buffer)
{
    *buffer = IORegisterFile[address];
}

void _writeIORegisterFile(unsigned short address, unsigned char* buffer)
{
    IORegisterFile[address] = *buffer;
}

void doOpIOPort(unsigned short address, unsigned char* buffer, int mode)
{
    switch (mode)
    {
        case 0:
            _readIORegisterFile(address, buffer);
        break;
        case 1:
            _writeIORegisterFile(address, buffer);
        break;
    }
}

unsigned char isBooting()
{
    return !IORegisterFile[BOOT_ROM_LOCK_REGISTER];
}

