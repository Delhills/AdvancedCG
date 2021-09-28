#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int mesh_selected = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}


SceneNode::SceneNode(const char * name)
{
	this->name = name;
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
		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0HELMET\0");

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
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

void Light::uploadLightParams(Shader* sh, bool linearize, float& hdr_gamma)
{

}

Skybox::Skybox()
{
	mesh = Mesh::Get("data/meshes/box.ASE");
	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
	HDRE* sky = HDRE::Get("data/environments/pisa.hdre");
	material->texture = new Texture();
	material->texture->cubemapFromHDRE(sky);
}

void Skybox::render(Camera* camera)
{
	model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
	material->render(mesh, model, camera);
}
