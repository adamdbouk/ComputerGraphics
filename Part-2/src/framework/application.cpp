#include "application.h"
#include "utils.h"
#include "image.h"
#include "mesh.h"

Mesh* mesh = NULL;
Camera* camera = NULL;
Image* texture = NULL;

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

	framebuffer.resize(w, h);
	zBuffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,10,20),Vector3(0,10,0),Vector3(0,1,0)); //define eye,center,up
	camera->perspective(60, window_width / (float)window_height, 0.1, 10000); //define fov,aspect,near,far

	//load a mesh
	mesh = new Mesh();
	if( !mesh->loadOBJ("lee.obj") )
		std::cout << "FILE Lee.obj NOT FOUND" << std::endl;

	//load the texture
	texture = new Image();
	texture->loadTGA("color.tga");
	zBuffer.fill(10000);	// Emplenem el zbuffer amb valors molt grans perquè així s'actualitzaran segur
}

//render one frame
void Application::render(Image& framebuffer)
{
	framebuffer.fill(Color(70, 75, 90));	//clear

	//for every point of the mesh (to draw triangles take three points each time and connect the points between them (1,2,3,   4,5,6,   ... )
	for (unsigned int i = 0; i < mesh->vertices.size(); i += 3)
	{
		Vector3 v0 = mesh->vertices[i];		//extract vertex from mesh
		Vector3 v1 = mesh->vertices[i + 1]; //extract vertex from mesh
		Vector3 v2 = mesh->vertices[i + 2]; //extract vertex from mesh

		Vector2 uv0 = mesh->uvs[i];		//texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)
		Vector2 uv1 = mesh->uvs[i + 1]; //texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)
		Vector2 uv2 = mesh->uvs[i + 2]; //texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)

		//project every point in the mesh to normalized coordinates using the viewprojection_matrix inside camera
		//you can use: projected_vertex = camera->projectVector(vertex);
		Vector3 projected_v0 = camera->projectVector(v0);
		Vector3 projected_v1 = camera->projectVector(v1);
		Vector3 projected_v2 = camera->projectVector(v2);


		//convert from normalized (-1 to +1) to framebuffer coordinates (0..W,0..H)
		projected_v0.set((framebuffer.width / 2) * (projected_v0.x + 1), (framebuffer.height / 2) * (projected_v0.y + 1), projected_v0.z);
		projected_v1.set((framebuffer.width / 2) * (projected_v1.x + 1), (framebuffer.height / 2) * (projected_v1.y + 1), projected_v1.z);
		projected_v2.set((framebuffer.width / 2) * (projected_v2.x + 1), (framebuffer.height / 2) * (projected_v2.y + 1), projected_v2.z);

		//paint point in framebuffer (using the fillTriangle function)
		framebuffer.drawTriangle(projected_v0, projected_v1, projected_v2, uv0, uv1, uv2, Color::BLACK, &zBuffer, texture, this->mode);
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	// Controls per moure la posició de la camera (la posició del eye)
	if (keystate[SDL_SCANCODE_LEFT]) {
		camera->eye.x -= 5 * seconds_elapsed;
		zBuffer.fill(1000);	
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		camera->eye.x += 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	// Controls per controlar la direcció en la que mira la camera (la posició del center) 
	if (keystate[SDL_SCANCODE_S]) {
		camera->center.y += 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	if (keystate[SDL_SCANCODE_D]) {
		camera->center.x -= 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	if (keystate[SDL_SCANCODE_W]) {
		camera->center.y -= 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	if (keystate[SDL_SCANCODE_A]) {
		camera->center.x += 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	// Controls per augmentar o disminuir el fov
	if (keystate[SDL_SCANCODE_F]) {
		if (camera->fov < 360) camera->fov += 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}
	if (keystate[SDL_SCANCODE_G]) {
		if (camera->fov > 0) camera->fov -= 5 * seconds_elapsed;
		zBuffer.fill(1000);
	}

	//if we modify the camera fields, then update matrices
	camera->updateViewMatrix();
	camera->updateProjectionMatrix();
}

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch (event.keysym.scancode)//event.keysym.sym)
	{
	case SDL_SCANCODE_ESCAPE://SDLK_ESCAPE: 
		exit(0);
		break; //ESC key, kill the app

	case SDL_SCANCODE_T:
		this->mode = 3;
		break;

	case SDL_SCANCODE_I:
		this->mode = 2;
		break;

	case SDL_SCANCODE_M:
		this->mode = 1;
		break;

	default: break;
	}
}

//keyboard released event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
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
