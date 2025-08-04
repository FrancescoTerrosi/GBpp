#include <iostream>
#include "ppu.h"
#include "vrammap.h"
#include "../io/videoio.h"
#include "../interfaces/cpuppuinterface.h"

unsigned char* SCREEN = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT];

unsigned char current_row = 0;
unsigned short current_col = 0;
unsigned char data = 0;

void ppuloop()
{

    doOpIOPort(LCD_CONTROL, &data, 0);
    if ((data >> 7) == 1)
    {
        //TODO fix magic number
        doOpIOPort(LCD_Y_COORDINATE, &current_row, 1);
        //std::cout << std::dec << "current_row = " << (unsigned short) current_row <<std::endl;
        //std::cout << std::dec << "current_col = " << (unsigned short) current_col <<std::endl;
        if (current_col == 159)
        {
            current_row = (current_row+1)%154;
        }   
        current_col = (current_col+1)%160;
    }

}

