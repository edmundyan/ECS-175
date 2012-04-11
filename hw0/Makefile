CC = g++
CFLAGS = -g -DGL_GLEXT_PROTOTYPES 
INCLUDES = -I./common/include/ -I./common/include/glm-0.9.3.2/ -I/usr/X11R6/
LDFLAGS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lX11 -lGLEW

RM = /bin/rm -f 

cube:  cube.o ./common/shader_utils.o res_texture.o
	$(CC) $(CFLAGS) -o cube ./common/shader_utils.o res_texture.o cube.o $(LDFLAGS) 
%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean: 
	$(RM) *.o cube

 
