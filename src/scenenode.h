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

	unsigned int mesh_selected;
	unsigned int material_selected;
	unsigned int texture_selected;

	float distance_to_cam; //Distance to the camera for sorting computations

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

	//Phong components
	Vector3 diffuse;
	Vector3 specular;

	//PBR components
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