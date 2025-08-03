#include <iostream>
#include <cstring>
#include "vram.h"
#include "../interfaces/cpudisplayinterface.h"

unsigned char* VRAM = new unsigned char[VRAM_SIZE];

void _readVRAM(unsigned short address, unsigned char* buffer)
{
    *buffer = VRAM[address];
}

void _writeVRAM(unsigned short address, unsigned char* buffer)
{
    std::cout << "write VRAM" <<std::endl;
    VRAM[address] = *buffer;
}

void doOpVRAM(unsigned short address, unsigned char* buffer, int mode)
{
    switch (mode)
    {
        case 0:
            _readVRAM(address, buffer);
        break;
        case 1:
            _writeVRAM(address, buffer);
        break;
    }
}

void printVRAM()
{
    unsigned int bytes_length = 1;
    unsigned char bytes = 0;
    for (int i = 0; i < VRAM_SIZE; i++)
    {
        if (VRAM[i] != 0)
        bytes = VRAM[i];
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

