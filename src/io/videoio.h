#ifndef VIDEOIO_H
#define VIDEOIO_H

#define LCD_CONTROL 0x40
#define LCD_STATUS 0x41
#define LCD_SCY 0x42
#define LCD_SCX 0x43
#define LCD_Y_COORDINATE 0x44
#define LCD_LYC 0x45
#define LCD_DMA 0x42
#define LCD_SCX 0x43
#define LCD_CONTROL_ADDRESS_END 0x4B

void doOpIOPort(unsigned short address, unsigned char* buffer, int mode);

#endif

