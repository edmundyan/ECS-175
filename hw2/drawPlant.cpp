#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
//#include <GLM/glm.h>
#include <glm/glm.hpp>
#include "shader_utils.h"
#include "drawPlant.h"


GLuint program;
GLint attribute_coord2d = 0;
GLint attribute_color = 1;
GLint uniform_matrix;  // pointer to uniform variable


// every vertex is position (2 floats), followed by color (3 floats)
GLfloat leaf_vertices[] = {
  0.0,0.0,   0.5,0.9,0.3,
  0.15,0.09, 0.1 ,0.5,0.1,
  0.20,0.38, 0.4,0.8,0.3,
  0.17,0.42, 0.1,0.5,0.1,
  0.0,0.7,   0.5,0.9,0.3,
  -0.17,0.42,  0.1,0.5,0.3,
  -0.20,0.38,  0.4,0.8,0.3,
  -0.15,0.09,  0.1,0.5,0.1
  };

GLubyte leaf_indicies[] = {
  0, 1, 2, 
  0, 2, 3,
  0, 3, 4, 
  0, 4, 5,
  0, 5, 6,
  0, 6, 7
};

GLfloat stem_height = 0.25;
GLfloat stem_width = stem_height / 5.0;
GLfloat stem_vertices[] = {
  -stem_width/2.0, 0.0,   .87, .72, .53,
  stem_width/2, 0.0,   .87, .72, .53,
  stem_width/2.0, stem_height,   .87, .72, .53,
  -stem_width/2.0, stem_height,   .87, .72, .53
};

GLubyte stem_indicies[] = {
  0, 1, 2, 3
};

// Turn left by pi/6
GLfloat TurnLeft[] = 
  {cos(M_PI/6), -sin(M_PI/6), 0.0, 
   sin(M_PI/6), cos(M_PI/6), 0.0,
   0.0,  0.0, 1.0};

GLfloat Nothing[] = 
  {1.0, 0.0, 0.0, 
   0.0, 1.0, 0.0,
   0.0,  -1.0, 1.0};

GLfloat TranslateStem[] = 
{
  1.0, 0.0, 0.0,
  0.0, 1.0, 0.0,
  0.0, stem_height, 1
};

GLfloat ScaleSmall[] = 
{
  0.2, 0.0, 0.0,
  0.0, 0.2, 0.0,
  0.0, 0.0, 1
};

// Set up the shaders, compile and link them, get pointers to 
// where the shader variables are in GPU memory. 
int init_resources()
{
  // Error flag is initially false
  GLint link_ok = GL_FALSE;
  // Indices for vertex and fragment shaders
  GLuint vs, fs;

  // create_shader is a function in shader_utils that reads in 
  // a vertex or fragment program from a file, creates a shader 
  // object, puts the program into the object, and compiles it.
  // If all goes well, returns 1. 
  vs = create_shader("plant.v.glsl", GL_VERTEX_SHADER);
  if (vs == 0) return 0;

  fs = create_shader("plant.f.glsl", GL_FRAGMENT_SHADER);
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

  // Ask for the index of shader variable coord2d. 
  attribute_coord2d = glGetAttribLocation(program, "coord2d");
  if (attribute_coord2d == -1) {
    fprintf(stderr, "Could not bind attribute coord2d\n");
    return 0;
  }

  // Ask for the index of shader variable coord2d. 
  attribute_color = glGetAttribLocation(program, "v_color");
  if (attribute_color == -1) {
    fprintf(stderr, "Could not bind attribute v_color\n");
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

// draw a leaf at point T
void drawLeaf(GLfloat t[]) {
  // t will be a transformation from the origin 0,0 to the current location.
  // Send the program to the GPU

  glUseProgram(program);

  // Now hook up input data to program.

  // Two attributes for the vertex, position and color.
  // Let OpenGL know we'll use both of them. 
  glEnableVertexAttribArray(attribute_coord2d);
  glEnableVertexAttribArray(attribute_color);

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_coord2d, // attribute ID
    2,                 // number of elements per vertex, here (x,y)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    leaf_vertices  // pointer to first position in the C array
  );

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_color, // attribute ID
    3,                 // number of elements per vertex, here (r,g,b)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    leaf_vertices+2    // pointer to first position index of a color in the C array
  );


  // give the matrix a value
  glUniformMatrix3fv(uniform_matrix, 1, GL_FALSE, t);

  // Send the triangle vertices to the GPU  - actually draw! 
  glDrawElements(GL_TRIANGLES, 7*3, GL_UNSIGNED_BYTE, leaf_indicies);

  // Done with the attributes
  glDisableVertexAttribArray(attribute_coord2d);
  glDisableVertexAttribArray(attribute_color);

}

GLfloat* drawStem(int i, GLfloat t[]) {
  GLfloat cos_theta, sin_theta;

  // t will be a transformation from the origin 0,0 to the current location.
  // Send the program to the GPU

  glUseProgram(program);

  // Now hook up input data to program.

  // Two attributes for the vertex, position and color.
  // Let OpenGL know we'll use both of them. 
  glEnableVertexAttribArray(attribute_coord2d);
  glEnableVertexAttribArray(attribute_color);

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_coord2d, // attribute ID
    2,                 // number of elements per vertex, here (x,y)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    stem_vertices  // pointer to first position in the C array
  );

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_color, // attribute ID
    3,                 // number of elements per vertex, here (r,g,b)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    stem_vertices+2    // pointer to first position index of a color in the C array
  );


  // give the matrix a value
  glUniformMatrix3fv(uniform_matrix, 1, GL_FALSE, t);

  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, stem_indicies);

  // Done with the attributes
  glDisableVertexAttribArray(attribute_coord2d);
  glDisableVertexAttribArray(attribute_color);

  cos_theta = t[0];
  sin_theta = t[1];
  t = matrix_translate(t, -stem_height * sin_theta, stem_height * cos_theta);

  if(i > 0) {
      t = drawStem(i-1, t);
  }
  return t;
}

GLfloat* matrix_translate(GLfloat t[], GLfloat x, GLfloat y) {
  GLfloat *ret = (GLfloat*) malloc(9 * sizeof(GLfloat));
  GLfloat trans[] = 
  {
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    x, y, 1
  };
  matrix_multiply3_3(t, trans, ret);
  //TODO free(t)
  return ret;
}

GLfloat* matrix_rotate(GLfloat t[], GLfloat rad, bool inPlace) {
  // rotate counter clockwise
  GLfloat *ret = (GLfloat*) malloc(9 * sizeof(GLfloat));

  GLfloat x, y;
  x = t[6];
  y = t[7];
  
  // move to origin
  t = matrix_translate(t, -x, -y);

  // rotate
  GLfloat trans[] = 
  {
    cos(rad), sin(rad), 0.0, 
    -sin(rad), cos(rad), 0.0,
    0.0,  0.0, 1.0
  };
  matrix_multiply3_3(t, trans, ret);

  // move back into original position
  ret = matrix_translate(ret, x, y);
  //TODO free(t)
  return ret;
}

void matrix_multiply3_3(GLfloat *a, GLfloat *b, GLfloat *c)
{
	int i, j, k;
	
	for(i = 0; i < 9; i++)
  {
    c[i] = 0.0;
  }

	// want this to be the result: out[i, j] += a[i, k] * b[k, j]
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				c[j * 3 + i] += a[j * 3 + k] * b[k * 3 + i];
}

// Draw the leaf
void drawPlant(int i, GLfloat t[]) {
  
  if(t == NULL) {
    t = Nothing;
  }

  GLfloat temp[9];
  GLfloat *pivot;
  
  if(i == 0) {
    // BASE CASE
    drawLeaf(t);
    printf("LEAF\n");
  } else {
    printf("STEM\n");
    t = drawStem(i, t);

    pivot = t;

    // recurse LEFT branch
    printf("rotate left\n");
    t = matrix_rotate(t, M_PI/6, true);
    drawPlant(i - 1, t);
    t = pivot;

    // recurse RIGHT branch
    printf("rotate right\n");
    t = matrix_rotate(t, -M_PI/6, true);
    drawPlant(i - 1, t);
  } 
  
  /*
  //drawLeaf(Nothing);
  drawStem(t);
  t = matrix_translate(t, 0, stem_height);
  drawLeaf(t);
  t = matrix_rotate(t, M_PI/6);
  drawLeaf(t);*/
}

void free_resources()
{
  glDeleteProgram(program);
}
