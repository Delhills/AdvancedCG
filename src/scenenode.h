#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"

class Light;

class SceneNode {
public:

	static unsigned int lastNameId;

	unsigned int mesh_selected = 0;
	unsigned int material_selected = 0;
	unsigned int texture_selected = 0;

	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	Material * material = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Matrix44 model;

	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

class Light : public SceneNode
{
public:

	static unsigned int lastLightId;

	Vector3 diffuse;
	Vector3 specular;
	float intensity;
	Vector3 color;

	Light();
	void renderInMenu();
};

class Skybox : public SceneNode
{
public:

	Skybox();
	void render(Camera* camera);
	void renderInMenu();
};
#endif