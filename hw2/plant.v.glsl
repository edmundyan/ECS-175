attribute vec2 coord2d;  
attribute vec3 v_color;
varying vec4 f_color;
// Matrix by which leaf should be moved
uniform mat3 m_transform;

void main(void) {
    // transformed vector
    vec3 xformed;

    // pad color to length 4 
    f_color = vec4(v_color,1.0);

    // multiply matrix times position vector
    xformed = m_transform * vec3(coord2d,1.0);

    // 3D point lies in z=0
    xformed[2] = 0.0;

    gl_Position = vec4(xformed, 1.0);
    }

