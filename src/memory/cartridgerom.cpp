#include "cartridgerom.h"

unsigned char* ROM;

std::string parseCROMBytes(unsigned char* bytes, int bytes_length)
{

    unsigned char bytes_copy[bytes_length];
    memcpy(&bytes_copy, bytes, bytes_length);

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

        if (i != bytes_length-1) {
            result += " ";
        }

    }

    return result;

}


void _readCartridgeROM(unsigned short address, unsigned char* buffer)
{
    *buffer = ROM[address];
}

void _writeCartridgeROM(unsigned short address, unsigned char* buffer)
{
    ROM[address] = *buffer;
}

void doOpCartridgeROM(unsigned short address, unsigned char* buffer, int mode)
{
    if (CARTRIDGE_INSERTED)
    {
        switch (mode)
        {
            case 0:
                _readCartridgeROM(address, buffer);
            break;
            case 1:
                _writeCartridgeROM(address, buffer);
            break;
        }
    }
}

int initCartridgeROM(char* rom_path)
{
    int bytes_read = 0;
    //TODO
    //FIX MAGIC NUMBER
    char* bytes_buffer = new char[(1024 * (1 << 10))];
    char* current_char = new char;
    std::ifstream gameBoyRom(rom_path);

    while (gameBoyRom.get(*current_char))
    {
        bytes_buffer[bytes_read] = (*current_char);
        bytes_read++;
    }

    ROM = new unsigned char[bytes_read];

    memcpy(ROM, bytes_buffer, bytes_read);

    gameBoyRom.close();

    delete[] bytes_buffer;
    delete current_char;

    CARTRIDGE_INSERTED = true;

    return bytes_read;

}

