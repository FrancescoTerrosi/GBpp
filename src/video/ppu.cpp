#include <iostream>
#include "ppu.h"
#include "vrammap.h"
#include "../io/videoio.h"
#include "../interfaces/cpuppuinterface.h"

unsigned char* SCREEN = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT];

unsigned char current_line = 0;
unsigned char data = 0;

void ppuloop()
{

    doOpIOPort(LCD_CONTROL, &data, 0);
    if ((data >> 7) == 1)
    {
        //TODO fix magic number
        current_line = (current_line+1)%154;
        doOpIOPort(LCD_Y_COORDINATE, &current_line, 1);
        std::cout << "current_line = " << current_line <<std::endl;
    }

}

