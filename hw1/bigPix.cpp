// Edmund Yan
// 996049162
// ECS 175 S12 - Amenta

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
#include <math.h>
#include <string.h>
#include <algorithm>
#include <vector>
using namespace std;


void readInput(void);

/* dimensions - each pixel is 10x10, and the window is 50x50 pixels */
static int pixelSize= 5;
static int numberOfPixels = 50;

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

int num_vertices = 0;
vector<vertex> verts; 
Image texture_img;
bool texture_mapping = 0;


vertex findLeftEdge(vertex v1, vertex v2) {
  return v1.x < v2.x ? v1 : v2;
}

vertex findRightEdge(vertex v1, vertex v2) {
  return v1.x > v2.x ? v1 : v2;
}

vertex findIntersection(vertex v1, vertex v2, float row) {
  vertex v;

  float p;  // proportion
  p = (row - v2.y) / (v1.y - v2.y);

  v.y = row;
  v.x = (1-p) * v2.x + p * v1.x;
  v.r = (1-p) * v2.r + p * v1.r;
  v.g = (1-p) * v2.g + p * v1.g;
  v.b = (1-p) * v2.b + p * v1.b;
  v.s = (1-p) * v2.s + p * v1.s;
  v.t = (1-p) * v2.t + p * v1.t;
  
  return v;
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
    // same line, take ONE point from vertext_pair[0] and the 3rd point not 
    // being used
    vertext_pair[1][0] = vertext_pair[0][0];
    vertext_pair[1][1] = third_vertex;
    vertex_pair_count++;
  }

  // testing for horizonal lines
  if(v1.y == v2.y && v2.y == row) {
    left_edge = findLeftEdge(v1, v2);
    right_edge = findRightEdge(v1, v2);
  } else if(v1.y == v3.y && v3.y == row) {
    left_edge = findLeftEdge(v1, v3);
    right_edge = findRightEdge(v1, v3);
  } else if(v2.y == v3.y && v3.y == row) {
    left_edge = findLeftEdge(v2, v3);
    right_edge = findRightEdge(v2, v3);
  } else if(vertex_pair_count == 0 && v1.y == row) {  // test for top/bot 
                                                      // vertices
    left_edge = v1;
    right_edge = v1;
  } else if (vertex_pair_count == 0 && v2.y == row) {
    left_edge = v2;
    right_edge = v2;
  } else if (vertex_pair_count == 0 && v3.y == row) {
    left_edge = v3;
    right_edge = v3;
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
  
  readInput();
  int i, j;
  float final_r, final_g, final_b;
  
  for(i = 0; i < num_vertices / 3;i++) {
    // send the three vertices to DrawTriangle()
    DrawTriangle(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);
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
      	printf("'%s' = ", line);
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

