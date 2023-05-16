
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
	GameObject(int type);
	virtual ~GameObject() {};
	void Update(App* app);
	void HandleBuffer(GLint uniformBlockAligment, Buffer buffer);

	mat4 TransformScale(const vec3& scaleFactors);
	mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors);
	mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors);

	// Get/Set Function
	virtual u32 MeshID(u32 meshID = -1) { return meshID != -1 ? meshIdx = meshID : meshIdx; }
	virtual u32 MaterialID(u32 index, u32 materialID = -1) { return materialID != -1 ? materialIdx[index] = materialID : materialIdx[index]; }

	// Get Funcion
	virtual vector<u32>& GetMaterialID() { return materialIdx; }
	virtual mat4 GetView() { return worldMatrix; }
	virtual mat4 GetProjection() { return worldViewProjection; }
	virtual u32* GetLocalParams() { u32 locParams[] = { localParamsOffset, localParamSize }; return locParams; }

private:
	//Model
	u32			meshIdx;
	vector<u32>	materialIdx;

	//Light
	Light light;

	//General
	ObjectType oType;
	vec3 pos;
	mat4 worldMatrix;
	mat4 worldViewProjection;
	u32 localParamsOffset;
	u32 localParamSize;
};