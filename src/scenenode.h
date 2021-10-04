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

	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	Material * material = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Matrix44 model;
	float distance_to_cam = 0;

	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

class LightNode : public SceneNode
{
public:
	Vector3 color;
	Vector3 diffuseColor;
	Vector3 specularColor;
	float intensity;
	float max_distance;

	LightNode();

	virtual void renderInMenu();
	void setLightUniforms(Shader* shader);

};

class SkyboxNode : public SceneNode
{
public:

	SkyboxNode();
	void render(Camera* camera);
	void renderInMenu();
};


#endif