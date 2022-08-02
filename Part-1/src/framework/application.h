/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the application, is in charge of creating the data, getting the user input, process the update and render.
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "includes.h"
#include "framework.h"
#include "image.h"

class Application
{
public:
	//window
	SDL_Window* window;
	float window_width;
	float window_height;

	Image framebuffer;

	float time;

	//keyboard state
	const Uint8* keystate;

	//mouse state
	int mouse_state; //tells which buttons are pressed
	Vector2 mouse_position; //last mouse position
	Vector2 mouse_delta; //mouse movement in the last frame

	//constructor
	Application(const char* caption, int width, int height);

	//main methods
	void init( void );
	void render( Image& framebuffer );
	void update( double dt );

	//methods for events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp( SDL_MouseButtonEvent event );

	//other methods to control the app
	void setWindowSize(int width, int height) {
		glViewport( 0,0, width, height );
		this->window_width = width;
		this->window_height = height;
		framebuffer.resize(width,height);
	}

	Vector2 getWindowSize()
	{
		int w,h;
		SDL_GetWindowSize(window,&w,&h);
		return Vector2(w,h);
	}

	void start();

	// Formes geomètriques
	Vector2 click;				// Ens guardarà el punt on hem fet click
	Vector2* clickpunt = NULL;	// Ens ajudarà a saber si encara estem apretant el ratolí o no

	// Punts triangle (quan tenen valors negatius vol dir que no estan posats a la pantalla)
	Vector2 p1 = Vector2(-1, -1);
	Vector2 p2 = Vector2(-1, -1);
	Vector2 p3 = Vector2(-1, -1);
	
	int estat = 0;  // Ens indica en quin estat de la app estem (que estem dibuixant)
	bool fill = false;
	bool punts = false;		//Serà true quan posem els 3 vertexs de triangle
	bool flat = false;
	Vector2* movepunt = NULL;	//En ajuda a l'hora de moure els vertexs del triangle, per saber quin hem de moure

};


#endif 