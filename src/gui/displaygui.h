#ifndef BOARDGUI_H
#define BOARDGUI_H
#include <GL/glut.h>
#include <iostream>

void Display(void);

void Reshape(int w, int h);

void initDisplay (int argc, char **argv, char* boot_rom_path, int boot_rom_size, char* rom_path);

#endif

