#ifndef HRAM_H
#define HRAM_H
#include "../interfaces/mmumemoryinterface.h"

#define HRAM_SIZE 127

void doOpHRAM(unsigned short address, unsigned char* buffer, int mode);

#endif

