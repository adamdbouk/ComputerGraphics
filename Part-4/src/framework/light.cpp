#include "light.h"



Light::Light()
{
	position.set(50, 50, 10);
	diffuse_color.set(0.6f, 0.6f, 0.6f);
	specular_color.set(0.6f, 0.6f, 0.6f);
	//diffuse_color.set(1.0f, 1.0f, 1.0f);
	//specular_color.set(1.0f, 1.0f, 1.0f);
}

void Light::uploadToShader(Shader* shader) {
	// Carreguem els atributs a la shader
	shader->setVector3("light_pos", this->position);
	shader->setVector3("light_diffuse", this->diffuse_color);
	shader->setVector3("light_specular", this->specular_color);
}



