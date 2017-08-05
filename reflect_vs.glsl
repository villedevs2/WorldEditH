#version 120

varying vec2			v_texcoord;
varying vec3			v_eyedir;
varying vec3			v_normal;
varying vec4			v_color;
varying vec3			v_light;
varying vec3			v_reflection;
attribute vec3			a_position;
attribute vec2			a_texcoord;
attribute vec3			a_normal;
attribute vec4			a_color;
uniform mat4			m_vp_matrix;
uniform mat4			m_v_matrix;
uniform vec3			v_camera_pos;

void main()
{
	//vec2 ap = vec2(a_position.x, a_position.y);
	//ap += v_location;
	//vec2 pos = ap * m_vp_matrix;
	//gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

	vec4 vertex = vec4(a_position, 1.0);

	vec3 refl = reflect(a_position - v_camera_pos, normalize(a_normal));
	v_reflection = vec3(refl.x, -refl.yz);

	gl_Position = vertex * m_vp_matrix;

	v_eyedir = vec3(m_v_matrix * vertex);
	v_normal = mat3(m_v_matrix) * a_normal;

	v_texcoord = a_texcoord;
	v_color = a_color;

	v_light = vec3(0.7, 0.7, -0.7);
}