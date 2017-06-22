varying vec2			v_texcoord;
varying float			v_depth;
varying vec4			v_color;
attribute vec3			a_position;
attribute vec2			a_texcoord;
attribute vec4			a_color;
uniform vec2			v_location;
uniform vec2			v_scale;
uniform mat2			m_vp_matrix;
uniform mat2			m_rot_matrix;

void main()
{
	vec2 ap = vec2(a_position.x, a_position.y) * vec2(v_scale.x, v_scale.y);
	ap *= m_rot_matrix;
	ap += v_location;
	vec2 pos = ap * m_vp_matrix;
	gl_Position = vec4(pos.x - 1.0, -(pos.y - 1.0), 0.0, 1.0);
	v_texcoord = a_texcoord;
	v_depth = 1.0 - (a_position.z * 0.005);		// scale 0..200 to 1..0
	v_color = a_color;
}