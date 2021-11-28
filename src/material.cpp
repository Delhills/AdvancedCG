#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"
#include "volume.h"

const int MAX_LIGHTS = 100;

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
	//In this case the mesh is not used because in this function at the moment is not necessary, 
	//but since we are overwriting an abstact method we needed, having a more agile code thanks to this functions
	texture = Texture::Get(("data/models/" + geometry + "/albedo.png").c_str()); //Set albedo texture
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
	//Set PBR shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");

	//Set helmet textures as default
	normal_texture = Texture::Get("data/models/Lee/normal.png");
	ao_texture = Texture::getWhiteTexture();//Texture::Get("data/models/helmet/ao.png");
	metallic_texture = NULL;
	roughness_texture = Texture::getBlackTexture();//Texture::Get("data/models/helmet/roughness.png");;
	emissive_texture = Texture::getBlackTexture();//Texture::Get("data/models/helmet/emissive.png");
	opacity_texture = Texture::getWhiteTexture();

	metallic_roughness = true;
	roughness = 1.0;
	metalness = 1.0;
	subsurface = 1.0;
	diffuse_type = 0;
}

PBRMaterial::~PBRMaterial()
{
	//Destroy stored data
	texture->~Texture();
	normal_texture->~Texture();
	ao_texture->~Texture();
	metallic_texture->~Texture();
	roughness_texture->~Texture();
	emissive_texture->~Texture();
	opacity_texture->~Texture();
	shader->~Shader();
}

void PBRMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//Set common uniforms
	StandardMaterial::setUniforms(camera, model);

	//Set other textures
	shader->setUniform("u_normal_texture", normal_texture, 1);
	shader->setUniform("u_emissive_texture", emissive_texture, 2);
	shader->setUniform("u_opacity_texture", opacity_texture, 3);
	shader->setUniform("u_ao_texture", ao_texture, 4);
	shader->setUniform("u_lut", Application::instance->BRDFlut, 5);

	//Set metallic and roughness depending on how are stored
	shader->setUniform("u_metallic_roughness", metallic_roughness);
	if (metallic_roughness)
		shader->setUniform("u_roughness_texture", roughness_texture, 6);
	else
	{
		shader->setUniform("u_roughness_texture", roughness_texture, 6);
		shader->setUniform("u_metallic_texture", metallic_texture, 7);
	}

	//Set enviroment related cubemaps
	SkyboxMaterial* sky = (SkyboxMaterial*)Application::instance->sky->material;
	shader->setUniform("u_environment_texture", sky->texture, 8);
	shader->setUniform("u_texture_prem_0", sky->texture_prem_0, 9);
	shader->setUniform("u_texture_prem_1", sky->texture_prem_1, 10);
	shader->setUniform("u_texture_prem_2", sky->texture_prem_2, 11);
	shader->setUniform("u_texture_prem_3", sky->texture_prem_3, 12);
	shader->setUniform("u_texture_prem_4", sky->texture_prem_4, 13);

	//Set factors
	shader->setUniform("u_metallic_factor", metalness);
	shader->setUniform("u_roughness_factor", roughness);
	shader->setUniform("u_subsurface_factor", subsurface);
	shader->setUniform("u_diffuse_type", diffuse_type);
}

void PBRMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//Setting flags
		glDepthFunc(GL_LESS); 

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);

		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//SINGLE PASS RENDER
		std::vector< Light* > lights = Application::instance->light_list;

		//Defining the vectors that will be passed to the GPU
		Vector3 light_position[MAX_LIGHTS];
		Vector3 light_color[MAX_LIGHTS];
		float light_intensity[MAX_LIGHTS];

		//Filling the vectors
		for (int i = 0; i < lights.size(); ++i)
		{
			Light* light = lights[i];
			light_position[i] = light->model.getTranslation();
			light_color[i] = light->color;
			light_intensity[i] = light->intensity;
		}

		//Setting the uniforms
		shader->setUniform3Array("u_light_position", (float*)&light_position, MAX_LIGHTS);
		shader->setUniform3Array("u_light_color", (float*)&light_color, MAX_LIGHTS);
		shader->setUniform1Array("u_light_intensity", (float*)&light_intensity, MAX_LIGHTS);
		shader->setUniform1("u_num_lights", (float)lights.size());

		mesh->render(GL_TRIANGLES); //Render the mesh for all the lights
	}

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

void PBRMaterial::renderInMenu()
{
	ImGui::ColorEdit4("Color", (float*)&color); // Edit 4 floats representing a color and alpha channel
	ImGui::SliderFloat("Metalness", (float*)&metalness, 0.0f, 1.0f);
	ImGui::SliderFloat("Roughness", (float*)&roughness, 0.0f, 1.0f);
	ImGui::SliderFloat("Subsurface Scattering", (float*)&subsurface, 0.0f, 1.0f);
	ImGui::Combo("Diffuse Mode", (int*)&diffuse_type, "BURLEY\0HAMMON\0LAMBERT");
}

void PBRMaterial::setTexture(std::string geometry, int mesh)
{
	//Setting the albedo texture
	StandardMaterial::setTexture(geometry, mesh);

	//Setting normalmap
	normal_texture = Texture::Get(("data/models/" + geometry + "/normal.png").c_str());

	//Setting ambient occlusion
	ao_texture = Texture::Get(("data/models/" + geometry + "/ao.png").c_str());
	if (ao_texture == NULL)
		ao_texture = Texture::getWhiteTexture();

	//Setting emissive map
	emissive_texture = Texture::Get(("data/models/" + geometry + "/emissive.png").c_str());
	if (emissive_texture == NULL)
		emissive_texture = Texture::getBlackTexture();

	//Setting opacity map
	opacity_texture = Texture::Get(("data/models/" + geometry + "/opacity.png").c_str());
	if (opacity_texture == NULL)
		opacity_texture = Texture::getWhiteTexture();

	//Setting metalness and roughness
	if (mesh == 2) //If the mesh is the helmet...
	{
		metallic_roughness = true;
		metallic_texture = NULL;
		roughness_texture = Texture::Get(("data/models/" + geometry + "/roughness.png").c_str());;
	}
	else //If not...
	{
		metallic_roughness = false;
		metallic_texture = Texture::Get(("data/models/" + geometry + "/metalness.png").c_str());
		roughness_texture = Texture::Get(("data/models/" + geometry + "/roughness.png").c_str());
	}
}

VolumeMaterial::VolumeMaterial()
{
	//Create texture
	texture = new Texture();

	//Set Volume shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volume.fs");
	
	//Set default parameters
	step = 0.01;
	intensity = 1.0;
	clipping_plane = Vector4(0.5, 0.5, 0.5, -1.5);
	transfer_function = NULL;

	//Set default options
	check_jittering = false;
	check_transfer_function = false;
	check_clipping_plane = false;
}

VolumeMaterial::~VolumeMaterial()
{
	texture->~Texture();
	shader->~Shader();

	if (transfer_function)
		transfer_function->~Texture();
}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//Upload standard material uniforms
	StandardMaterial::setUniforms(camera, model);

	//Upload parameters
	shader->setUniform("u_intensity", intensity);
	shader->setUniform("u_step_length", step);
	shader->setUniform("u_clipping_plane", clipping_plane);

	//Visualization info
	shader->setUniform("u_threshold", threshold);
	shader->setUniform("u_noise_texture", Application::instance->noise, 1);
	shader->setUniform("u_texture_lut", transfer_function, 2);

	//Check for options
	shader->setUniform("u_jittering", check_jittering);
	shader->setUniform("u_transfer_function", check_transfer_function);
	shader->setUniform("u_clipping_plane_check", check_clipping_plane);

	//Upload inverse model
	Matrix44 inv_model = model;
	inv_model.inverse();
	shader->setUniform("u_inv_model", inv_model);
}

void VolumeMaterial::renderInMenu()
{
	//Use jittering
	ImGui::Checkbox("Jittering", &check_jittering);

	//Use transfer function
	ImGui::Checkbox("Transfer Function", &check_transfer_function);
	if (check_transfer_function)
	{
		if (num_intervals >= 1)
			ImGui::ColorEdit4("Color Interval 1", (float*)&color1); //Edit 4 floats representing a color and alpha channel
		if (num_intervals >= 2)
			ImGui::ColorEdit4("Color Interval 2", (float*)&color2); //Edit 4 floats representing a color and alpha channel
		if (num_intervals >= 3)
			ImGui::ColorEdit4("Color Interval 3", (float*)&color3); //Edit 4 floats representing a color and alpha channel

		//Set a new transfer function with the actual colors
		if (ImGui::Button("Set Transfer Function", ImVec2(200.0, 20.0)))
			setTransferFunction();
	}

	//Use clipping plane
	ImGui::Checkbox("Clipping", &check_clipping_plane);
	if (check_clipping_plane)
		ImGui::DragFloat4("Cutting Plane", (float*)&clipping_plane, 0.01f); //Edit 4 floats representing the plane

	//More parameters
	if (ImGui::TreeNode("Material"))
	{
		ImGui::ColorEdit4("Color", (float*)&color); //Edit 4 floats representing a color and alpha channel
		ImGui::SliderFloat("Step Length", (float*)&step, 0.001f, 0.1f);
		ImGui::SliderFloat("Brightness", (float*)&intensity, 0.0f, 100.0f);
		ImGui::SliderFloat("Lower Threshold (Isosurface Mode: Isusurface Value)", (float*)&threshold, 0.001f, 1.0f);
	}
}

void VolumeMaterial::setTransferFunction()
{
	//Pass from Vector4 (0.0-1.0) to Color (0-255)
	Color color1_ub = Color(color1.x * 255, color1.y * 255, color1.z * 255, color1.w * 255);
	Color color2_ub = Color(color2.x * 255, color2.y * 255, color2.z * 255, color2.w * 255);
	Color color3_ub = Color(color3.x * 255, color3.y * 255, color3.z * 255, color3.w * 255);

	//Create image for the texture and set the colors by interval
	Image* image = new Image(129, 1, 4);
	for (int i = 0; i < 129; i++)
	{
		if (i < int_1)
			image->setPixel(i, 0, color1_ub);
		else if (i < int_2)
			image->setPixel(i, 0, color2_ub);
		else if (i < int_3)
			image->setPixel(i, 0, color3_ub);
	}

	//Create texture
	Texture* text = new Texture(image);
	
	//Destroy old texture
	if (transfer_function)
		transfer_function->~Texture(); 
	
	//Set new transfer function
	transfer_function = text;
}

void VolumeMaterial::setVolumeProperties(int vol, Matrix44& model)
{
	//Create new volume
	Volume* volume;

	//Set parameters depending on selected volume
	switch (vol)
	{
	case 0:
		volume = Volume::Get("data/volumes/bonsai_16_16.png", PNG_VOL);
		threshold = 0.13;
		int_1 = 25;
		int_2 = 55;
		int_3 = 129;
		num_intervals = 3;
		color1 = Vector4(1 / 255.0, 107 / 255.0, 12 / 255.0, 255.0 / 255.0);
		color2 = Vector4(74 / 255.0, 51 / 255.0, 0 / 255.0, 255.0 / 255.0);
		color3 = Vector4(255 / 255.0, 114 / 255.0, 0 / 255.0, 255.0 / 255.0);
		break;
	case 1:
		volume = Volume::Get("data/volumes/CT-Abdomen.pvm", PVM_VOL);
		threshold = 0.075;
		int_1 = 31;
		int_2 = 48;
		int_3 = 129;
		num_intervals = 3;
		color1 = Vector4(192 / 255.0, 152 / 255.0, 93 / 255.0, 255.0 / 255.0);
		color2 = Vector4(164 / 255.0, 6 / 255.0, 6 / 255.0, 255.0 / 255.0);
		color3 = Vector4(255 / 255.0, 255 / 255.0, 255 / 255.0, 255.0 / 255.0);
		break;
	case 2:
		volume = Volume::Get("data/volumes/Daisy.pvm", PVM_VOL);
		threshold = 0.0;
		int_1 = 129;
		num_intervals = 1;
		color1 = Vector4(95 / 255.0, 8 / 255.0, 187 / 255.0, 255.0 / 255.0);
		break;
	case 3:
		volume = Volume::Get("data/volumes/foot_16_16.png", PNG_VOL);
		threshold = 0.06;
		int_1 = 42;
		int_2 = 129;
		num_intervals = 2;
		color1 = Vector4(192 / 255.0, 152 / 255.0, 93 / 255.0, 255.0 / 255.0);
		color2 = Vector4(255 / 255.0, 255 / 255.0, 255 / 255.0, 255.0 / 255.0);
		break;
	case 4:
		volume = Volume::Get("data/volumes/Orange.pvm", PVM_VOL);
		threshold = 0.06;
		int_1 = 26;
		int_2 = 129;
		num_intervals = 2;
		color1 = Vector4(255 / 255.0, 114 / 255.0, 0 / 255.0, 255.0 / 255.0);
		color2 = Vector4(189 / 255.0, 158 / 255.0, 33 / 255.0, 255.0 / 255.0);
		break;
	case 5:
		volume = Volume::Get("data/volumes/teapot_16_16.png", PNG_VOL);
		threshold = 0.1;
		int_1 = 28;
		int_2 = 62;
		int_3 = 129;
		num_intervals = 3;
		color1 = Vector4(74 / 255.0, 49 / 255.0, 0 / 255.0, 255.0 / 255.0);
		color2 = Vector4(11 / 255.0, 230 / 255.0, 35 / 255.0, 255.0 / 255.0);
		color3 = Vector4(242 / 255.0, 11 / 255.0, 11 / 255.0, 255.0 / 255.0);
		break;
	}

	//Set scale
	float width_size = volume->width * volume->widthSpacing;
	float height_size = volume->height * volume->heightSpacing;
	float depth_size = volume->depth * volume->depthSpacing;
	float max_size = max(width_size, max(height_size, depth_size));
	model.setScale(width_size / max_size, height_size / max_size, depth_size / max_size);

	//Create 3D texture
	texture->create3DFromVolume(volume);

	//Set new transfer function
	setTransferFunction();
}

VolumeMaterialPhong::VolumeMaterialPhong()
{
	//Setting the default parameters
	color = vec4(1.f, 1.f, 1.f, 1.f);
	ambient = vec3(1.f, 1.f, 1.f);
	diffuse = vec3(1.f, 1.f, 1.f);
	specular = vec3(1.f, 1.f, 1.f);
	shininess = 256.0f;
	h = 0.01;
	normals = false;

	//Set Volume shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volume_isosurfaces.fs");
}

VolumeMaterialPhong::~VolumeMaterialPhong()
{
}

void VolumeMaterialPhong::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	VolumeMaterial::setUniforms(camera, model);

	//Pass light parameters
	Light* light = Application::instance->light_list[0];
	shader->setUniform("u_light_ambient", Application::instance->ambient_light); //Pass ambient light for the first pass
	shader->setUniform("u_light_position", light->model.getTranslation());
	shader->setUniform("u_light_diffuse", light->diffuse);
	shader->setUniform("u_light_specular", light->specular);

	//Upload material related uniforms
	shader->setUniform("u_material_ambient", ambient);
	shader->setUniform("u_material_diffuse", diffuse);
	shader->setUniform("u_material_specular", specular);
	shader->setUniform("u_material_shininess", shininess);
	shader->setUniform("u_h", h);
	shader->setUniform("u_normals", normals);
}

void VolumeMaterialPhong::renderInMenu()
{
	//Render parameters in menu
	ImGui::Checkbox("Normals", &normals);
	ImGui::ColorEdit3("Ambient", (float*)&ambient); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Diffuse", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular", (float*)&specular); // Edit 3 floats representing a color
	ImGui::DragFloat("Shininess", (float*)&shininess, 1.0f, 1.0f, 256.f);
	ImGui::SliderFloat("h Value", (float*)&h, 0.001f, 0.1f);

	//Render VolumeMaterial menu
	VolumeMaterial::renderInMenu();
}