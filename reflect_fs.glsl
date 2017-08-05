#ifdef GL_ES_VERSION_2_0
precision mediump float;
#endif

varying vec2			v_texcoord;
varying vec3			v_normal;
varying vec3			v_eyedir;
varying vec4			v_color;
varying vec3			v_light;
varying vec3			v_reflection;
uniform sampler2D		s_color_texture;
uniform samplerCube		s_env_texture;

void main()
{
	float ambient = 0.2;

	//vec3 color = vec3(v_color) - vec3(0.5,0.5,0.5);
	//gl_FragColor = texture2D(s_color_texture, v_texcoord) + vec4(color,0);

	vec3 color = vec3(v_color) - vec3(0.5, 0.5, 0.5);
	//float d = 1.0 - (distance(v_texcoord2, vec3(0.0, 0.0, 0.1)) * 0.5);
	//gl_FragColor = vec4(d, d, d, 1) * texture2D(s_color_texture, v_texcoord);
	//gl_FragColor = vec4(1, d, 0, 1) * texture2D(s_color_texture, v_texcoord);
	
	//gl_FragColor = texture2D(s_color_texture, v_texcoord);

	float col = clamp(dot(-v_light, v_normal), 0.0, 1.0);
	gl_FragColor = (vec4(col, col, col, 1) + ambient) * textureCube(s_env_texture, v_reflection);

	//vec3 refl = normalize(reflect(normalize(-v_eyedir), normalize(v_normal)));
	//gl_FragColor = textureCube(s_env_texture, refl);	
	
	//gl_FragColor = textureCube(s_env_texture, v_eyedir);
	//gl_FragColor = vec4(1.0, v_eyedir.x * 0.1, v_eyedir.y * 0.1, 1);

	//gl_FragColor = (vec4(col, col, col, 1) + ambient) * texture2D(s_color_texture, v_texcoord);
}