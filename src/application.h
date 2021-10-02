/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "scenenode.h"

enum EOutput {
	COMPLETE,
	ALBEDO,
	ROUGHNESS,
	METALNESS,
	NORMALS
};

class Application
{
public:
	static Application* instance;

	std::vector< SceneNode* > node_list; //List for al the scene nodes
	std::vector< Light* > light_list; //List for al the scene lights

	//Varibles to add new lights and nodes
	bool new_light = false;
	bool new_model = false;

	Skybox* sky; //The actual skybox

	Vector3 ambient_light; //Ambient light vector

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;
	bool render_debug;
	float scene_exposure;
	int output;

	//some vars
	static Camera* camera; //our GLOBAL camera
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Application( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
	void onFileChanged(const char* filename);

	//skybox
	void renderSkybox(Texture* skybox, Camera* camera);
};


#endif 