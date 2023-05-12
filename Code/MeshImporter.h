
#pragma once

#include <glad/glad.h>

struct App;

struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};
struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8								   stride;
};
struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};
struct  VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};
struct Vao
{
	GLuint handle;
	GLuint programHandle;
};


struct VertexV3V2
{
	glm::vec3 pos;
	glm::vec2 uv;

};
const VertexV3V2 vertices[] =
{
	{glm::vec3(-0.5, -0.5,  0.0),    glm::vec2(0.0, 0.0)},   // Bottom-Left Vertex
	{glm::vec3(0.5, -0.5,  0.0),    glm::vec2(1.0, 0.0)},   // Bottom-Right Vertex
	{glm::vec3(0.5,  0.5,  0.0),    glm::vec2(1.0, 1.0)},   // Top-Right Vertex
	{glm::vec3(-0.5,  0.5,  0.0),    glm::vec2(0.0, 1.0)},   // Top-Left Vertex
};
const u16 indices[] =
{
	0, 1, 2,
	0, 2, 3
};
struct Image
{
	void* pixels;
	ivec2 size;
	i32   nchannels;
	i32   stride;
};


struct Texture
{
	GLuint      handle;
	std::string filepath;
};
struct Material
{
	std::string name;
	glm::vec3	albedo;
	glm::vec3	emissive;
	f32			smoothness;
	u32			albedoTextureIdx;
	u32			emissiveTextureIdx;
	u32			specularTextureIdx;
	u32			normalsTextureIdx;
	u32			bumpTextureIdx;
};
struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32>   indices;
	u32				   vertexOffset;
	u32				   indexOffset;

	std::vector<Vao>   vaos;
};
struct Mesh
{
	std::vector<Submesh> submeshes;
	GLuint				 vertexBufferHandle;
	GLuint				 indexBufferHandle;
};
struct  Model
{
	u32				 meshIdx;
	std::vector<u32> materialIdx;
};
struct Program
{
	GLuint             handle;
	std::string        filepath;
	std::string        programName;
	u64                lastWriteTimestamp; // What is this for?
	VertexShaderLayout vertexInputLayout;
	GLsizei            lenght;
};


u32 LoadModel(App* app, const char* filename);