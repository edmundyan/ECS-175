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

/* GLOBAL VARAIBLES */
/* (storage is actually allocated here) */
int W=600;  /* window width */
int H=600;  /* window height */


void display() {

  // black background
  glClearColor(0.0, 0.0, 0.0, 0.0); 

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

void keyPressed (unsigned char key, int x, int y) {
  if (key == 'w') 
    moveCamera(0.2);  // move forward
  else if (key == 's') 
    moveCamera(-0.2); // move backward
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
    // turn on depth buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
  }

  free_resources();
  return 0;
}










