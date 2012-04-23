/* Functions implemented in drawplant.h */

GLfloat* moveOneStem(GLfloat*);
void drawPlant(int, GLfloat*);
int init_resources(void);
void free_resources(void);
void matrix_multiply3_3(GLfloat *a, GLfloat *b, GLfloat *c);
GLfloat* matrix_translate(GLfloat t[], GLfloat x, GLfloat y);