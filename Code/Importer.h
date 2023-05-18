
#pragma once

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <stb_image_write.h>
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
	vector<VertexBufferAttribute> attributes;
	u8							  stride;
};
struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};
struct  VertexShaderLayout
{
	vector<VertexShaderAttribute> attributes;
};
struct Vao
{
	GLuint handle;
	GLuint programHandle;
};


struct VertexV3V2
{
	vec3 pos;
	vec2 uv;

};
const VertexV3V2 vertices[] =
{
	{vec3(-1, -1,  0.0),   vec2(0.0, 0.0)},   // Bottom-Left Vertex
	{vec3( 1, -1,  0.0),   vec2(1.0, 0.0)},   // Bottom-Right Vertex
	{vec3( 1,  1,  0.0),   vec2(1.0, 1.0)},   // Top-Right Vertex
	{vec3(-1,  1,  0.0),   vec2(0.0, 1.0)},   // Top-Left Vertex
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
	GLuint handle;
	string filepath;
};
struct Material
{
	string	name;
	vec3	albedo;
	vec3	emissive;
	f32		smoothness;
	u32		albedoTextureIdx;
	u32		emissiveTextureIdx;
	u32		specularTextureIdx;
	u32		normalsTextureIdx;
	u32		bumpTextureIdx;
};
struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	vector<float>	   vertices;
	vector<u32>		   indices;
	u32				   vertexOffset;
	u32				   indexOffset;
					   
	vector<Vao>		   vaos;
};
struct Mesh
{
	vector<Submesh> submeshes;
	vector<u32>		materialIdx;
	GLuint			vertexBufferHandle;
	GLuint			indexBufferHandle;
};
struct Program
{
	GLuint             handle;
	string			   filepath;
	string			   programName;
	u64                lastWriteTimestamp; // What is this for?
	VertexShaderLayout vertexInputLayout;
	GLsizei            lenght;
};



/* --------------------- SHADER --------------------- */
GLuint CreateProgramFromSource(String programSource, const char* shaderName);

u32 LoadProgram(App* app, const char* filepath, const char* programName);


/* --------------------- TEXTURE --------------------- */
void FreeImage(Image image);

GLuint CreateTexture2DFromImage(Image image);

Image LoadImage(const char* filename);

u32 LoadTexture2D(App* app, const char* filepath);


/* --------------------- MESH --------------------- */
void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, vector<u32>& submeshMaterialIndices);

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, vector<u32>& submeshMaterialIndices);

u32 LoadModel(App* app, const char* filename);