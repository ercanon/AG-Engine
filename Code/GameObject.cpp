#include "Engine.h"

GameObject::GameObject()
{

}

GameObject::~GameObject()
{
}

void GameObject::Update(App* app)
{
    mat4 world = TransformPositionScale(vec3(2.5f, 1.5f, -2.0f), vec3(0.45f));
    mat4 worldViewProjection = app->camera.projection * app->camera.view * world;
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