#include "Mesh.hpp"
#include <fstream>
#include <sstream>

unsigned int Mesh::addVertex(const Vertex& vertex) {
    // Check if vertex already exists to avoid duplicates (optional optimization)
    // For now, we'll add directly for simplicity
    vertices.push_back(vertex);
    return static_cast<unsigned int>(vertices.size() - 1);
}

void Mesh::addTriangle(unsigned int i0, unsigned int i1, unsigned int i2) {
    // Add three indices that form a triangle
    indices.push_back(i0);
    indices.push_back(i1);
    indices.push_back(i2);
}

void Mesh::addTriangle(const Triangle& tri) {
    // Add vertices and get their indices
    unsigned int i0 = addVertex(tri.v0);
    unsigned int i1 = addVertex(tri.v1);
    unsigned int i2 = addVertex(tri.v2);
    
    // Add triangle using indices
    addTriangle(i0, i1, i2);
}

Triangle Mesh::getTriangle(size_t triangleIndex) const {
    // Validate triangle index
    if (triangleIndex >= getTriangleCount()) {
        // Return a default triangle if index is out of bounds
        return Triangle(Vertex(), Vertex(), Vertex());
    }
    
    // Get the three vertex indices for this triangle
    size_t baseIndex = triangleIndex * 3;
    unsigned int i0 = indices[baseIndex];
    unsigned int i1 = indices[baseIndex + 1];
    unsigned int i2 = indices[baseIndex + 2];
    
    // Reconstruct triangle from vertex buffer
    return Triangle(vertices[i0], vertices[i1], vertices[i2]);
}

Triangle Mesh::getTriangleWorldSpace(size_t triangleIndex) const {
    Triangle localTriangle = getTriangle(triangleIndex);
    Matrix4 worldMatrix = getWorldTransformMatrix();
    
    // Transform vertices to world space
    Vertex v0 = localTriangle.v0;
    Vertex v1 = localTriangle.v1;
    Vertex v2 = localTriangle.v2;
    
    v0.position = worldMatrix.multiply(v0.position);
    v1.position = worldMatrix.multiply(v1.position);
    v2.position = worldMatrix.multiply(v2.position);
    
    return Triangle(v0, v1, v2);
}

// World space transformation methods
void Mesh::translateWorld(float x, float y, float z) {
    worldPosition.x += x;
    worldPosition.y += y;
    worldPosition.z += z;
}

void Mesh::rotateWorldX(float angle) {
    worldRotation.x += angle;
}

void Mesh::rotateWorldY(float angle) {
    worldRotation.y += angle;
}

void Mesh::rotateWorldZ(float angle) {
    worldRotation.z += angle;
}

void Mesh::scaleWorld(float sx, float sy, float sz) {
    worldScale.x *= sx;
    worldScale.y *= sy;
    worldScale.z *= sz;
}

void Mesh::scaleWorld(float uniformScale) {
    scaleWorld(uniformScale, uniformScale, uniformScale);
}

Matrix4 Mesh::getWorldTransformMatrix() const {
    // Build transformation matrix: T * R * S
    // (Translation * Rotation * Scale)
    
    // Scale matrix
    Matrix4 scaleMatrix = Matrix4::scale(worldScale.x, worldScale.y, worldScale.z);
    
    // Rotation matrices (apply in order: Z, Y, X)
    Matrix4 rotationX = Matrix4::rotationX(worldRotation.x);
    Matrix4 rotationY = Matrix4::rotationY(worldRotation.y);
    Matrix4 rotationZ = Matrix4::rotationZ(worldRotation.z);
    Matrix4 rotationMatrix = rotationZ * rotationY * rotationX;
    
    // Translation matrix
    Matrix4 translationMatrix = Matrix4::translation(worldPosition.x, worldPosition.y, worldPosition.z);
    
    // Combine: T * R * S
    return translationMatrix * rotationMatrix * scaleMatrix;
}

Vector3 Mesh::transformToWorldSpace(const Vector3& localPos) const {
    Matrix4 worldMatrix = getWorldTransformMatrix();
    return worldMatrix.multiply(localPos);
}

bool Mesh::loadFromOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    clear(); // Clear existing data
    
    std::vector<Vector3> tempVertices;
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex position
            float x, y, z;
            iss >> x >> y >> z;
            tempVertices.push_back(Vector3(x, y, z));
        }
        else if (prefix == "f") {
            // Face (triangle) - assuming triangulated OBJ
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;
            
            // Parse vertex indices (OBJ indices start from 1, not 0)
            unsigned int i0 = static_cast<unsigned int>(std::stoi(vertex1.substr(0, vertex1.find('/'))) - 1);
            unsigned int i1 = static_cast<unsigned int>(std::stoi(vertex2.substr(0, vertex2.find('/'))) - 1);
            unsigned int i2 = static_cast<unsigned int>(std::stoi(vertex3.substr(0, vertex3.find('/'))) - 1);
            
            // Validate indices
            if (i0 < tempVertices.size() && i1 < tempVertices.size() && i2 < tempVertices.size()) {
                // Add vertices to our vertex buffer
                if (vertices.empty()) {
                    // First time: copy all temp vertices to our buffer
                    for (const Vector3& pos : tempVertices) {
                        vertices.push_back(Vertex(pos));
                    }
                }
                
                // Add triangle indices
                addTriangle(i0, i1, i2);
            }
        }
    }
    
    file.close();
    return !vertices.empty() && !indices.empty();
}

void Mesh::clear() {
    vertices.clear();
    indices.clear();
    // Reset world transformation to defaults
    worldPosition = Vector3(0, 0, 0);
    worldRotation = Vector3(0, 0, 0);
    worldScale = Vector3(1, 1, 1);
}

void Mesh::reserve(size_t vertexCount, size_t triangleCount) {
    vertices.reserve(vertexCount);
    indices.reserve(triangleCount * 3); // 3 indices per triangle
}