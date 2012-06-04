// Input position vector
attribute vec3 coord3d;
varying vec3 f_coord3d;


attribute vec2 texcoord;
varying vec2 f_texcoord;

// Brightness for fragment program ----- additional
varying float f_brightness;

// Input normal vector ----- additional
attribute vec3 normal;

// Projection matrix
uniform mat4 m_proj;

// Input matrix by which normals should be normalized ----- additional
uniform mat4 normal_transform;

// Input matrix by which normals of the floor should be normalized ----- additional
uniform mat4 normal_floor_transform;


void main(void) {        

	// create a light source
	// use the coordinates (1.0, 1.0, 2.0) as specified by the instructions
	// normalize the light source
	vec4 light_source = normalize(vec4(1.0, 1.0, 1.0, 0.0));

	// transform the light source using the normal_transform matrix
	vec4 n = normal_transform * vec4(normalize(normal), 0.0);

	// get the brightness
	float brightness = max(dot(n, light_source), 0.0);

	f_brightness = brightness;



    f_texcoord = texcoord;
	f_coord3d = coord3d;
    // multiply matrix times position vector
    gl_Position  = m_proj *  vec4(coord3d,1.0);

}

