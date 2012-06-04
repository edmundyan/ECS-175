#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_utils.h"
#include "tdraw.h"
#include "readObj.h"
#include "readBMP.h"

GLuint program;
GLint attribute_coord3d;
GLint attribute_texcoord;
GLint uniform_proj_matrix;  // pointer to uniform variable - total matrix
GLint uniform_texture;
//GLint attribute_normal;

glm::mat4 projMatrix; // Current total transform

glm::vec4 cameraPosition = glm::vec4(0.0, 0.0, -5.0, 1.0);

Image texImage;
Image floorTexImage;
GLuint textureId;
GLuint floorTextureId;

int size = 4; // number of triangles to draw in scene

GLfloat vertices[] = {
  -4, -4, 0.0, -2.0, -2.0, 
  4, -4, 0.0, 2.0, -2.0, 
  4, 4, 0.0, 2.0, 2.0, 
  -4, 4, 0.0, -2.0, 2.0,

    -20, -4, -25, -10.0, -12.5,
    20, -4, -25, 10.0, -12.5,
    20, -4, 25, 10.0, 12.5, 
    -20, -4, 25, -10.0, 12.5
};


GLubyte elements[] = {
  0, 1, 2,
  2, 3, 0,
  4, 5, 6,
  6, 7, 4
};

// global matricies
// specify by columns

// move cube into box centered at (0,0,-5)
glm::mat4 view  = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
			     glm::vec4(0.0, 1.0, 0.0, 0.0),
			     glm::vec4(0.0, 0.0, 1.0, 0.0),
			     glm::vec4(0.0, 0.0, -5.0, 1.0));

// projection
// n = near plane = -3
// f = far plane = 21*(-3) = -63
glm::mat4 proj = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
			   glm::vec4(0.0, 1.0, 0.0, 0.0),
			   glm::vec4(0.0, 0.0, -22.0/(20*3), -0.33),
			   glm::vec4(0.0, 0.0, -2.0*21/20, 0.0));


// print out matrix by rows
void printMat(glm::mat4  mat){
  int i,j;
  for (j=0; j<4; j++){
    for (i=0; i<4; i++){
    printf("%f ",mat[i][j]);
  }
  printf("\n");
 }
}

void moveCamera(float move) {
  cameraPosition[2] += move;
  view[3] = cameraPosition;
}


// Set up the shader programs, compile and link them, get pointers to 
// where the shader variables are in GPU memory. 
int init_resources()
{
  // Projection matrix
  projMatrix = proj * view;

  // Read in texture images

  // load up the BRICK into GL_TEXTURE0
  int flag = ImageLoad("brick.bmp", &texImage);
  if (flag != 1) {
    printf("Trouble reading image\n");
  }

  glActiveTexture(GL_TEXTURE0); // Load texture into GPU texture unit 0
  glGenTextures(1,&textureId); // Make a texture object
  glBindTexture(GL_TEXTURE_2D, textureId); // Use this object as the 
  // current 2D texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Load the image into GPU texture memory
  glTexImage2D(GL_TEXTURE_2D, // the current 2D texture
	       0, // Mipmap level
	       GL_RGB, //internal format for texture
	       128, // size in s coord
	       128, // size in t coord
	       0, // should always be 0
	       GL_RGB, // incoming data format; should match internal
	       GL_UNSIGNED_BYTE, // type of incoming data
	       texImage.data // pointer to the data
	       );


  // load up the WOOD texture into GL_TEXTURE1 position
  flag = ImageLoad("wood.bmp", &floorTexImage);
  if (flag != 1) {
    printf("Trouble reading floor image\n");
  }

  glActiveTexture(GL_TEXTURE1); // Load texture into GPU texture unit 1
  glGenTextures(1,&floorTextureId); // Make a texture object
  glBindTexture(GL_TEXTURE_2D, floorTextureId); // Use this object as the 
  // current 2D texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Load the image into GPU texture memory
  glTexImage2D(GL_TEXTURE_2D, // the current 2D texture
	       0, // Mipmap level
	       GL_RGB, //internal format for texture
	       128, // size in s coord
	       128, // size in t coord
	       0, // should always be 0
	       GL_RGB, // incoming data format; should match internal
	       GL_UNSIGNED_BYTE, // type of incoming data
	       floorTexImage.data // pointer to the data
	       );

  // Error flag is initially false
  GLint link_ok = GL_FALSE;
  // Indices for vertex and fragment shaders
  GLuint vs, fs;

  // create_shader is a function in shader_utils that reads in 
  // a vertex or fragment program from a file, creates a shader 
  // object, puts the program into the object, and compiles it.
  // If all goes well, returns 1. 
  vs = create_shader("maze.v.glsl", GL_VERTEX_SHADER);
  if (vs == 0) return 0;

  fs = create_shader("maze.f.glsl", GL_FRAGMENT_SHADER);
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


  // Ask for the index of shader variable texcoord
  attribute_texcoord = glGetAttribLocation(program, "texcoord");
  if (attribute_texcoord == -1) {
    fprintf(stderr, "Could not bind attribute texcoord\n");
    return 0;
  }

  
  uniform_texture = glGetUniformLocation(program, "u_texture");
  if (uniform_texture == -1) {
    fprintf(stderr, "Could not bind uniform u_texture \n");
    return 0;
  }

  // Get linker's index to the uniform variable
  uniform_proj_matrix = glGetUniformLocation(program, "m_proj");
  if (uniform_proj_matrix == -1) {
    fprintf(stderr, "Could not bind uniform variable m_proj \n");
    return 0;
  }

  // If all went well....
  return 1;
}

// Draw the floor and wall
void drawScene(void) {

  // Send the program to the GPU
  glUseProgram(program);

  // Projection matrix
  projMatrix = proj * view;

  // Now hook up input data to program.

  // Two attributes for the vertex, position and texture coordinate
  // Let OpenGL know we'll use both of them. 
  glEnableVertexAttribArray(attribute_coord3d);
  glEnableVertexAttribArray(attribute_texcoord);



  /////////////////////////////////////////////
  // draw the wall(brick)
  /////////////////////////////////////////////

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_coord3d, // attribute ID
    3,                 // number of elements per vertex, here (x,y,z)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    vertices  // pointer to first position in the C array
  );

  glVertexAttribPointer(
    attribute_texcoord, // attribute
    2,                  // number of elements per vertex, (s,t)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    5*sizeof(float),    // stride to next texture element
    vertices+3              // offset of first element
  );

  // Send GPU projection matrix
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix));

  // Tell GPU to use Texture Unit 0
  glUniform1i(uniform_texture, 0);  
  //glUniform1i(uniform_floor_texture, 1);

  // draw the wall....
  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_BYTE, elements);  // draw the wall

  /////////////////////////////////////////////
  // draw the floor(wood)
  /////////////////////////////////////////////

  // Describe the position attribute and where the data is in the array
  glVertexAttribPointer(
    attribute_coord3d, // attribute ID
    3,                 // number of elements per vertex, here (x,y,z)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is, don't normalize
    5*sizeof(float),  // stride between one position and the next
    vertices +  20 // pointer to first position in the C array
  );

  glVertexAttribPointer(
    attribute_texcoord, // attribute
    2,                  // number of elements per vertex, (s,t)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    5*sizeof(float),    // stride to next texture element
    vertices+3 + 20              // offset of first element
  );

  // Send GPU projection matrix
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix));

  // Tell GPU to use Texture Unit 1(wood)
  glUniform1i(uniform_texture, 1);  

  // draw the floor....
  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_BYTE, elements);  // draw the wall



  glDisableVertexAttribArray(attribute_coord3d);
  glDisableVertexAttribArray(attribute_texcoord);
}


void free_resources()
{
  glDeleteProgram(program);
  glDeleteTextures(1,&textureId);
}
