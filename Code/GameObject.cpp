#include "Engine.h"


GameObject::GameObject(string name, vec3 position, vec3 scale, vec3 rotation, Mesh mesh)
{
    objName = name;

    objMesh = mesh;

    //General
    pos = position;
    scl = scale;
    rot = rotation;

    localParamsOffset = 0;
    localParamSize = 0;
}

void GameObject::Update(App* app)
{
    worldMatrix = TransformPositionScale(pos, scl);
    worldViewProjection = app->camera.projection * app->camera.view * translate(worldMatrix, vec3(1.0f, 1.0f, 0.0f));
}

void GameObject::HandleBuffer(GLint uniformBlockAligment, Buffer* bufferModel)
{
    AlignHead(*bufferModel, uniformBlockAligment);

    localParamsOffset = bufferModel->head;

    PushMat4(*bufferModel, worldMatrix);
    PushMat4(*bufferModel, worldViewProjection);

    localParamSize = bufferModel->head - localParamsOffset;
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