#ifndef MMUMEMORYINTERFACE_H
#define MMUMEMORYINTERFACE_H

unsigned char isBooting();


void dispatchMemOp(unsigned short address, unsigned char* buffer, int mode);

void doOpWRAM(unsigned short address, unsigned char* buffer, int mode);
void initWRAM(const char* boot_rom_path, const int boot_rom_size);

void doOpVRAM(unsigned short address, unsigned char* buffer, int mode);
void doOpHRAM(unsigned short address, unsigned char* buffer, int mode);
void doOpIOPort(unsigned short address, unsigned char* buffer, int mode);

void doOpCartridgeROM(unsigned short address, unsigned char* buffer, int mode);
int initCartridgeROM(char* rom_path);

void printWRAM();

#endif
