#ifndef CARTRIDGEROM_H
#define CARTRIDGEROM_H

#include <cstring>
#include <iostream>
#include <fstream>
#include "../interfaces/mmumemoryinterface.h"

bool CARTRIDGE_INSERTED = false;

void doOpCartridgeROM(unsigned short address, unsigned char* buffer, int mode);
int initCartridgeROM(char* rom_path);

#endif

