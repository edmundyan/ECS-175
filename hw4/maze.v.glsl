attribute vec3 coord3d;
attribute vec2 texcoord;
varying vec2 f_texcoord;

// Projection matrix
uniform mat4 m_proj;


void main(void) {        
     f_texcoord = texcoord;
     // multiply matrix times position vector
     gl_Position  = m_proj *  vec4(coord3d,1.0);

}

