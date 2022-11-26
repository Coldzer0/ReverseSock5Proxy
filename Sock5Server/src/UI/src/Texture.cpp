#include "Texture.h"

#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Loads a texture using stb_image
//Stores it as an OpenGL texture id for use in ImGui.

Texture::Texture(std::string filename) {
  bool ret = LoadTextureFromFile(filename.c_str(), &_TextureID, &_Width, &_Height);

  if (!ret) {
    exit(1);
  }
}

Texture::~Texture() {
  glDeleteTextures(1, &_TextureID);
}

void Texture::Render() {
  ImGui::Image((void *) (intptr_t) _TextureID, ImVec2(_Width, _Height));
}

bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height) {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL) {
    printf("Error loading image %s\n", filename);
    return false;
  }

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}
