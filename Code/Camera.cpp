#include "Engine.h"

Camera::Camera()
{
	zNear = 0.1f;
	zFar = 1000.0f;
	projection = perspective(radians(60.0f), aspectRatio, zNear, zFar);

	//Camera Position
	pos = vec3(-10.0f, 5.0f, -0.0f);
	//Camera Direction
	target = vec3(0.0f, 2.0f, 0.0f);
	direction = normalize(pos - target);
	//Right axis' camera
	upAxis = vec3(0.0f, 1.0f, 0.0f);
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

	projection = perspective(radians(60.0f), aspectRatio, zNear, zFar);

	view = lookAt(pos, target, upAxis);

	ControlCamera(app);
}

void Camera::ControlCamera(App* app)
{
	
	float speed = 15.0f * app->dt();
	if (app->input.keys[K_SPACE] == BUTTON_PRESSED) speed *= 2;

	if (app->input.keys[K_W] == BUTTON_PRESSED)
		pos += forwardAxis * speed;
	if (app->input.keys[K_S] == BUTTON_PRESSED)
		pos -= forwardAxis * speed;
	if (app->input.keys[K_A] == BUTTON_PRESSED)
		pos -= rightAxis * speed;
	if (app->input.keys[K_D] == BUTTON_PRESSED)
		pos += rightAxis * speed;

		/*
	// Auxiliar variables
	vec3 newPos = pos;
	vec3 newFront = rightAxis;
	vec3 newUp = upAxis;

	// Inputs for the camera
	if (app->input.keys[K_T] == BUTTON_PRESS)
	{
		newUp = vec3(0, 1, 0);
		newFront = -vec3(0, 0, 1);
	}
	if (app->input.mouseButtons[RIGHT] == BUTTON_PRESSED)
	{
		if (app->input.keys[K_A] == BUTTON_PRESSED) newPos += cameraFrustum.Front() * speed;
		if (app->input.keys[K_D] == BUTTON_PRESSED) newPos -= cameraFrustum.Front() * speed;

		if (app->input.keys[K_A] == BUTTON_PRESSED) newPos -= cameraFrustum.WorldRight() * speed;
		if (app->input.keys[K_D] == BUTTON_PRESSED) newPos += cameraFrustum.WorldRight() * speed;

		if (app->input.keys[K_A] == BUTTON_PRESSED) newPos -= cameraFrustum.Up() * speed;
		if (app->input.keys[K_D] == BUTTON_PRESSED) newPos += cameraFrustum.Up() * speed;

		RotateAround(app->deltaTime, newFront, newUp);
	}

	cameraFrustum.SetFrame(newPos, newFront, newUp);
	CalculateViewMatrix();
	*/
}
/*
void Camera::UpdateCameraVectors()
{
	// calculate the new Front vector
	forwardAxis.x = cos(radians(yaw)) * cos(radians(pitch));
	forwardAxis.y = sin(radians(pitch));
	forwardAxis.z = sin(radians(yaw)) * cos(radians(pitch));
	forwardAxis = normalize(front);
	// also re-calculate the Right and Up vector
	rightAxis = normalize(cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	upAxis = normalize(cross(right, front));
}
*/
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