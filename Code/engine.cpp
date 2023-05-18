//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>


bool DrawVec3(const char* name, vec3& vec)
{
    vec3 lastVec = vec;
    ImGui::PushID(name);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text(name);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::Button("X");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &vec.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::Button("Y");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &vec.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::Button("Z");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &vec.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();

    if (lastVec.x != vec.x || lastVec.y != vec.y || lastVec.z != vec.z)
        return true;
    else return false;
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

    lightSize = 0;
    pickedGO = nullptr;
    frameBuffer.color    = true;
    frameBuffer.normal   = true;
    frameBuffer.position = true;
    frameBuffer.depth    = true;
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

    // Uniform Buffer
    GLint maxUniformBufferSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAligment);

    lBuffer = CreateConstantBuffer(maxUniformBufferSize);
    mBuffer = CreateConstantBuffer(maxUniformBufferSize);

    // FrameBuffer
    glGenTextures(1, &frameBuffer.colorAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.colorAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA8, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(  GL_TEXTURE_2D, 0);
    
    glGenTextures(1, &frameBuffer.normalAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.normalAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA8, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(  GL_TEXTURE_2D, 0);

    glGenTextures(1, &frameBuffer.positionAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.positionAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA8, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(  GL_TEXTURE_2D, 0);

    glGenTextures(1, &frameBuffer.depthAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.depthAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, displaySize.x, displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(  GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &frameBuffer.frameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.frameBufferHandle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBuffer.colorAttachment,   0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, frameBuffer.normalAttachment,  0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, frameBuffer.positionAttachment,0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  frameBuffer.depthAttachment,   0);

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

    glDrawBuffers(1, &frameBuffer.colorAttachment);
    glBindFramebuffer(GL_FRAMEBUFFER, 6);

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


    mode = Mode_TexturedMesh;

    texturedGeometryProgramIdx = LoadProgram(this, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedGeometryProgram = programs[texturedGeometryProgramIdx];

    texturedMeshProgramIdx = LoadProgram(this, "shaders.glsl", "TEXTURED_MESH");
    Program& texturedMeshProgram = programs[texturedMeshProgramIdx];

    LoadTexture2D(this, "dice.png");
    LoadTexture2D(this, "color_white.png");
    LoadTexture2D(this, "color_black.png");
    LoadTexture2D(this, "color_normal.png");
    LoadTexture2D(this, "color_magenta.png");
    LoadModel(this, "Patrick/Patrick.obj");

    Light newLight = { 
    LightType::Directional,
    vec3 ( 1.0f, 1.0f, 1.0f ),
    vec3 ( 1.0f, 1.0f, 1.0f )};
    lightSize++;
    gameObject.push_back(GameObject{ "Light", vec3(0.0), vec3(1.0), vec3(0.0), newLight});
}

void App::Gui()
{
    ImGui::Begin("Hierarchy");
    for (GameObject& go : gameObject)
    {
        if (ImGui::TreeNodeEx(go.objName.c_str(), ImGuiTreeNodeFlags_Leaf))
        {
            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
                pickedGO = &go;
            ImGui::TreePop();
        }
    }

    if (ImGui::IsWindowHovered() && (ImGui::GetIO().MouseClicked[0] || ImGui::GetIO().MouseClicked[1]))
        pickedGO = nullptr;
    ImGui::End();

   
    if (pickedGO != nullptr)
    {
        ImGui::Begin("Inspector");
        
        ImGui::Text("Name");
        ImGui::SameLine();
        ImGui::InputText("##Name", &pickedGO->objName[0], 30);

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Transform"))
        {
            ImGui::PushItemWidth(90);

            DrawVec3("Position: ", pickedGO->objPos);
            DrawVec3("Rotation: ", pickedGO->objRot);
            /*
            rotationInEuler.x = RADTODEG * rotationEditor.x;
            rotationInEuler.y = RADTODEG * rotationEditor.y;
            rotationInEuler.z = RADTODEG * rotationEditor.z;
            if (DrawVec3("Rotation: ", rotationInEuler))
            {
                rotationInEuler.x = DEGTORAD * rotationInEuler.x;
                rotationInEuler.y = DEGTORAD * rotationInEuler.y;
                rotationInEuler.z = DEGTORAD * rotationInEuler.z;

                mat4 rotationDelta = mat4::FromEulerXYZ(rotationInEuler.x - rotationEditor.x, rotationInEuler.y - rotationEditor.y, rotationInEuler.z - rotationEditor.z);
                rotation = rotation * rotationDelta;
                rotationEditor = rotationInEuler;
            }
            */
            DrawVec3("Scale: ", pickedGO->objScale);

            ImGui::Separator();
        }

        ImGui::End();
    }

    ImGui::Begin("Rendering Attachment");
    if (frameBuffer.normal && frameBuffer.position && frameBuffer.depth) frameBuffer.color = true;

    if (ImGui::Checkbox("Color", &frameBuffer.color))
        frameBuffer.normal = frameBuffer.position = frameBuffer.depth = frameBuffer.color;
    if (ImGui::Checkbox("Normal", &frameBuffer.normal))
        if (!frameBuffer.normal) frameBuffer.color = false;
    if (ImGui::Checkbox("Position", &frameBuffer.position))
        if (!frameBuffer.position) frameBuffer.color = false;
    if (ImGui::Checkbox("Depth", &frameBuffer.depth))
        if (!frameBuffer.depth) frameBuffer.color = false;
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
    PushUInt(lBuffer, lightSize);

    for (GameObject& go : gameObject)
    {
        go.Update(this);
        go.HandleBuffer(&lBuffer);
    }
    globalParamsSize = lBuffer.head - globalParamsOffset;
    UnmapBuffer(lBuffer);

    MapBuffer(mBuffer, GL_WRITE_ONLY);
    for (GameObject& go : gameObject)
        go.HandleBuffer(uniformBlockAligment, &mBuffer);
    UnmapBuffer(mBuffer);
}

void App::Render()
{
    switch (mode)
    {
        case Mode::Mode_TexturedMesh:
        {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.frameBufferHandle);

            GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, drawBuffers);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& texturedMeshProgram = programs[texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), lBuffer.handle, globalParamsOffset, globalParamsSize);

            for (GameObject& go : gameObject)
            {
                if (go.IsType(ObjectType::Model))
                {
                    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), mBuffer.handle, go.GetLocalOffset(), go.GetLocalSize());
                    Mesh mesh = go.GetMesh();
                    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                    {
                        GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                        glBindVertexArray(vao);

                        Material& submeshMaterial = materials[go.GetMesh().materialIdx[i]];

                        glUniform1i(glGetUniformLocation(texturedMeshProgram.handle, "uTexture"), 0);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, textures[submeshMaterial.albedoTextureIdx].handle);

                        Submesh& submesh = mesh.submeshes[i];
                        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                        glBindVertexArray(0);
                    }
                }
            }

            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        case Mode_TexturedQuad:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Program& texturedGeomProgram = programs[texturedGeometryProgramIdx];
            glUseProgram(texturedGeomProgram.handle);
            glBindVertexArray(vao);

            //glEnable(GL_BLEND);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUniform1i(glGetUniformLocation(texturedGeomProgram.handle, "uTexture"), 0);
            glActiveTexture(GL_TEXTURE0);
            if (frameBuffer.color)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.colorAttachment);
            else if (frameBuffer.normal)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.normalAttachment);
            else if (frameBuffer.position)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.positionAttachment);
            else if (frameBuffer.depth)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.depthAttachment);
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
        break;
        default:;
    }
}