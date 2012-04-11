/*
 *  bigPix.cpp
 *  Draws a 50x50 array of big pixels, for practice in low-level graphics
 */

#include <stdio.h>
#include <stdlib.h>
#include "shader_utils.h";
#include <GL/glut.h>
#include "lowlevel.h"        /* faux framebuffer definitions */


void readInput(void);

/* dimensions - each pixel is 10x10, and the window is 50x50 pixels */
static int pixelSize= 10;
static int numberOfPixels = 50;


/* Call your triangle drawing program from here; this is the 
   function which draws the contents of the framebuffer by 
   coloring pixels. */
void drawContents(void) {
  
  /* The colorPixel function: int x, int y, GLubyte r, GLubyte g, GLubyte b
     x,y = pixel address
     r,g,b = pixel color
  */
  readInput();

  colorPixel(25,25,0.8,0.1,0.1);



}

/* Reads in the triangle vertices and colors from stdin */
void readInput(void) {
  int length = 100;
  char* line;
  int items;
  float x,y;

  line = (char*) malloc(length+1);
  while (fgets(line,length,stdin) != NULL) {
    items = sscanf(line, "%f %f \n", &x, &y);
    if (items != 2) 
      printf("Failed to read x and y \n");
    else 
      printf("x %f y %f\n",x,y);
  }
}

/* Called by glut when window is refreshed */
void display(void)
{

/* Low-level system does it */
  colorPixel(25,25,0.8,0.1,0.1);
  drawBigPixelArray();

}

/* Called by main on initialization */
int main(int argc, char** argv)
{
  int size;
  size = numberOfPixels*pixelSize;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(size,size);
  glutCreateWindow("Big Pixels");

   ////
  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
    return 1;
  }
  ////

  if (initBigPixelArray(numberOfPixels,numberOfPixels,pixelSize)) {
    glutDisplayFunc(display);
    glutMainLoop();
  }

  free_resources();
  return 0;

}

