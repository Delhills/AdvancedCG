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
<<<<<<< Updated upstream
=======
		bool changed = false;
		changed |= ImGui::Combo("Material Type", (int*)&material_selected, "PHONG\0REFLECTIVE\0TEXTURED\0WIREFRAME\0");
		if (changed)
		{
			switch (material_selected)
			{
			case 0: material = new PhongMaterial(); break;
			case 1: material = new ReflectiveMaterial(); break;
			case 2: material = new TexturedMaterial(); break;
			case 3: material = new WireframeMaterial(); break;
			}

			//Setting the texture once the material has been changed and uses texture
			if (material_selected == 0 || material_selected == 2)
			{
				switch (texture_selected)
				{
				case 0: material->texture = Texture::Get("data/models/ball/albedo.png"); break;
				case 1: material->texture = Texture::Get("data/models/basic/albedo.png"); break;
				case 2: material->texture = Texture::Get("data/models/bench/albedo.png"); break;
				case 3: material->texture = Texture::Get("data/models/helmet/albedo.png"); break;
				case 4: material->texture = Texture::Get("data/models/lantern/albedo.png"); break;
				}
			}
		}

>>>>>>> Stashed changes
		material->renderInMenu();
		ImGui::TreePop();
	}

<<<<<<< Updated upstream
=======
	//Texture
	if (material->texture && ImGui::TreeNode("Texture"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Texture", (int*)&texture_selected, "BALL\0BOX\0BENCH\0HELMET\0LANTERN\0");
		if (changed)
		{
			switch (texture_selected)
			{
			case 0: material->texture = Texture::Get("data/models/ball/albedo.png"); break;
			case 1: material->texture = Texture::Get("data/models/basic/albedo.png"); break;
			case 2: material->texture = Texture::Get("data/models/bench/albedo.png"); break;
			case 3: material->texture = Texture::Get("data/models/helmet/albedo.png"); break;
			case 4: material->texture = Texture::Get("data/models/lantern/albedo.png"); break;
			}
		}

		ImGui::TreePop();
	}

>>>>>>> Stashed changes
	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0HELMET\0");

		ImGui::TreePop();
	}
}
<<<<<<< Updated upstream
=======

Light::Light()
{
	//Setting name
	this->name = std::string("Light" + std::to_string(lastLightId++));

	//Setting default parameters
	model.setTranslation(10.0f, 10.0f, 10.0f);
	diffuse = vec3(1.0f, 1.0f, 1.0f);
	specular = vec3(1.0f, 1.0f, 1.0f);

	//Not adding a node name id
	lastNameId--;
}

void Light::renderInMenu()
{
	//Light properties
	ImGui::ColorEdit3("Diffuse Color", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular Color", (float*)&specular); // Edit 3 floats representing a color

	//Position
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
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
	material->texture->cubemapFromImages("data/environments/dragonvale");

	//Not adding a node name id
	lastNameId--;
}

void Skybox::render(Camera* camera)
{
	//Setting the center at the camera center
	model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);

	//Render skybox
	SceneNode::render(camera);
}

void Skybox::renderInMenu()
{
	bool changed = false;
	changed |= ImGui::Combo("Environment", (int*)&environment_selected, "DRAGONVALE\0CITY\0SNOW\0");
	if (changed)
	{
		switch (environment_selected)
		{
		case 0: material->texture->cubemapFromImages("data/environments/dragonvale"); break;
		case 1: material->texture->cubemapFromImages("data/environments/city"); break;
		case 2: material->texture->cubemapFromImages("data/environments/snow"); break;
		}
	}
}
>>>>>>> Stashed changes
