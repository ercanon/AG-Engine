#include "Engine.h"

Camera::Camera()
{
	zNear = 0.1f;
	zFar = 1000.0f;

	//Camera Position
	pos = vec3(0.0f, 0.0f, 0.0f);

	//Camera Direction
	target = vec3(0.0f, 0.0f, 1.0f);
	direction = normalize(pos - target);

	//Right axis' camera
	rightAxis = normalize(cross(upAxis, direction));

	//Up axis' camera
	forwardAxis = normalize(cross(upAxis, rightAxis));
}

Camera::~Camera()
{
}

void Camera::Update(App* app)
{
	ivec2 displaySize = app->dispSize();
	aspectRatio = (float)displaySize.x / (float)displaySize.y;

	if (app->input.keys[K_F] == BUTTON_PRESSED)
	{
		GameObject* goSelect = app->GetGameObject();
		if (goSelect != nullptr)
			target = goSelect->objPos;
	}

	projection = perspective(radians(60.0f), aspectRatio, zNear, zFar);
	view = lookAt(pos, target, upAxis);

	ControlCamera(app);
}

void Camera::ControlCamera(App* app)
{
	float speed = 15.0f * app->dt();
	if (app->input.keys[K_CONTROL] == BUTTON_PRESSED) speed *= 2;

	if (app->input.keys[K_W] == BUTTON_PRESSED)
		pos += forwardAxis * speed;
	if (app->input.keys[K_S] == BUTTON_PRESSED)
		pos -= forwardAxis * speed;
	if (app->input.keys[K_A] == BUTTON_PRESSED)
		pos -= rightAxis * speed;
	if (app->input.keys[K_D] == BUTTON_PRESSED)
		pos += rightAxis * speed;
	if (app->input.keys[K_Q] == BUTTON_PRESSED)
		pos -= upAxis * speed;
	if (app->input.keys[K_E] == BUTTON_PRESSED)
		pos += upAxis * speed;
}
/*
void Camera::RotateAround(float dt, vec3& newFront, vec3& newUp)
{
	float dX = -app->input->GetMouseXMotion();
	float dY = -app->input->GetMouseYMotion();
	if (dY != 0)
	{
		quat rotateVertical;
		rotateVertical = rotateVertical.RotateAxisAngle(normalize(rightAxis), dY * dt * 2);
		newFront = rotateVertical * newFront;
		newUp = rotateVertical * newUp;
		normalize(newFront);
		normalize(newUp);
		vec3::Orthonormalize(newFront, newUp);
	}
	if (dX != 0)
	{
		quat rotateHorizontal;
		rotateHorizontal = rotateHorizontal.RotateY(dX * dt);
		newFront = rotateHorizontal * newFront;
		newUp = rotateHorizontal * newUp;
		normalize(newFront);
		normalize(newUp);
		float3::Orthonormalize(newFront, newUp);
	}
}
*/