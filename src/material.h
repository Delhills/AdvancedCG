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

	Texture* texture_prem_0;
	Texture* texture_prem_1;
	Texture* texture_prem_2;
	Texture* texture_prem_3;
	Texture* texture_prem_4;

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
	Texture* metallic_texture;
	Texture* roughness_texture; //it may contain also the metallic texture
	Texture* ao_texture;
	Texture* emissive_texture;
	Texture* BRDFlut;

	bool use_metaltxt;
	bool use_aotxt;
	bool use_emissivetxt;

	float roughness_factor;
	float metallic_factor;
	float emission_factor;

	PBRMaterial();
	~PBRMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

#endif