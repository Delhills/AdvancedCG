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
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");

	color.set(1.f, 1.f, 1.f, 1.f);

	ambient.set(1, 1, 1); //reflected ambient light
	diffuse.set(1, 1, 1); //reflected diffuse light
	specular.set(1, 1, 1); //reflected specular light
	shininess = 30.0; //glosiness coefficient (plasticity)
}

PhongMaterial::~PhongMaterial()
{

}

void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
{

	StandardMaterial::setUniforms(camera, model);

	//phong uniforms
	shader->setUniform("u_material_ambient", ambient);
	shader->setUniform("u_material_diffuse", diffuse);
	shader->setUniform("u_material_specular", specular);
	shader->setUniform("u_material_gloss", shininess);
}

void PhongMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_ONE, GL_ONE);
		
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);
		shader->setUniform("u_ambient_light", Application::instance->ambient_light);

		std::vector< LightNode* > lights = Application::instance->light_list;

		//do the draw call
		for (size_t i = 0; i < lights.size() && i < 5; i++) {
			LightNode* light = lights[i];

			shader->setUniform("u_light_pos", light->model.getTranslation());
			shader->setUniform("u_light_diff", light->diffuseColor);
			shader->setUniform("u_light_spec", light->specularColor);
			shader->setUniform("u_light_intensity", light->intensity);

			if (i != 0) {
				shader->setUniform("u_ambient_light", Vector3(0.0, 0.0, 0.0));

				glDepthFunc(GL_LEQUAL);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				glEnable(GL_BLEND);
			}
			mesh->render(GL_TRIANGLES);
		}
		glDepthFunc(GL_LESS);

		//disable shader
		shader->disable();
	}
}

void PhongMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Reflected ambient", (float*)&ambient); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Reflected diffuse", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Reflected specular", (float*)&specular); // Edit 3 floats representing a color
	ImGui::SliderFloat("Shininess", (float*)&shininess, 0.0f, 50.0f);
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