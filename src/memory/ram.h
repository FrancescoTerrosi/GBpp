#ifndef RAM_H
#define RAM_H
#include "../interfaces/mmumemoryinterface.h"

#define RAM_SIZE 8*(1 << 10)

void doOpWRAM(unsigned short address, unsigned char* buffer, int mode);

void initWRAM(const char* boot_rom_path, const int boot_rom_size);

#endif

