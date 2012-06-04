varying vec2 f_texcoord;
uniform sampler2D u_texture;
//uniform sampler2D u_floor_texture;

void main(void) {        
  //gl_FragColor = texture2D(u_floor_texture, f_texcoord);
  gl_FragColor = texture2D(u_texture, f_texcoord);
}

