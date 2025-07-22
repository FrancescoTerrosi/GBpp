#include <cstring>
#include <iostream>
#include <fstream>
#include "ram.h"

unsigned char* RAM = new unsigned char[RAM_SIZE];

void _readRAM(unsigned short address, unsigned char* buffer)
{
    *buffer = RAM[address];
}

void _writeRAM(unsigned short address, unsigned char* buffer)
{
    RAM[address] = *buffer;
}

void doOpWRAM(unsigned short address, unsigned char* buffer, int mode)
{
    switch (mode)
    {
        case 0:
            _readRAM(address, buffer);
        break;
        case 1:
            _writeRAM(address, buffer);
        break;
    }
}

void initWRAM(const char* boot_rom_path, const int boot_rom_size)
{
    //LOAD BOOT ROM
    std::ifstream gameBoyBootRom(boot_rom_path);

    char* bytes = new char[boot_rom_size];

    gameBoyBootRom.get(bytes, boot_rom_size);

    gameBoyBootRom.close();

    memcpy(RAM, bytes, boot_rom_size);

    delete[] bytes;

}

