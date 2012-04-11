CC = g++
CFLAGS = -g -DGL_GLEXT_PROTOTYPES 
INCLUDES = -I/usr/X11R6/
LDFLAGS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lX11 -lGLEW

RM = /bin/rm -f 

bigPix:  bigPix.o lowlevel.o shader_utils.o
	$(CC) $(CFLAGS) -o bigPix bigPix.o shader_utils.o lowlevel.o $(LDFLAGS) 
%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean: 
	$(RM) *.o bigPix
