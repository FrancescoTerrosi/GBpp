#include <GL/glut.h>
#include <iostream>
#include "displaygui.h"
#include "../core/cpu.h"
#include "../video/ppu.h"
#include "../interfaces/ppumemoryinterface.h"

#define BORDER_LENGTH 160
#define BORDER_HEIGHT 144

unsigned int width = 160;
unsigned int height = 144;
float x_right_border;
float y_top_border;
float x_left_border;
float y_bottom_border;
float x_center;
float y_center;
float current_x;
float current_y;
float w_step;
float current_col;
float h_step;
float current_row;

void Display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    x_right_border = (1.0-0.33);
    x_left_border = (0.33-1.00);
    y_top_border = (1.0-0.22);
    y_bottom_border = (0.22-1.00);
    x_center = 0;
    y_center = 0;
    current_x = -x_left_border;
    current_y = y_top_border;

    w_step = (x_right_border-x_left_border)/BORDER_LENGTH;
    h_step = (y_top_border-y_bottom_border)/BORDER_HEIGHT;

    glColor3f(0.0,0.0,0.0);
    glBegin(GL_LINES);
        //LEFT BORDER
        glVertex2f(x_left_border,y_top_border);
        glVertex2f(x_left_border,y_bottom_border);

        //BOTTOM BORDER
        glVertex2f(x_left_border,y_bottom_border);
        glVertex2f(x_right_border,y_bottom_border);

        //RIGHT BORDER
        glVertex2f(x_right_border,y_bottom_border);
        glVertex2f(x_right_border,y_top_border);

        //TOP BORDER
        glVertex2f(x_right_border,y_top_border);
        glVertex2f(x_left_border,y_top_border);


        int c = 0;
        while (c*h_step < (y_top_border-y_bottom_border))
        {
            glVertex2f(x_right_border,y_bottom_border+(c*h_step));
            glVertex2f(x_left_border,y_bottom_border+(c*h_step));
            c++;
        }
        c = 0;
        while (c*w_step < (x_right_border-x_left_border))
        {
            glVertex2f(x_left_border+(c*w_step),y_top_border);
            glVertex2f(x_left_border+(c*w_step),y_bottom_border);
            c++;
        }

    glEnd();

    glBegin(GL_TRIANGLES);

        int cx = 0;
        int cy = 0;
        while (cy < BORDER_HEIGHT)
        {
            cx = 0;
            while (cx < BORDER_LENGTH)
            {
                if (SCREEN[cx+(BORDER_HEIGHT*cy)] > 0) {

                    //This is a rectangle
                    glVertex2f(x_left_border+(cx*w_step), y_top_border-(cy*h_step));
                    glVertex2f(x_left_border+(cx*w_step)+w_step, y_top_border-(cy*h_step));
                    glVertex2f(x_left_border+(cx*w_step), y_top_border-(cy*h_step)-h_step);

                    glVertex2f(x_left_border+(cx*w_step), y_top_border-(cy*h_step)-h_step);
                    glVertex2f(x_left_border+(cx*w_step)+w_step, y_top_border-(cy*h_step)-h_step);
                    glVertex2f(x_left_border+(cx*w_step)+w_step, y_top_border-(cy*h_step));
                    //
                }
                cx++;
            }
            cy++;
        }
        std::cout << "GRAPHICS COORDINATES " << cx << " " << cy << std::endl;

    glEnd();

    glFlush();          //Finish rendering
}

void Reshape(int w, int h)
{
    if (h == 0 || w == 0) return;  //Nothing is visible then, so return
    //Set a new projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glViewport(0,0,w,h);  //Use the whole window for rendering
    //glViewport(w/4.0, h/5.0, w*0.5, h*0.6);  //Use the whole window for rendering
}

void GameLoop(int value)
{
    if (isBooting())
    {
        cpuloop();
        ppuloop();
    }
    else
    {
        glutPostRedisplay();
        //printVRAM();
        //exit(0);
        //ppuloop();
    }
    glutTimerFunc(value, GameLoop, value);
}

void initDisplay (int argc, char **argv, char* boot_rom_path, int boot_rom_size, char* rom_path)
{

    unsigned short timestep = 0;

    //Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
    glutInitWindowPosition(0, 0);

    //Create a window with rendering context and everything else we need
    glutCreateWindow("GB++");
    glClearColor(255.0,255.0,255.0,0.0);

    //Assign the two used Msg-routines
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutTimerFunc(0, GameLoop, timestep);

    cpuboot(boot_rom_path, boot_rom_size, rom_path);

    //Let GLUT get the msgs
    glutMainLoop();
}

