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

struct Vertex {
  float x;
  float y;
} ;

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

GLfloat leaf_bezier_root[] = {
  0.0,0.0,   0.5,0.9,0.3,
  0.15,0.35, 0.4,0.8,0.3,
  0.0,0.40, 0.5,0.9,0.3,
  -0.15,0.35, 0.4,0.8,0.3,
};

GLubyte leaf_indicies_bezier_root[] = {
  0, 1, 2, 
  0, 2, 3,
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
{
  1.0, 0.0, 0.0, 
  0.0, 1.0, 0.0,
  0.0,  -1.0, 1.0
};

GLfloat ScaleDown[] = 
{
  0.75, 0.0, 0.0, 
  0.0, 0.75, 0.0,
  0.0,  0.0, 1.0
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

GLfloat* makeStemObj(GLfloat stem_height, GLfloat stem_width) {
  /*GLfloat* stem_vertices = new GLfloat[15] {
    -stem_width/2.0, 0.0,   .87, .72, .53,
    stem_width/2, 0.0,   .87, .72, .53,
    stem_width/2.0, stem_height,   .87, .72, .53,
    -stem_width/2.0, stem_height,   .87, .72, .53
  };*/
  
  GLfloat stem_vertices[] = {
    -stem_width/2.0, 0.0,   .87, .72, .53,
    stem_width/2, 0.0,   .87, .72, .53,
    stem_width/2.0, stem_height,   .87, .72, .53,
    -stem_width/2.0, stem_height,   .87, .72, .53
  };

  GLfloat* stem_vertices_dyn = new GLfloat[20];

  memcpy(stem_vertices_dyn, stem_vertices, 20 * sizeof(GLfloat));


  /*
  stem_vertices[0] = -stem_width/2.0;
  stem_vertices[1] = -stem_width/2.0;
  stem_vertices[2] = -stem_width/2.0;
  stem_vertices[3] = -stem_width/2.0;
  stem_vertices[4] = -stem_width/2.0;
  stem_vertices[5] = -stem_width/2.0;
  stem_vertices[6] = -stem_width/2.0;
  stem_vertices[7] = -stem_width/2.0;
  stem_vertices[8] = -stem_width/2.0;
  
  {
    -stem_width/2.0, 0.0,   .87, .72, .53,
    stem_width/2, 0.0,   .87, .72, .53,
    stem_width/2.0, stem_height,   .87, .72, .53,
    -stem_width/2.0, stem_height,   .87, .72, .53
  };*/

  return stem_vertices_dyn;
}

void drawLeafBezier(GLfloat t[]) {
  // 1. generate leaf_vertices and leaf_indices using bezier curves
  
  int n = 20; // number of segments
  Vertex a, b, c, r;

  a.x = leaf_bezier_root[0];  // 0
  a.y = leaf_bezier_root[1];
  b.x = leaf_bezier_root[5];  // 1
  b.y = leaf_bezier_root[6];
  c.x = leaf_bezier_root[10]; // 2
  c.y = leaf_bezier_root[11];

  
  GLfloat *leaf_bezier = new GLfloat[(n+1) * 5 * 2];
  GLubyte *leaf_indicies_bezier =new GLubyte[(n) * 3 * 2];

  int i;
  for(i = 0; i < n; i++) {
    //double percent = 0.0; percent <= 1.0; percent += 1.0/n

    double percent = (double)i/n;
    r.x = (1-percent*percent)*a.x + 2*percent*(1-percent)*b.x + percent*percent*c.x;
    r.y = (1-percent*percent)*a.y + 2*percent*(1-percent)*b.y + percent*percent*c.y;

    leaf_bezier[i * 5] = r.x;
    leaf_bezier[i * 5 + 1] = r.y;
    // printf("%d:(%f, %f) pers:%f\n", i, r.x, r.y, percent);
    // rgb
    leaf_bezier[i * 5 + 2] = 0.5;
    leaf_bezier[i * 5 + 3] = 0.8;
    leaf_bezier[i * 5 + 4] = 0.3;
  } 
  


  // LEFT SIDE

  // the other side of the leaf
  a.x = leaf_bezier_root[0];  // 0
  a.y = leaf_bezier_root[1];
  b.x = leaf_bezier_root[15];  // 2
  b.y = leaf_bezier_root[16];
  c.x = leaf_bezier_root[10]; // 3
  c.y = leaf_bezier_root[11];

  
  for(i = i; i < 2*n; i++) {
    double percent = 1 - (double)(i - n)/(n);
    r.x = (1-percent*percent)*a.x + 2*percent*(1-percent)*b.x + percent*percent*c.x;
    r.y = (1-percent*percent)*a.y + 2*percent*(1-percent)*b.y + percent*percent*c.y;

    leaf_bezier[i * 5] = r.x;
    leaf_bezier[i * 5 + 1] = r.y;
    //printf("%d:(%f, %f) pers:%f\n", i, r.x, r.y, percent);
    // rgb
    leaf_bezier[i * 5 + 2] = 0.5;
    //leaf_bezier[i * 5 + 3] = 0.8;
    leaf_bezier[i * 5 + 3] = percent;
    leaf_bezier[i * 5 + 4] = 0.3;
  }

  for(int i = 0; i < (n-1) * 2; i++) {
    leaf_indicies_bezier[i * 3] = 0;
    leaf_indicies_bezier[i * 3 + 1] = i + 1;
    leaf_indicies_bezier[i * 3 + 2] = i + 2;
    //printf("(%d, %d, %d)\n", 0, i+1, i+2);
  }



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
    leaf_bezier  // pointer to first position in the C array
  );

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_color, // attribute ID
    3,                 // number of elements per vertex, here (r,g,b)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    leaf_bezier+2    // pointer to first position index of a color in the C array
  );


  // give the matrix a value
  glUniformMatrix3fv(uniform_matrix, 1, GL_FALSE, t);

  // Send the triangle vertices to the GPU  - actually draw! 
  // NOTE: It's not exactly n*3*2 b/c the "right" side has (n-1)*3 triangles, whereas the "left side" has n*3 triangles b/c it repeat the middle twice
  glDrawElements(GL_TRIANGLES, (n-1)*3 * 2, GL_UNSIGNED_BYTE, leaf_indicies_bezier);

  // Done with the attributes
  glDisableVertexAttribArray(attribute_coord2d);
  glDisableVertexAttribArray(attribute_color);

  free(leaf_indicies_bezier);
  free(leaf_bezier);
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
  // t will be a transformation from the origin 0,0 to the current location.
  // Send the program to the GPU

  glUseProgram(program);

  // Now hook up input data to program.

  // Two attributes for the vertex, position and color.
  // Let OpenGL know we'll use both of them. 
  glEnableVertexAttribArray(attribute_coord2d);
  glEnableVertexAttribArray(attribute_color);
  
  
  GLfloat stem_height_base = 0.15;
  double r = (double)rand() / (double)RAND_MAX;
  GLfloat stem_height = (stem_height_base * r + 0.15) * i;  // (0.15 - 0.30)
  GLfloat stem_width = stem_height_base / 5.0 * (1);
  GLfloat *stem_vertices = makeStemObj(stem_height, stem_width);

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

  GLfloat cos_theta, sin_theta;
  cos_theta = t[0];
  sin_theta = t[1];
  t = matrix_translate(t, -stem_height * sin_theta, stem_height * cos_theta);

  if(i > 0) {
      //t = drawStem(i-1, t);
  }
  return t;
}


void drawBgObjects() {
  int n = 100; // number of segments
  Vertex a, b, c, d, r;


  GLfloat sun_vertices_root[] = {
    -0.25, 0.0,   .87, .72, .53,
    0.0, 0.0,   .87, .72, .53,
    0.0, 0.5,   .87, .72, .53,
    -0.25, 0.5,   .87, .72, .53,
    0.25, 0.0,   .87, .72, .53,
    0.25, 0.5,   .87, .72, .53
  };

  GLfloat *sun_bezier = new GLfloat[(n+1) * 5  *2];
  GLubyte *sun_indices_bezier = new GLubyte[(n) * 3 * 2];


  
  a.x = sun_vertices_root[5];  // p0
  a.y = sun_vertices_root[6];
  b.x = sun_vertices_root[0];  // p1
  b.y = sun_vertices_root[1];
  c.x = sun_vertices_root[15]; // p2
  c.y = sun_vertices_root[16];
  d.x = sun_vertices_root[10]; // p3
  d.y = sun_vertices_root[11];

  int i = 0;

  
  for(i = 0; i < n; i++) {
    double percent = (double) i / n;
    r.x = pow(1-percent, 3) * a.x + 3*pow(1-percent, 2)*percent*b.x + 3*(1-percent)*pow(percent,2)*c.x + pow(percent, 3)*d.x;
    r.y = pow(1-percent, 3) * a.y + 3*pow(1-percent, 2)*percent*b.y + 3*(1-percent)*pow(percent,2)*c.y + pow(percent, 3)*d.y;

    sun_bezier[i * 5] = r.x;
    sun_bezier[i * 5 + 1] = r.y;

    sun_bezier[i * 5 + 2] = 255.0/255.0;
    sun_bezier[i * 5 + 3] = 212.0/255.0;
    sun_bezier[i * 5 + 4] = 19.0/255.0;
  }


  
  a.x = sun_vertices_root[5];  // p0
  a.y = sun_vertices_root[6];
  b.x = sun_vertices_root[20];  // p1
  b.y = sun_vertices_root[21];
  c.x = sun_vertices_root[25]; // p2
  c.y = sun_vertices_root[26];
  d.x = sun_vertices_root[10]; // p3
  d.y = sun_vertices_root[11];



  for(; i < n * 2; i++) {
    double percent = 1 - ((double) (i - n) / n);
    r.x = pow(1-percent, 3) * a.x + 3*pow(1-percent, 2)*percent*b.x + 3*(1-percent)*pow(percent,2)*c.x + pow(percent, 3)*d.x;
    r.y = pow(1-percent, 3) * a.y + 3*pow(1-percent, 2)*percent*b.y + 3*(1-percent)*pow(percent,2)*c.y + pow(percent, 3)*d.y;

    sun_bezier[i * 5] = r.x;
    sun_bezier[i * 5 + 1] = r.y;

    sun_bezier[i * 5 + 2] = 255.0/255.0;
    sun_bezier[i * 5 + 3] = 212.0/255.0;
    sun_bezier[i * 5 + 4] = 19.0/255.0;
  }


  printf("Sun:----------------\n");
  for(int i = 0; i < n * 2; i++) {
    printf("(%f, %f)\n", sun_bezier[i * 5], sun_bezier[i * 5 + 1]);
  }

  

  for(int i = 0; i < n * 2; i++) {
      sun_indices_bezier[i * 3] = 0;
      sun_indices_bezier[i * 3 + 1] = i + 1;
      sun_indices_bezier[i * 3 + 2] = i + 2;
  }


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
    sun_bezier  // pointer to first position in the C array
  );

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_color, // attribute ID
    3,                 // number of elements per vertex, here (r,g,b)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    sun_bezier+2    // pointer to first position index of a color in the C array
  );


  // give the matrix a value
  GLfloat *temp;
  temp = matrix_translate(Nothing, -0.5, 1.5);
  glUniformMatrix3fv(uniform_matrix, 1, GL_FALSE, temp);

  glDrawElements(GL_TRIANGLES, (n-1) * 3 * 2, GL_UNSIGNED_BYTE, sun_indices_bezier);

  // Done with the attributes
  glDisableVertexAttribArray(attribute_coord2d);
  glDisableVertexAttribArray(attribute_color);
}


void drawBackground() {
  int n = 255; // number of segments
  Vertex a, b, c, d, r;


  // NOTE, left/right edges are beyond the [-1,1] range
  GLfloat floor_vertices_root[] = {
    -1.5, 0.0,   .87, .72, .53,
    -0.5, 0.7,   .87, .72, .53,
    0.5, 0.0,   .87, .72, .53,
    1.5, 0.3,   .87, .72, .53
  };

  GLfloat *floor_bezier = new GLfloat[(n+1) * 5];
  GLubyte *floor_indices_bezier = new GLubyte[(n) * 3];


  // the other side of the leaf
  a.x = floor_vertices_root[0];  // p0
  a.y = floor_vertices_root[1];
  b.x = floor_vertices_root[5];  // p3
  b.y = floor_vertices_root[6];
  c.x = floor_vertices_root[10]; // p2
  c.y = floor_vertices_root[11];
  d.x = floor_vertices_root[15]; // p1
  d.y = floor_vertices_root[16];

  int i = 0;


  for(i = 0; i < (n ); i++) {
    double percent = (double) i / n;
    r.x = pow(1-percent, 3) * a.x + 3*pow(1-percent, 2)*percent*b.x + 3*(1-percent)*pow(percent,2)*c.x + pow(percent, 3)*d.x;
    r.y = pow(1-percent, 3) * a.y + 3*pow(1-percent, 2)*percent*b.y + 3*(1-percent)*pow(percent,2)*c.y + pow(percent, 3)*d.y;

    floor_bezier[i * 5] = r.x;
    floor_bezier[i * 5 + 1] = r.y;

    floor_bezier[i * 5 + 2] = 238.0/255.0;
    floor_bezier[i * 5 + 3] = 168.0/255.0;
    floor_bezier[i * 5 + 4] = 36.0/255.0;
  }

  // last triangle will cover the bottom-right corner.

  floor_bezier[i * 5] = 1;
  floor_bezier[i * 5 + 1] = 0;


  floor_bezier[i * 5 + 2] = 238.0/255.0;
  floor_bezier[i * 5 + 3] = 168.0/255.0;
  floor_bezier[i * 5 + 4] = 36.0/255.0;



  for(int i = 0; i < (n + 1); i++) {
    printf("(%f, %f)\n", floor_bezier[i * 5], floor_bezier[i * 5 + 1]);
  }

  printf("----------------\n");

  for(int i = 0; i < n; i++) {
      floor_indices_bezier[i * 3] = 0;
      floor_indices_bezier[i * 3 + 1] = i + 1;
      floor_indices_bezier[i * 3 + 2] = i + 2;
  }


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
    floor_bezier  // pointer to first position in the C array
  );

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_color, // attribute ID
    3,                 // number of elements per vertex, here (r,g,b)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    floor_bezier+2    // pointer to first position index of a color in the C array
  );


  // give the matrix a value
  glUniformMatrix3fv(uniform_matrix, 1, GL_FALSE, Nothing);

  glDrawElements(GL_TRIANGLES, (n-1) * 3, GL_UNSIGNED_BYTE, floor_indices_bezier);

  // Done with the attributes
  glDisableVertexAttribArray(attribute_coord2d);
  glDisableVertexAttribArray(attribute_color);
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

  GLfloat temp[9];
  GLfloat *pivot;
  double rotate_angle;
  double r;
  if(t == NULL) {
    t = Nothing;
  }


  if(i <= 0) {
    // BASE CASE
    //drawLeaf(t);
    drawLeafBezier(t);
    
  } else {
    matrix_multiply3_3(ScaleDown, t , temp);
    t = temp;

    
    t = drawStem(i, t);
    pivot = t;

    drawPlant(i - 1, t);

    // recurse LEFT branch
    // rotate between PI/8 to PI/4
    r = (double)rand() / (double)RAND_MAX;
    rotate_angle = (M_PI / 8) + r * M_PI / 8;
    t = matrix_rotate(t, rotate_angle, true);
    drawPlant(i - 1, t);
    t = pivot;

    // recurse RIGHT branch
    r = (double)rand() / (double)RAND_MAX;
    rotate_angle = (M_PI / 8) + r * M_PI / 8;
    t = matrix_rotate(t, -rotate_angle, true);
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
