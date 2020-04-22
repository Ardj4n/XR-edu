#include "Engine.h"
#include "GL/freeglut.h"
#include <FreeImage.h>

//Defines for Anisotropic filtering
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define ANISOTROPIC_LEVEL 1

LIB_API Texture::Texture(string textureName) : Object()
{
	if (textureName.compare("[none]") == 0)
	{
		return;
	}
	this->setName(textureName);
	glGenTextures(1, &textureId);
	std::string texturePath = "../resources/";
	const char* fileName = texturePath.append(textureName).c_str();
	//create bitmap containing our texture
	FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(fileName, 0), fileName);
	FreeImage_FlipVertical(bitmap);
	//in/out formats
	int format = GL_RGB;
	GLenum extFormat = GL_BGR_EXT;
	//if the bitmap has the alpha channel...
	if (FreeImage_GetBPP(bitmap) == 32)
	{
		format = GL_RGBA;
		extFormat = GL_BGRA_EXT;
	}
	// Update texture content:
	glBindTexture(GL_TEXTURE_2D, textureId);
	// Set circular coordinates:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//check for anisotropic
	bool isAnisotropicSupported = false;
	int anisotropicLevel = ANISOTROPIC_LEVEL;
	/*if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
	{
		isAnisotropicSupported = true;
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropicLevel);
	}
	if (isAnisotropicSupported)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicLevel);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
	*/
	//magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//build 2d mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, format, FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap), extFormat, GL_UNSIGNED_BYTE, (void *)FreeImage_GetBits(bitmap));
	//unload the texture from the main memory 
	FreeImage_Unload(bitmap);
}


LIB_API Texture::~Texture()
{
	glDeleteTextures(1, &textureId);
}

void LIB_API Texture::render()
{
	glBindTexture(GL_TEXTURE_2D, textureId);
}

string LIB_API Texture::getType()
{
	return "texture";
}

Texture LIB_API &Texture::getBlankTexture()
{
	static Texture blank("blank");
	return blank;
}
