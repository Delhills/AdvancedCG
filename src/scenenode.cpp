#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"
#include "volume.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastLightId = 0;
unsigned int environment_selected = 0;

SceneNode::SceneNode()
{
	//Setting name
	this->name = std::string("Node" + std::to_string(lastNameId++));

	//Setting default mesh and material as the helmet
	mesh = Mesh::Get("data/models/Lee/Lee.obj");
	material = new PBRMaterial();
	material->texture = Texture::Get("data/models/Lee/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");
	mesh_selected = 2;
	material_selected = 0;
	texture_selected = 3;

	//Setting distance to cam to minimum
	distance_to_cam = 0;
}


SceneNode::SceneNode(const char * name)
{
	//Setting name
	this->name = name;

	//Setting default mesh and material as the helmet
	mesh = Mesh::Get("data/models/Lee/Lee.obj");
	material = new PBRMaterial();
	material->texture = Texture::Get("data/models/Lee/albedo.png");
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");
	mesh_selected = 2;
	material_selected = 0;
	texture_selected = 3;

	//Setting distance to cam to minimum
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
			//Selecting the material type
			switch (material_selected)
			{
			case 0: material = new PBRMaterial(); break;
			case 1: material = new PhongMaterial(); break;
			case 2: material = new ReflectiveMaterial(); break;
			case 3: material = new TexturedMaterial(); break;
			case 4: material = new WireframeMaterial(); break;
			}

			//If it is a colored (light) material
			if (material_selected == 0 || material_selected == 1 || material_selected == 3)
			{
				//Selecting the geometry
				std::string geometry;
				switch (mesh_selected)
				{
				case 0: geometry = "ball"; break;
				case 1: geometry = "basic"; break;
				case 2: geometry = "helmet"; break;
				case 3: geometry = "bench"; break;
				case 4: geometry = "lantern"; break;
				}

				material->setTexture(geometry, mesh_selected); //Setting the textures
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
			//Selecting the geometry which textures will be applied
			std::string geometry;
			switch (texture_selected)
			{
			case 0: geometry = "ball"; break;
			case 1: geometry = "basic"; break;
			case 2: geometry = "bench"; break;
			case 3: geometry = "helmet"; break;
			case 4: geometry = "lantern"; break;
			}
			material->setTexture(geometry, mesh_selected); //Setting the textures
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
			//Select the corresponding geometry
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
				texture_selected = 3; //In texture menu they are changed, sorry
				break;
			case 3: 
				mesh = Mesh::Get("data/models/bench/bench.obj"); 
				geometry = "bench";
				texture_selected = 2; //In texture menu they are changed, sorry
				break;
			case 4:
				mesh = Mesh::Get("data/models/lantern/lantern.obj");
				geometry = "lantern";
				texture_selected = 4; 
				break;
			}

			material->setTexture(geometry, mesh_selected); //Set the textures
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
	//Position
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

	//PBR parameters
	if (ImGui::TreeNode("PBR")) {
		ImGui::DragFloat("Intensity", (float*)&intensity, 0.1f, 0.0f, 100.f);
		ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
		ImGui::TreePop();
	}

	//Phong parameters
	if (ImGui::TreeNode("Phong")) {
		ImGui::ColorEdit3("Diffuse Color", (float*)&diffuse); // Edit 3 floats representing a color
		ImGui::ColorEdit3("Specular Color", (float*)&specular); // Edit 3 floats representing a color
		ImGui::TreePop();
	}
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

VolumeNode::VolumeNode()
{
	material = new VolumeMaterial();

	mesh = new Mesh();
	mesh->createCube();

	int_1 = 28;
	int_2 = 62;
	int_3 = 129;
	num_intervals = 3;

	color1 = Vector4(74 / 255.0, 49 / 255.0, 0 / 255.0, 255.0 / 255.0);
	color2 = Vector4(11 / 255.0, 230 / 255.0, 35 / 255.0, 255.0 / 255.0);
	color3 = Vector4(242 / 255.0, 11 / 255.0, 11 / 255.0, 255.0 / 255.0);

	Color color1_ub = Color(color1.x * 255.0, color1.y * 255.0, color1.z * 255.0, color1.w * 255.0);
	Color color2_ub = Color(color2.x * 255.0, color2.y * 255.0, color2.z * 255.0, color2.w * 255.0);
	Color color3_ub = Color(color3.x * 255.0, color3.y * 255.0, color3.z * 255.0, color3.w * 255.0);
	Color color4_ub = Color(color4.x * 255.0, color4.y * 255.0, color4.z * 255.0, color4.w * 255.0);
	Image* image = new Image(129, 1, 4);
	for (int i = 0; i < 129; i++)
	{
		if (i < int_1)
			image->setPixel(i, 0, color1_ub);
		else if (i < int_2)
			image->setPixel(i, 0, color2_ub);
		else if (i < int_3)
			image->setPixel(i, 0, color3_ub);
		else if (i < int_4)
			image->setPixel(i, 0, color4_ub);
	}
	//image->flipY();
	Texture* text = new Texture(image);

	((VolumeMaterial*)material)->transfer_function = text;
}

void VolumeNode::renderInMenu()
{
	bool changed = false;
	changed |= ImGui::Combo("Volume", (int*)&volume_selected, "BONSAI\0ABDOMEN\0DAISY\0FOOT\0ORANGE\0TEAPOT\0");
	if (changed)
	{
		Volume* volume = new Volume();

		switch (volume_selected)
		{
		case 0: volume->loadPNG("data/volumes/bonsai_16_16.png"); 
			((VolumeMaterial*)material)->threshold = 0.13;
			int_1 = 25;
			int_2 = 55;
			int_3 = 129;
			num_intervals = 3;
			color1 = Vector4(1 / 255.0, 107 / 255.0, 12 / 255.0, 255.0 / 255.0);
			color2 = Vector4(74 / 255.0, 51 / 255.0, 0 / 255.0, 255.0 / 255.0);
			color3 = Vector4(255 / 255.0, 114 / 255.0, 0 / 255.0, 255.0 / 255.0);
			break;
		case 1: volume->loadPVM("data/volumes/CT-Abdomen.pvm");
			((VolumeMaterial*)material)->threshold = 0.075;
			int_1 = 31;
			int_2 = 48;
			//int_3 = 51;
			int_3 = 129;
			num_intervals = 3;
			color1 = Vector4(192 / 255.0, 152 / 255.0, 93 / 255.0, 255.0 / 255.0);
			color2 = Vector4(164 / 255.0, 6 / 255.0, 6 / 255.0, 255.0 / 255.0);
			//color3 = Vector4(106 / 255.0, 78 / 255.0, 25 / 255.0, 255.0 / 255.0);
			color3 = Vector4(255 / 255.0, 255 / 255.0, 255 / 255.0, 255.0 / 255.0);
			break;
		case 2: volume->loadPVM("data/volumes/Daisy.pvm"); 
			((VolumeMaterial*)material)->threshold = 0.0;
			int_1 = 129;
			num_intervals = 1;
			color1 = Vector4(95 / 255.0, 8 / 255.0, 187 / 255.0, 255.0 / 255.0);
			break;
		case 3: volume->loadPNG("data/volumes/foot_16_16.png"); 
			((VolumeMaterial*)material)->threshold = 0.02;
			int_1 = 42;
			int_2 = 129;
			num_intervals = 2;
			color1 = Vector4(192 / 255.0, 152 / 255.0, 93 / 255.0, 255.0 / 255.0);
			color2 = Vector4(255 / 255.0, 255 / 255.0, 255 / 255.0, 255.0 / 255.0);
			break;
		case 4: volume->loadPVM("data/volumes/Orange.pvm"); 
			((VolumeMaterial*)material)->threshold = 0.02;
			int_1 = 26;
			int_2 = 129;
			num_intervals = 2;
			color1 = Vector4(255 / 255.0, 114 / 255.0, 0 / 255.0, 255.0 / 255.0);
			color2 = Vector4(189 / 255.0, 158 / 255.0, 33 / 255.0, 255.0 / 255.0);
			break;
		case 5: volume->loadPNG("data/volumes/teapot_16_16.png"); 
			((VolumeMaterial*)material)->threshold = 0.1;
			int_1 = 28;
			int_2 = 62;
			int_3 = 129;
			num_intervals = 3;
			color1 = Vector4(74 / 255.0, 49 / 255.0, 0 / 255.0, 255.0 / 255.0);
			color2 = Vector4(11 / 255.0, 230 / 255.0, 35 / 255.0, 255.0 / 255.0);
			color3 = Vector4(242 / 255.0, 11 / 255.0, 11 / 255.0, 255.0 / 255.0);
			break;
		}

		material->texture->create3DFromVolume(volume);

		Color color1_ub = Color(color1.x * 255.0, color1.y * 255.0, color1.z * 255.0, color1.w * 255.0);
		Color color2_ub = Color(color2.x * 255.0, color2.y * 255.0, color2.z * 255.0, color2.w * 255.0);
		Color color3_ub = Color(color3.x * 255.0, color3.y * 255.0, color3.z * 255.0, color3.w * 255.0);
		Color color4_ub = Color(color4.x * 255.0, color4.y * 255.0, color4.z * 255.0, color4.w * 255.0);
		Image* image = new Image(129, 1, 4);
		for (int i = 0; i < 129; i++)
		{
			if (i < int_1)
				image->setPixel(i, 0, color1_ub);
			else if (i < int_2)
				image->setPixel(i, 0, color2_ub);
			else if (i < int_3)
				image->setPixel(i, 0, color3_ub);
			else if (i < int_4)
				image->setPixel(i, 0, color4_ub);
		}
		Texture* text = new Texture(image);

		((VolumeMaterial*)material)->transfer_function = text;// Texture::Get("data/volumes/bonsai.png", true, GL_CLAMP_TO_EDGE);
	}

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

	changed |= ImGui::Checkbox("Isosurface", &isourface);
	if (changed)
	{
		Texture* tf = ((VolumeMaterial*)material)->transfer_function;
		Texture* vol = ((VolumeMaterial*)material)->texture;
		float thres = ((VolumeMaterial*)material)->threshold;

		if (isourface)
			material = new VolumeMaterialPhong();
		else
			material = new VolumeMaterial();
		
		((VolumeMaterial*)material)->transfer_function = tf;
		((VolumeMaterial*)material)->texture = vol;
		((VolumeMaterial*)material)->threshold = thres;
	}

	if (num_intervals >= 1)
		ImGui::ColorEdit4("Color1", (float*)&color1); // Edit 4 floats representing a color and alpha channel
	if (num_intervals >= 2)
		ImGui::ColorEdit4("Color2", (float*)&color2); // Edit 4 floats representing a color and alpha channel
	if (num_intervals >= 3)
		ImGui::ColorEdit4("Color3", (float*)&color3); // Edit 4 floats representing a color and alpha channel

	if (ImGui::Button("Set Transfer Function", ImVec2(200.0, 20.0)))
	{
		Color color1_ub = Color(color1.x * 255.0, color1.y * 255.0, color1.z * 255.0, color1.w * 255.0);
		Color color2_ub = Color(color2.x * 255.0, color2.y * 255.0, color2.z * 255.0, color2.w * 255.0);
		Color color3_ub = Color(color3.x * 255.0, color3.y * 255.0, color3.z * 255.0, color3.w * 255.0);
		Color color4_ub = Color(color4.x * 255.0, color4.y * 255.0, color4.z * 255.0, color4.w * 255.0);
		Image* image = new Image(129, 1, 4);
		for (int i = 0; i < 129; i++)
		{
			if (i < int_1)
				image->setPixel(i, 0, color1_ub);
			else if (i < int_2)
				image->setPixel(i, 0, color2_ub);
			else if (i < int_3)
				image->setPixel(i, 0, color3_ub);
			else if (i < int_4)
				image->setPixel(i, 0, color4_ub);
		}
		//image->flipY();
		Texture* text = new Texture(image);

		((VolumeMaterial*)material)->transfer_function = text;
	}

	material->renderInMenu();
}
