#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_utils.h"
#include "draw.h"


/* GLOBAL VARAIBLES */
/* (storage is actually allocated here) */
int W=600;  /* window width */
int H=600;  /* window height */

int x_start;
int y_start;  // initial x/y values when button is pressed down

extern glm::mat4 turtleMatrix;  // from draw.cpp

void display() {

  // black background
  glClearColor(0.0, 0.0, 0.0, 0.0); 

  // clear both the frame buffer and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // call drawing code
  drawScene();

  // Swap the screen (forces drawing, and used for animation)
  glutSwapBuffers();
}

// glut callback function, called when mouse moves with button depressed
// (x,y) is cursor position within window, *after* the mouse motion
void mouseMotion(int x2, int y2) {
  printf("%d, %d\n", x_start - x2, y_start - y2);
  x_start = x2;
  y_start = y2;
  // ask glut to redraw the image (glut will call display())

  moveObj();
  glutPostRedisplay();
}

void moveObj() {
	float theta = (M_PI / 540.0) * length;
	glm::mat4 Ra = rotate_around_Z(-beta) * rotate_around_X(theta) * rotate_around_Z(beta);

	turtleMatrix = Ra * turtleMatrix;
	totalMatrix = proj * view * turtleMatrix;

}
// glut callback function, called when a mouse butten is pushed
// button is which button, state is up or down, (x,y) is cursor 
// position within window.
void mouseButton (int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
      printf("button down\n");
      x_start = x;
      y_start = y;
    }
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
    // setup callback functions for events that will occur
    glutDisplayFunc(display);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    // turn on depth buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    // wait for events
    glutMainLoop();
  }

  // after OpenGL window is closed
  free_resources();
  return 0;
}










