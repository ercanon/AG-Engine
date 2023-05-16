//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;

    GLuint vaoHandle = 0;

    // Create a new vao for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        // We have to link all vertex inputs attributes to attributes in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset; // attribute offset + vertex offset
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked); // The submesh should provide an attribute for each vertex inputs
        }

        glBindVertexArray(0);
    }

    // Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

App::App(f32 dt, ivec2 dispSize, bool running)
{
    deltaTime = dt;
    displaySize = dispSize;
    isRunning = running;
}

void App::Init()
{
    glInfo.glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    glInfo.glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    glInfo.glVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    glInfo.glShadingVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (GLint i = 0; i < numExtensions; ++i)
        glInfo.glExtensions.push_back(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, GLuint(i))));

    mode = Mode_TexturedMesh;

    // Uniform Buffer
    GLint maxUniformBufferSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAligment);

    cBuffer = CreateBuffer(maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
    globalParamsOffset = cBuffer.head;

    // Geometry
    glGenBuffers(1, &embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attribute state
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glBindVertexArray(0);

    texturedGeometryProgramIdx = LoadProgram(this, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedGeometryProgram = programs[texturedGeometryProgramIdx];
    programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");

    texturedMeshProgramIdx = LoadProgram(this, "shaders.glsl", "TEXTURED_MESH");
    Program& texturedMeshProgram = programs[texturedMeshProgramIdx];
    glGetProgramiv(texturedMeshProgram.handle, GL_ACTIVE_ATTRIBUTES, &texturedMeshProgram.lenght);

    GLchar attribName[128];
    GLsizei attribLenght;
    GLint attribSize;
    GLenum attribType;
    for (u32 i = 0; i < texturedMeshProgram.lenght; ++i)
    {
        glGetActiveAttrib(texturedMeshProgram.handle, i, ARRAY_COUNT(attribName), &attribLenght, &attribSize, &attribType, attribName);
        GLuint attributeLocation = glGetAttribLocation(texturedMeshProgram.handle, attribName);

        texturedMeshProgram.vertexInputLayout.attributes.push_back({ (u8)attributeLocation, (u8)attribSize });
    }

    diceTexIdx = LoadTexture2D(this, "dice.png");
    LoadTexture2D(this, "color_white.png");
    LoadTexture2D(this, "color_black.png");
    LoadTexture2D(this, "color_normal.png");
    LoadTexture2D(this, "color_magenta.png");
    LoadModel(this, "Patrick/Patrick.obj");
}

void App::Gui()
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f / deltaTime);
    ImGui::End();

    if (input.keys[Key::K_SPACE] == ButtonState::BUTTON_PRESSED)
        ImGui::OpenPopup("OpenGL Info");

    if (ImGui::BeginPopup("OpenGL Info"))
    {
        ImGui::Text("Version: %s", glInfo.glVersion.c_str());
        ImGui::Text("Renderer: %s", glInfo.glRenderer.c_str());
        ImGui::Text("Vendor: %s", glInfo.glVendor.c_str());
        ImGui::Text("GLSL Version: %s", glInfo.glShadingVersion.c_str());

        ImGui::Separator();
        ImGui::Text("Extensions");
        for (GLint i = 0; i < glInfo.glExtensions.size(); ++i)
            ImGui::Text("GLSL Version: %s", glInfo.glExtensions[i].c_str());

        ImGui::EndPopup();
    };
}

void App::Update()
{
    //HandleInput(app);

    for (Program& program : programs)
    {
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    camera.Update(this);


    MapBuffer(cBuffer, GL_WRITE_ONLY);
    globalParamsOffset = cBuffer.head;

    PushVec3(cBuffer, camera.pos);
    PushUInt(cBuffer, gameObject.size());

    for (GameObject& go : gameObject)
    {
        go.Update(this);
        go.HandleBuffer(uniformBlockAligment, cBuffer);
    }

    globalParamsSize = cBuffer.head - globalParamsOffset;

    UnmapBuffer(cBuffer);
}

void App::Render()
{
    switch (mode)
    {
        case Mode_TexturedQuad:
        {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& programTexturedGeometry = programs[texturedGeometryProgramIdx];
            glUseProgram(programTexturedGeometry.handle);
            glBindVertexArray(vao);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUniform1i(programUniformTexture, 0);
            glActiveTexture(GL_TEXTURE0);
            GLuint textureHandle = textures[diceTexIdx].handle;
            glBindTexture(GL_TEXTURE_2D, textureHandle);

            glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(u16), GL_UNSIGNED_SHORT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
        break;
        case Mode::Mode_TexturedMesh:
        {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& texturedMeshProgram = programs[texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), cBuffer.head, globalParamsOffset, globalParamsSize);

            for (GameObject& go : gameObject)
            {
                u32* localParams = go.GetLocalParams();
                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), cBuffer.head, localParams[0], localParams[1]);
                Mesh& mesh = meshes[go.MeshID()];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = go.MaterialID(i);
                    Material& submeshMaterial = materials[submeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textures[submeshMaterial.albedoTextureIdx].handle);
                    glUniform1i(texturedMeshProgram_uTexture, 0);

                    glUniformMatrix4fv(glGetUniformLocation(texturedMeshProgram.handle, "uWorldMatrix"), 1, GL_FALSE, value_ptr(go.GetView()));
                    glUniformMatrix4fv(glGetUniformLocation(texturedMeshProgram.handle, "uWorldViewProjectionMatrix"), 1, GL_FALSE, value_ptr(go.GetProjection()));

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    glBindVertexArray(0);
                }
            }

            glUseProgram(0);
        }
        default:;
    }
}