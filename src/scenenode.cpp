#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastLightId = 0;
unsigned int environment_selected = 0;

SceneNode::SceneNode()
{
	//Setting name
	this->name = std::string("Node" + std::to_string(lastNameId++));

	//Setting default mesh and material
	mesh = Mesh::Get("data/models/helmet/helmet.obj");
	material = new PBRMaterial();
	material->texture = Texture::Get("data/models/helmet/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");

	distance_to_cam = 0;
}


SceneNode::SceneNode(const char * name)
{
	//Setting name
	this->name = name;

	//Setting default mesh and material
	mesh = Mesh::Get("data/meshes/sphere.obj");
	material = new PhongMaterial();
	material->texture = Texture::Get("data/models/ball/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");

	distance_to_cam = 0;
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

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Material Type", (int*)&material_selected, "PBR\0PHONG\0REFLECTIVE\0TEXTURED\0WIREFRAME\0");
		if (changed)
		{
			switch (material_selected)
			{
			case 0: material = new PBRMaterial(); break;
			case 1: material = new PhongMaterial(); break;
			case 2: material = new ReflectiveMaterial(); break;
			case 3: material = new TexturedMaterial(); break;
			case 4: material = new WireframeMaterial(); break;
			}

			if (material_selected == 0 || material_selected == 2)
			{
				std::string geometry;
				switch (mesh_selected)
				{
				case 0: geometry = "ball"; break;
				case 1: geometry = "basic"; break;
				case 2: geometry = "helmet"; break;
				case 3: geometry = "bench"; break;
				case 4: geometry = "lantern"; break;
				}

				material->setTexture(geometry, mesh_selected);
			}
		}

		material->renderInMenu();
		ImGui::TreePop();
	}

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

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0BOX\0HELMET\0BENCH\0LANTERN\0");
		if (changed)
		{
			std::string geometry;
			switch (mesh_selected)
			{
			case 0: 
				mesh = Mesh::Get("data/meshes/sphere.obj"); 
				geometry = "ball";
				texture_selected = 0; 
				break;
			case 1: 
				mesh = Mesh::Get("data/meshes/box.ASE");
				geometry = "basic";
				texture_selected = 1; 
				break;
			case 2: 
				mesh = Mesh::Get("data/models/helmet/helmet.obj"); 
				geometry = "helmet";
				texture_selected = 3; 
				break;
			case 3: 
				mesh = Mesh::Get("data/models/bench/bench.obj"); 
				geometry = "bench";
				texture_selected = 2; 
				break;
			case 4:
				mesh = Mesh::Get("data/models/lantern/lantern.obj");
				geometry = "lantern";
				texture_selected = 4; 
				break;
			}

			material->setTexture(geometry, mesh_selected);
		}
		ImGui::TreePop();
	}
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
	//Light properties
	ImGui::ColorEdit3("Diffuse Color", (float*)&diffuse); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular Color", (float*)&specular); // Edit 3 floats representing a color
	ImGui::DragFloat("Intenity", (float*)&intensity, 0.1f, 0.0f, 100.f);
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color

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
	HDRE* hdre = HDRE::Get("data/environments/PANORAMA.hdre");

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
		HDRE* hdre;
		switch (environment_selected)
		{
		case 0: hdre = HDRE::Get("data/environments/panorama.hdre"); break;
		case 1: hdre = HDRE::Get("data/environments/pisa.hdre"); break;
		case 2: hdre = HDRE::Get("data/environments/san_giuseppe_bridge.hdre"); break;
		case 3: hdre = HDRE::Get("data/environments/studio.hdre"); break;
		case 4: hdre = HDRE::Get("data/environments/tv_studio.hdre"); break;
		}


		material->texture->cubemapFromHDRE(hdre, 0);
		((SkyboxMaterial*)material)->texture_prem_0->cubemapFromHDRE(hdre, 1);
		((SkyboxMaterial*)material)->texture_prem_1->cubemapFromHDRE(hdre, 2);
		((SkyboxMaterial*)material)->texture_prem_2->cubemapFromHDRE(hdre, 3);
		((SkyboxMaterial*)material)->texture_prem_3->cubemapFromHDRE(hdre, 4);
		((SkyboxMaterial*)material)->texture_prem_4->cubemapFromHDRE(hdre, 5);
	}
}

/*switch (mesh_selected)
			{
			case 0: 
				mesh = Mesh::Get("data/meshes/sphere.obj"); 
				material->texture = Texture::Get("data/models/ball/albedo.png"); 
				((PBRMaterial*)material)->normal_texture = Texture::Get("data/models/ball/normal.png");
				((PBRMaterial*)material)->ao_texture = Texture::getWhiteTexture();
				((PBRMaterial*)material)->metallic_roughness_texture = NULL;
				((PBRMaterial*)material)->metallic_texture = Texture::Get("data/models/ball/metalness.png");
				((PBRMaterial*)material)->roughness_texture = Texture::Get("data/models/ball/roughness.png");
				((PBRMaterial*)material)->emissive_texture = Texture::getBlackTexture();
				((PBRMaterial*)material)->opacity_texture = Texture::getWhiteTexture();
				((PBRMaterial*)material)->BRDFlut = Texture::Get("data/brdfLUT.png");
				texture_selected = 0; 
				break;
			case 1: 
				mesh = Mesh::Get("data/meshes/box.ASE");
				material->texture = Texture::Get("data/models/basic/albedo.png"); 
				((PBRMaterial*)material)->normal_texture = Texture::Get("data/models/basic/normal.png");
				((PBRMaterial*)material)->ao_texture = Texture::getWhiteTexture();
				((PBRMaterial*)material)->metallic_roughness_texture = NULL;
				((PBRMaterial*)material)->metallic_texture = Texture::Get("data/models/basic/metalness.png");
				((PBRMaterial*)material)->roughness_texture = Texture::Get("data/models/basic/roughness.png");
				((PBRMaterial*)material)->emissive_texture = Texture::getBlackTexture();
				((PBRMaterial*)material)->opacity_texture = Texture::getWhiteTexture();
				((PBRMaterial*)material)->BRDFlut = Texture::Get("data/brdfLUT.png");
				texture_selected = 1; 
				break;
			case 2: 
				mesh = Mesh::Get("data/models/helmet/helmet.obj"); 
				material->texture = Texture::Get("data/models/helmet/albedo.png");
				((PBRMaterial*)material)->normal_texture = Texture::Get("data/models/helmet/normal.png");
				((PBRMaterial*)material)->ao_texture = Texture::Get("data/models/helmet/ao.png");
				((PBRMaterial*)material)->metallic_roughness_texture = Texture::Get("data/models/helmet/roughness.png");
				((PBRMaterial*)material)->metallic_texture = NULL;
				((PBRMaterial*)material)->roughness_texture = NULL;
				((PBRMaterial*)material)->emissive_texture = Texture::Get("data/models/helmet/emissive.png");
				((PBRMaterial*)material)->opacity_texture = Texture::getWhiteTexture();
				((PBRMaterial*)material)->BRDFlut = Texture::Get("data/brdfLUT.png");
				texture_selected = 3; 
				break;
			case 3: 
				mesh = Mesh::Get("data/models/bench/bench.obj"); 
				material->texture = Texture::Get("data/models/lantern/albedo.png");
				((PBRMaterial*)material)->normal_texture = Texture::Get("data/models/lantern/normal.png");
				((PBRMaterial*)material)->ao_texture = Texture::Get("data/models/lantern/ao.png");
				((PBRMaterial*)material)->metallic_roughness_texture = NULL;
				((PBRMaterial*)material)->metallic_texture = Texture::Get("data/models/lantern/metalness.png");
				((PBRMaterial*)material)->roughness_texture = Texture::Get("data/models/lantern/roughness.png");
				((PBRMaterial*)material)->emissive_texture = Texture::Get("data/models/lantern/emissive.png");
				((PBRMaterial*)material)->opacity_texture = Texture::Get("data/models/lantern/opacity.png");
				((PBRMaterial*)material)->BRDFlut = Texture::Get("data/brdfLUT.png");
				texture_selected = 2; 
				break;
			case 4:
				mesh = Mesh::Get("data/models/lantern/lantern.obj");
				material->texture = Texture::Get("data/models/lantern/albedo.png");
				((PBRMaterial*)material)->normal_texture = Texture::Get("data/models/lantern/normal.png");
				((PBRMaterial*)material)->ao_texture = Texture::Get("data/models/lantern/ao.png");
				((PBRMaterial*)material)->metallic_roughness_texture = NULL;
				((PBRMaterial*)material)->metallic_texture = Texture::Get("data/models/lantern/metalness.png");
				((PBRMaterial*)material)->roughness_texture = Texture::Get("data/models/lantern/roughness.png");
				((PBRMaterial*)material)->emissive_texture = Texture::getBlackTexture();
				((PBRMaterial*)material)->opacity_texture = Texture::Get("data/models/lantern/opacity.png");
				((PBRMaterial*)material)->BRDFlut = Texture::Get("data/brdfLUT.png");
				texture_selected = 4; 
				break;*/
