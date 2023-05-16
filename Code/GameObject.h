
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

	virtual u32 MeshID(u32 meshID = -1) { return meshID != -1 ? meshIdx = meshID : meshIdx; };
	virtual u32 MaterialID(u32 index, u32 materialID = -1) { return materialID != -1 ? materialIdx[index] = materialID : materialIdx[index]; };
	virtual vector<u32>& GetMaterialID() { return materialIdx; };

private:
	u32			meshIdx;
	vector<u32>	materialIdx;
};