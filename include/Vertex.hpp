#pragma once
#include "Vector3.hpp"

class Vertex {
public:
    Vector3 position;
    // Optional: color, normal, uv
    Vertex(const Vector3& pos = Vector3());
};
