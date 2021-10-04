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

LightNode::LightNode()
{
	name = "Light";
	color.set(1.0, 0, 1.0);
	intensity = 1.0;
	max_distance = 1000.0;

	diffuseColor.set(0.6f, 0.6f, 0.6f);
	specularColor.set(0.6f, 0.6f, 0.6f);

	model.setTranslation(10.0, 10.0, 10.0);
}

void LightNode::renderInMenu()
{
	SceneNode::renderInMenu();

	ImGui::ColorEdit3("Diffuse color", diffuseColor.v);
	ImGui::ColorEdit3("Specular color", specularColor.v);
	ImGui::SliderFloat("Intensity", &intensity, 0, 50);
	ImGui::SliderFloat("Max distance", &max_distance, 0, 3700);

}

SkyboxNode::SkyboxNode()
{
	name = "Skybox";

	mesh = Mesh::Get("data/meshes/box.ASE");

	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");

	HDRE* sky = HDRE::Get("data/environments/panorama.hdre");
	material->texture = new Texture();
	material->texture->cubemapFromHDRE(sky);

}

void SkyboxNode::render(Camera* camera)
{
	{

		model.translate(camera->eye.x, camera->eye.y, camera->eye.z);

		glDisable(GL_DEPTH_TEST);

		material->render(mesh, model, camera);

		glEnable(GL_CULL_FACE);

	}
}

void SkyboxNode::renderInMenu()
{
}
