#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{
	
}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_color", color);
	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	if (texture)
		shader->setUniform("u_texture", texture, 0);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

void StandardMaterial::setTexture(std::string geometry, int mesh)
{
	texture = Texture::Get(("data/models/" + geometry + "/albedo.png").c_str());
}

PhongMaterial::PhongMaterial()
{
	//Setting the default parameters
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
	ambient = vec3(1.f, 1.f, 1.f);
	diffuse = vec3(1.f, 1.f, 1.f);
	specular = vec3(1.f, 1.f, 1.f);
	shininess = 256.0f;
}

PhongMaterial::~PhongMaterial()
{
	//Destroy stored data
	texture->~Texture();
	shader->~Shader();
}

void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	StandardMaterial::setUniforms(camera, model);

	//Upload material related uniforms
	shader->setUniform("u_material_ambient", ambient);
	shader->setUniform("u_material_diffuse", diffuse);
	shader->setUniform("u_material_specular", specular);
	shader->setUniform("u_material_shininess", shininess);
}

void PhongMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//Establecemos un multi pass render
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_ONE, GL_ONE);

		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//MULTI PASS RENDER
		Vector3 ambient_light = Application::instance->ambient_light;
		std::vector< Light* > lights = Application::instance->light_list;

		shader->setUniform("u_light_ambient", Application::instance->ambient_light); //Pass ambient light for the first pass

		//Use all the lights
		for (int i = 0; i < lights.size(); ++i)
		{
			Light* light = lights[i];

			if (i == 1) //Blending on after first pass and no ambient light
			{
				glEnable(GL_BLEND);
				shader->setVector3("u_light_ambient", Vector3(0, 0, 0));
			}
			
			//Pass light parameters
			shader->setUniform("u_light_position", light->model.getTranslation());
			shader->setUniform("u_light_diffuse", light->diffuse);
			shader->setUniform("u_light_specular", light->specular);

			mesh->render(GL_TRIANGLES); //Render the mesh for every light
		}

		//disable shader
		shader->disable();

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS); //as default
	}
}

void PhongMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Ambient", (float*)&ambient); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Diffuse", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular", (float*)&specular); // Edit 3 floats representing a color
	ImGui::DragFloat("Shininess", (float*)&shininess, 1.0f, 1.0f, 256.f);
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{
	shader->~Shader();
}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

ReflectiveMaterial::ReflectiveMaterial()
{
	//Use reflective shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/reflective.fs");

	//Use the skybox cubemap as the texture
	texture = Application::instance->sky->material->texture;
}

ReflectiveMaterial::~ReflectiveMaterial()
{
	//Destroy stored data
	shader->~Shader();
	texture->~Texture();
}

TexturedMaterial::TexturedMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
}

TexturedMaterial::~TexturedMaterial()
{
	//Destroy stored data
	shader->~Shader();
	texture->~Texture();
}

SkyboxMaterial::SkyboxMaterial()
{
	//Use reflective shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");

	texture_prem_0 = new Texture();
	texture_prem_1 = new Texture();
	texture_prem_2 = new Texture();
	texture_prem_3 = new Texture();
	texture_prem_4 = new Texture();
}

SkyboxMaterial::~SkyboxMaterial()
{
	//Destroy stored data
	shader->~Shader();
	texture->~Texture();
}

PBRMaterial::PBRMaterial()
{
	normal_texture = Texture::Get("data/models/helmet/normal.png");
	ao_texture = Texture::Get("data/models/helmet/ao.png");
	metallic_roughness_texture = Texture::Get("data/models/helmet/roughness.png");
	metallic_texture = NULL;
	roughness_texture = NULL;
	emissive_texture = Texture::Get("data/models/helmet/emissive.png");
	opacity_texture = Texture::getWhiteTexture();
	BRDFlut = Texture::Get("data/brdfLUT.png");

	normal = 1.0;
	roughness = 1.0;
	metalness = 1.0;

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");
}

PBRMaterial::~PBRMaterial()
{
	normal_texture = NULL;
	ao_texture = NULL;
	metallic_roughness_texture = NULL;
	metallic_texture = NULL;
	roughness_texture = NULL;
	emissive_texture = NULL;
	opacity_texture = NULL;
	BRDFlut = Texture::Get("data/brdfLUT.png");
}

void PBRMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	StandardMaterial::setUniforms(camera, model);

	shader->setUniform("u_normal_texture", normal_texture, 1);
	shader->setUniform("u_emissive_texture", emissive_texture, 2);

	if (metallic_roughness_texture)
	{
		shader->setUniform("u_metallic_roughness_texture", metallic_roughness_texture, 3);
		shader->setUniform("u_metallic_roughness", 1.0);
	}
	else
	{
		shader->setUniform("u_metallic_roughness", 0.0);
		shader->setUniform("u_roughness_texture", roughness_texture, 3);
		shader->setUniform("u_metallic_texture", metallic_texture, 4);
	}
	shader->setUniform("u_opacity_texture", opacity_texture, 5);
	shader->setUniform("u_ao_texture", ao_texture, 6);
	shader->setUniform("u_lut", Texture::Get("data/brdfLUT.png"), 7);

	SkyboxMaterial* sky = (SkyboxMaterial*)Application::instance->sky->material;
	shader->setUniform("u_environment_texture", sky->texture, 8);
	shader->setUniform("u_texture_prem_0", sky->texture_prem_0, 9);
	shader->setUniform("u_texture_prem_1", sky->texture_prem_1, 10);
	shader->setUniform("u_texture_prem_2", sky->texture_prem_2, 11);
	shader->setUniform("u_texture_prem_3", sky->texture_prem_3, 12);
	shader->setUniform("u_texture_prem_4", sky->texture_prem_4, 13);

	shader->setUniform("u_metallic_factor", metalness);
	shader->setUniform("u_roughness_factor", roughness);
	shader->setUniform("u_normal_factor", normal);
}

void PBRMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//Establecemos un single pass render
		glDepthFunc(GL_LEQUAL); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//SINGLE PASS RENDER
		std::vector< Light* > lights = Application::instance->light_list;

		//Defining the vectors that will be passed to the GPU
		Vector3 light_position[100];
		Vector3 light_color[100];
		float light_intensity[100];

		//Filling the vectors
		for (int i = 0; i < lights.size(); ++i)
		{
			Light* light = lights[i];
			light_position[i] = light->model.getTranslation();
			light_color[i] = light->color;
			light_intensity[i] = light->intensity;
		}

		shader->setUniform3Array("u_light_position", (float*)&light_position, 100);
		shader->setUniform3Array("u_light_color", (float*)&light_color, 100);
		shader->setUniform1Array("u_light_intensity", (float*)&light_intensity, 100);
		shader->setUniform1("u_num_lights", (float)lights.size());

		mesh->render(GL_TRIANGLES); //Render the mesh for every light
	}
}

void PBRMaterial::renderInMenu()
{
	ImGui::DragFloat("Metalness", (float*)&metalness, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Roughness", (float*)&roughness, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Normal Scale", (float*)&normal, 0.01f, -1.0f, 2.0f);
}

void PBRMaterial::setTexture(std::string geometry, int mesh)
{
	StandardMaterial::setTexture(geometry, mesh);

	normal_texture = Texture::Get(("data/models/" + geometry + "/normal.png").c_str());
	BRDFlut = Texture::Get("data/brdfLUT.png");

	ao_texture = Texture::Get(("data/models/" + geometry + "/ao.png").c_str());
	if (ao_texture == NULL)
		ao_texture = Texture::getWhiteTexture();

	emissive_texture = Texture::Get(("data/models/" + geometry + "/emissive.png").c_str());
	if (emissive_texture == NULL)
		emissive_texture = Texture::getBlackTexture();

	opacity_texture = Texture::Get(("data/models/" + geometry + "/opacity.png").c_str());
	if (opacity_texture == NULL)
		opacity_texture = Texture::getWhiteTexture();

	if (mesh == 2)
	{
		metallic_roughness_texture = Texture::Get(("data/models/" + geometry + "/roughness.png").c_str());
		metallic_texture = NULL;
		roughness_texture = NULL;
	}
	else
	{
		metallic_roughness_texture = NULL;
		metallic_texture = Texture::Get(("data/models/" + geometry + "/metalness.png").c_str());
		roughness_texture = Texture::Get(("data/models/" + geometry + "/roughness.png").c_str());
	}
}

/*
//Establecemos un multi pass render
glDepthFunc(GL_LEQUAL);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//enable shader
shader->enable();

//upload uniforms
setUniforms(camera, model);

//MULTI PASS RENDER
float first_pass = 1.0;
std::vector< Light* > lights = Application::instance->light_list;
shader->setUniform("u_first_pass", first_pass);

//Use all the lights
for (int i = 0; i < lights.size(); ++i)
{
	Light* light = lights[i];

	if (i != 0) //Blending on after first pass and no ambient light
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		first_pass = 0.0;
		shader->setUniform("u_first_pass", first_pass);
	}

	//Pass light parameters
	shader->setUniform("u_light_position", light->model.getTranslation());
	shader->setUniform("u_light_intensity", light->intensity);
	shader->setUniform("u_light_color", light->color);

	mesh->render(GL_TRIANGLES); //Render the mesh for every light
}

//disable shader
shader->disable();

glDisable(GL_BLEND);
glDepthFunc(GL_LESS); //as default
*/