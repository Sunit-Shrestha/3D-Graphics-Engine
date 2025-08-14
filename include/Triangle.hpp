#pragma once
#include "Vertex.hpp"

class Triangle {
public:
    Vertex v0, v1, v2;

    Triangle(const Vertex& a, const Vertex& b, const Vertex& c);

    Vector3 normal() const;  // Compute normal for back-face culling
};
