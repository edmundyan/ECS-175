#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shader_utils.h"
#include "drawPlant.h"


/* GLOBAL VARAIBLES */
/* (storage is actually allocated here) */
int W=1000;  /* window width */
int H=1000;  /* window height */

/* local function declarations */
void display(void);
void init(void);


int main (int argc, char** argv) {
  int win;

  glutInit(&argc,argv);
  glutInitWindowSize(800,800);
  glutInitWindowPosition(100,100);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  win = glutCreateWindow("plant");
  //glutSetWindow(win);

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
      fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
      return 1;
    }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
    return 1;
  }

  // for testing purposes - does this crash? 
  GLuint res = glCreateShader(GL_VERTEX_SHADER);
  // so there is a rendering context now? 


  // try to set up GPU to draw; if it works, fire off GLUT
  if (init_resources()) {
    glutDisplayFunc(display);
    glutMainLoop();
  }

  free_resources();
  return 0;
}



void display() {

  // black background
  glClearColor(0.0, 0.0, 0.0, 0.0);  
  glClear(GL_COLOR_BUFFER_BIT);

  
  /* put plant drawing code here */
  drawPlant(2, NULL);
  /* end drawing code */

  // Swap the screen (forces drawing, and used for animation)
  glutSwapBuffers();
}








