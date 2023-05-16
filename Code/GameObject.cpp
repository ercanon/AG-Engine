#include "Engine.h"

GameObject::GameObject()
{

}

GameObject::~GameObject()
{
}

void GameObject::Update(App* app)
{
    worldMatrix = TransformPositionScale(vec3(2.5f, 1.5f, -2.0f), vec3(0.45f));
    worldViewProjection = app->camera.projection * app->camera.view * worldMatrix;
}

void GameObject::HandleBuffer(GLint uniformBlockAligment, u32 bufferHead, u8* bufferData)
{
    bufferHead = Align(bufferHead, uniformBlockAligment);

    localParamsOffset = bufferHead;

    memcpy(bufferData + bufferHead, value_ptr(worldMatrix), sizeof(mat4));
    bufferHead += sizeof(mat4);

    memcpy(bufferData + bufferHead, value_ptr(worldViewProjection), sizeof(mat4));
    bufferHead += sizeof(mat4);
    localParamSize = bufferHead - localParamsOffset;
}

mat4 GameObject::TransformScale(const vec3& scaleFactors)
{
    mat4 transform = scale(scaleFactors);
    return transform;
}

mat4 GameObject::TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
    mat4 transform = translate(pos);
    transform = scale(transform, scaleFactors);
    return transform;
}

mat4 GameObject::TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors)
{
    mat4 transform = translate(pos);
    transform = rotate(transform, radians(90.0f), rotation);
    transform = scale(transform, scaleFactors);
    return transform;
}