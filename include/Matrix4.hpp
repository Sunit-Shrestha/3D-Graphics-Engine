#pragma once
#include "Vector3.hpp"

class Matrix4 {
public:
    float m[4][4];

    Matrix4(); // default constructor

    static Matrix4 identity();
    static Matrix4 rotationX(float angle);
    static Matrix4 rotationY(float angle);
    static Matrix4 rotationZ(float angle);
    static Matrix4 translation(float x, float y, float z);
    static Matrix4 scale(float sx, float sy, float sz);
    static Matrix4 projection(float fov, float aspect, float near, float far);

    Vector3 multiply(const Vector3& v) const;
    Matrix4 operator*(const Matrix4& other) const;
};
