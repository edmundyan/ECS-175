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

#include <iostream>
using namespace std;

#define M_PI       3.14159265358979323846

GLuint program;
GLint attribute_coord3d;
GLint attribute_texcoord;

GLint attribute_normal;	// added attribute for the normals

GLint uniform_proj_matrix;  // pointer to uniform variable - total matrix
GLint uniform_texture;



GLint uniform_normals; 
GLint uniform_floor_normals; // may not need

glm::mat4 projMatrix; // Current total transform
glm::mat4 walleTurtleMatrix; // Current total transform

glm::vec4 cameraPosition = glm::vec4(0.0, 0.0, -5.0, 1.0);

Image texImage;
Image floorTexImage;
GLuint textureId;
GLuint floorTextureId;

int size = 4; // number of triangles to draw in scene


Mesh *mesh;
GLfloat *obj_verts;
GLfloat *obj_normals;
GLfloat *obj_lighting;
GLuint *obj_elements;

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


// Normals for the vertices
// same as the vertices?
GLfloat normals[] = {
	-4, -4, 0.0,
	4, -4, 0.0,
	4, 4, 0.0,
	-4, 4, 0.0,
	
	-20, -4, -25,
	20, -4, -25,
	20, -4, 25,
	-20, -4, 25
	
};

GLubyte elements[] = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4
};


// Rotation Matrices
// returns the rotation matrix depending on the axis specified
 
// global matricies
// specify by columns


// move cube into box centered at (0,0,-10)
glm::mat4 view = 
  glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
  glm::vec4(0.0, 1.0, 0.0, 0.0),
  glm::vec4(0.0, 0.0, 1.0, 0.0),
  glm::vec4(0.0, 0.0, -5.0, 1.0));

// projection
// n = near plane = -3
// f = far plane = 21*(-3) = -63
glm::mat4 proj = 
  glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
  glm::vec4(0.0, 1.0, 0.0, 0.0),
  glm::vec4(0.0, 0.0, -22.0/(20*3), -0.33),
  glm::vec4(0.0, 0.0, -2.0*21/20, 0.0));





// walle initial turtle matrix:
glm::mat4 rot_y = 
  glm::mat4(glm::vec4(cos(M_PI/8), 0.0, sin(M_PI/8), 0.0),
  glm::vec4(0.0, 1.0, 0.0, 0.0),
  glm::vec4(-sin(M_PI/8), 0.0, cos(M_PI/8), 0.0),
  glm::vec4(0.0, 0.0, 0.0, 1.0));

// Rotate around x axis
glm::mat4 rot_x = 
  glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
  glm::vec4(0.0, cos(M_PI/3), sin(M_PI/3), 0.0),
  glm::vec4(0.0, -sin(M_PI/3), cos(M_PI/3), 0.0),
  glm::vec4(0.0, 0.0, 0.0, 1.0));

// Rotate around the X-axis
glm::mat4 rotate_around_X(float angle)
{
	glm::mat4 rotationMatrix = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), 
										 glm::vec4(0.0, cos(angle), sin(angle), 0.0),
										 glm::vec4(0.0, -sin(angle), cos(angle), 0.0),
										 glm::vec4(0.0, 0.0, 0.0, 1.0));					 
	return rotationMatrix;
}

// Rotate around the Y-axis
glm::mat4 rotate_around_Y(float angle)
{
	glm::mat4 rotationMatrix = glm::mat4(glm::vec4(cos(angle), 0.0, sin(angle), 0.0),
										 glm::vec4(0.0, 1.0, 0.0, 0.0),
										 glm::vec4(-sin(angle), 0.0, cos(angle), 0.0),
										 glm::vec4(0.0, 0.0, 0.0, 1.0));
	return rotationMatrix;
}

// Rotate around the Z-axis
glm::mat4 rotate_around_Z(float angle)
{
	glm::mat4 rotationMatrix = glm::mat4(glm::vec4(cos(angle), sin(angle), 0.0, 0.0),
										 glm::vec4(-sin(angle), cos(angle), 0.0, 0.0),
										 glm::vec4(0.0, 0.0, 1.0, 0.0),
										 glm::vec4(0.0, 0.0, 0.0, 1.0));
	return rotationMatrix;
}


// Translation Matrix
glm::mat4 translate(float x, float y, float z) 
{
	glm::mat4 translationMatrix = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
											glm::vec4(0.0, 1.0, 0.0, 0.0),
											glm::vec4(0.0, 0.0, 1.0, 0.0),
											glm::vec4(x, y, z, 1.0));
	return translationMatrix;
}

// Scaling Matrix
glm::mat4 scaleMatrix(float factor) 
{
	glm::mat4 scaleMat = glm::mat4(glm::vec4(factor, 0.0, 0.0, 0.0),
								   glm::vec4(0.0, factor, 0.0, 0.0),
								   glm::vec4(0.0, 0.0, factor, 0.0),
								   glm::vec4(0.0, 0.0, 0.0, 1.0));
	return scaleMat;
}




// print out matrix by rows
void printMat(glm::mat4  mat)
{
	int i,j;
	for (j=0; j<4; j++) {
		for (i=0; i<4; i++) {
			printf("%f ",mat[i][j]);
		}
		printf("\n");
	}
}

// moves the camera view forward or backwards
void moveCamera(float move) 
{
	cameraPosition[2] += move;
	view[3] = cameraPosition;
}

// moves the camera view Left or Right (strafe)
void moveCameraX(float move)
{
  cameraPosition[0] += move;
  view[3] = cameraPosition;
}

// pans the camera view left or right
void panCamera(float angle)
{
	cameraPosition = rotate_around_Y(angle) * cameraPosition;
	view = rotate_around_Y(angle) * view;
}


// Set up the shader programs, compile and link them, get pointers to 
// where the shader variables are in GPU memory. 
int init_resources()
{
  // Projection matrix
  projMatrix = proj * view;

  // Read in texture images

  // load up the BRICK into GL_TEXTURE0
  int flag = ImageLoad("./brick.bmp", &texImage);
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
  flag = ImageLoad("./wood.bmp", &floorTexImage);
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


	// for the normals
	attribute_normal = glGetAttribLocation(program, "normal");
	if (attribute_normal == -1) {
		fprintf(stderr, "Could not bind attribute normal\n");
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

	// for the normal_transform
	uniform_normals = glGetUniformLocation(program, "normal_transform");
	if (uniform_normals == -1) {
		fprintf(stderr, "Could not bind uniform variable normal_transform\n");
		return 0;
	}
	/*
	// for the normal_floor_transform
	uniform_floor_normals = glGetUniformLocation(program, "normal_floor_transform");
	if (uniform_floor_normals == -1) {
		fprintf(stderr, "Could not bind uniform variable normal_floor_transform\n");
		return 0;
	}
	*/





  /////////////////////////////////////////////
  // Load up .obj files
  /////////////////////////////////////////////
  // READING IN .OBJ FILES
  mesh = new Mesh;
  load_obj("./walle1.obj", mesh);

  int size;
  float max_x, max_y, max_z, min_x, min_y, min_z, len_x, len_y, len_z, mid_x, mid_y, mid_z;
  max_x = max_y = max_z = FLT_MIN;
  min_x = min_y = min_z = FLT_MAX;

  // initialize vertices
  size = mesh->vertices.size();
  obj_verts = new GLfloat[size * 3];
  for (int i = 0; i < size; i++)
  {
    // assign
    obj_verts[(i*3)+0] = mesh->vertices[i][0];
    obj_verts[(i*3)+1] = mesh->vertices[i][1];
    obj_verts[(i*3)+2] = mesh->vertices[i][2];

    // get the max/min of x/y/z
    max_x = max(max_x, obj_verts[i*3 + 0]);
    max_y = max(max_y, obj_verts[i*3 + 1]);
    max_z = max(max_z, obj_verts[i*3 + 2]);

    min_x = min(min_x, obj_verts[i*3 + 0]);
    min_y = min(min_y, obj_verts[i*3 + 1]);
    min_z = min(min_z, obj_verts[i*3 + 2]);
  }

  // initialize elements aka each triangle.
  size = mesh->elements.size();
  obj_elements = new GLuint[size];
  for (int i = 0; i < size / 3; i++)
  {
    // copy the 3 vertex indices for each triangle
    obj_elements[i * 3 + 0] = mesh->elements[i * 3 + 0];
    obj_elements[i * 3 + 1] = mesh->elements[i * 3 + 1];
    obj_elements[i * 3 + 2] = mesh->elements[i * 3 + 2];
  }


  // initialize normals
  size = mesh->normals.size();
  obj_normals = new GLfloat[size * 3];
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      obj_normals[(i*3)+j] = mesh->normals[i][j];
    }
  }

  // initialize lighting 1, a WHITE light
  size = mesh->vertices.size(); // may not need
  obj_lighting = new GLfloat[size * 3];
  for (int i = 0; i < size; i++)
  {
    obj_lighting[i * 3 + 0] = 1.0;
    obj_lighting[i * 3 + 1] = 1.0;
    obj_lighting[i * 3 + 2] = 1.0;
  }


  // find the ranges of x,y,z
  len_x = max_x - min_x;
  len_y = max_y - min_y;
  len_z = max_z - min_z;
  // translate model to origin
  mid_x = min_x + len_x/2.0; // origin of model
  mid_y = min_y + len_y/2.0; // origin of model
  mid_z = min_z + len_z/2.0; // origin of model

  // scale everything to fit into the (-1,-1,-1)-(1,1,1) cube
  float max_dim = max(len_x, max(len_y, len_z)); // the largest dimension will be what we scale to
  float scale = (2 / max_dim);
  
  // move to origin
  for (int i = 0; i < size; i++)
  {
    obj_verts[i*3 + 0] = (obj_verts[i*3 + 0] - mid_x);
    obj_verts[i*3 + 1] = (obj_verts[i*3 + 1] - mid_y);
    obj_verts[i*3 + 2] = (obj_verts[i*3 + 2] - mid_z);
  }

  // scale
  for (int i = 0; i < size; i++)
  {
    obj_verts[i*3 + 0] = (obj_verts[i*3 + 0] * scale);
    obj_verts[i*3 + 1] = (obj_verts[i*3 + 1] * scale);
    obj_verts[i*3 + 2] = (obj_verts[i*3 + 2] * scale);
  }


  // initial walle position
  walleTurtleMatrix = translate(5,0,0)* rot_x * rot_y;

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
  glEnableVertexAttribArray(attribute_normal);


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

	// for the normals
	glVertexAttribPointer(
		attribute_normal, // attribute
		3,                  // number of elements per vertex, (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		3*sizeof(float),    // stride to next texture element
		normals              // offset of first element
	);



  // Send GPU projection matrix
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix));
  glUniformMatrix4fv(uniform_normals, 1, GL_FALSE, glm::value_ptr(projMatrix));
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
    vertices + 20 // pointer to first position in the C array
  );

  glVertexAttribPointer(
    attribute_texcoord, // attribute
    2,                  // number of elements per vertex, (s,t)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    5*sizeof(float),    // stride to next texture element
    vertices+3 + 20              // offset of first element
  );
  
	
	// for the normals
	glVertexAttribPointer(
		attribute_normal, // attribute
		3,                  // number of elements per vertex, (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		3*sizeof(float),    // stride to next texture element
		normals              // offset of first element
	);
	
  
  // Send GPU projection matrix
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix ));
  glUniformMatrix4fv(uniform_normals, 1, GL_FALSE, glm::value_ptr(projMatrix));
  // Tell GPU to use Texture Unit 1(wood)
  glUniform1i(uniform_texture, 1);  

  // draw the floor....
  glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_BYTE, elements);  // draw the wall
  

  ///////////////////////////////
  // wall-e
  ///////////////////////////////
  glVertexAttribPointer(
    attribute_coord3d, // attribute ID
    3, // number of elements per vertex, here (x,y,z)
    GL_FLOAT, // the type of each element
    GL_FALSE, // take our values as-is, don't normalize
    3*sizeof(float), // stride between one position and the next
    obj_verts
  );

  // wtf this is dumb
  glVertexAttribPointer(
    attribute_texcoord, // attribute
    2,                  // number of elements per vertex, (s,t)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    2*sizeof(float),    // stride to next texture element
    obj_verts              // offset of first element
  );

	// FOR NORMALS
	// Describe the position attribute and where the data is in the array
	glVertexAttribPointer(
		attribute_normal, // attribute ID
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is, don't normalize
		3*sizeof(float),  // stride between one position and the next
		obj_normals
	);

  // CENTER WALLE (NON ANIMATION)
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniformMatrix4fv(uniform_normals, 1, GL_FALSE, glm::value_ptr(projMatrix));
  glDrawElements(GL_TRIANGLES, mesh->elements.size(), GL_UNSIGNED_INT, obj_elements);

  // ORVITING WALLE (ANIMATION)
  glUniformMatrix4fv(uniform_proj_matrix, 1, GL_FALSE, glm::value_ptr(projMatrix * walleTurtleMatrix));
	glUniformMatrix4fv(uniform_normals, 1, GL_FALSE, glm::value_ptr(projMatrix * walleTurtleMatrix));
  glDrawElements(GL_TRIANGLES, mesh->elements.size(), GL_UNSIGNED_INT, obj_elements);


  glDisableVertexAttribArray(attribute_coord3d);
  glDisableVertexAttribArray(attribute_texcoord);
  glDisableVertexAttribArray(attribute_normal);
}

void Timer(int extra)
{
    walleTurtleMatrix = walleTurtleMatrix * rotate_around_Z(.1); // rotate in place
    walleTurtleMatrix = rotate_around_Y(.1) * walleTurtleMatrix; // rotate around origin
    // move Walle object
    //moveCameraX(0.2);
    glutPostRedisplay();
    glutTimerFunc(1000,Timer,0);
 
}

void free_resources()
{
	glDeleteProgram(program);
	glDeleteTextures(1,&textureId);
}
