varying vec4			v_color;
attribute vec2			a_position;
attribute float			a_color;
uniform vec2			v_location;
uniform vec2			v_scale;
uniform mat2			m_vp_matrix;
uniform mat2			m_rot_matrix;

void main()
{
	vec2 ap = a_position * vec2(v_scale.x, v_scale.y);
	ap *= m_rot_matrix;
	ap += v_location;
	vec2 pos = ap * m_vp_matrix;
	gl_Position = vec4(pos.x - 1.0, -(pos.y - 1.0), 0.0, 1.0);
	v_color = vec4(a_color, a_color, a_color, 1.0);
}