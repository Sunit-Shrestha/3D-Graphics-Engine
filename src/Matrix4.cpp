#include "Matrix4.hpp"
#include <cmath>

Matrix4::Matrix4() {
    // Initialize to identity matrix
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

Matrix4 Matrix4::identity() {
    return Matrix4();
}

Matrix4 Matrix4::rotationX(float angle) {
    Matrix4 result;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    result.m[0][0] = 1.0f;  result.m[0][1] = 0.0f;   result.m[0][2] = 0.0f;    result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f;  result.m[1][1] = cosA;   result.m[1][2] = -sinA;   result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f;  result.m[2][1] = sinA;   result.m[2][2] = cosA;    result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;  result.m[3][1] = 0.0f;   result.m[3][2] = 0.0f;    result.m[3][3] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::rotationY(float angle) {
    Matrix4 result;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    result.m[0][0] = cosA;   result.m[0][1] = 0.0f;  result.m[0][2] = sinA;    result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f;   result.m[1][1] = 1.0f;  result.m[1][2] = 0.0f;    result.m[1][3] = 0.0f;
    result.m[2][0] = -sinA;  result.m[2][1] = 0.0f;  result.m[2][2] = cosA;    result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;   result.m[3][1] = 0.0f;  result.m[3][2] = 0.0f;    result.m[3][3] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::rotationZ(float angle) {
    Matrix4 result;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    result.m[0][0] = cosA;   result.m[0][1] = -sinA;  result.m[0][2] = 0.0f;   result.m[0][3] = 0.0f;
    result.m[1][0] = sinA;   result.m[1][1] = cosA;   result.m[1][2] = 0.0f;   result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f;   result.m[2][1] = 0.0f;   result.m[2][2] = 1.0f;   result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;   result.m[3][1] = 0.0f;   result.m[3][2] = 0.0f;   result.m[3][3] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::translation(float x, float y, float z) {
    Matrix4 result;
    
    result.m[0][0] = 1.0f;  result.m[0][1] = 0.0f;  result.m[0][2] = 0.0f;  result.m[0][3] = x;
    result.m[1][0] = 0.0f;  result.m[1][1] = 1.0f;  result.m[1][2] = 0.0f;  result.m[1][3] = y;
    result.m[2][0] = 0.0f;  result.m[2][1] = 0.0f;  result.m[2][2] = 1.0f;  result.m[2][3] = z;
    result.m[3][0] = 0.0f;  result.m[3][1] = 0.0f;  result.m[3][2] = 0.0f;  result.m[3][3] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::scale(float sx, float sy, float sz) {
    Matrix4 result;
    
    result.m[0][0] = sx;    result.m[0][1] = 0.0f;  result.m[0][2] = 0.0f;  result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f;  result.m[1][1] = sy;    result.m[1][2] = 0.0f;  result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f;  result.m[2][1] = 0.0f;  result.m[2][2] = sz;    result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;  result.m[3][1] = 0.0f;  result.m[3][2] = 0.0f;  result.m[3][3] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::projection(float fov, float aspect, float near, float far) {
    Matrix4 result;
    
    // Clear the matrix first
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0.0f;
        }
    }
    
    // Perspective projection matrix
    float f = 1.0f / std::tan(fov * 0.5f);
    float range = near - far;
    
    result.m[0][0] = f / aspect;
    result.m[1][1] = f;
    result.m[2][2] = (far + near) / range;
    result.m[2][3] = (2.0f * far * near) / range;
    result.m[3][2] = -1.0f;
    
    return result;
}

Vector3 Matrix4::multiply(const Vector3& v) const {
    // Treat Vector3 as homogeneous coordinate (x, y, z, 1)
    float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * 1.0f;
    float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * 1.0f;
    float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * 1.0f;
    float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * 1.0f;
    
    // Perspective divide (if w != 1)
    if (w != 0.0f && w != 1.0f) {
        x /= w;
        y /= w;
        z /= w;
    }
    
    return Vector3(x, y, z);
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    
    // Initialize result matrix to zero
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0.0f;
        }
    }
    
    // Matrix multiplication: result[i][j] = sum(this[i][k] * other[k][j])
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    
    return result;
}