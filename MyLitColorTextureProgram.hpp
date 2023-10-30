#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"

// https://learnopengl-cn.github.io/01%20Getting%20started/06%20Textures/#stb_imageh

// Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct MyLitColorTextureProgram
{
	MyLitColorTextureProgram();
	~MyLitColorTextureProgram();

	GLuint program = 0;
	// std::string img_path = "Boss.png";
	//   Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint Normal_vec3 = -1U;
	GLuint Color_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;

	// Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;
	GLuint NORMAL_TO_LIGHT_mat3 = -1U;

	// lighting:
	GLuint LIGHT_TYPE_int = -1U;
	GLuint LIGHT_LOCATION_vec3 = -1U;
	GLuint LIGHT_DIRECTION_vec3 = -1U;
	GLuint LIGHT_ENERGY_vec3 = -1U;
	GLuint LIGHT_CUTOFF_float = -1U;

	std::string SetImgPath(std::string img_path)
	{
		std::string path = img_path;
		return path;
	}
	// Textures:
	// TEXTURE0 - texture that is accessed by TexCoord
};

extern Load<MyLitColorTextureProgram> my_lit_color_texture_program;

// For convenient scene-graph setup, copy this object:
//  NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
extern Scene::Drawable::Pipeline my_lit_color_texture_program_pipeline;
