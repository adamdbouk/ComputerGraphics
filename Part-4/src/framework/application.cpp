#include "application.h"
#include "utils.h"
#include "image.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "material.h"

Camera* camera = NULL;
Mesh* mesh = NULL;
Shader* shader = NULL;
Texture* texture_color = NULL;
Texture* texture_normals = NULL;
Light* light = NULL;
Material* material = NULL;
Material* material_2 = NULL;
Matrix44 model_matrix;

int meshes = 1;
float angle = PI/2;

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
	mesh = new Mesh();
	mesh->loadOBJ("lee.obj");
	
	//load the texture
	texture_color = new Texture();
	texture_normals = new Texture();
	if(!texture_color->load("lee_color_specular.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}
	if (!texture_normals->load("lee_normal.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}

	//we load a shader
	shader = Shader::Get("texture.vs","texture.ps");

	//load whatever you need here
	light = new Light();
	material = new Material();
	material_2 = new Material();
	
	// Damos valores distintos al segundo material que utilizaremos
	material_2->ambient.set(0.7 * (1 / 255.0), 1 * (1 / 255.0), 1 * (1 / 255.0));
	material_2->diffuse.set(0.7 * (174 / 255.0), 1 * (174 / 255.0), 1 * (174 / 255.0));
	material_2->specular.set(0.7 * (80 / 255.0), 1 * (80 / 255.0), 1 * (80 / 255.0));
	material_2->shininess = 10;
}

//render one frame
void Application::render(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );

	//Get the viewprojection
	camera->aspect = window_width / window_height;
	Matrix44 viewprojection = camera->getViewProjectionMatrix();
	
	//enable the shader
	shader->enable();

	for (int j = 0; j < meshes; j++) {
		Matrix44 model_matrix;

		// Si nomès volem una mesh la deixem centrada.
		if (meshes == 1) {
			model_matrix.setIdentity();
			model_matrix.rotate(angle - PI / 2, Vector3(0, 1, 0));
			model_matrix.translate(0, 0, 0);
		}
		else {
			model_matrix.setIdentity();
			model_matrix.rotate(angle + PI * j, Vector3(0, 1, 0));
			model_matrix.translate(-13 + j * 26, 0, -10);
		}

		shader->setMatrix44("model", model_matrix); //upload info to the shader
		shader->setMatrix44("viewprojection", viewprojection); //upload info to the shader
		shader->setTexture("color_texture", texture_color, 0); //set texture in slot 0
		shader->setTexture("normals_texture", texture_normals, 1);

		light->uploadToShader(shader);

		// Cargamos el material a la shader.
		if (j == 0) material->uploadToShader(shader);
		if (j == 1) material_2->uploadToShader(shader);

		//material->uploadToShader(shader);
		Vector3 ambient_color = Vector3(0.6f, 0.6f, 0.6f);

		shader->setVector3("light_ambient", ambient_color);
		shader->setVector3("eye_pos", camera->eye);

		//render the data
		mesh->render(GL_TRIANGLES);
	}
	

	//disable shader
	shader->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
	{
		model_matrix.rotateLocal(seconds_elapsed,Vector3(0,1,0));
	}

	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye = camera->eye + Vector3(1, 0, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_LEFT])
		camera->eye = camera->eye + Vector3(-1, 0, 0) * seconds_elapsed * 10.0;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye = camera->eye + Vector3(0, 1, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_DOWN])
		camera->eye = camera->eye + Vector3(0, -1, 0) * seconds_elapsed * 10.0;

	if (keystate[SDL_SCANCODE_A])
		light->position = light->position + Vector3(-1, 0, 0) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_D])
		light->position = light->position + Vector3(1, 0, 0) * seconds_elapsed * 40.0;
	if (keystate[SDL_SCANCODE_W])
		light->position = light->position + Vector3(0, 1, 0) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_S])
		light->position = light->position + Vector3(0, -1, 0) * seconds_elapsed * 40.0;
	
	if (keystate[SDL_SCANCODE_Z])
		light->position = light->position + Vector3(0, 0, 1) * seconds_elapsed * 40.0;
	else if (keystate[SDL_SCANCODE_X])
		light->position = light->position + Vector3(0, 0, -1) * seconds_elapsed * 40.0;

}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_r: Shader::ReloadAll(); break;
		case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
		case SDLK_F1: meshes = 1; break; // Escollim tindre una mesh
		case SDLK_F2: meshes = 2; break; // Escollim tindre dues meshs
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
