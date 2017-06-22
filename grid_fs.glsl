#ifdef GL_ES_VERSION_2_0
precision mediump float;
#endif

varying vec4	v_color;

void main()
{
	gl_FragColor = v_color;
}