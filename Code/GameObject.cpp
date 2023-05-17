#include "Engine.h"

GameObject::GameObject(int type) //Delete
{
    oType = (ObjectType)type;

    //General
    pos = vec3(2.5f, 1.5f, -2.0f);
    scl = vec3(0.45f);
    rot = vec3(0);

    worldMatrix = mat4(1.0);
    worldViewProjection = mat4(0.0);
    localParamsOffset = 0;
    localParamSize = 0;

}

GameObject::GameObject(vec3 position, vec3 scale, vec3 rotation, Light newLight)
{
    oType = Lightning;

    light = newLight;

    //General
    pos = position;
    scl = scale;
    rot = rotation;

    localParamsOffset = 0;
    localParamSize = 0;
}

GameObject::GameObject(vec3 position, vec3 scale, vec3 rotation, u32 meshID)
{
    oType = Model;

    meshIdx = meshID;

    //General
    pos = position;
    scl = scale;
    rot = rotation;

    localParamsOffset = 0;
    localParamSize = 0;
}

void GameObject::Update(App* app)
{
    worldMatrix = TransformPositionRotationScale(pos, rot, scl);
    worldViewProjection = app->camera.projection * app->camera.view * worldMatrix;
}

void GameObject::HandleBuffer(GLint uniformBlockAligment, Buffer* bufferModel, Buffer* bufferLight)
{
    switch (oType)
    {
    case Model:
        AlignHead(*bufferModel, uniformBlockAligment);

        localParamsOffset = bufferModel->head;

        PushMat4(*bufferModel, worldMatrix);
        PushMat4(*bufferModel, worldViewProjection);

        localParamSize = bufferModel->head - localParamsOffset;
        break;
    case Lightning:
        AlignHead(*bufferLight, sizeof(vec4));

        PushUInt(*bufferLight, light.type);
        PushVec3(*bufferLight, light.color);
        PushVec3(*bufferLight, light.direction);
        PushVec3(*bufferLight, pos);
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