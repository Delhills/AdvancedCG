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
		shader->setUniform("u_texture", texture);
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
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
	ambient = vec3(1.f, 1.f, 1.f);
	diffuse = vec3(1.f, 1.f, 1.f);
	specular = vec3(1.f, 1.f, 1.f);
	shininess = 256.0f;
}

PhongMaterial::~PhongMaterial()
{

}

void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
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
		shader->setUniform("u_texture", texture);

	shader->setUniform("material_ambient", ambient);
	shader->setUniform("material_diffuse", diffuse);
	shader->setUniform("material_specular", specular);
	shader->setUniform("material_shininess", shininess);
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

		shader->setUniform("light_ambient", vec3(0.5, 0.5, 0.5));

		//Recorremos la luces de las escena
		for (int i = 0; i < Application::instance->light_list.size(); ++i)
		{
			Light* light = Application::instance->light_list[i];

			if (i != 0) //A partir de la primera luz usamos blending (muti pass)
			{
				glEnable(GL_BLEND);
				shader->setVector3("light_ambient", Vector3(0, 0, 0));
			}

			light->model.translate(0.0f, 0.0f, 10.0f);
			shader->setUniform("light_position", light->model * vec3(0.0f, 0.0f, 0.0f));
			shader->setUniform("light_diffuse", light->diffuse);
			shader->setUniform("light_specular", light->specular);

			mesh->render(GL_TRIANGLES);
		}


		mesh->render(GL_TRIANGLES);

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
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/reflective.fs");
	texture = Application::instance->sky->material->texture;
	reflectivity = 0.0;
}

ReflectiveMaterial::~ReflectiveMaterial()
{
}

void ReflectiveMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_color", color);
	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	shader->setUniform("u_environment_texture", texture);
	shader->setUniform("u_reflectivity", reflectivity);
}

void ReflectiveMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
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

void ReflectiveMaterial::renderInMenu()
{
	ImGui::DragFloat("Reflectivity", (float*)&reflectivity, 1.0f, 0.0f, 10.f);
}
