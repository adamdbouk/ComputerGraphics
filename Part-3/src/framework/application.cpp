#include "application.h"
#include "utils.h"
#include "includes.h"
#include "utils.h"

#include "image.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "material.h"
#include "light.h"

#define MAX_LIGHTS 4
#define MAX_MESHES 2
Camera* camera = NULL;
Mesh* mesh[MAX_MESHES];
Shader* shader = NULL;

//might be useful...
Material* material = NULL;
Light* light[MAX_LIGHTS];
Shader* phong_shader = NULL;
Shader* gouraud_shader = NULL;

float angle = PI/2;
Vector3 ambient_color;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,20,20),Vector3(0,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	// Inicialitzem les dues mesh
	for (int m = 0; m < MAX_MESHES; m++) {
		mesh[m] = new Mesh();
		if (!mesh[m]->loadOBJ("Lee.obj"))
			std::cout << "FILE Lee.obj NOT FOUND " << std::endl;
	}
	//we load one or several shaders...
	phong_shader = new Shader();
	gouraud_shader = new Shader();

	//load your Gouraud and Phong shaders here
	gouraud_shader->load("gouraud.vs", "gouraud.ps");
	phong_shader->load("phong.vs", "phong.ps");
	shader = phong_shader;

	//CODE HERE:
	material = new Material();

	// Li donem una posició a cada llum
	light[0] = new Light();
	light[0]->position.set(100, 0, 50);

	light[1] = new Light();
	light[1]->position.set(-100, 0, 50);

	light[2] = new Light();
	light[2]->position.set(100, 0, -50);

	light[3] = new Light();
	light[3]->position.set(-100, 0, -50);
	//create a light (or several) and a materials
	//...
}

//render one frame
void Application::render(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);


	//Get the viewprojection matrix from our camera
	for (int i = 0; i < this->lights; i++) { // Per a cada llum
		if (i == 0) {
			ambient_color = Vector3(0.3f, 0.3f, 0.3f);
			glDisable(GL_BLEND);
		}
		else { //Si passem més d'una llum haurem de canviar les flags.
			ambient_color = Vector3(0.0f, 0.0f, 0.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}
		Matrix44 viewprojection = camera->getViewProjectionMatrix();

		//choose a shader and enable it
		shader->enable();
		for (int j = 0; j < this->meshes; j++) { // Per a cada mesh
			Matrix44 model_matrix;
			model_matrix.setIdentity();

			if (meshes == 1) {
				// Si nomès volem una mesh la deixem centrada.
				model_matrix.rotate(angle - PI / 2, Vector3(0, 1, 0));
				model_matrix.translate(0, 0, 0); 
			}
			else {
				model_matrix.rotate(angle + PI * j, Vector3(0, 1, 0));
				model_matrix.translate(-13 + j * 26, 0, -10); 
			}
			shader->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
			shader->setMatrix44("viewprojection", viewprojection); //upload viewprojection info to the shader

			//CODE HERE: pass all the info needed by the shader to do the computations
			//send the material and light uniforms to the shader


			light[i]->uploadToShader(shader);
			material->uploadToShader(shader);
			shader->setVector3("light_ambient", ambient_color);
			shader->setVector3("eye_pos", camera->eye);
			//...

			//do the draw call into the GPU
			mesh[j]->render(GL_TRIANGLES);
		}
		//disable shader when we do not need it any more
		shader->disable();
	}
	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
		angle += seconds_elapsed;

	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye = camera->eye + Vector3(1, 0, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_LEFT])
		camera->eye = camera->eye + Vector3(-1, 0, 0) * seconds_elapsed * 10.0;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye = camera->eye + Vector3(0, 1, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_DOWN])
		camera->eye = camera->eye + Vector3(0, -1, 0) * seconds_elapsed * 10.0;

	if (keystate[SDL_SCANCODE_A])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(-1, 0, 0) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_D])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(1, 0, 0) * seconds_elapsed * 40.0;
	if (keystate[SDL_SCANCODE_W])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(0, 1, 0) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_S])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(0, -1, 0) * seconds_elapsed * 40.0;
	if (keystate[SDL_SCANCODE_Z])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(0, 0, 1) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_X])
		light[this->current_light]->position = light[this->current_light]->position + Vector3(0, 0, -1) * seconds_elapsed * 40.0;


	// Rotem la camera al voltant de la mesh
	if (keystate[SDL_SCANCODE_Q]) {
		camera->eye = camera->eye - camera->getLocalVector(Vector3(1, 0, 0).normalize()) * seconds_elapsed * 50.0;
	}
	else if (keystate[SDL_SCANCODE_E]) {
		camera->eye = camera->eye + camera->getLocalVector(Vector3(1, 0, 0).normalize()) * seconds_elapsed * 50.0;
	}

}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
		
		case SDLK_r: 
			Shader::ReloadAll();
			break; //ESC key, kill the app
		
		// Augmentem el número de llums
		case SDLK_PLUS:
			if (this->lights < MAX_LIGHTS) this->lights++;
			break;
		
		// Disminuim el número de llums
		case SDLK_MINUS:
			if (this->lights > 0) this->lights--;
			break;

		// Escollim tindre una mesh
		case SDLK_F1:
			this->meshes = 1;
			break;

		// Escollim tindre dues meshs
		case SDLK_F2:
			this->meshes = 2;
			break;

		// Escollim quina llum volem moure
		case SDLK_1:
			if (this->lights >= 1) this->current_light = 0;
			break;
		case SDLK_2:
			if (this->lights >= 2) this->current_light = 1;
			break;
		case SDLK_3:
			if (this->lights >= 3) this->current_light = 2;
			break;
		case SDLK_4:
			if (this->lights == 4) this->current_light = 3;
			break;

		// Escollim quina shader volem utilitzar.
		case SDLK_p:
			shader = phong_shader;
			break;
		case SDLK_g:
			shader = gouraud_shader;
			break;
	}
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
		
	}

}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
