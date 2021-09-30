#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int mesh_selected = 0;
unsigned int material_selected = 0;
unsigned int environment_selected = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
	mesh = Mesh::Get("data/meshes/sphere.obj");

	material = new PhongMaterial();
	material->texture[0] = Texture::Get("data/models/ball/albedo.png");
	material->texture[1] = Texture::Get("data/models/ball/normal.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
}


SceneNode::SceneNode(const char * name)
{
	this->name = name;
	mesh = Mesh::Get("data/meshes/sphere.obj");

	material = new PhongMaterial();
	material->texture[0] = Texture::Get("data/models/ball/albedo.png");
	material->texture[1] = Texture::Get("data/models/ball/normal.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
}

SceneNode::~SceneNode()
{

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
			switch (material_selected)
			{
				case 0: material = new PhongMaterial(); break;
				case 1: material = new ReflectiveMaterial(); break;
				case 2: material = new StandardMaterial(); break;
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
			case 0: mesh = Mesh::Get("data/meshes/sphere.obj"); material->texture[0] = Texture::Get("data/models/ball/albedo.png"); material->texture[1] = Texture::Get("data/models/ball/normal.png"); break;
			case 1: mesh = Mesh::Get("data/meshes/box.ASE"); material->texture[0] = Texture::Get("data/models/basic/albedo.png"); material->texture[1] = Texture::Get("data/models/basic/normal.png"); break;
			case 2: mesh = Mesh::Get("data/models/helmet/helmet.obj"); material->texture[0] = Texture::Get("data/models/helmet/albedo.png"); material->texture[1] = Texture::Get("data/models/helmet/normal.png"); break;
			case 3: mesh = Mesh::Get("data/models/bench/bench.obj"); material->texture[0] = Texture::Get("data/models/bench/albedo.png"); material->texture[1] = Texture::Get("data/models/bench/normal.png"); break;
			case 4: mesh = Mesh::Get("data/models/lantern/lantern.obj"); material->texture[0] = Texture::Get("data/models/lantern/albedo.png"); material->texture[1] = Texture::Get("data/models/lantern/normal.png"); break;
			}

		}
		ImGui::TreePop();
	}
}

Light::Light()
{
	color = vec3(1.0f, 1.0f, 1.0f);
	intensity = 1.0f;
	max_distance = 10.0f;

	diffuse = vec3(1.0f, 1.0f, 1.0f);
	specular = vec3(0.5f, 0.5f, 0.5f);
}

void Light::renderInMenu()
{
	ImGui::DragFloat("Intensity", (float*)&intensity, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("Distance", (float*)&max_distance, 1.0f, 0.0f, 1000.0f);

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

	ImGui::TreePop();
}

void Light::uploadLightParams(Shader* sh, bool linearize, float& hdr_gamma)
{

}

Skybox::Skybox()
{
	name = "Skybox";
	mesh = Mesh::Get("data/meshes/box.ASE");
	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
	
	HDRE* sky = HDRE::Get("data/environments/panorama.hdre");
	material->texture[0] = new Texture();
	material->texture[0]->cubemapFromHDRE(sky);
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
		case 5: material->texture[0]->cubemapFromImages("data/environments/city"); break;
		case 6: material->texture[0]->cubemapFromImages("data/environments/dragonvale"); break;
		case 7:  material->texture[0]->cubemapFromImages("data/environments/snow"); break;
		}
		//material->texture = new Texture();
		if (environment_selected < 5)
			material->texture[0]->cubemapFromHDRE(sky);
	}
}
