#version 120

varying vec2			v_texcoord;
varying vec2			v_ambcoord;
varying vec4			v_color;
varying vec3			v_normal;
varying vec3			v_light;
attribute vec3			a_position;
attribute vec2			a_texcoord;
attribute vec2			a_ambcoord;
attribute vec3			a_normal;
attribute vec4			a_color;
uniform vec2			v_location;
uniform vec2			v_scale;
uniform vec3			u_light;
uniform mat4			m_vp_matrix;
uniform mat4			m_v_matrix;

void main()
{
	//vec2 ap = vec2(a_position.x, a_position.y);
	//ap += v_location;
	//vec2 pos = ap * m_vp_matrix;
	//gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

	gl_Position = vec4(a_position, 1.0) * m_vp_matrix;

	v_texcoord = a_texcoord;
	v_ambcoord = a_ambcoord;
	v_color = a_color;

	v_normal = mat3(m_v_matrix) * a_normal;	

	//v_light = normalize(vec3(-0.7, -0.7, -0.3));
	v_light = normalize(u_light);
}