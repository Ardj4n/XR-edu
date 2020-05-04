#pragma once
/**
* Supsi-GE, texture management class
* This class' purpose is to manage a material's texture, a picture in memory.
* They are used to add more detail to a material without the need to have more complex meshes
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Texture :
	public virtual Object
{
private:
	/**
	@var textureId
	Integer identifier of the texture
	*/
	unsigned int textureId;
	static Texture* blank;
public:
	/**
	Creates a Texture object from the application's resources folder
	@param textureName The name of the Texture, corresponds to the file name
	in the $appFolder/resources folder.
	@see Object.h
	*/
	Texture(string textureName);

	/**
	Destroys the created object
	@see Object.h
	*/
	~Texture();

	/**
	@see Object.h
	*/
	void render();

	/**
	Returns "texture"
	@see Object.h
	*/
	string getType();

	static Texture& getBlankTexture();
};

