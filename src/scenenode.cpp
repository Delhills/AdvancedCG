#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"
#include "volume.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastLightId = 0;
unsigned int environment_selected = 0;

//Since there is only one node, set these parameters as global
unsigned int volume_selected = 5;
bool isourface = false;

SceneNode::SceneNode()
{
	//Setting name
	this->name = std::string("Node" + std::to_string(lastNameId++));

	//Set material
	material = new VolumeMaterial();
	((VolumeMaterial*)material)->setVolumeProperties(volume_selected, model);
	
	//Set mesh
	mesh = new Mesh();
	mesh->createCube();
}


SceneNode::SceneNode(const char * name)
{
	//Setting name
	this->name = name;

	//Set material
	material = new VolumeMaterial();
	((VolumeMaterial*)material)->setVolumeProperties(volume_selected, model);

	//Set mesh
	mesh = new Mesh();
	mesh->createCube();
}

SceneNode::~SceneNode()
{
	//Destroying stored data
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

	//Change volume
	bool changed = false;
	changed |= ImGui::Combo("Volume", (int*)&volume_selected, "BONSAI\0ABDOMEN\0DAISY\0FOOT\0ORANGE\0TEAPOT\0");
	if (changed)
	{
		//Set new volume properties
		((VolumeMaterial*)material)->setVolumeProperties(volume_selected, model);
	}

	//Isosurface mode
	changed = false;
	changed |= ImGui::Checkbox("Isosurface", (bool*)&isourface);
	if (changed)
	{
		//Set material
		if (isourface)
			material = new VolumeMaterialPhong();
		else
			material = new VolumeMaterial();

		//Set properties
		((VolumeMaterial*)material)->setVolumeProperties(volume_selected, model);
	}

	//Render material menu
	material->renderInMenu();
}

Light::Light()
{
	//Setting name
	this->name = std::string("Light" + std::to_string(lastLightId++));

	//Setting default parameters
	model.setTranslation(10.0f, 10.0f, 10.0f);
	diffuse = vec3(1.0f, 1.0f, 1.0f);
	specular = vec3(1.0f, 1.0f, 1.0f);
	intensity = 1.0;
	color = vec3(1.0f, 1.0f, 1.0f);

	//Not adding a node name id
	lastNameId--;
}

void Light::renderInMenu()
{
	//Position
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

	//Phong parameters
	ImGui::ColorEdit3("Diffuse Color", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular Color", (float*)&specular); // Edit 3 floats representing a color
}

Skybox::Skybox()
{
	//Set name
	name = "Skybox";

	//Set cube as the mesh for the cubemap
	mesh = Mesh::Get("data/meshes/box.ASE");

	//A skybox material 
	material = new SkyboxMaterial();
	
	//Set panorama as the default skybox
	material->texture = new Texture();
	HDRE* hdre = HDRE::Get("data/environments/PANORAMA.hdre");

	//Setting the different cubemap levels
	material->texture->cubemapFromHDRE(hdre, 0);
	((SkyboxMaterial*)material)->texture_prem_0->cubemapFromHDRE(hdre, 1);
	((SkyboxMaterial*)material)->texture_prem_1->cubemapFromHDRE(hdre, 2);
	((SkyboxMaterial*)material)->texture_prem_2->cubemapFromHDRE(hdre, 3);
	((SkyboxMaterial*)material)->texture_prem_3->cubemapFromHDRE(hdre, 4);
	((SkyboxMaterial*)material)->texture_prem_4->cubemapFromHDRE(hdre, 5);

	//Not adding a node name id
	lastNameId--;
}

void Skybox::render(Camera* camera)
{
	//Setting the center at the camera center
	model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);

	//Render skybox
	material->render(mesh, model, camera);
}

void Skybox::renderInMenu()
{
	bool changed = false;
	changed |= ImGui::Combo("Environment", (int*)&environment_selected, "PANORAMA\0PISA\0SAN GIUSEPPE BRIDGE\0STUDIO\0TV STUDIO");
	if (changed)
	{
		//Selecting the HDRe
		HDRE* hdre;
		switch (environment_selected)
		{
		case 0: hdre = HDRE::Get("data/environments/panorama.hdre"); break;
		case 1: hdre = HDRE::Get("data/environments/pisa.hdre"); break;
		case 2: hdre = HDRE::Get("data/environments/san_giuseppe_bridge.hdre"); break;
		case 3: hdre = HDRE::Get("data/environments/studio.hdre"); break;
		case 4: hdre = HDRE::Get("data/environments/tv_studio.hdre"); break;
		}

		//Setting the different level textures
		material->texture->cubemapFromHDRE(hdre, 0);
		((SkyboxMaterial*)material)->texture_prem_0->cubemapFromHDRE(hdre, 1);
		((SkyboxMaterial*)material)->texture_prem_1->cubemapFromHDRE(hdre, 2);
		((SkyboxMaterial*)material)->texture_prem_2->cubemapFromHDRE(hdre, 3);
		((SkyboxMaterial*)material)->texture_prem_3->cubemapFromHDRE(hdre, 4);
		((SkyboxMaterial*)material)->texture_prem_4->cubemapFromHDRE(hdre, 5);
	}
}