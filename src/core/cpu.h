#ifndef CPU_H
#define CPU_H

//FOR STATISTICS AND DEBUG
extern int PROGRAM_SIZE, INSTRUCTION_COUNTER, NOP_COUNTER, INSTRUCTION_FAILS, UNIQUE_INST, UNIQUE_FAILS;
extern short unsigned PC;

unsigned char isBooting();

void cpuloop();

void cpuboot(char* boot_rom_path, int boot_rom_size, char* rom_path);

#endif

