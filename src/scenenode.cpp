#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastLightId = 0;
unsigned int mesh_selected = 0;
unsigned int material_selected = 0;
unsigned int environment_selected = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
	mesh = Mesh::Get("data/meshes/sphere.obj");

	std::cout << "SceneNode: " + std::to_string(SceneNode::lastNameId) + name << std::endl;

	material = new PhongMaterial();
	material->texture = Texture::Get("data/models/ball/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
}


SceneNode::SceneNode(const char * name)
{
	this->name = name;
	mesh = Mesh::Get("data/meshes/sphere.obj");

	material = new PhongMaterial();
	material->texture = Texture::Get("data/models/ball/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
}

SceneNode::~SceneNode()
{
	material->shader->~Shader();
	material->texture->~Texture();
	mesh->~Mesh();
}

void SceneNode::render(Camera* camera)
{
	if (material)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		
		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Material Type", (int*)&material_selected, "PHONG\0REFLECTIVE\0TEXTURED\0");
		if (changed)
		{
			Texture* texture = material->texture;

			switch (material_selected)
			{
				case 0: material = new PhongMaterial(); material->texture = texture; break;
				case 1: material = new ReflectiveMaterial(); break;
				case 2: material = new StandardMaterial(); material->texture = texture; break;
			}
		}

		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0BOX\0HELMET\0BENCH\0LANTERN\0");
		if (changed)
		{
			switch (mesh_selected)
			{
			case 0: mesh = Mesh::Get("data/meshes/sphere.obj"); material->texture = Texture::Get("data/models/ball/albedo.png");break;
			case 1: mesh = Mesh::Get("data/meshes/box.ASE"); material->texture = Texture::Get("data/models/basic/albedo.png"); break;
			case 2: mesh = Mesh::Get("data/models/helmet/helmet.obj"); material->texture = Texture::Get("data/models/helmet/albedo.png"); break;
			case 3: mesh = Mesh::Get("data/models/bench/bench.obj"); material->texture = Texture::Get("data/models/bench/albedo.png"); break;
			case 4: mesh = Mesh::Get("data/models/lantern/lantern.obj"); material->texture = Texture::Get("data/models/lantern/albedo.png"); break;
			}

		}
		ImGui::TreePop();
	}
}

Light::Light()
{
	this->name = std::string("Light" + std::to_string(lastLightId++));

	model.setTranslation(10.0f, 10.0f, 10.0f);
	diffuse = vec3(1.0f, 1.0f, 1.0f);
	specular = vec3(1.0f, 1.0f, 1.0f);

	lastNameId--;
}

void Light::renderInMenu()
{
	ImGui::ColorEdit3("Diffuse Color", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular Color", (float*)&specular); // Edit 3 floats representing a color

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
}

Skybox::Skybox()
{
	name = "Skybox";
	mesh = Mesh::Get("data/meshes/box.ASE");
	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
	
	HDRE* sky = HDRE::Get("data/environments/panorama.hdre");
	material->texture = new Texture();
	material->texture->cubemapFromHDRE(sky);

	lastNameId--;
}

void Skybox::render(Camera* camera)
{
	model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
	material->render(mesh, model, camera);
}

void Skybox::renderInMenu()
{
	bool changed = false;
	changed |= ImGui::Combo("Environment", (int*)&environment_selected, "PANORAMA\0PISA\0SAN GIUSEPPE BRIDGE\0STUDIO\0TV STUDIO\0CITY\0DRAGONVALE\0SNOW\0");
	if (changed)
	{
		HDRE* sky;
		switch (environment_selected)
		{
		case 0: sky = HDRE::Get("data/environments/panorama.hdre"); break;
		case 1: sky = HDRE::Get("data/environments/pisa.hdre"); break;
		case 2: sky = HDRE::Get("data/environments/san_giuseppe_bridge.hdre"); break;
		case 3: sky = HDRE::Get("data/environments/studio.hdre"); break;
		case 4: sky = HDRE::Get("data/environments/tv_studio.hdre"); break;
		case 5: material->texture->cubemapFromImages("data/environments/city"); break;
		case 6: material->texture->cubemapFromImages("data/environments/dragonvale"); break;
		case 7: material->texture->cubemapFromImages("data/environments/snow"); break;
		}
		//material->texture = new Texture();
		if (environment_selected < 5)
			material->texture->cubemapFromHDRE(sky);
	}
}
