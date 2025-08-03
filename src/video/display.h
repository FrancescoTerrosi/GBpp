#ifndef DISPLAY_H
#define DISPLAY_H

//RESOLUTION IN PIXELS
unsigned int SCREEN_WIDTH = 160;
unsigned int SCREEN_HEIGHT = 144;
unsigned int SPRITE_SIZE = 8;

unsigned int VIDEO_RAM_SIZE = 8*(1 << 10);
unsigned char* VIDEO_RAM = new unsigned char[VIDEO_RAM_SIZE];

#endif

