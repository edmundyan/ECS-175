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

vertex findLeftEdge(vertex v1, vertex v2, float row) {
  vertex left_edge;
  
  if(v1.y == v2.y) {
    // they're on the same y-axis
    if(v1.x < v2.x)
        return v1;
    else
        return v2;
  }

  float p;  // proportion
  p = (row - v2.y) / (v1.y - v2.y);

  //float col;
  //col = (1-p) * v2.x + p * v1.x;
  left_edge.y = row;
  left_edge.x = ceil((1-p) * v2.x + p * v1.x);
  left_edge.r = (1-p) * v2.r + p * v1.r;
  left_edge.g = (1-p) * v2.g + p * v1.g;
  left_edge.b = (1-p) * v2.b + p * v1.b;
  left_edge.s = (1-p) * v2.s + p * v1.s;
  left_edge.t = (1-p) * v2.t + p * v1.t;
  
  //printf("==left, row%f = %f\n", row, col);

  // TODO, need to return a vertex object with also the interpolated rgb values at the left edge
  return left_edge;
}

vertex findRightEdge(vertex v1, vertex v2, float row) {
  vertex right_edge;
  
  if(v1.y == v2.y) {
    // they're on the same y-axis
    if(v1.x > v2.x)
        return v1;
    else
        return v2;
  }

  float p;  // proportion
  p = (row - v2.y) / (v1.y - v2.y);

  right_edge.y = row;
  right_edge.x = floor((1-p) * v2.x + p * v1.x);
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
  vertex vl1, vl2, vr1, vr2;
  vertex vertex_x_sorted[3] = {v1, v2, v3}; // temp to hold vertices
  vertex vertex_y_sorted[3] = {v1, v2, v3}; // temp to hold vertices
  float alpha, beta, gamma;
  float bc_v1, bc_v2, bc_v3;  // the barycentric coordintes for point r
  float final_r, final_g, final_b, final_s, final_t;

  // 1. which two sets of vertices are we looking at? Options: ab, bc, cb
  /*
  sortByX(vertex_x_sorted);
  sortByY(vertex_y_sorted);

  // 1. Are we above/below the middle?
  if(row <= vertex_y_sorted[1].y) {
    // below, so vertex_y_sorted[0] will be used for both
    if(vertex_y_sorted[0].x <= vertex_x_sorted[1].x) {
      vl1 = vertex_y_sorted[0];
    }
  } else {

  }

  */





  //  v1.y <= row <= v2.y
  if((min(v1.y, v2.y) < row) && (row < max(v1.y, v2.y))) {
    // then v1-v2 is one of the pairs of vertices we need
    if((min(v1.y, v3.y) < row) && (row < max(v1.y, v3.y))) {
      // then v1-v3 is the other pair of vertices we need
      // out of v1-v2, v1-v3, need to find the left edge one
      if((v1.x + v2.x) < (v1.x + v3.x)) {
        vl1 = v1; vl2 = v2;
        vr1 = v1; vr2 = v3;
      } else {
        vl1 = v1; vl2 = v3;
        vr1 = v1; vr2 = v2;
      }
    } else {
      // then v2-v3 is the other pair of vertices we need
      if((v1.x + v2.x) < (v2.x + v3.x)) {
        vl1 = v2; vl2 = v1;
        vr1 = v2; vr2 = v3;
      } else {
        vl1 = v2; vl2 = v3;
        vr1 = v2; vr2 = v1;
      }
    }
  } else if(row == v1.y) {
    // means v1 should only be used atleast ONCE. The other ones are arbitrary
    if(v1.x <= min(v2.x, v3.x)) {
      // v1 IS the left edge, what is the right edge?
      vl1 = v1; vl2 = v1;
      if((v1.y < min(v2.y, v3.y)) || (v1.y > max(v2.y, v3.y))) {
        vr1 = v1; vr2 = v1; // v1 is the top/bottom corner, so also right edge
      } else {
        vr1 = v2; vr2 = v3; // right edge must be the other two vertices
      }
    } else {
      // v1 IS the right edge
      vr1 = v1; vr2 = v1; 
      if((v1.y < min(v2.y, v3.y)) || (v1.y > max(v2.y, v3.y))) {
        vl1 = v1; vl2 = v1; // v1 is the top/bottom corner, so also left edge
      } else {
        vl1 = v2; vl2 = v3; // left edge must be the other two vertices
      }
    }
  } else if(row == v2.y) {
    // means v2 should only be used atleast ONCE. The other ones are arbitrary
    if(v2.x <= min(v1.x, v3.x)) {
      // v2 IS the left edge
      vl1 = v2; vl2 = v2;
      if((v2.y < min(v1.y, v3.y)) || (v2.y > max(v1.y, v3.y))) {
        vr1 = v2; vr2 = v2; // v2 is the top/bottom corner, so also right edge
      } else {
        vr1 = v1; vr2 = v3; // right edge must be the other two vertices
      }
    } else {
      // v2 IS the right edge
      vr1 = v2; vr2 = v2; 
      if((v2.y < min(v1.y, v3.y)) || (v2.y > max(v1.y, v3.y))) {
        vl1 = v2; vl2 = v2; // v1 is the top/bottom corner, so also left edge
      } else {
        vl1 = v1; vl2 = v3; // left edge must be the other two vertices
      }
    }
  } else {
    // then we KNOW v1-v3 and v2-v3 are the two pairs we are looking for
    if((v1.x + v3.x) < (v2.x + v3.x)) {
      vl1 = v3; vl2 = v1;
      vr1 = v3; vr2 = v2;
    } else {
      vl1 = v3; vl2 = v2;
      vr1 = v3; vr2 = v1;
    }
  }

  left_edge =  findLeftEdge(vl1, vl2, row);
  right_edge =  findRightEdge(vr1, vr2, row);

  alpha = (row - vl2.y) / (vl1.y - vl2.y);
  beta = (row - vr2.y) / (vr1.y - vr2.y);
  for(i = (int)left_edge.x; i < (int)right_edge.x; i++) {
    gamma = ((float)i - left_edge.x) / (right_edge.x - left_edge.x);
    bc_v1 = gamma * (1 - beta); // a
    bc_v2 = (1 - gamma) * alpha + gamma * beta; // b
    bc_v3 = (1 - gamma) * (1 - alpha);  // c
    
    final_r = (bc_v1 * vr2.r + bc_v2 * vr1.r + bc_v3 * vl2.r) / 254;
    final_g = (bc_v1 * vr2.g + bc_v2 * vr1.g + bc_v3 * vl2.g)  / 254;
    final_b = (bc_v1 * vr2.b + bc_v2 * vr1.b + bc_v3 * vl2.b) /  254; 

    final_s = (bc_v1 * vr2.s + bc_v2 * vr1.s + bc_v3 * vl2.s);
    final_t = (bc_v1 * vr2.t + bc_v2 * vr1.t + bc_v3 * vl2.t);

    // method 2, just interpolate through left_edge to right_edge
    final_r = ((1-gamma) * left_edge.r + gamma * right_edge.r) / 254;
    final_g = ((1-gamma) * left_edge.g + gamma * right_edge.g) / 254;
    final_b = ((1-gamma) * left_edge.b + gamma * right_edge.b) / 254;
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

  colorPixel(v1.x, v1.y, 254, 254, 254);
  colorPixel(v2.x, v2.y, 254, 254, 254);
  colorPixel(v3.x, v3.y, 254, 254, 254);
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

