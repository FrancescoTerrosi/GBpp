#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <fstream>
#include <cstring>
#include "mmu.h"
#include "../interfaces/mmumemoryinterface.h"

//FOR STATISTICS
extern int PROGRAM_SIZE, INSTRUCTION_COUNTER, NOP_COUNTER, INSTRUCTION_FAILS, UNIQUE_INST, UNIQUE_FAILS;

std::string parseBytes(unsigned char* bytes, int bytes_length);

void fetch();

void execute();

void loop();

void boot(char* boot_rom_path, int boot_rom_size, char* rom_path);

#endif

