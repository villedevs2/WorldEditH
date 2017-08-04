#ifdef GL_ES_VERSION_2_0
precision mediump float;
#endif

varying vec2			v_texcoord;
varying vec3			v_texcoord2;
varying vec4			v_color;
uniform sampler2D		s_color_texture;

void main()
{
	//vec3 color = vec3(v_color) - vec3(0.5,0.5,0.5);
	//gl_FragColor = texture2D(s_color_texture, v_texcoord) + vec4(color,0);

	vec3 color = vec3(v_color) - vec3(0.5, 0.5, 0.5);
	float d = 1.0 - (distance(v_texcoord2, vec3(0.0, 0.0, 0.1)) * 0.5);
	//gl_FragColor = vec4(d, d, d, 1) * texture2D(s_color_texture, v_texcoord);
	gl_FragColor = vec4(1, d, 0, 1) * texture2D(s_color_texture, v_texcoord);
}