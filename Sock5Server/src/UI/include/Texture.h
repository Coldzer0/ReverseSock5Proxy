#pragma once

#include <string>

#include "GLFW/glfw3.h"

#define GL_CLAMP_TO_EDGE 0x812F


extern bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

class Texture
{
private:
	GLuint _TextureID = 0;
	int _Width = 0;
	int _Height = 0;

public:
	Texture(std::string filename);
	~Texture();

	void Render();
};
