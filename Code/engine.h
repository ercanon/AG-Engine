//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"
#include "Importer.h"
#include "Camera.h"
#include "GameObject.h"
#include "BufferManager.h"


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

class App
{
public:
    App(f32 dt, ivec2 dispSize, bool running);
    virtual ~App() {};
    void Init();
    void Gui();
    void Update();
    void Render();

    virtual bool IsRunning(bool run = -1) { return run != -1 ? isRunning = run : isRunning; }
    virtual ivec2 dispSize(vec2 size = vec2(-1)) { return size != vec2(-1) ? displaySize = size : displaySize; }
    virtual f32 dt(f32 set = -1) { return set != -1 ? deltaTime = set : deltaTime; }

    vector<Texture>     textures;
    vector<Material>    materials;
    vector<Mesh>        meshes;
    vector<GameObject>  gameObject;
    vector<Program>     programs;
    Camera              camera;
    Input               input;


private:
    // Loop
    f32  deltaTime;
    bool isRunning;
    OpenGLInfo glInfo;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];
    ivec2 displaySize;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 texturedMeshProgram_uTexture;
    
    // texture indices
    u32 diceTexIdx;

    // Mode
    Mode mode;

    // Buffer
    Buffer cBuffer;
    GLint uniformBlockAligment;
    u32 globalParamsOffset;
    u32 globalParamsSize;

    GLuint framebufferHandle;
    GLuint colorAttachmentHandle;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
};


GLuint CreateProgramFromSource(String programSource, const char* shaderName);


