#ifndef VRAMMAP_H
#define VRAMMAP_H

//TILE DATA
/*
    A tile is an 8x8 square and each pixel has associated 2 bits
    for color (gray-scale), thus each tile takes 16 bytes to store.
    Given that each tile is 128 bits, this memory area can store 384 tiles.
*/

#define VRAM_BLOCK_0_START 0x0
#define VRAM_BLOCK_0_END 0x07FF

#define VRAM_BLOCK_1_START 0x0800
#define VRAM_BLOCK_1_END 0x0FFF

#define VRAM_BLOCK_2_START 0x1000
#define VRAM_BLOCK_2_END 0x17FF

#define VRAM_TILE_MAP_SIZE 32*32
#define VRAM_TILE_MAP_0 0x1800
#define VRAM_TILE_MAP_1 0x9C00

#endif
