#ifndef VRAM_H
#define VRAM_H

#define VRAM_SIZE 8*(1 << 10)

#define VRAM_BLOCK_0_START 0x0;
#define VRAM_BLOCK_0_END 0x07FF;

#define VRAM_BLOCK_1_START 0x0800;
#define VRAM_BLOCK_1_END 0x0FFF;

#define VRAM_BLOCK_2_START 0x1000;
#define VRAM_BLOCK_2_END 0x17FF;

void doOpVRAM(unsigned short address, unsigned char* buffer, int mode);

#endif

