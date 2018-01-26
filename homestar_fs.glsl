#ifdef GL_ES_VERSION_2_0
precision mediump float;
#endif

varying vec2			v_texcoord;
varying vec2			v_ambcoord;
varying vec4			v_color;
varying vec3			v_light;
varying vec3			v_normal;
uniform sampler2D		s_color_texture;
uniform sampler2D		s_ambient_texture;

void main()
{
	//float ambient = 0.2;
	//float col = clamp(dot(-v_light, v_normal), 0.0, 1.0);

	//vec4 aotex = texture2D(s_ambient_texture, v_ambcoord);
	//vec4 ao = vec4(aotex.g, aotex.g, aotex.g, 1);

	vec4 texel = texture2D(s_color_texture, v_texcoord);
	gl_FragColor = texel * vec4(v_color.xyz, 1);
	//gl_FragColor = (vec4(col, col, col, 1) + ambient) * ao;
}