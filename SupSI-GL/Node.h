#pragma once
/**
* Supsi-GE, Scene graph's nodes management class
* Node contains the scene's objects. It contains method to modify the various relationships in the scene graph,
* to search a specific node amongst a node's children or to calculate a complete positioning Matrix
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API Node :
	public virtual Object
{
private:
	/**
	@var parent
	Pointer to the Node's parent
	*/
	Node *parent;

	/**
	@var children
	List of the Node's children
	*/
	vector<Node*> children;

	/**
	@var posMatrix
	The Node's positioning matrix relative to the parent
	*/
	glm::mat4 posMatrix;
public:

	/**
	Constructor
	@see Object.h
	*/
	Node();

	/**
	Destructor
	@see Object.h
	*/
	virtual ~Node();

	/**
	Returns the child node with the specified id with a recursive search
	@param id The identifier to be searched
	*/
	Node* search(int id);

	/**
	Adds a new Node to the childrens' list
	@param child The node to be added
	*/
	void appendChild(Node *child);

	/**
	Set the previous Node in the graph to "parent"
	@param parent The new parent Node
	*/
	void setParent(Node *parent);

	/**
	Returns the pointer to the parent Node
	*/
	Node* getParent();

	/**
	Returns the children list
	*/
	vector<Node*> getChildren();

	/**
	Clears the children list
	*/
	void deleteChildren();

	/**
	Returns the Node's positioning matrix relative to the parent's
	*/
	glm::mat4 getPosMatrix();

	/**
	Returns the Node's final positioning matrix in screen coordinates (as seen by a camera)
	*/
	glm::mat4 getFinal();

	/**
	Set the Node's positioning matrix
	*/
	void setPosMatrix(glm::mat4 posMatrix);

	/**
	@see Object.h
	*/
	void render();

	void traverse(const std::string & tab = "");
	/**
	Returns "node"
	@see Object.h
	*/
	string getType();
};

