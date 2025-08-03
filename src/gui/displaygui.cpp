#include <GL/glut.h>
#include <iostream>
#include "displaygui.h"
#include "../core/cpu.h"


#define BORDER_LENGTH 144

void Display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    float x_border = 1.0-(1.0/width);
    float y_border = 1.0-(1.0/height);
    float x_center = 0+(1.0/width);
    float y_center = 0-(1.0/height);

    glColor3f(0.0,0.0,0.0);
    glBegin(GL_LINES);
        //LEFT BORDER
        glVertex2f(-x_border,y_border);
        glVertex2f(-x_border,-y_border);

        //BOTTOM BORDER
        glVertex2f(-x_border,-y_border);
        glVertex2f(x_border,-y_border);

        //RIGHT BORDER
        glVertex2f(x_border,-y_border);
        glVertex2f(x_border,y_border);

        //TOP BORDER
        glVertex2f(x_border,y_border);
        glVertex2f(-x_border,y_border);

        float w_step = 2.0/(float)BORDER_LENGTH;
        float current_col = -1.0+w_step;
        while (current_col < x_border) {
            glVertex2f(current_col, 1);
            glVertex2f(current_col, -1);
            current_col += w_step;
        }

        float h_step = 2.0/(float)BORDER_LENGTH;
        float current_row = -1.0+h_step;
        while (current_row < y_border) {
            glVertex2f(1, current_row);
            glVertex2f(-1, current_row);
            current_row += h_step;
        }

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
    glutPostRedisplay();
    glutTimerFunc(value, GameLoop, value);
}

void initDisplay (int argc, char **argv, char* boot_rom_path, int boot_rom_size, char* rom_path)
{

    unsigned short timestep = 500;

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


    boot(boot_rom_path, boot_rom_size, rom_path);
    loop();



    //Let GLUT get the msgs
    //glutMainLoop();
}

