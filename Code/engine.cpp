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

void App::PassBlur(FrameBuffer fb, const ivec2 &view, GLenum colorAttach, GLuint inputTExture, GLint inputLod, const ivec2 &direction)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fb.frameBufferHandle);

    glDrawBuffer(colorAttach);
    glViewport(0, 0, view.x, view.y);
    
    glUseProgram(blurProgram.handle);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTExture);
    
    glUniform1i(glGetUniformLocation(blurProgram.handle, "uColorMap"), 0);
    glUniform2f(glGetUniformLocation(blurProgram.handle, "uDirection"), direction.x, direction.y);
    glUniform1i(glGetUniformLocation(blurProgram.handle, "uInputLod"), inputLod);
    
    for (GameObject& go : gameObject)
    {
        if (go.IsType(ObjectType::Model))
        {
            Mesh mesh = go.GetMesh();
            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }
        }
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

App::App(f32 dt, ivec2 dispSize, bool running)
{
    deltaTime = dt;
    displaySize = dispSize;
    isRunning = running;

    lightSize = 0;
    pickedGO = nullptr;
    frameBuffer = FrameBuffer{};
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

    /* --------------------- Uniform Buffer --------------------- */
    GLint maxUniformBufferSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAligment);

    lBuffer = CreateConstantBuffer(maxUniformBufferSize);
    mBuffer = CreateConstantBuffer(maxUniformBufferSize);

    /* --------------------- FrameBuffer --------------------- */
    glGenTextures(1, &frameBuffer.colorAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.colorAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 0);
    
    glGenTextures(1, &frameBuffer.normalAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.normalAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 0);

    glGenTextures(1, &frameBuffer.positionAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.positionAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 0);

    glGenTextures(1, &frameBuffer.emissiveAttachment);
    glBindTexture(GL_TEXTURE_2D, frameBuffer.emissiveAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &frameBuffer.depthAttachment);
    glBindTexture(  GL_TEXTURE_2D, frameBuffer.depthAttachment);
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, displaySize.x, displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_2D, 0);

    frameBuffer.Bind();
    frameBuffer.TextureAttach(GL_COLOR_ATTACHMENT0, frameBuffer.colorAttachment);
    frameBuffer.TextureAttach(GL_COLOR_ATTACHMENT1, frameBuffer.normalAttachment);
    frameBuffer.TextureAttach(GL_COLOR_ATTACHMENT2, frameBuffer.positionAttachment);
    frameBuffer.TextureAttach(GL_COLOR_ATTACHMENT3, frameBuffer.emissiveAttachment);
    frameBuffer.TextureAttach(GL_DEPTH_ATTACHMENT,  frameBuffer.depthAttachment);
    frameBuffer.CheckStatus();

    /* --------------------- Geometry --------------------- */
    glGenBuffers(1, &embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* --------------------- Attribute state --------------------- */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glBindVertexArray(0);

    /* --------------------- Bloom --------------------- */
    if (bloom.rtBright != 0)
        glDeleteTextures(1, &bloom.rtBright);
    glGenTextures(1, &bloom.rtBright);
    glBindTexture(GL_TEXTURE_2D, bloom.rtBright);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, MIPMAP_BASE_LEVEL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_MAX_LEVEL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x/2, displaySize.y/2, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, displaySize.x/4, displaySize.y/4, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, displaySize.x/8, displaySize.y/8, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, displaySize.x/16, displaySize.y/16, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, displaySize.x/32, displaySize.y/32, 0, GL_RGBA, GL_FLOAT, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (bloom.rtBloomH != 0)
        glDeleteTextures(1, &bloom.rtBloomH);
    glGenTextures(1, &bloom.rtBloomH);
    glBindTexture(GL_TEXTURE_2D, bloom.rtBloomH);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, MIPMAP_BASE_LEVEL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_MAX_LEVEL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x / 2, displaySize.y / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, displaySize.x / 4, displaySize.y / 4, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, displaySize.x / 8, displaySize.y / 8, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, displaySize.x / 16, displaySize.y / 16, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, displaySize.x / 32, displaySize.y / 32, 0, GL_RGBA, GL_FLOAT, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    bloom.fboBloom.push_back(FrameBuffer{});
    bloom.fboBloom.push_back(FrameBuffer{});
    bloom.fboBloom.push_back(FrameBuffer{});
    bloom.fboBloom.push_back(FrameBuffer{});
    bloom.fboBloom.push_back(FrameBuffer{});

    for (FrameBuffer& fb : bloom.fboBloom)
    {
        fb.Bind();
        fb.TextureAttach(GL_COLOR_ATTACHMENT0, bloom.rtBright);
        fb.TextureAttach(GL_COLOR_ATTACHMENT1, bloom.rtBloomH);
        fb.CheckStatus();
    }



    /* --------------------- Init Engine --------------------- */
    mode = Mode_TexturedMesh;

    texturedGeometryProgram = LoadProgram(this, "shaders.glsl", "TEXTURED_GEOMETRY");
    texturedMeshProgram = LoadProgram(this, "shaders.glsl", "TEXTURED_MESH");
    blitProgram = LoadProgram(this, "shaders.glsl", "BLIT");
    blurProgram = LoadProgram(this, "shaders.glsl", "BLUR");
    bloomProgram = LoadProgram(this, "shaders.glsl", "BLOOM");

    LoadTexture2D(this, "dice.png");
    LoadTexture2D(this, "color_white.png");
    LoadTexture2D(this, "color_black.png");
    LoadTexture2D(this, "color_normal.png");
    LoadTexture2D(this, "color_magenta.png");
    LoadModel(this, "Patrick/Patrick.obj");
    LoadModel(this, "Patrick/Patrick.obj");
    gameObject.back().objPos.x += 5.0f;
    LoadModel(this, "Patrick/Patrick.obj");
    gameObject.back().objPos.x -= 5.0f;

    Light pointLight = { 
    LightType::Point,
    vec3 ( 1.0f, 1.0f, 1.0f ),
    vec3 (1.0f, 1.0f, 1.0f)};
    lightSize++;
    gameObject.push_back(GameObject{ "Point Light", vec3(-1.0f, 0.5f, -9.0f), vec3(0.2f, 1.0f, 0.5f), vec3(0.0f), pointLight });

    Light directLight = {
    LightType::Directional,
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f) };
    lightSize++;
    gameObject.push_back(GameObject{ "Directional Light", vec3(0.0f, 0.0f, -15.0f), vec3(0.2f, 1.0f, 0.5f), vec3(0.0f), directLight });
}

void App::Gui()
{
    ImGui::Begin("Generate");
    ImGui::Text("Model");
    if (ImGui::Button("Patrick", ImVec2(ImGui::GetWindowSize().x - 15, 20)))
        LoadModel(this, "Patrick/Patrick.obj");
    ImGui::Text("Light");
    if (ImGui::Button("Direct", ImVec2((ImGui::GetWindowSize().x/2) - 12, 20)))
    {
        Light newLight = {
        LightType::Directional,
        vec3(1.0f),
        vec3(1.0f) };
        lightSize++;
        gameObject.push_back(GameObject{ "Directional Light", vec3(0.0f), vec3(0.2f, 1.0f, 0.5f), vec3(0.0f), newLight });
    }
    ImGui::SameLine();
    if (ImGui::Button("Pointer", ImVec2((ImGui::GetWindowSize().x/2) - 12, 20)))
    {
        Light newLight = {
        LightType::Point,
        vec3(1.0f),
        vec3(1.0f) };
        lightSize++;
        gameObject.push_back(GameObject{ "Point Light", vec3(0.0f), vec3(0.2f, 1.0f, 0.5f), vec3(0.0f), newLight });
    }
    if (ImGui::IsWindowHovered())
        pickedGO = nullptr;
    ImGui::End();

    ImGui::Begin("Hierarchy");
    if (input.keys[Key::K_P] == ButtonState::BUTTON_PRESSED && pickedGO != nullptr);
        // Aqui borras el objeto pero el vector es una putisima mierda
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

            if (pickedGO->IsType(ObjectType::Model))
            {
                DrawVec3("Position: ",  pickedGO->objPos);
                DrawVec3("Rotation: ",  pickedGO->objRot);
                DrawVec3("Scale: ",     pickedGO->objScale);
                ImGui::Spacing();
                ImGui::Spacing();
                if (ImGui::Button("Reset", ImVec2(ImGui::GetWindowSize().x - 15, 20)))
                {
                    pickedGO->objPos = vec3(0.0f, 0.0f, -10.0f);
                    pickedGO->objRot = vec3(0.0f);
                    pickedGO->objScale = vec3(1.0f);
                }
            }
            else if (pickedGO->IsType(ObjectType::Lightning))
            {
                DrawVec3("Position: ",  pickedGO->objPos);
                DrawVec3("Intensity: ", pickedGO->objScale);
                DrawVec3("Color: ",     pickedGO->GetLight()->color);
                DrawVec3("Direction: ", pickedGO->GetLight()->direction);
                ImGui::Spacing();
                ImGui::Spacing();
                if (ImGui::Button("Reset", ImVec2(ImGui::GetWindowSize().x - 15, 20)))
                {
                    pickedGO->objPos = vec3(0.0f);
                    pickedGO->objScale = vec3(0.2f, 1.0f, 0.5f);
                    pickedGO->GetLight()->color = vec3(1.0f);
                    pickedGO->GetLight()->direction = vec3(0.0f, 0.0f, 1.0f);
                }
            }
            ImGui::Separator();
        }
        ImGui::End();
    }

    ImGui::Begin("Rendering Attachment");
    if (frameBuffer.normal && frameBuffer.position && frameBuffer.depth && frameBuffer.color && frameBuffer.emissive) frameBuffer.shader = true;

    if (ImGui::Checkbox("Shader", &frameBuffer.shader))
        frameBuffer.normal = frameBuffer.position = frameBuffer.depth = frameBuffer.color = frameBuffer.emissive = frameBuffer.shader;
    if (ImGui::Checkbox("Color", &frameBuffer.color))
        if (!frameBuffer.color) frameBuffer.shader = false;
    if (ImGui::Checkbox("Normal", &frameBuffer.normal))
        if (!frameBuffer.normal) frameBuffer.shader = false;
    if (ImGui::Checkbox("Position", &frameBuffer.position))
        if (!frameBuffer.position) frameBuffer.shader = false;
    if (ImGui::Checkbox("Emissive", &frameBuffer.emissive))
        if (!frameBuffer.emissive) frameBuffer.shader = false;
    if (ImGui::Checkbox("Depth", &frameBuffer.depth))
        if (!frameBuffer.depth) frameBuffer.shader = false;

    if (ImGui::IsWindowHovered())
        pickedGO = nullptr;
    ImGui::End();

    if (input.keys[Key::K_F1] == ButtonState::BUTTON_PRESSED)
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

    ImGui::Begin("Controles");
    ImGui::Text("AWSD para mover");
    ImGui::Text("QE para la altura de la camara");
    ImGui::Text("CONTROL para mayor velocidad de camara");
    ImGui::Text("DOBLE CLICK IZQUIERDO en Hierarchy para seleccionar GameObject");
    ImGui::Text("Con GameObject seleccionado, F para centrar");
    //ImGui::Text("Con GameObject seleccionado, P para eliminar GameObject");
    ImGui::Text("ENTER para cerrar la aplicacion");
    ImGui::Text("F1 para informacion OpenGL");
    ImGui::End();
}

void App::Update()
{
    if (input.keys[Key::K_ENTER] == ButtonState::BUTTON_PRESSED)
        isRunning = false;
    /*
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
    */

    camera.Update(this);

    MapBuffer(lBuffer, GL_WRITE_ONLY);
    globalParamsOffset = lBuffer.head;

    PushVec3(lBuffer, camera.pos);
    PushUInt(lBuffer, lightSize);

    for (GameObject& go : gameObject)
    {
        go.Update(this);
        go.HandleBuffer(lBuffer);
    }
    globalParamsSize = lBuffer.head - globalParamsOffset;
    UnmapBuffer(lBuffer);

    MapBuffer(mBuffer, GL_WRITE_ONLY);
    for (GameObject& go : gameObject)
        go.HandleBuffer(uniformBlockAligment, mBuffer);
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
            glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

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
        case Mode_Bloom:
        {
            {
                glBindFramebuffer(GL_FRAMEBUFFER, bloom.fboBloom[0].frameBufferHandle);

                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                glViewport(0, 0, displaySize.x, displaySize.y);

                glUseProgram(blitProgram.handle);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, frameBuffer.emissiveAttachment);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                glUniform1i(glGetUniformLocation(blitProgram.handle, "uColorTexture"), 0);
                glUniform1f(glGetUniformLocation(blitProgram.handle, "uThreshold"), 1.0f);
                
                for (GameObject& go : gameObject)
                {
                    if (go.IsType(ObjectType::Model))
                    {
                        Mesh mesh = go.GetMesh();
                        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                        {
                            Submesh& submesh = mesh.submeshes[i];
                            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                        }
                    }
                }
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glUseProgram(0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            glBindTexture(GL_TEXTURE_2D, bloom.rtBright);
            glGenerateMipmap(GL_TEXTURE_2D);

            for (int i = 0; i < bloom.fboBloom.size(); i++)
            {
                PassBlur(bloom.fboBloom[i], displaySize / (2 * (i + 1)), GL_COLOR_ATTACHMENT1, bloom.rtBright, i, ivec2(1.0f, 0.0f));
            }
            
            for (int i = 0; i < bloom.fboBloom.size(); i++)
            {
                PassBlur(bloom.fboBloom[i], displaySize / (2 * (i + 1)), GL_COLOR_ATTACHMENT0, bloom.rtBloomH, i, ivec2(0.0f, 1.0f));
            }

            {
                glBindFramebuffer(GL_FRAMEBUFFER, bloom.fboBloom[0].frameBufferHandle);

                glDrawBuffer(GL_COLOR_ATTACHMENT3);
                glViewport(0, 0, displaySize.x, displaySize.y);

                glUseProgram(bloomProgram.handle);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, bloom.rtBright);

                glUniform1i(glGetUniformLocation(bloomProgram.handle, "uColorMap"), 0);
                glUniform1i(glGetUniformLocation(bloomProgram.handle, "uMaxLod"), 4.0f);
                
                for (GameObject& go : gameObject)
                {
                    if (go.IsType(ObjectType::Model))
                    {
                        Mesh mesh = go.GetMesh();
                        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                        {
                            Submesh& submesh = mesh.submeshes[i];
                            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                        }
                    }
                }
                
                glUseProgram(0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        case Mode_TexturedQuad:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(texturedGeometryProgram.handle);
            glBindVertexArray(vao);

            glUniform1i(glGetUniformLocation(texturedGeometryProgram.handle, "uTexture"), 0);
            glUniform1i(glGetUniformLocation(texturedGeometryProgram.handle, "isDepth"), frameBuffer.depth && !frameBuffer.normal && !frameBuffer.position && !frameBuffer.color);
            glActiveTexture(GL_TEXTURE0);

           
            if (frameBuffer.shader)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.frameBufferHandle);
            else if (frameBuffer.color)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.colorAttachment);
            else if (frameBuffer.normal)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.normalAttachment);
            else if (frameBuffer.position)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.positionAttachment);
            else if (frameBuffer.emissive)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.emissiveAttachment);
            else if (frameBuffer.depth)
                glBindTexture(GL_TEXTURE_2D, frameBuffer.depthAttachment);
            glGenerateMipmap(GL_TEXTURE_2D);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
        break;
        default:;
    }
}