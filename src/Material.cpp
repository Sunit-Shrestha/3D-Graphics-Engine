#include "Material.hpp"

// Default constructor
Material::Material() 
    : kAmbient(0.1f),      // ~0.1 * 255
      kDiffuse(0.7f),   // ~0.7 * 255
      kSpecular(0.3f),  // full specular
      shininess(16.0f)
{
}

// Parameterized constructor
Material::Material(float a, float d, float s, float sh)
    : kAmbient(a), kDiffuse(d), kSpecular(s), shininess(sh)
{
}
