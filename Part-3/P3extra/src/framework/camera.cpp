#include "camera.h"

Camera::Camera()
{
	this->fov = 45;
	this->aspect = 1;
	this->near_plane = 0.01;
	this->far_plane = 10000;

	eye = Vector3(0, 10, 20);
	center = Vector3(0, 10, 0);
	up = Vector3(0, 1, 0);

	updateViewMatrix();
	updateProjectionMatrix();

}

void Camera::updateViewMatrix()
{
	//IMPLEMENT THIS using eye, center and up, store in this->view_matrix
	//Careful with the order of matrix multiplications, and be sure to use normalized vectors

	// Creem els tres vectors front, side i top
	Vector3 front = center - eye;
	front.normalize();

	Vector3 side = front.cross(up);
	side.normalize();

	Vector3 top = side.cross(front);
	top.normalize();

	// Emplenem la matrius amb els valors corresponents
	view_matrix.setIdentity();
	
	view_matrix.M[0][0] = side.x; 	view_matrix.M[0][1] = top.x;	view_matrix.M[0][2] = -front.x;		view_matrix.M[0][3] = 0.0;
	view_matrix.M[1][0] = side.y; 	view_matrix.M[1][1] = top.y;	view_matrix.M[1][2] = -front.y;		view_matrix.M[1][3] = 0.0;
	view_matrix.M[2][0] = side.z; 	view_matrix.M[2][1] = top.z;	view_matrix.M[2][2] = -front.z;		view_matrix.M[2][3] = 0.0;
	view_matrix.M[3][0] = 0.0; 		view_matrix.M[3][1] = 0.0;		view_matrix.M[3][2] = 0.0;			view_matrix.M[3][3] = 1.0;

	//ADD ANY MATRIX POSTCOMPUTATIONS IF 
	view_matrix.traslateLocal(-eye.x, -eye.y, -eye.z);

	//update the viewprojection_matrix
	viewprojection_matrix = view_matrix * projection_matrix;
}

void Camera::updateProjectionMatrix()
{
	//IMPLEMENT THIS using fov, aspect, near_plane and far_plane, store in this->projection_matrix
	//Careful with using degrees in trigonometric functions, must be radians, and use float types in divisions

	float angle = (fov / 2) * (PI / 180); // El fov esta en graus però la funció tangent només accepta radians per això ho passem a radians
	float f = 1 / (tan(angle));


	//CHANGE THE MATRIX VALUES USING YOUR OWN...
	projection_matrix.setIdentity();

	projection_matrix.M[0][0] = f / aspect; projection_matrix.M[0][1] = 0.0; projection_matrix.M[0][2] = 0.0;  projection_matrix.M[0][3] = 0.0;
	projection_matrix.M[1][0] = 0.0; 	projection_matrix.M[1][1] = f; projection_matrix.M[1][2] = 0.0;  projection_matrix.M[1][3] = 0.0;
	projection_matrix.M[2][0] = 0.0; 	projection_matrix.M[2][1] = 0.0; projection_matrix.M[2][2] = (far_plane + near_plane) / (near_plane - far_plane); projection_matrix.M[2][3] = -1;
	projection_matrix.M[3][0] = 0.0; 	projection_matrix.M[3][1] = 0.0; projection_matrix.M[3][2] = 2 * ((far_plane * near_plane) / (near_plane - far_plane));  projection_matrix.M[3][3] = 0.0;

	//update the viewprojection_matrix
	viewprojection_matrix = view_matrix * projection_matrix;
}

Vector3 Camera::projectVector( Vector3 pos )
{
	Vector4 pos4 = Vector4(pos.x, pos.y, pos.z, 1.0);
	Vector4 result = viewprojection_matrix * pos4;
	return result.getVector3() / result.w;
}

void Camera::lookAt( Vector3 eye, Vector3 center, Vector3 up )
{
	this->eye = eye;
	this->center = center;
	this->up = up;

	this->updateViewMatrix();
}

void Camera::perspective( float fov, float aspect, float near_plane, float far_plane )
{
	this->fov = fov;
	this->aspect = aspect;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	this->updateProjectionMatrix();
}

Matrix44 Camera::getViewProjectionMatrix()
{
	viewprojection_matrix = view_matrix * projection_matrix;
	return viewprojection_matrix ;
}


Vector3 Camera::calcLight(Vector3 punt, Vector3 normal, Light* light, Material* material, Vector3 ambient_light) {
	// Utilitzem la formula per calcular la iluminacio
	Vector3 l = (light->position - punt).normalize();
	Vector3 v = (eye - punt).normalize();
	Vector3 aux = punt - light->position;
	Vector3 r = (aux - (normal * (2.0 * normal.dot(aux)))).normalize();

	float ln = clamp(l.dot(normal), 0, 1);
	Vector3 diffuse = material->diffuse * ln * light->diffuse_color;

	float rv = r.dot(v);
	rv = clamp(rv, 0, 1);
	rv = pow(rv, material->shininess);
	Vector3 specular = material->specular * rv * light->specular_color;

	Vector3 light_inten = diffuse + specular;
	//light_inten.set(clamp(light_inten.x, 0, 1), clamp(light_inten.y, 0, 1), clamp(light_inten.z, 0, 1));

	return light_inten;

}

