
#pragma once

struct Buffer;

enum ObjectType
{
	Model,
	Lightning
};

enum LightType
{
	Directional,
	Point
};

struct Light
{
	LightType type;
	vec3	  color;
	vec3	  direction;
};

class GameObject
{
public:
	GameObject(string name, vec3 position, vec3 scale, vec3 rotation, Mesh mesh);
	GameObject(string name, vec3 position, vec3 scale, vec3 rotation, Light newLight);
	virtual ~GameObject() {};
	void Update(App* app);
	void HandleBuffer(GLint uniformBlockAligment, Buffer* buffer);
	void HandleBuffer(Buffer* buffer);

	mat4 TransformScale(const vec3& scaleFactors);
	mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors);
	mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors);

	// Get Funcion
	virtual Mesh GetMesh() { return objMesh; }
	virtual u32 GetLocalOffset() { return localParamsOffset; }
	virtual u32 GetLocalSize() { return localParamSize; }
	virtual string GetName() { return objName; }
	virtual bool IsType(ObjectType type) { return type == objType; }

private:
	//General
	string objName;
	ObjectType objType;
	vec3 pos;
	vec3 scl;
	vec3 rot;

	mat4 worldMatrix;
	mat4 worldViewProjection;
	u32 localParamsOffset;
	u32 localParamSize;

	//Model
	Mesh objMesh;

	//Light
	Light light;
};