#version 150 core

out vec4 gl_FragColor;

//As basic as possible for the shadow map. Note: we output a colour that never actually gets written (we disabled colour writing
// in the program), but the depth buffer will be written to!
void main(void){
	gl_FragColor = vec4(1.0);
}