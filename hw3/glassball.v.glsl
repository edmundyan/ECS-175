// Input position vector
attribute vec3 coord3d;  
// Output color, to be interpolated
varying vec4 f_color;
// Input matrix by which cube should be moved
uniform mat4 m_transform;

void main(void) {

    // use vertex position as color; pad to length 4 
    f_color = vec4(coord3d,1.0);

    // multiply matrix times position vector
    gl_Position  = m_transform * vec4(coord3d,1.0);
    }

