#include "Light.hpp"
#include <algorithm>
#include <cmath>

Light::Light() = default;

Light::Light(const Vector3& pos, const Vector3& dir,
             const Color& amb, const Color& diff, const Color& spec)
    : position(pos), direction(dir), ambient(amb), diffuse(diff), specular(spec) {}

Color Light::computeColor(const Vector3& normal,
                          const Vector3& viewDir,
                          const Material& material) const
{
    // Normalize vectors
    Vector3 N = normal.normalized();
    Vector3 L = direction.normalized(); // assuming directional light
    Vector3 V = viewDir.normalized();
    Vector3 R = (N * 2.0f * N.dot(L) - L).normalized();

    // Ambient
    Color ambientC = ambient * material.kAmbient;

    // Diffuse
    float diffFactor = std::max(N.dot(L), 0.0f);
    Color diffuseC = diffuse * material.kDiffuse * diffFactor;

    // Specular
    float specFactor = std::pow(std::max(R.dot(V), 0.0f), material.shininess);
    Color specularC = specular * material.kSpecular * specFactor;

    return ambientC + diffuseC + specularC;
}
