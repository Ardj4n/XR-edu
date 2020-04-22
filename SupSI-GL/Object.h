#pragma once
/**
* Supsi-GE, Object management class
* This class is the base over wich all other classes representing the scene
* are built upon, except Engine and Vertex. Therefore it only has the most common fields and methods
* across the class structure.
* The methods render() and getType() need to be implemented by the children, due to
* them being virtual (not implemented)
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Object
{
private:
	/**
	@var id
	The object's identifier
	*/
	int id;

	/**
	@var name
	The object's name
	*/
	string name;

	/**
	@static @var currId
	Global variable used during ID generation
	*/
	static int currId;

	/**
	@static Method
	Used during construction to generate the object's ID
	*/
	static int getNextId();
public:

	/**
	Creates an object's instance
	*/
	Object();

	/**
	Deletes a created object.
	It is completely virtual, REQUIRES implementation in subclasses
	*/
	virtual ~Object()=0;

	/**
	Returns an object's ID
	*/
	int getId();

	/**
	Returns the object's name
	*/
	string getName();

	/**
	Set an object's ID
	@param id the new ID
	*/
	void setId(int id);

	/**
	Set an object's name
	@param name the new name
	*/
	void setName(string name);
	
	/**
	Specifies how the object has to be rendered.
	It is completely virtual, REQUIRES implementation in subclasses
	*/
	virtual void render()=0;
	
	/**
	Returns a string describing the Object's type (e.g "Object").
	It is completely virtual, REQUIRES implementation in subclasses
	*/
	virtual string getType() = 0;
};