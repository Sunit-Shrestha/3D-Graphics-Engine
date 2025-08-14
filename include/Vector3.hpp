#pragma once

class Vector3 {
public:
    float x, y, z;

    Vector3(float x = 0, float y = 0, float z = 0);

    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;

    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
    float length() const;
    Vector3 normalize() const;
};
