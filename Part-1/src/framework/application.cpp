#include "application.h"
#include "utils.h"
#include "image.h"

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
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;

	//here add your init stuff
}

//render one frame
void Application::render(Image& framebuffer)
{
	//clear framebuffer if we want to start from scratch
	//here you can add your code to fill the framebuffer

	// Segons el mode actual s'executaràn unes funcions o altres, que comprovaràn la interacció de l'usuari previament.

	switch (this->estat)
	{
		case 1: // Linia DDA
			if (clickpunt != NULL) {
				framebuffer.fill(Color::BLACK);
				framebuffer.DDA(click.x, click.y, mouse_position.x, mouse_position.y, Color::NEWCOLOR);
			}
			framebuffer.DDA(15, 5, 25, 15, Color::NEWCOLOR);
			break;

		case 2: // Linia Bressenham
			if (clickpunt != NULL) {
				framebuffer.fill(Color::BLACK);
				framebuffer.bresenham(click.x, click.y, mouse_position.x, mouse_position.y, Color::NEWCOLOR);
			}
			framebuffer.bresenham(15, 5, 25, 15, Color::NEWCOLOR);
			break;

		case 3:  // Cercle
			if (clickpunt != NULL) {
				framebuffer.fill(Color::BLACK);
				//Calculem la distancia del click a la posició del ratolí.
				int dist = this->click.distance(mouse_position);
				framebuffer.bresenhamCircle(this->click.x, this->click.y, dist, Color::NEWCOLOR, this->fill);
			}
			framebuffer.bresenhamCircle(10, 10, 5, Color::NEWCOLOR, this->fill);
			break;

		case 4: // Triangle
			framebuffer.fill(Color::BLACK);
			//Dibuixem un cercle per cada vertex seleccionat.
			if (p1.x >= 0) framebuffer.bresenhamCircle(p1.x, p1.y, 5, Color::RED, false);
			if (p2.x >= 0) framebuffer.bresenhamCircle(p2.x, p2.y, 5, Color::BLUE, false);
			if (p3.x >= 0) framebuffer.bresenhamCircle(p3.x, p3.y, 5, Color::GREEN, false);
			if (p1.x >= 0 && p2.x >= 0 && p3.x >= 0) {
				//Si tenim un valor per a cada vertex dibuixem el triangle. 
				framebuffer.drawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Color::NEWCOLOR, this->fill, this->flat);
				this->punts = true;
			}
			framebuffer.drawTriangle(5, 5, 15, 9, 10, 15, Color::NEWCOLOR, this->fill, this->flat);
			break;
		default:
			break;
	}

}

//called after render
void Application::update(double seconds_elapsed)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (keystate[SDL_SCANCODE_SPACE]) //if key space is pressed
	{
		//...	
	}

	//Entrarem al if si estem apretant un dels tres punts del triangle. 
	//En cas de que sigui cert actualitzem la posició del vertex amb la posicio del mouse
	if (this->estat == 4 && clickpunt != NULL && this->punts && movepunt != NULL) *movepunt = mouse_position;
	//to read mouse position use mouse_position
}

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch(event.keysym.scancode)
	{
		case SDL_SCANCODE_ESCAPE:
			exit(0); break; //ESC key, kill the app

		case SDL_SCANCODE_D:
			this->estat = 1;
			framebuffer.fill(Color::BLACK);
			break;

		case SDL_SCANCODE_B:
			this->estat = 2;
			framebuffer.fill(Color::BLACK);
			break;

		case SDL_SCANCODE_C:
			this->estat = 3;
			framebuffer.fill(Color::BLACK);
			break;

		case SDL_SCANCODE_T:
			this->estat = 4;
			framebuffer.fill(Color::BLACK);
			break;

		//Inicialitcem els punts del triangle amb valors negatius per indicar que encara no s'han posat.
		case SDL_SCANCODE_N:
			if (this->estat == 4) {
				p1 = Vector2(-1, -1);
				p2 = Vector2(-1, -1);
				p3 = Vector2(-1, -1);
			} break;

		//Decidim si ho volem d'un color llis o no.
		case SDL_SCANCODE_E:
			if (this->estat == 4) {
				if (this->flat == false) this->flat = true;
				else this->flat = false;
			} break;

		//Decidim si volem el polígon ple.
		case SDL_SCANCODE_F:
			if (this->estat == 3 || this->estat == 4) {
				if (this->fill == false) this->fill = true;
				else this->fill = false;
				framebuffer.fill(Color::BLACK);
			} break;
				
		//Canviem el valor de color per defecte a un aleatori.
		case SDL_SCANCODE_F1:
			Color::NEWCOLOR.random(); break;
	} 
}

//keyboard key up event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//...
}

//mouse button event
void Application::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
		//if you read mouse position from the event, careful, Y is reversed, use mouse_position instead
		if (this->estat != 0) {
			this->click = Vector2(mouse_position.x, mouse_position.y); // Guardem la posició del click i apuntem al click per indicar
			this->clickpunt = &click;								   // que encara estem apretant
		}
		//Donem el valor als vèrtex dels triangles.
		if (this->estat == 4) {
			if (p1.x < 0) p1.set(this->click.x, this->click.y);
			else if (p2.x < 0) p2 = this->click;
			else if (p3.x < 0) p3 = this->click;
			if (punts) { // Si hem posat tots els 3 vertex del triangle i apretem en un dels vertex vol dir que el volem moure així apuntem a la direccio
						 // del vertex que hem apretat amb la variable movepunt
				if (click.distance(p1) <= 5) movepunt = &p1;
				if (click.distance(p2) <= 5) movepunt = &p2;
				if (click.distance(p3) <= 5) movepunt = &p3;
			}
		}
	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{
		this->clickpunt = NULL; // Quan deixem d'apretar deixem d'apuntar al click
		this->movepunt = NULL;	// i deixem d'apuntar al punt
	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}