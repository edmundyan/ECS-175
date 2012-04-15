/*
 *  bigPix.cpp
 *  Draws a 50x50 array of big pixels, for practice in low-level graphics
 */

#include <stdio.h>
#include <stdlib.h>
#include "shader_utils.h"
#include <GL/glut.h>
#include "lowlevel.h"        /* faux framebuffer definitions */
#include "readBMP.h"
#include <algorithm>
#include <vector>
using namespace std;


void readInput(void);

/* dimensions - each pixel is 10x10, and the window is 50x50 pixels */
static int pixelSize= 1;
static int numberOfPixels = 350;

// GLOBALS //
struct vertex {
  float x;
  float y;
  float r;
  float g;
  float b;
  float s;
  float t;
} ;

struct triangle {
  vertex v[3];
} ;

int num_vertices = 0;
vector<vertex> verts; 
Image texture_img;
bool texture_mapping = 0;

int flip_color = 0;

vertex findLeftEdge(vertex v1, vertex v2) {
  return v1.x < v2.x ? v1 : v2;
}

vertex findRightEdge(vertex v1, vertex v2) {
  return v1.x > v2.x ? v1 : v2;
}

vertex findIntersection(vertex v1, vertex v2, float row) {
  vertex right_edge;

  float p;  // proportion
  p = (row - v2.y) / (v1.y - v2.y);

  right_edge.y = row;
  right_edge.x = (1-p) * v2.x + p * v1.x;
  right_edge.r = (1-p) * v2.r + p * v1.r;
  right_edge.g = (1-p) * v2.g + p * v1.g;
  right_edge.b = (1-p) * v2.b + p * v1.b;
  right_edge.s = (1-p) * v2.s + p * v1.s;
  right_edge.t = (1-p) * v2.t + p * v1.t;

  //printf("~right, row%f = %f\n", row, col);
  return right_edge;
}

void sortByY(vertex *arr) {
  vertex temp;
  // sorting 3 vertex objects by Y value
  if(arr[1].y < arr[0].y) {
    // swap
    temp = arr[0];
    arr[0] = arr[1];
    arr[1] = temp;
  }
  if(arr[2].y < arr[1].y) {
    // swap
    temp = arr[1];
    arr[1] = arr[2];
    arr[2] = temp;
  }
  if(arr[1].y < arr[0].y) {
    // swap
    temp = arr[0];
    arr[0] = arr[1];
    arr[1] = temp;
  }
}

void sortByX(vertex *arr) {
  vertex temp;
  // sorting 3 vertex objects by Y value
  if(arr[1].x < arr[0].x) {
    // swap
    temp = arr[0];
    arr[0] = arr[1];
    arr[1] = temp;
  }
  if(arr[2].x < arr[1].x) {
    // swap
    temp = arr[1];
    arr[1] = arr[2];
    arr[2] = temp;
  }
  if(arr[1].x < arr[0].x) {
    // swap
    temp = arr[0];
    arr[0] = arr[1];
    arr[1] = temp;
  }
}

void fillRow(vertex v1, vertex v2, vertex v3, float row) {
  int i;
  
  // find left/right edge points
  vertex left_edge;
  vertex right_edge;
  vertex temp;
  vertex vl1, vl2, vr1, vr2;
  vertex vertext_pair[2][2];
  vertex third_vertex;
  int vertex_pair_count = 0;
  int vertex_pair_combo = 0;
  float gamma;
  float bc_v1, bc_v2, bc_v3;  // the barycentric coordintes for point r
  float final_r, final_g, final_b, final_s, final_t;


  
  if((min(v1.y, v2.y) < row) && (row < max(v1.y, v2.y))) {
    vertext_pair[vertex_pair_count][0]  = v1;
    vertext_pair[vertex_pair_count][1]  = v2;
    vertex_pair_count++;
    vertex_pair_combo += 1;
    third_vertex = v3;
  }
  if((min(v1.y, v3.y) < row) && (row < max(v1.y, v3.y))) {
    vertext_pair[vertex_pair_count][0]  = v1;
    vertext_pair[vertex_pair_count][1]  = v3;
    third_vertex = v2;
    vertex_pair_count++;
    vertex_pair_combo += 2;
  }
  if((min(v2.y, v3.y) < row) && (row < max(v2.y, v3.y))) {
    vertext_pair[vertex_pair_count][0]  = v2;
    vertext_pair[vertex_pair_count][1]  = v3;
    third_vertex = v1;
    vertex_pair_count++;
    vertex_pair_combo += 4;
  }
  if(vertex_pair_count == 1) {
    // same line, take ONE point from vertext_pair[0] and the 3rd point not being used
    vertext_pair[1][0] = vertext_pair[0][0];
    vertext_pair[1][1] = third_vertex;
    vertex_pair_count++;
  } else if(vertex_pair_count == 0) {
    // dummy?
    vertext_pair[0][0] = v1;
    vertext_pair[0][0] = v1;
  }

  if(v1.y == v2.y && v2.y == row) {
    left_edge = findLeftEdge(v1, v2);
    right_edge = findRightEdge(v1, v2);
  } else if(v1.y == v3.y && v3.y == row) {
    left_edge = findLeftEdge(v1, v3);
    right_edge = findRightEdge(v1, v3);
  } else if(v2.y == v3.y && v3.y == row) {
    left_edge = findLeftEdge(v2, v3);
    right_edge = findRightEdge(v2, v3);
  } else {
    left_edge = findIntersection(vertext_pair[0][0], vertext_pair[0][1], row);
    right_edge = findIntersection(vertext_pair[1][0], vertext_pair[1][1], row);
  }

  if(left_edge.x > right_edge.x) {
    temp = left_edge;
    left_edge = right_edge;
    right_edge = temp;
  }

  left_edge.x = ceil(left_edge.x);
  right_edge.x = floor(right_edge.x);


  for(i = (int)left_edge.x; i <= (int)right_edge.x; i++) {
    if(right_edge.x == left_edge.x) {
      gamma = 0;
    } else {
      gamma = ((float)i - left_edge.x) / (right_edge.x - left_edge.x);
    }

    // method 2, just interpolate through left_edge to right_edge
    final_r = ((1-gamma) * left_edge.r + gamma * right_edge.r) / 255;
    final_g = ((1-gamma) * left_edge.g + gamma * right_edge.g) / 255;
    final_b = ((1-gamma) * left_edge.b + gamma * right_edge.b) / 255;
    final_s = ((1-gamma) * left_edge.s + gamma * right_edge.s);
    final_t = ((1-gamma) * left_edge.t + gamma * right_edge.t);

    if(texture_mapping) {
      // change the rgb
      int x_pixel = (float)texture_img.sizeX * final_s;
      int y_pixel = (float)texture_img.sizeY * final_t;
      final_r = (unsigned char)texture_img.data[(y_pixel * (texture_img.sizeX+1) + x_pixel) * 3] / 255.0;
      final_g = (unsigned char)texture_img.data[(y_pixel * (texture_img.sizeX+1) + x_pixel) * 3 + 1] / 255.0;
      final_b = (unsigned char)texture_img.data[(y_pixel * (texture_img.sizeX+1) + x_pixel) * 3 + 2] / 255.0;
    }
    colorPixel((float)i, row, final_r, final_g, final_b);
  }
}

void DrawTriangle(vertex v1, vertex v2, vertex v3) {
  int i;
  
  // calculate the bot/top_row
  int bot_row = (int)ceil(min(min(v1.y, v2.y), v3.y));
  int top_row = (int)floor(max(max(v1.y, v2.y), v3.y));

  for(i = bot_row; i <= top_row; i++) {
    fillRow(v1, v2, v3, (float)i);
  }

  // color the vertices white
  colorPixel(v1.x, v1.y, 255, 255, 255);
  colorPixel(v2.x, v2.y, 255, 255, 255);
  colorPixel(v3.x, v3.y, 255, 255, 255);
}

/* Call your triangle drawing program from here; this is the 
   function which draws the contents of the framebuffer by 
   coloring pixels. */
void drawContents(void) {
  
  /* The colorPixel function: int x, int y, GLubyte r, GLubyte g, GLubyte b
     x,y = pixel address
     r,g,b = pixel color
  */
  readInput();
  int i, j;
  float final_r, final_g, final_b;
  
  /*
  // COPY THE PICTURE
  for(i = 0; i < numberOfPixels-10; i++) {
    for(j = 0; j < numberOfPixels-10; j++) {
      // change the rgb
      final_r = (unsigned char)texture_img.data[(i * (texture_img.sizeX+1) + j) * 3] / 255.0;
      final_g = (unsigned char)texture_img.data[(i * (texture_img.sizeX+1) + j) * 3 + 1] / 255.0;
      final_b = (unsigned char)texture_img.data[(i * (texture_img.sizeX+1) + j) * 3 + 2] / 255.0;
      colorPixel(j, i, final_r, final_g, final_b);
    }
  }
  return;
  */
  
  for(i = 0; i < num_vertices / 3;i++) {

    // send the three vertices to DrawTriangle()
    DrawTriangle(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);
    flip_color = 1 - flip_color;
  }
}

/* Reads in the triangle vertices and colors from stdin */
void readInput(void) {
  int length = 100;
  char* line;
  int items;
  float x, y, r, g, b, s, t;
  vertex v;

  line = (char*) malloc(length+1);
  if(fgets(line,length,stdin) != NULL) {
    if(strcmp(line, "textured\n") == 0) {
      printf("TEXTURE");
      texture_mapping = 1;
      fgets(line,length,stdin); // get the filename
      line[strlen(line)-1] = 0; // remove ending '\n'
      if(!ImageLoad(line, &texture_img))
        return;
      fgets(line,length,stdin);
    }
    do {
      memset(&v, 0, sizeof(vertex));
      if((items = sscanf(line, "%f %f %f %f %f\n", &x, &y, &r, &g, &b)) == 5) {
        printf("x %f y %f r %f g %f b %f\n", x, y, r, g, b);
        v.x = x;
        v.y = y;
        v.r = r;
        v.g = g;
        v.b = b;
      } else if((items = sscanf(line, "%f %f %f %f\n", &x, &y, &s, &t)) == 4) {
        printf("x %f y %f s %f t %f\n",x,y, s, t);
        v.x = x;
        v.y = y;
        v.s = s;
        v.t = t;
      } else if((items = sscanf(line, "%f %f \n", &x, &y)) == 2) {
        printf("x %f y %f\n",x,y);
        v.x = x;
        v.y = y;
      } else {
        printf("No format found for this line\n");
      }
      verts.insert(verts.begin() + num_vertices, v);
      num_vertices++;
    } while (fgets(line,length,stdin) != NULL);
  }
}

/* Called by glut when window is refreshed */
void display(void)
{

/* Low-level system does it */
  drawContents();
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

