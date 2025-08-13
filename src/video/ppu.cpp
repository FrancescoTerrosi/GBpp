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

unsigned short TILE_COUNTER = 0;
unsigned short tile_size = 16;

/*

    FIRST TWO BYTES HOLD COLOR. TO BE READ AS FOLLOWS:

    First byte holds lsb.
    Second byte holds msb.

    You pair each bit in the first byte with the bit in the same position of the second byte.

*/


unsigned short* current_pixel = new unsigned short*;

void fetchTile()
{

    //Get tile
    doOpVRAM(0x1800+TILE_COUNTER, &data, 0);
    current_pixel = data;
    TILE_COUNTER++;
    doOpVRAM(0x1800+TILE_COUNTER, &data, 0);
    current_pixel = (data << 8);
    TILE_COUNTER++;

}

void renderTile()
{
    ;
}

void ppuloop()
{

    doOpIOPort(LCD_CONTROL, &data, 0);
    if ((data >> 7) == 1)
    {
        //TODO fix magic number
        doOpIOPort(LCD_Y_COORDINATE, &CURRENT_ROW, 1);

        fetchTile();
        renderTile();
        //doOpVRAM(0x1800+(CURRENT_COL+(SCREEN_WIDTH*CURRENT_ROW)), &data, 0);
        //SCREEN[CURRENT_COL+(SCREEN_WIDTH*CURRENT_ROW)] = 1;

        if (CURRENT_COL == 159)
        {
            CURRENT_ROW = (CURRENT_ROW+1)%(SCREEN_HEIGHT+10);
        }
        CURRENT_COL = (CURRENT_COL+1)%SCREEN_WIDTH;
    }

}

