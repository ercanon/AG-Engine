//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>


ImGuiTreeNodeFlags SetFlags(Mesh node)
{
    // This flags allow to open the tree if you click on arrow or doubleClick on object, by default the tree is open  
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // If GameObject doesn't childrens = no collapsing and no arrow
    if (node.submeshes.size() == 0)
        flags |= ImGuiTreeNodeFlags_Leaf;


    return flags;
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

    lBuffer = CreateBuffer(maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
    mBuffer = CreateBuffer(maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
    /*
    // FrameBuffer
    glGenTextures(1, &colorAttachmentHandle);
    glBindTexture(  GL_TEXTURE_2D, colorAttachmentHandle);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA8, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 6);
    
    GLuint depthAttachmentHandle;
    glGenTextures(1, &depthAttachmentHandle);
    glBindTexture(  GL_TEXTURE_2D, depthAttachmentHandle);
    glTexImage2D(   GL_TEXTURE_2D, 8, GL_DEPTH_COMPONENT24, displaySize.x, displaySize.y, 8, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 6);
    
    glGenFramebuffers(1, &framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorAttachmentHandle, 6); 
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachmentHandle, 6);
    
    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (framebufferStatus)
        {
        case GL_FRAMEBUFFER_UNDEFINED:                      ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
        case GL_FRAMEBUFFER_UNSUPPORTED:                    ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
        default: ELOG("Unknown framebuffer status error");
        }
    }

    glDrawBuffers(1, &colorAttachmentHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, 6);
    */
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
    texturedMeshTexture = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    diceTexIdx = LoadTexture2D(this, "dice.png");
    LoadTexture2D(this, "color_white.png");
    LoadTexture2D(this, "color_black.png");
    LoadTexture2D(this, "color_normal.png");
    LoadTexture2D(this, "color_magenta.png");
    LoadModel(this, "Patrick/Patrick.obj");
    LoadModel(this, "Patrick/Patrick.obj");

    Light newLight = { 
    LightType::Point,
    vec3 ( 1.0f, 0.0f, 0.0f ),
    vec3 ( 0.0f, 0.0f, 0.0f ),
    vec3 (0.0f, 0.0f, 0.0f) };
    lights.push_back(newLight);
}

void App::Gui()
{
    ImGui::Begin("Hierarchy");
    for (GameObject& go : gameObject)
    {
        if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())
            goPicked(&go);

        if (ImGui::TreeNodeEx(go.GetName().c_str(), SetFlags(go.GetMesh())))
        {
            int size = go.GetMesh().submeshes.size();
            for (int i = 0; i < size; ++i)
            {
                string subName = "Submesh " + to_string(i);
                if (ImGui::TreeNodeEx(subName.c_str(), ImGuiTreeNodeFlags_Leaf))
                    ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsWindowFocused() && (ImGui::GetIO().MouseClicked[0] || ImGui::GetIO().MouseClicked[1]))
        goPicked(nullptr);
    ImGui::End();

    ImGui::Begin("Inspector");
    if (goPicked() != nullptr)
    {
        //if (item == ItemType::NONE)
        //{
        //    DrawDefaultInspector(obj);
        //    obj->DrawEditor();
        //}
        //else
        //{
        //    DrawEditLists();
        //}
    }
    ImGui::End();

    if (input.keys[Key::K_F12] == ButtonState::BUTTON_PRESSED)
        ImGui::OpenPopup("OpenGL Info");

    if (ImGui::BeginPopup("OpenGL Info"))
    {
        ImGui::Text("FPS: %f", 1.0f / deltaTime);

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


    MapBuffer(lBuffer, GL_WRITE_ONLY);
    globalParamsOffset = lBuffer.head;

    PushVec3(lBuffer, camera.pos);
    PushUInt(lBuffer, lights.size());

    for (Light& light : lights)
    {
        AlignHead(lBuffer, sizeof(vec4));

        PushUInt(lBuffer, (unsigned int)light.type);
        PushVec3(lBuffer, light.color);
        PushVec3(lBuffer, light.direction);
        PushVec3(lBuffer, light.pos);
    }
    globalParamsSize = lBuffer.head - globalParamsOffset;
    UnmapBuffer(lBuffer);

    MapBuffer(mBuffer, GL_WRITE_ONLY);
    for (GameObject& go : gameObject)
    {
        go.Update(this);
        go.HandleBuffer(uniformBlockAligment, &mBuffer);
    }
    UnmapBuffer(mBuffer);
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
            //glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);

            //GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
            //glDrawBuffers(1, drawBuffers);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            //glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& texturedMeshProgram = programs[texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), lBuffer.head, globalParamsOffset, globalParamsSize);

            for (GameObject& go : gameObject)
            {
                Mesh mesh = go.GetMesh();
                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                    glBindVertexArray(vao);

                    Material& submeshMaterial = materials[go.GetMesh().materialIdx[i]];

                    glUniform1i(texturedMeshTexture, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textures[submeshMaterial.albedoTextureIdx].handle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), mBuffer.head, go.GetLocalOffset(), go.GetLocalSize());

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    glBindVertexArray(0);
                }
            }

            glUseProgram(0);
            //glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);
        }
        break;
        default:;
    }
}