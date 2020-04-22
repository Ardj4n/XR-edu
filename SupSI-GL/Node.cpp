#include "Engine.h"
#include "GL/freeglut.h"


LIB_API Node::Node() : Object()
{
	parent = nullptr;
}


LIB_API Node::~Node()
{
}

Node LIB_API * Node::getParent()
{
	return parent;
}

void LIB_API Node::setParent(Node *parent)
{
	if (parent != nullptr) {
		for (int i = 0; i < parent->children.size(); i++)
		{
			if (parent->children.at(i)->getId() == this->getId())
			{
				parent->children.erase(parent->children.begin() + i);
			}
		}
	}
	this->parent = parent;
	parent->children.push_back(this);
}

vector<Node*> LIB_API Node::getChildren()
{
	return children;
}

void  LIB_API Node::deleteChildren()
{
	this->children.clear();
}

glm::mat4 LIB_API Node::getPosMatrix()
{
	return posMatrix;
}

void LIB_API Node::setPosMatrix(glm::mat4 posMatrix)
{
	this->posMatrix=posMatrix;
}

void LIB_API Node::appendChild(Node *child)
{
	child->setParent(this);
}


//recoursive search in children
Node LIB_API * Node::search(int id)
{
	if (getId() == id)
		return this;
	else
	{
		for (auto child : children)
		{
			Node* r = child->search(id);
			if (r) return r;
		}
	}
	return nullptr;
}

//return the final position matrix (it applies all parents transformations)
glm::mat4 LIB_API Node::getFinal()
{
	if (parent == nullptr)
		return posMatrix;
	return parent->getFinal() * posMatrix;
}

void LIB_API Node::render()
{
}

string LIB_API Node::getType()
{
	return "node";
}

void LIB_API Node::traverse(const std::string &tab)
{
	std::cout << tab << getName() << std::endl;

	for (auto c : children)
		c->traverse(std::string(tab + "\t"));
}