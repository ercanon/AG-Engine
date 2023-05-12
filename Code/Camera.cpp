#include "Engine.h"

Camera::Camera()
{
	zNear = 0.1f;
	zFar = 1000.0f;
	projection = perspective(radians(60.0f), aspectRatio, zNear, zFar);

	//Camera Position
	pos = vec3(0.0f, 0.0f, -3.0f);
	//Camera Direction
	target = vec3(0.0f, 0.0f, 0.0f);
	direction = normalize(pos - target);
	//Right axis' camera
	upVector = vec3(0.0f, 1.0f, 0.0f);
	rightAxis = normalize(cross(upVector, direction));
	//Up axis' camera
	upAxis = cross(direction, rightAxis);
}

Camera::~Camera()
{
}

void Camera::Update(App* app)
{
	aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;

	projection = perspective(radians(60.0f), aspectRatio, zNear, zFar);

	view = lookAt(pos, target, upVector);
}