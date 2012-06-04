#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"
#include "tdraw.h"

#include <iostream>
using namespace std;

#define M_PI       3.14159265358979323846

/* GLOBAL VARAIBLES */
/* (storage is actually allocated here) */
int W=600;  /* window width */
int H=600;  /* window height */


void display() {

	// black background
	//glClearColor(0.0, 0.0, 0.0, 0.0); 
	glClearColor(0.3,0.4,0.9,0.1);
	// clear both the frame buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* put plant drawing code here */
	drawScene();
	/* end drawing code */

	// Swap the screen (forces drawing, and used for animation)
	glutSwapBuffers();
}

/*
void onIdle() {
	setNewRotationMatrix();
	glutPostRedisplay();
}
*/

void keyPressed (unsigned char key, int x, int y) 
{
	if (key == 't') 
		moveCamera(0.2);
	else if (key == 'b') 
		moveCamera(-0.2);
	glutPostRedisplay();
}

void specialKeyPressed(int key, int x, int y)
{
	float degrees = 5;
	if (key == GLUT_KEY_UP)
		moveCamera(0.3);
	else if (key == GLUT_KEY_DOWN)
		moveCamera(-0.3);
	else if (key == GLUT_KEY_LEFT)
		panCamera(M_PI/180 * degrees); // rotate left by degrees
	else if (key == GLUT_KEY_RIGHT)
		panCamera(-M_PI/180 * degrees); // rotate right by degrees
	glutPostRedisplay();
}


int main (int argc, char** argv) {
	int win;

	glutInit(&argc,argv);
	glutInitWindowSize(W,H);
	glutInitWindowPosition(50,50);
	// Tell glut we will be using depth buffering (as well as rgba color and
	// double precision if possible). 
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	win = glutCreateWindow("cube");
	glutSetWindow(win);

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
    }

	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "Error: your graphics card does not support OpenGL 2.0\n");
		return 1;
	}


	// try to set up GPU to draw; if it works, fire off GLUT
	if (init_resources()) {
		// set display callback function
		glutDisplayFunc(display);
		// set key press callback function
		glutKeyboardFunc(keyPressed);
		glutSpecialFunc(specialKeyPressed);

		// turn on depth buffering in OpenGL
		glEnable(GL_DEPTH_TEST);
		glutMainLoop();
	}

	free_resources();
	return 0;
}










