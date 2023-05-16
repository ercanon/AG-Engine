//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"
#include "Importer.h"
#include "Camera.h"
#include "GameObject.h"
#include <glad/glad.h>


struct OpenGLInfo
{
    string         glVersion;
    string         glRenderer;
    string         glVendor;
    string         glShadingVersion;
    vector<string> glExtensions;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_TexturedMesh,
    Mode_Count
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    Camera camera;
    ivec2 displaySize;

    vector<Texture>  textures;
    vector<Material> materials;
    vector<Mesh>     meshes;
    vector<GameObject> gameObject;
    vector<Program>  programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 texturedMeshProgram_uTexture;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 patrickMeshIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    OpenGLInfo glInfo;
};


GLuint CreateProgramFromSource(String programSource, const char* shaderName);

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

