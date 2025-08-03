#include <iostream>
#include "mmu.h"
#include "../interfaces/mmumemoryinterface.h"
#include "../interfaces/cpudisplayinterface.h"

/*
dispatchMemOp
    address: memory location for read/write op
    buffer: channel used to read/write data
    mode: 0 = read, 1 = write
*/
void dispatchMemOp(unsigned short address, unsigned char* buffer, int mode)
{

    if (isBooting() && address < 0x100)
    {
        //std::cout << "ACCESS WRAM 0 " << std::hex << address << std::dec << std::endl;
        doOpWRAM(address, buffer, mode);
    }
    else
    {

        if (ROM_BANK_00_ADDRESS_START <= address && address <= ROM_BANK_00_ADDRESS_END)
        {
            //std::cout << "ACCESS CARTRIDGE ROM 0" << std::endl;
            doOpCartridgeROM(address-ROM_BANK_00_ADDRESS_START, buffer, mode);
        }
        else if (ROM_BANK_01_ADDRESS_START <= address && address <= ROM_BANK_01_ADDRESS_END)
        {
            //std::cout << "ACCESS CARTRIDGE ROM 1" << std::endl;
            doOpCartridgeROM(address-ROM_BANK_01_ADDRESS_START, buffer, mode);
        }

        else if (VRAM_ADDRESS_START <= address && address <= VRAM_ADDRESS_END)
        {
            //std::cout << "ACCESS VRAM 0" << std::endl;
            doOpVRAM(address-VRAM_ADDRESS_START, buffer, mode);
        }

        else if (EXTERNAL_RAM_ADDRESS_START <= address && address <= EXTERNAL_RAM_ADDRESS_END)
        {
            //std::cout << "ACCESS EXTERNAL RAM" << std::endl;
            ;//doOpExternalRAM(address-EXTERNAL_RAM_ADDRESS_START, buffer, mode);
        }

        else if (WRAM_BANK_0_ADDRESS_START <= address && address <= WRAM_BANK_0_ADDRESS_END)
        {
            //std::cout << "ACCESS WRAM 0 " << std::hex << address << std::dec << std::endl;
            doOpWRAM(address-WRAM_BANK_0_ADDRESS_START, buffer, mode);
        }

        else if (WRAM_BANK_1_ADDRESS_START <= address && address <= WRAM_BANK_1_ADDRESS_END)
        {
            //std::cout << "ACCESS WRAM 1" << std::endl;
            ;//doOpWRAM(address-WRM_BANK_1_ADDRESS_START, buffer, mode);
        }

        else if (WRAM_BANK_0_ECHO_ADDRESS_START <= address && address <= WRAM_BANK_0_ECHO_ADDRESS_END)
        {
            //std::cout << "ACCESS WRAM 0 ECHO" << std::endl;
            ;//doOpWRAM(address-WRAM_BANK_0_ECHO_ADDRESS_START, buffer, mode);
        }

        else if (SPRITE_ATTRIBUTE_TABLE_ADDRESS_START <= address && address <= SPRITE_ATTRIBUTE_TABLE_ADDRESS_END)
        {
            //std::cout << "ACCESS SPRITE ATTRIBUTE TABLE" << std::endl;
            ;
        }

        else if (NOT_USABLE_ADDRESS_START <= address && address <= NOT_USABLE_ADDRESS_END)
        {
            //std::cout << "ACCESS NOT USABLE #TODO" << std::endl;
            ;
        }

        else if (IO_PORTS_ADDRESS_START <= address && address <= IO_PORTS_ADDRESS_END)
        {
            //std::cout << "ACCESS IO PORTS" << std::endl;
            doOpIOPort(address - IO_PORTS_ADDRESS_START, buffer, mode);
        }

        else if (HIGH_RAM_ADDRESS_START <= address && address <= HIGH_RAM_ADDRESS_END)
        {
            std::cout << "ACCESS HIGH RAM" << std::endl;
            doOpHRAM(address - HIGH_RAM_ADDRESS_START, buffer, mode);
        }
        else if (address == INTERRUPT_ENABLE_REGISTER_ADDRESS)
        {
            //std::cout << "ENABLE INTERRUPT REGISTER" << std::endl;
            ;
        }
        else
        {
            std::cout << "Should never happen." << std::endl;
            //should never happen.
            ;
        }
    }
}

