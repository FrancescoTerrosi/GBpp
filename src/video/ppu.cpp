#include <iostream>
#include "ppu.h"
#include "vrammap.h"
#include "../io/videoio.h"
#include "../interfaces/ppumemoryinterface.h"

#define VBLANK 144

unsigned char* SCREEN = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT];

unsigned char CURRENT_ROW = 0;
unsigned short CURRENT_COL = 0;
unsigned char data = 0;

unsigned short tile_size = 16;
unsigned char* current_tile = new unsigned char[tile_size];

void ppuloop()
{

    doOpIOPort(LCD_CONTROL, &data, 0);
    if ((data >> 7) == 1)
    {
        //TODO fix magic number
        doOpIOPort(LCD_Y_COORDINATE, &CURRENT_ROW, 1);

        
        if (CURRENT_COL == 159)
        {
            CURRENT_ROW = (CURRENT_ROW+1)%(VBLANK+10);
        }
        else
        {
            doOpVRAM(0x1800+(CURRENT_COL+(SCREEN_WIDTH*CURRENT_ROW)), &SCREEN[CURRENT_COL+(SCREEN_WIDTH*CURRENT_ROW)], 0);
        }
        CURRENT_COL = (CURRENT_COL+1)%160;
    }

}

