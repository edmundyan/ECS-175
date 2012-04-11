#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader_utils.h";
#include <GL/glut.h>
#include "lowlevel.h"



/* Variables local to this file */
GLuint program;
GLint A_COORD = 0;
GLint A_COLOR = 1;

int W; /* width of frame buffer */
int H; /* height of frame buffer */
int S; /* size of each pixel */

/*****************************************************************************/
/* Will be pointer to the vertex array, containing the big pixels. */
GLfloat *varr;

int init_resources(void) {
{
  // Error flags are initially false
  GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;

  // Make a shader object
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);

  // The source of the shader program. 
  // It is in a string. 
  // Just passes on triangle coordinates to the rasterizer
  const char *vs_source =
#ifdef GL_ES_VERSION_2_0
    "#version 100\n"  // OpenGL ES 2.0
#else
    "#version 120\n"  // OpenGL 2.1
#endif
    "attribute vec2 coord2d;                  "
    "attribute vec3 a_color;                  "
    "varying vec4 v_color;                   "
    "void main(void) {                        "
    "  gl_Position = vec4(coord2d, 0.0, 1.0); "
    "  v_color = vec4(a_color,1.0);                     "
    "}";
  // Let OpenGL know that this string is the source code
  glShaderSource(vs, 1, &vs_source, NULL);
  // Compile the source code
  glCompileShader(vs);
  // Compiler errors? 
  glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
  if (!compile_ok) {
    fprintf(stderr, "Error in vertex shader\n");
    return 0;
  }

  // Now make the fragment shader; similar process. 
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fs_source =
#ifdef GL_ES_VERSION_2_0
    "#version 100\n"  // OpenGL ES 2.0
#else
    "#version 120\n"  // OpenGL 2.1
#endif
    "varying vec4 v_color;                   "
    "void main(void) {        "
    "  gl_FragColor = v_color; "
    "}";
  glShaderSource(fs, 1, &fs_source, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
  if (!compile_ok) {
    fprintf(stderr, "Error in fragment shader\n");
    return 0;
  }

  // The GPU program contains both the vertex and shader programs. 
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  // Making the GPU program is like linking parts of a CPU program.
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    fprintf(stderr, "glLinkProgram:");
    return 0;
  }

  // Tell OpenGL what names you used for the variables
  glBindAttribLocation(program, A_COORD, "coord2d");
  glBindAttribLocation(program, A_COLOR, "a_color");

  return 1;
 }
}

// Convert 0 to scale coords to -1.0 to 1.0 range visible in window
GLfloat pix2coord(int i, int scale) {
  GLfloat x;

  x = i*2.0/scale - 1.0;
  return x;
}

/* Adds a vertex of the given color to the array at position x,y */
void addVert(GLfloat* a, int i,  GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b) {
  a[i++] = x;
  a[i++] = y;
  a[i++] = r;
  a[i++] = g;
  a[i++] = b;
}


/* Color a pixel in the faux frame buffer */
void colorPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  int i;

  // 5 floats per vertex, 4 vertices per pixel, W pixels per row
  i = 5*4*(W*y+x);
  addVert(varr, i, pix2coord(x,W), pix2coord(y,H), r,g,b);
  addVert(varr, i+5, pix2coord(x,W), pix2coord(y+1,H), r,g,b);
  addVert(varr, i+10, pix2coord(x+1,W), pix2coord(y+1,H), r,g,b);
  addVert(varr, i+15, pix2coord(x+1,W), pix2coord(y,H), r,g,b);
}



/* Allocate storage for fake frame buffer, and fill it with gray */
bool initBigPixelArray(int w, int h, int s) {
  int i,x,y;
  GLfloat r,g,b;

  // Set up shaders
  if (!init_resources())
      return 0;


  // remember in globals
  W = w;
  H = h; 
  S = s;
  

  // Allocate local array of vertex positions and colors
  // Will included positions for four vertices as well as color
  // at vertex for each pixel
  // colors are initially gray
  varr = (GLfloat*) malloc(W*H*20*sizeof(GLfloat));
  i = 0;
  // store matrix by rows
  for (y=0; y<H; y++) {
    for (x=0; x<W; x++) {
      r = g = b = 0.5;

      addVert(varr, i, pix2coord(x,W), pix2coord(y,H), r,g,b);
      i += 5;
      addVert(varr, i, pix2coord(x,W), pix2coord(y+1,H), r,g,b);
      i += 5;
      addVert(varr, i, pix2coord(x+1,W), pix2coord(y+1,H), r,g,b);
      i += 5;
      addVert(varr, i, pix2coord(x+1,W), pix2coord(y,H), r,g,b);
      i += 5;
    }
  }

  return 1;
}



/* Draws fake framebuffer into OpenGL window  */
void drawBigPixelArray(void) {
  int i;

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);

  // Two attributes for the vertex, position and color.
  glEnableVertexAttribArray(A_COORD);
  glEnableVertexAttribArray(A_COLOR);

  // Describe the first attribute and where it is in the array
  glVertexAttribPointer(
    A_COORD,           // attribute ID
    2,                 // number of elements per vertex, here (x,y)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),                 // no extra data between each position
    varr               // pointer to first position in the C array
  );

  // Describe the color attribute and where it is in the array
  glVertexAttribPointer(
    A_COLOR,   // attribute ID
    4,                 // number of elements per vertex, here (r,g,b,a)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),                 // no extra data between each position
    varr+2             // pointer to first color in the C array
  );

  // Draw the array of big pixels
  // Each set of four adjacent vertices makes a quad.
  for (i=0; i<W*H; i++) {
    // a quad is a triangle fan with four vertices
    glDrawArrays(GL_TRIANGLE_FAN, i*4, 4);
  }

  // Done drawing this
  glDisableVertexAttribArray(A_COORD);
  glDisableVertexAttribArray(A_COLOR);

  glutSwapBuffers();
  
}

void free_resources(void)
{
  glDeleteProgram(program);
}


