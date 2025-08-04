#include <cstring>
#include <iostream>
#include <fstream>
#include "hram.h"

unsigned char* HRAM = new unsigned char[HRAM_SIZE];

void _readHRAM(unsigned short address, unsigned char* buffer)
{
    *buffer = HRAM[address];
    //std::cout << std::hex << "BUFFER: " << (unsigned short)*buffer << std::endl;
}

void _writeHRAM(unsigned short address, unsigned char* buffer)
{
    //std::cout << "Write hram" << std::endl;
    HRAM[address] = *buffer;
    //std::cout << std::hex << "ADDRESS " << address << " VAL " <<  (unsigned short)HRAM[address] << std::endl;
}

void doOpHRAM(unsigned short address, unsigned char* buffer, int mode)
{
    switch (mode)
    {
        case 0:
            _readHRAM(address, buffer);
        break;
        case 1:
            _writeHRAM(address, buffer);
        break;
    }
}

