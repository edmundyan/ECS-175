CC = g++
CFLAGS = -g -DGL_GLEXT_PROTOTYPES 
INCLUDES = -I/usr/include/GL
LDFLAGS = -lglut -lGLU -lGL -lX11 -lGLEW

RM = /bin/rm -f 

glassball:  glassball.o draw.o shader_utils.o readObj.o
	$(CC) $(CFLAGS) -o glassball glassball.o shader_utils.o draw.o readObj.o $(LDFLAGS) 
%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean: 
	$(RM) *.o glassball 
