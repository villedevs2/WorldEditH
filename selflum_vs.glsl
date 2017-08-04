varying vec2			v_texcoord;
varying vec3			v_texcoord2;
varying vec4			v_color;
attribute vec3			a_position;
attribute vec2			a_texcoord;
attribute vec3			a_texcoord2;
attribute vec4			a_color;
uniform vec2			v_location;
uniform vec2			v_scale;
uniform mat4			m_vp_matrix;

void main()
{
	//vec2 ap = vec2(a_position.x, a_position.y);
	//ap += v_location;
	//vec2 pos = ap * m_vp_matrix;
	//gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

	gl_Position = vec4(a_position, 1.0) * m_vp_matrix;

	v_texcoord = a_texcoord;
	v_texcoord2 = a_texcoord2;
	v_color = a_color;
}