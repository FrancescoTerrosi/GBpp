#include "cartridgerom.h"

unsigned char* ROM;

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

    std::cout << "HELLO! " << bytes_read << std::endl;
    ROM = new unsigned char[bytes_read];

    memcpy(ROM, bytes_buffer, bytes_read);

    gameBoyRom.close();

    delete[] bytes_buffer;
    delete current_char;

    CARTRIDGE_INSERTED = true;

    return bytes_read;

}

