#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class PhongMaterial : public Material {
public:

	Vector3 ambient;
	Vector3 diffuse;
	Vector3 specular;
	float shininess;

	PhongMaterial();
	~PhongMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class ReflectiveMaterial : public Material {
public:

	float reflectivity;

	ReflectiveMaterial();
	~ReflectiveMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

#endif