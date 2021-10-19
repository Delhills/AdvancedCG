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
	shader->~Shader(); //Destroy the shader
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
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");

	roughness_factor = 1.0;
	metallic_factor = 1.0;
	emission_factor = 1.0;

	use_metaltxt = true;
	use_aotxt = true;
	use_emissivetxt = true;

	normal_texture = NULL;
	ao_texture = NULL;
	roughness_texture = NULL;
	metallic_texture = NULL;
	emissive_texture = NULL;
	BRDFlut = NULL;
	
	//normal_texture = Texture::Get("data/models/helmet/normal.png");
	//ao_texture = Texture::Get("data/models/helmet/ao.png");
	//roughness_texture = Texture::Get("data/models/helmet/roughness.png");
	//metallic_texture = Texture::Get("data/models/helmet/metalness.png");
	//emissive_texture = Texture::Get("data/models/helmet/emissive.png");
	//BRDFlut = Texture::Get("data/brdfLUT.png");

}

PBRMaterial::~PBRMaterial()
{
}

void PBRMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	StandardMaterial::setUniforms(camera, model);

	shader->setUniform("u_normal_texture", normal_texture, 1);
	if(use_emissivetxt)	shader->setUniform("u_emissive_texture", emissive_texture, 2);
	if (use_metaltxt) {
		std::cout << "ta bien" << "\n";
		shader->setUniform("u_metallic_texture", metallic_texture, 3);
		shader->setUniform("u_roughness_texture", roughness_texture, 4);
	}
	else {
		std::cout << "ta bien 2" << "\n";
		shader->setUniform("u_metallic_texture", roughness_texture, 3);
	}
	if (use_aotxt) shader->setUniform("u_ao_texture", ao_texture, 5);

	shader->setUniform("u_use_emissivetxt", use_emissivetxt);
	shader->setUniform("u_use_metaltxt", use_metaltxt);
	shader->setUniform("u_use_aotxt", use_aotxt);

	shader->setUniform("u_roughness_factor", roughness_factor);
	shader->setUniform("u_metallic_factor", metallic_factor);
	shader->setUniform("u_emission_factor", emission_factor);

	shader->setUniform("u_lut", BRDFlut, 6);

	SkyboxMaterial* sky = (SkyboxMaterial*)Application::instance->sky->material;
	shader->setUniform("u_environment_texture", sky->texture, 7);
	shader->setUniform("u_texture_prem_0", sky->texture_prem_0, 8);
	shader->setUniform("u_texture_prem_1", sky->texture_prem_1, 9);
	shader->setUniform("u_texture_prem_2", sky->texture_prem_2, 10);
	shader->setUniform("u_texture_prem_3", sky->texture_prem_3, 11);
	shader->setUniform("u_texture_prem_4", sky->texture_prem_4, 12);
}

void PBRMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
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
		shader->setUniform("u_first_pass", true);
		int output = Application::instance->output;
		shader->setUniform("u_output", output);

		//Use all the lights
		for (int i = 0; i < lights.size(); ++i)
		{
			Light* light = lights[i];

			if (i == 1) //Blending on after first pass and no ambient light
			{
				glEnable(GL_BLEND);
				shader->setVector3("u_light_ambient", Vector3(0, 0, 0));
				int first_pass = 0;
				shader->setUniform("u_first_pass", first_pass);
			}

			//Pass light parameters
			shader->setUniform("u_light_position", light->model.getTranslation());
			shader->setUniform("u_light_diffuse", light->diffuse);
			shader->setUniform("u_light_specular", light->specular);
			shader->setUniform("u_light_intensity", light->intensity);

			mesh->render(GL_TRIANGLES); //Render the mesh for every light
		}

		//disable shader
		shader->disable();

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS); //as default
	}
}

void PBRMaterial::renderInMenu()
{
	ImGui::SliderFloat("Roughness Factor", (float*)&roughness_factor, 0.01, 1.00);
	ImGui::SliderFloat("Metallic Factor", (float*)&metallic_factor, 0.01, 1.00);
	ImGui::SliderFloat("Emission Factor", (float*)&emission_factor, 0.01, 1.00);
}
