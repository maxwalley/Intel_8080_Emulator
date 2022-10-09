//
//  main.cpp
//  Intel_8080_Emulator
//
//  Created by Max Walley on 09/04/2022.
//

#include "SpaceInvaders.hpp"

#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>

#include <cmath>

SpaceInvaders emulator;

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(256 * 2, 224 * 2);
    
    glutInitWindowPosition(50, 50);
    
    glutCreateWindow("Test");
    
    glutSpecialFunc([](int keycode, int x, int y){
        emulator.triggerKeyDown(keycode, x, y);
    });
    
    glutSpecialUpFunc([](int keycode, int x, int y){
        emulator.triggerKeyUp(keycode, x, y);
    });
    
    const auto displayFunc = [](){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBegin(GL_TRIANGLES);
                glVertex3f(-0.5,-0.5,0.0);
                glVertex3f(0.5,0.0,0.0);
                glVertex3f(0.0,0.5,0.0);
            glEnd();
        
        glFlush();
    };
    
    glutDisplayFunc(displayFunc);
    
    glutIdleFunc(displayFunc);
    
    glutMainLoop();
    
    return 0;
}
