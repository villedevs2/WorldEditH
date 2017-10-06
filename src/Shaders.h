#pragma once

namespace Shaders
{
	struct LevelShader
	{
		int position;
		int tex_coord;
		int color;
		int location;
		int scale;
		int vp_matrix;
		int rot_matrix;
	};

	struct GridShader
	{
		int position;
		int color;
		int location;
		int scale;
		int vp_matrix;
		int rot_matrix;
	};

	struct HomestarShader
	{
		int position;
		int tex_coord;
		int amb_coord;
		int color;
		int normal;
		int vp_matrix;
		int v_matrix;
		int light;
		int diff_sampler;
		int amb_sampler;
	};

	struct SelflumShader
	{
		int position;
		int texcoord;
		int normal;
		int color;
		int vp_matrix;
	};

	struct ReflectShader
	{
		int position;
		int texcoord;
		int normal;
		int color;
		int vp_matrix;
		int v_matrix;
		int cam_pos;
	};
};