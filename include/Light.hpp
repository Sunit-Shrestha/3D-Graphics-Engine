#pragma once
#include "Vector3.hpp"

class Light {
public:
    enum class Type {
        DIRECTIONAL,
        POINT
    };

    // Light properties
    Type type;
    Vector3 position;     // For point lights
    Vector3 direction;    // For directional lights (normalized)
    Vector3 color;        // RGB values (0.0 - 1.0)
    float intensity;
    
    // Attenuation properties (for point lights)
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;

    // Constructors
    Light();
    Light(Type lightType, const Vector3& pos, const Vector3& dir, const Vector3& col, float intens);

    // Factory methods for easy creation
    static Light createDirectionalLight(const Vector3& direction, const Vector3& color, float intensity = 1.0f);
    static Light createPointLight(const Vector3& position, const Vector3& color, float intensity = 1.0f);

    // Calculate attenuation for point lights
    float calculateAttenuation(float distance) const;
};
