CC = g++
CFLAGS = -g -DGL_GLEXT_PROTOTYPES 
INCLUDES = -I/usr/include/GL
LDFLAGS = -lglut -lGLU -lGL -lX11 -lGLEW

RM = /bin/rm -f 

plant:  plant.o drawPlant.o shader_utils.o
	$(CC) $(CFLAGS) -o plant plant.o shader_utils.o drawPlant.o $(LDFLAGS) 
%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean: 
	$(RM) *.o drawPlant
