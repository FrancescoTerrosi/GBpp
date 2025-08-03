#ifndef IOCONTROLLER_H
#define IOCONTROLLER_H

#define IO_REGISTERS_SIZE 128

#define BOOT_ROM_LOCK_REGISTER 0xFF50

void doOpIOPort(unsigned short address, unsigned char* buffer, int mode);

#endif
