#pragma once
#include <vector>
#include "Triangle.hpp"
#include "Vertex.hpp"
#include "Matrix4.hpp"
#include "Vector3.hpp"
#include <string>

class Mesh {
public:
    // Vertex buffer: stores unique vertices in LOCAL/OBJECT space
    std::vector<Vertex> vertices;
    
    // Index buffer: stores indices that form triangles (3 indices per triangle)
    std::vector<unsigned int> indices;

private:
    // World space transformation properties
    Vector3 worldPosition;    // Position in world space
    Vector3 worldRotation;    // Rotation angles (Euler angles: X, Y, Z)
    Vector3 worldScale;       // Scale factors (X, Y, Z)

public:
    Mesh() : worldPosition(0, 0, 0), worldRotation(0, 0, 0), worldScale(1, 1, 1) {}

    // Add vertex to buffer and return its index
    unsigned int addVertex(const Vertex& vertex);
    
    // Add triangle using vertex indices (more efficient than storing duplicate vertices)
    void addTriangle(unsigned int i0, unsigned int i1, unsigned int i2);
    
    // Add triangle by vertices (automatically handles indexing)
    void addTriangle(const Triangle& tri);
    
    // Get triangle by index (reconstructed from vertex and index buffers)
    Triangle getTriangle(size_t triangleIndex) const;
    
    // Get triangle in WORLD space (applies world transformation)
    Triangle getTriangleWorldSpace(size_t triangleIndex) const;
    
    // Get total number of triangles
    size_t getTriangleCount() const { return indices.size() / 3; }
    
    // World space transformation properties (getters/setters)
    void setWorldPosition(const Vector3& position) { worldPosition = position; }
    void setWorldPosition(float x, float y, float z) { worldPosition = Vector3(x, y, z); }
    const Vector3& getWorldPosition() const { return worldPosition; }
    
    void setWorldRotation(const Vector3& rotation) { worldRotation = rotation; }
    void setWorldRotation(float x, float y, float z) { worldRotation = Vector3(x, y, z); }
    const Vector3& getWorldRotation() const { return worldRotation; }
    
    void setWorldScale(const Vector3& scale) { worldScale = scale; }
    void setWorldScale(float x, float y, float z) { worldScale = Vector3(x, y, z); }
    void setWorldScale(float uniformScale) { worldScale = Vector3(uniformScale, uniformScale, uniformScale); }
    const Vector3& getWorldScale() const { return worldScale; }

    // World space transformation methods (modify world properties, not geometry)
    void translateWorld(float x, float y, float z);
    void rotateWorldX(float angle);
    void rotateWorldY(float angle);
    void rotateWorldZ(float angle);
    void scaleWorld(float sx, float sy, float sz);
    void scaleWorld(float uniformScale);

    // Get the world transformation matrix
    Matrix4 getWorldTransformMatrix() const;
    
    // Transform a vertex from object space to world space
    Vector3 transformToWorldSpace(const Vector3& localPos) const;

    // File I/O
    bool loadFromOBJ(const std::string& filename);
    
    // Utility methods
    void clear();
    void reserve(size_t vertexCount, size_t triangleCount);
    
    // Statistics
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }
};
