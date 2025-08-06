#ifndef PPU_H
#define PPU_H

//RESOLUTION IN PIXELS 
#define SCREEN_WIDTH 160 
#define SCREEN_HEIGHT 144 
#define SPRITE_SIZE 8

extern unsigned char* SCREEN;
extern unsigned char CURRENT_ROW;
extern unsigned short CURRENT_COL;

void ppuloop();

#endif

