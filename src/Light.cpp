#include "Light.hpp"

Light::Light() 
    : type(Type::DIRECTIONAL)
    , position(0.0f, 0.0f, 0.0f)
    , direction(0.0f, -1.0f, 0.0f)
    , color(1.0f, 1.0f, 1.0f)
    , intensity(1.0f)
    , constantAttenuation(1.0f)
    , linearAttenuation(0.0f)
    , quadraticAttenuation(0.0f)
{
}

Light::Light(Type lightType, const Vector3& pos, const Vector3& dir, const Vector3& col, float intens)
    : type(lightType)
    , position(pos)
    , direction(dir)
    , color(col)
    , intensity(intens)
    , constantAttenuation(1.0f)
    , linearAttenuation(0.09f)
    , quadraticAttenuation(0.032f)
{
}

Light Light::createDirectionalLight(const Vector3& direction, const Vector3& color, float intensity) {
    Light light;
    light.type = Type::DIRECTIONAL;
    light.direction = direction.normalize();
    light.color = color;
    light.intensity = intensity;
    return light;
}

Light Light::createPointLight(const Vector3& position, const Vector3& color, float intensity) {
    Light light;
    light.type = Type::POINT;
    light.position = position;
    light.color = color;
    light.intensity = intensity;
    light.constantAttenuation = 1.0f;
    light.linearAttenuation = 0.09f;
    light.quadraticAttenuation = 0.032f;
    return light;
}

float Light::calculateAttenuation(float distance) const {
    if (type == Type::DIRECTIONAL) {
        return 1.0f; // No attenuation for directional lights
    }
    
    return 1.0f / (constantAttenuation + 
                   linearAttenuation * distance + 
                   quadraticAttenuation * distance * distance);
}
