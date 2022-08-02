#include "material.h"



Material::Material()
{
	ambient.set(1 * (220/255.0), 1 * (202/255.0), 1 * (176/255.0)); //reflected ambient light
	diffuse.set(1 * (203/255.0), 1 * (174/255.0), 1 * (174/255.0)); //reflected diffuse light
	specular.set(1 * (80/255.0), 1 * (80 / 255.0), 1 * (80 / 255.0)); //reflected specular light
	shininess = 13.0; //glosiness coefficient (plasticity)
}

void Material::uploadToShader(Shader* shader) {
	// Carreguem els atributs a la shader
	shader->setVector3("mat_ambient", this->ambient);
	shader->setVector3("mat_diffuse", this->diffuse);
	shader->setVector3("mat_specular", this->specular);
	shader->setFloat("alfa", this->shininess);
}

