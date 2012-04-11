/* 
lowlevel.h - constants, macros, data structures 
and function defs for faux low-level graphics system.
*/

/* definitions of public functions */

bool initBigPixelArray(int, int, int);
void drawBigPixelArray(void);
void colorPixel(int, int, GLfloat, GLfloat, GLfloat);
void free_resources(void);

