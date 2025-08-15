#pragma once
#include "Vector3.hpp"
#include "Color.hpp"
#include "Material.hpp"

class Light {
public:
    Vector3 position;    // For point light
    Vector3 direction;   // For directional light
    Color ambient;       // Ambient light color/intensity
    Color diffuse;       // Diffuse light color/intensity
    Color specular;      // Specular light color/intensity

    Light();
    Light(const Vector3& pos, const Vector3& dir,
          const Color& amb, const Color& diff, const Color& spec);

    // Compute vertex color for Gouraud shading
    Color computeColor(const Vector3& normal,
                       const Vector3& viewDir,
                       const Material& material) const;
};
