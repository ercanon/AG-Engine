
#pragma once

class Camera
{
public:
	Camera();
	virtual ~Camera();
	void Update(App* app);

	float aspectRatio;
	float zNear;
	float zFar;
	mat4 projection;
	mat4 view;

	vec3 pos;
	vec3 target;
	vec3 direction;

	const vec3 forwardAxis = vec3(1.0f, 0.0f, 0.0f);
	const vec3 upAxis =		 vec3(0.0f, 1.0f, 0.0f);
	const vec3 rightAxis =   vec3(0.0f, 0.0f, 1.0f);
	vec3 forwardCamera;
	vec3 rightCamera;
	vec3 upCamera;

private:
	void RotateAround(float dt, vec3& newFront, vec3& newUp);
	void ControlCamera(App* app);
	void UpdateCameraVectors();
};