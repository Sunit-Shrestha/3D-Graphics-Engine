#include "Triangle.hpp"

Triangle::Triangle(const Vertex& a, const Vertex& b, const Vertex& c) 
    : v0(a), v1(b), v2(c) {
    // Constructor initializes the three vertices of the triangle
}

Vector3 Triangle::normal() const {
    // Calculate the normal vector using cross product
    // Normal = (v1 - v0) × (v2 - v0)
    Vector3 edge1 = v1.position - v0.position;
    Vector3 edge2 = v2.position - v0.position;
    
    // Cross product gives us the normal (perpendicular to the triangle surface)
    Vector3 normal = edge1.cross(edge2);
    
    // Normalize the normal vector to unit length
    return normal.normalized();
}