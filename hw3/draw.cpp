/*
  Example of 3D drawing.  Uses a perspective projection matrix. 
 */
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


GLuint program;
GLint attribute_coord3d;  // index of vertex attribute
GLint uniform_matrix;  // indices of uniform variables
GLint uniform_color;

#define X .525731112119133606 
#define Z .850650808352039932

static GLfloat icosoVerts [] = {    
  -X, 0.0, Z, 
  X, 0.0, Z, 
  -X, 0.0, -Z, 
  X, 0.0, -Z,    
  0.0, Z, X, 
  0.0, Z, -X, 
  0.0, -Z, X, 
  0.0, -Z, -X,    
  Z, X, 0.0, 
  -Z, X, 0.0, 
  Z, -X, 0.0, 
  -Z, -X, 0.0 
};

static GLubyte icosoElements[] = { 
  0,4,1, 
  0,9,4, 
  9,5,4, 
  4,5,8, 
  4,8,1,    
  8,10,1, 
  8,3,10, 
  5,3,8, 
  5,2,3, 
  2,7,3,    
  7,10,3, 
  7,6,10, 
  7,11,6, 
  11,0,6, 
  0,1,6, 
  6,1,10, 
  9,0,11, 
  9,11,2, 
  9,2,5, 
  7,2,11
};

// Current total transformation
glm::mat4 totalMatrix;

// Current turtle part of the transformation
glm::mat4 turtleMatrix;

// Matrices used for projection part of the transformation
// projection
glm::mat4 proj = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
			   glm::vec4(0.0, 1.0, 0.0, 0.0),
			   glm::vec4(0.0, 0.0, -0.66, -0.33),
			   glm::vec4(0.0, 0.0, -3.0, 0.0));

// move object into box centered at (0,0,-7)
glm::mat4 view  = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
			     glm::vec4(0.0, 1.0, 0.0, 0.0),
			     glm::vec4(0.0, 0.0, 1.0, 0.0),
			     glm::vec4(0.0, 0.0, -7.0, 1.0));

/*
// print out matrix by rows
// use for debugging if you want
void printMat(void){
  int i,j;
  for (j=0; j<4; j++){
    for (i=0; i<4; i++){
    printf("%f ",myMatrix[i][j]);
  }
  printf("\n");
 }
}
*/

// Set up matrices, set up the shaders, compile and link them, 
// get pointers to 
// where the shader variables are in GPU memory. 
int init_resources()
{
  // Initial turtle is tilts box so we can see three sides.
  // Rotate around y axis 
  glm::mat4 rot_y = glm::mat4(glm::vec4(cos(M_PI/8), 0.0, sin(M_PI/8), 0.0),
			       glm::vec4(0.0, 1.0, 0.0, 0.0),
			       glm::vec4(-sin(M_PI/8), 0.0, cos(M_PI/8), 0.0),
			       glm::vec4(0.0, 0.0, 0.0, 1.0));

  // Rotate around x axis
  glm::mat4 rot_x = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
			       glm::vec4(0.0, cos(M_PI/3), sin(M_PI/3), 0.0),
			       glm::vec4(0.0, -sin(M_PI/3), cos(M_PI/3), 0.0),
			       glm::vec4(0.0, 0.0, 0.0, 1.0));

  turtleMatrix = rot_x * rot_y;
  // total matrix is formed by multiplying projection by turtle
  totalMatrix = proj * view * turtleMatrix;

  // Error flag is initially false
  GLint link_ok = GL_FALSE;
  // Indices for vertex and fragment shaders
  GLuint vs, fs;

  // create_shader is a function in shader_utils that reads in 
  // a vertex or fragment program from a file, creates a shader 
  // object, puts the program into the object, and compiles it.
  // If all goes well, returns 1. 
  vs = create_shader("glassball.v.glsl", GL_VERTEX_SHADER);
  if (vs == 0) return 0;

  fs = create_shader("glassball.f.glsl", GL_FRAGMENT_SHADER);
  if (fs == 0) return 0;


  // The GPU program contains both the vertex and shader programs. 
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);

  // Link them together.
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    fprintf(stderr, "glLinkProgram: failed\n");
    return 0;
  }
  
  // Now figure out where the linker put everything
  // We don't know where the vertex program is going to store all its
  // input data until after linking. 
  // Results are indicies into some table on the GPU. 

  // Ask for the index of shader variable coord3d. 
  attribute_coord3d = glGetAttribLocation(program, "coord3d");
  if (attribute_coord3d == -1) {
    fprintf(stderr, "Could not bind attribute coord3d\n");
    return 0;
  }

  // Get linker's index to the uniform variable
  uniform_matrix = glGetUniformLocation(program, "m_transform");
  if (uniform_matrix == -1) {
    fprintf(stderr, "Could not bind uniform variable m_transform \n");
    return 0;
  }

  // If all went well....
  return 1;
}

// Draw object
void drawScene(void) {

  // Send the program to the GPU
  glUseProgram(program);

  // Now hook up input data to program.

  // Only attribute for the vertex is position. 
  glEnableVertexAttribArray(attribute_coord3d);

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_coord3d, // attribute ID
    3,                 // number of elements per vertex, here (x,y,z)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    3*sizeof(float),  // stride between one position and the next
    icosoVerts  // pointer to first position in the C array
  );

  // give the matrix a value
  glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, glm::value_ptr(totalMatrix));

  glDrawElements(GL_TRIANGLES, 20*3, GL_UNSIGNED_BYTE, icosoElements);

  // Done with the attribute
  glDisableVertexAttribArray(attribute_coord3d);
}


void free_resources()
{
  glDeleteProgram(program);
}
