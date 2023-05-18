#include "Engine.h"

GameObject::GameObject()
{
    objType = ObjectType::Empty;
    objName = "Empty GameObject";

    //General
    objPos = vec3(0.0f);
    objScale = vec3(1.0f);
    objRot = vec3(0.0f);

    localParamsOffset = 0;
    localParamSize = 0;
}

GameObject::GameObject(string name, vec3 position, vec3 scale, vec3 rotation, Mesh mesh)
{
    objType = ObjectType::Model;
    objName = name;

    objMesh = mesh;

    //General
    objPos = position;
    objScale = scale;
    objRot = rotation;

    localParamsOffset = 0;
    localParamSize = 0;
}

GameObject::GameObject(string name, vec3 position, vec3 scale, vec3 rotation, Light newLight)
{
    objType = ObjectType::Lightning;
    objName = name;

    light = newLight;

    //General
    objPos = position;
    objScale = scale;
    objRot = rotation;

    localParamsOffset = 0;
    localParamSize = 0;
}

void GameObject::Update(App* app)
{
    worldMatrix = TransformPositionScale(objPos, objScale);
    worldViewProjection = app->camera.projection * app->camera.view * translate(worldMatrix, vec3(1.0f, 1.0f, 0.0f));
}

void GameObject::HandleBuffer(GLint uniformBlockAligment, Buffer* buffer)
{
    if (objType == ObjectType::Model)
    {
        AlignHead(*buffer, uniformBlockAligment);

        localParamsOffset = buffer->head;

        PushMat4(*buffer, worldMatrix);
        PushMat4(*buffer, worldViewProjection);

        localParamSize = buffer->head - localParamsOffset;
    }
}
void GameObject::HandleBuffer(Buffer* buffer)
{
    if (objType == ObjectType::Lightning)
    {
        AlignHead(*buffer, sizeof(vec4));

        PushUInt(*buffer, light.type);
        PushVec3(*buffer, light.color);
        PushVec3(*buffer, light.direction);
        PushVec3(*buffer, objPos);
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