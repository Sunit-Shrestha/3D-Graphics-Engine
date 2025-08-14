#include "Camera.hpp"

Camera::Camera() 
    : position(0.0f, 0.0f, 5.0f),   // Default camera position (behind the origin)
      target(0.0f, 0.0f, 0.0f),     // Looking at the origin
      up(0.0f, 1.0f, 0.0f),         // Y-axis up vector
      fieldOfView(3.14159f / 4.0f), // 45 degrees in radians
      aspectRatio(16.0f / 9.0f),    // Common widescreen aspect ratio
      nearPlane(0.1f),              // Near clipping plane
      farPlane(100.0f)              // Far clipping plane
{
}

Camera::Camera(float fov, float aspect, float near, float far)
    : position(0.0f, 0.0f, 5.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fieldOfView(fov),
      aspectRatio(aspect),
      nearPlane(near),
      farPlane(far)
{
}

Matrix4 Camera::getViewMatrix() const {
    // Calculate camera basis vectors using the "look-at" method
    
    // Forward vector: direction from camera to target (normalized)
    Vector3 forward = (target - position).normalize();
    
    // Right vector: cross product of forward and up (normalized)
    Vector3 right = forward.cross(up).normalize();
    
    // Recalculate up vector to ensure orthogonality
    Vector3 realUp = right.cross(forward).normalize();
    
    // Negate forward for right-handed coordinate system
    // (camera looks down the negative Z-axis)
    forward = forward * -1.0f;
    
    // Create the view matrix
    // This is the inverse of the camera's transformation matrix
    Matrix4 viewMatrix;
    
    // Set rotation part (transpose of camera orientation matrix)
    viewMatrix.m[0][0] = right.x;    viewMatrix.m[0][1] = right.y;    viewMatrix.m[0][2] = right.z;
    viewMatrix.m[1][0] = realUp.x;   viewMatrix.m[1][1] = realUp.y;   viewMatrix.m[1][2] = realUp.z;
    viewMatrix.m[2][0] = forward.x;  viewMatrix.m[2][1] = forward.y;  viewMatrix.m[2][2] = forward.z;
    viewMatrix.m[3][0] = 0.0f;       viewMatrix.m[3][1] = 0.0f;       viewMatrix.m[3][2] = 0.0f;
    
    // Set translation part (negative dot product with camera position)
    viewMatrix.m[0][3] = -right.dot(position);
    viewMatrix.m[1][3] = -realUp.dot(position);
    viewMatrix.m[2][3] = -forward.dot(position);
    viewMatrix.m[3][3] = 1.0f;
    
    return viewMatrix;
}

Matrix4 Camera::getProjectionMatrix() const {
    // Create perspective projection matrix
    return Matrix4::projection(fieldOfView, aspectRatio, nearPlane, farPlane);
}

Matrix4 Camera::getViewProjectionMatrix() const {
    // Combine projection and view matrices
    // Order: Projection * View (right-to-left multiplication)
    return getProjectionMatrix() * getViewMatrix();
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    fieldOfView = fov;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
}

void Camera::setAspectRatio(float aspect) {
    aspectRatio = aspect;
}

void Camera::setFieldOfView(float fov) {
    fieldOfView = fov;
}

void Camera::setClippingPlanes(float near, float far) {
    nearPlane = near;
    farPlane = far;
}