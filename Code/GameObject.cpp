#include "Engine.h"

GameObject::GameObject()
{

}

void GameObject::Update(App* app)
{
    worldMatrix = TransformPositionScale(vec3(2.5f, 1.5f, -2.0f), vec3(0.45f));
    worldViewProjection = app->camera.projection * app->camera.view * worldMatrix;
}

void GameObject::HandleBuffer(GLint uniformBlockAligment, Buffer buffer)
{
    switch (oType)
    {
    case Model:
        AlignHead(buffer, uniformBlockAligment);

        localParamsOffset = buffer.head;

        PushMat4(buffer, worldMatrix);
        PushMat4(buffer, worldViewProjection);
        localParamSize = buffer.head - localParamsOffset;
        break;
    case Lightning:
        AlignHead(buffer, sizeof(vec4));

        PushUInt(buffer, light.type);
        PushVec3(buffer, light.color);
        PushVec3(buffer, light.direction);
        PushVec3(buffer, pos);
        break;
    default:;
    }
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