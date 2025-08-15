#pragma once

struct Material {
    float kAmbient;    // Ambient reflection coefficient (Ka)
    float kDiffuse;    // Diffuse reflection coefficient (Kd)
    float kSpecular;   // Specular reflection coefficient (Ks)
    float shininess;  // Specular exponent (alpha)

    Material();
    Material(float a, float d, float s, float sh);
};
