#pragma once

/**
* Supsi-GE, List management class
* This class is necessary to render scene with light sources.
* In these operations lights are rendered first with the priority model (see Light.h)
* since already rendered polygons in the graph are NOT updated to their illuminated variant
*
* @authors D.Nasi, J.Petralli, D.Calabria
*/

class LIB_API List :
	public Object
{
private:
	/**
	@var lightsCount
	The number of light in the scene (defaults to 0)
	*/
	int lightsCount = 0;

	/**
	Priority based light search method.
	If two or more lights have coincidental priority, the first get an automatic priority "bump"
	@param priority the priority value to search
	*/
	int findLightPos(int priority);

	/**
	@struct NodeMat
	Is the structure of the list's nodes. Comprised of
	*  - node Pointer to the node
	*  - finalMat Rendering matrix
	*/
	struct NodeMat 
	{
		Node* node;
		glm::mat4 finalMat;
	};

	/**
	@var list
	List of the grahp's nodes in "struct NodeMat" format
	*/
	vector<NodeMat> list;
public:
	/**
	Constructor
	@see Object.h
	*/
	List();

	/**
	Destructor
	@see Object.h
	*/
	~List();

	/**
	Creates a NodeMat with the specified parameters and adds it to the list.
	@param node The node to be added
	@param finalMat The node's matrix with all the previous transformations applied to.
	*/
	void addNode(Node* node, glm::mat4 finalMat);

	/**
	Empties the list.
	*/
	void clear();

	/**
	See "Object.h" for the base principle.
	It asks the Engine for the maximum number of the lights and assigns the lights' lightNumber.
	Then it traverse the list to fill the maximum number available of light sources (if there are enough in the scene)
	and renders the scene, lights first.
	@see Object.h
	*/
	void render();

	/**
	Renders the world from a specific Camera, corresponding to the "invCamera" camera.
	For additional details.
	@see Camera.h
	@param invCamera The chosen Camera's inverse matrix
	*/
	void renderWithCamera(glm::mat4 invCamera);

	/**
	Returns a standard list with all the nodes without their matrices
	*/
	void renderXR(glm::mat4 proj, glm::mat4 head);
	vector<Node*> getNodes();

	/**
	Return lightsCount
	*/
	int getLightsCount();

	/**
	Returns "list"
	@see Object.h
	*/
	string getType();
};

