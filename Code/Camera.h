
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

	vec3 upVector;
	vec3 upAxis;
	vec3 rightAxis;
};