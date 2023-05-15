
#pragma once

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();
	void Update(App* app);

	mat4 TransformScale(const vec3& scaleFactors);
	mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors);
	mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors);


	mat4 projection;
	mat4 view;
	vec3 pos;

	mat4 worldMatrix;
	u32 modelIndex;
	u32 localParamsOffset;
	u32 localParamSize;
};