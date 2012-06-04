varying vec3 f_coord3d;
varying vec2 f_texcoord;
uniform sampler2D u_texture;
//uniform sampler2D u_floor_texture;

varying float f_brightness;


void main(void) {

	vec4 color = texture2D(u_texture, f_texcoord);
	color = min(color * f_brightness + 0.1, 1.0);


	//gl_FragColor = texture2D(u_floor_texture, f_texcoord);
	//gl_FragColor = texture2D(u_texture, f_texcoord);
	gl_FragColor = color;


	// FOG
	vec4 phi = vec4(vec3(f_coord3d.z + 63)/70, 0.0);
	gl_FragColor = phi*color + (1-phi)*vec4(0.3,0.4,0.9,0.1);
	
}