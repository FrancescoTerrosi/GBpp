#ifndef MMUMEMORYINTERFACE_H
#define MMUMEMORYINTERFACE_H

void dispatchMemOp(unsigned short address, unsigned char* buffer, int mode);

void doOpWRAM(unsigned short address, unsigned char* buffer, int mode);
void initWRAM(const char* boot_rom_path, const int boot_rom_size); 

void doOpCartridgeROM(unsigned short address, unsigned char* buffer, int mode);
int initCartridgeROM(char* rom_path);

#endif
