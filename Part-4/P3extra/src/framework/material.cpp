#include "material.h"




Material::Material()
{
	ambient.set(1.0, 1.0, 1.0); //reflected ambient light
	diffuse.set(1.0, 1.0, 1.0); //reflected diffuse light
	specular.set(1.0, 1.0, 1.0); //reflected specular light
	shininess = 30.0; //glosiness coefficient (plasticity)
}
