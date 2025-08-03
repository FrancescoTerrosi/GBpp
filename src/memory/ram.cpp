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

void printWRAM()
{

    unsigned int bytes_length = 1;
    unsigned char bytes = 0;
    for (int i = 0; i < RAM_SIZE; i++)
    {   
        bytes = RAM[i];
        unsigned char bytes_copy[bytes_length];
        memcpy(&bytes_copy, &bytes, bytes_length);
        
        char current_byte[8];
        int counter = 0;
        
        std::string result = "";
        
        for (int i = 0; i < bytes_length; i++)
        {   
            counter = 7;
            while (bytes_copy[i] > 0)
            {
                if (bytes_copy[i]%2 == 1)
                {
                    current_byte[counter] = '1';
                }
                else
                {
                    current_byte[counter] = '0';
                }
                bytes_copy[i] = (bytes_copy[i] >> 1);
                counter--;
            }

            while (counter >= 0)
            {
                current_byte[counter] = '0';
                counter--;
            }

            result += current_byte;

            if (i != bytes_length-1)
            {
                result += " ";
            }

        }
        std::cout << result << std::endl;
    }

}

