#ifdef GL_ES_VERSION_2_0
precision mediump float;
#endif

varying vec2			v_texcoord;
varying vec4			v_color;
uniform sampler2D		s_color_texture;

void main()
{
	vec3 color = vec3(v_color) - vec3(0.5,0.5,0.5);
	gl_FragColor = texture2D(s_color_texture, v_texcoord) + vec4(color,0);
}