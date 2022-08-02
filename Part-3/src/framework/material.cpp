#include "material.h"




Material::Material()
{
	ambient.set(1.0, 1.0, 1.0); //reflected ambient light
	diffuse.set(1.0, 1.0, 1.0); //reflected diffuse light
	specular.set(1.0, 1.0, 1.0); //reflected specular light
	shininess = 30.0; //glosiness coefficient (plasticity)
}


void Material::uploadToShader(Shader* shader) {
	// Carreguem els atributs a la shader
	shader->setVector3("mat_ambient", this->ambient);
	shader->setVector3("mat_diffuse", this->diffuse);
	shader->setVector3("mat_specular", this->specular);
	shader->setFloat("alfa", this->shininess);
}
