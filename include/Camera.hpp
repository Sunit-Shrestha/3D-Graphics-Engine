#pragma once
#include "Vector3.hpp"
#include "Matrix4.hpp"

class Camera {
public:
    // View transformation properties
    Vector3 position;
    Vector3 target;
    Vector3 up;
    
    // Projection properties
    float fieldOfView;      // Field of view in radians
    float aspectRatio;      // Width / Height ratio
    float nearPlane;        // Near clipping plane distance
    float farPlane;         // Far clipping plane distance

    Camera();
    Camera(float fov, float aspect, float near, float far);

    // View transformation
    Matrix4 getViewMatrix() const;
    
    // Projection transformation
    Matrix4 getProjectionMatrix() const;
    
    // Combined view-projection matrix
    Matrix4 getViewProjectionMatrix() const;
    
    // Utility methods for projection settings
    void setPerspective(float fov, float aspect, float near, float far);
    void setAspectRatio(float aspect);
    void setFieldOfView(float fov);
    void setClippingPlanes(float near, float far);
};
