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

class PhongMaterial : public StandardMaterial {
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

class TexturedMaterial : public StandardMaterial {
public:

	TexturedMaterial();
	~TexturedMaterial();
};

class ReflectiveMaterial : public StandardMaterial {
public:

	ReflectiveMaterial();
	~ReflectiveMaterial();

	void renderInMenu() {}; //Anything to render in menu
};

class SkyboxMaterial : public StandardMaterial {
public:

	SkyboxMaterial();
	~SkyboxMaterial();

	void renderInMenu() {}; //Anything to render in menu
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class PBRMaterial : public StandardMaterial
{
public:

	Texture* normal_texture;
	Texture* metallic_roughness_texture;
	Texture* ao_texture;
	Texture* emissive_texture;
	Texture* BRDFlut;

	Texture* texture_prem_0;
	Texture* texture_prem_1;
	Texture* texture_prem_2;
	Texture* texture_prem_3;
	Texture* texture_prem_4;

	PBRMaterial();
	~PBRMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

#endif