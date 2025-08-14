#include "Renderer.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

Renderer::Renderer(int width, int height) 
    : screenWidth(width), screenHeight(height)
{
    // Initialize frame buffer
    frameBuffer.resize(screenWidth * screenHeight, Color(0, 0, 0));
    
    // Initialize depth buffer
    initZBuffer();
    
    // Initialize minimal SFML objects for display
    displayImage = new sf::Image(sf::Vector2u(screenWidth, screenHeight), sf::Color::Black);
    
    displayTexture = new sf::Texture();
    if (!displayTexture->resize(sf::Vector2u(screenWidth, screenHeight))) {
        printf("ERROR: Failed to resize texture to %dx%d\n", screenWidth, screenHeight);
    }
    displaySprite = new sf::Sprite(*displayTexture);
    
    printf("Renderer initialized: %dx%d\n", screenWidth, screenHeight);
}

Renderer::~Renderer() {
    delete displayImage;
    delete displayTexture;
    delete displaySprite;
}

void Renderer::clear(const Color& clearColor) {
    // Clear frame buffer
    std::fill(frameBuffer.begin(), frameBuffer.end(), clearColor);
    
    // Clear depth buffer
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::max());
}

void Renderer::render(const std::vector<Mesh>& meshes, const Camera& camera) {
    // Get combined view-projection matrix
    Matrix4 viewProjMatrix = camera.getViewProjectionMatrix();
    
    // Simple color palette for different meshes
    Color meshColors[] = {
        Color(255, 100, 100),  // Red
        Color(100, 255, 100),  // Green
        Color(100, 100, 255),  // Blue
        Color(255, 255, 100),  // Yellow
        Color(255, 100, 255),  // Magenta
        Color(100, 255, 255)   // Cyan
    };
    
    // Render each mesh
    for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex) {
        const Mesh& mesh = meshes[meshIndex];
        Color meshColor = meshColors[meshIndex % 6]; // Cycle through colors
        
        // Get model-view-projection matrix
        Matrix4 modelMatrix = mesh.getWorldTransformMatrix();
        Matrix4 mvpMatrix = viewProjMatrix * modelMatrix;
        
        // Render each triangle in the mesh
        for (size_t i = 0; i < mesh.getTriangleCount(); ++i) {
            Triangle triangle = mesh.getTriangle(i);
            
            // Transform vertices to clip space
            Vector3 v0_clip = mvpMatrix.multiply(triangle.v0.position);
            Vector3 v1_clip = mvpMatrix.multiply(triangle.v1.position);
            Vector3 v2_clip = mvpMatrix.multiply(triangle.v2.position);
            
            // Perspective division (clip space to NDC)
            // Assuming w = z for perspective projection
            if (v0_clip.z <= 0.0f || v1_clip.z <= 0.0f || v2_clip.z <= 0.0f) continue; // Behind camera
            
            Vector3 v0_ndc = Vector3(v0_clip.x / v0_clip.z, v0_clip.y / v0_clip.z, v0_clip.z);
            Vector3 v1_ndc = Vector3(v1_clip.x / v1_clip.z, v1_clip.y / v1_clip.z, v1_clip.z);
            Vector3 v2_ndc = Vector3(v2_clip.x / v2_clip.z, v2_clip.y / v2_clip.z, v2_clip.z);
            
            // Simple clipping: skip triangles that are completely outside the view volume
            if (v0_ndc.x < -1.0f && v1_ndc.x < -1.0f && v2_ndc.x < -1.0f) continue; // Left of screen
            if (v0_ndc.x > 1.0f && v1_ndc.x > 1.0f && v2_ndc.x > 1.0f) continue;   // Right of screen
            if (v0_ndc.y < -1.0f && v1_ndc.y < -1.0f && v2_ndc.y < -1.0f) continue; // Below screen
            if (v0_ndc.y > 1.0f && v1_ndc.y > 1.0f && v2_ndc.y > 1.0f) continue;   // Above screen
            
            // Transform to screen coordinates
            Vector3 v0_screen = viewportTransform(v0_ndc);
            Vector3 v1_screen = viewportTransform(v1_ndc);
            Vector3 v2_screen = viewportTransform(v2_ndc);
            
            // Check if triangle is visible on screen
            if (!isTriangleVisible(v0_screen, v1_screen, v2_screen)) {
                continue;
            }
            
            // Back-face culling
            if (isBackFace(v0_screen, v1_screen, v2_screen)) {
                continue;
            }
            
            // Rasterize triangle
            fillTriangle_Scanline(v0_screen, v1_screen, v2_screen, meshColor);
        }
    }
    
    // Simple completion message for first render only
    static bool firstRender = true;
    if (firstRender) {
        printf("Renderer: Successfully processed %zu meshes\n", meshes.size());
        firstRender = false;
    }
}

void Renderer::present(sf::RenderWindow& window) {
    // Copy frame buffer to SFML image
    for (int y = 0; y < screenHeight; ++y) {
        for (int x = 0; x < screenWidth; ++x) {
            const Color& pixel = frameBuffer[y * screenWidth + x];
            displayImage->setPixel(sf::Vector2u(x, y), sf::Color(pixel.r, pixel.g, pixel.b));
        }
    }
    
    // Create texture from image each frame (less efficient but more reliable for debugging)
    sf::Texture frameTexture;
    if (frameTexture.loadFromImage(*displayImage)) {
        sf::Sprite frameSprite(frameTexture);
        window.draw(frameSprite);
    } else {
        static bool errorShown = false;
        if (!errorShown) {
            printf("ERROR: Failed to load texture from image!\n");
            errorShown = true;
        }
    }
}

// Core pipeline implementation
Vector3 Renderer::viewportTransform(const Vector3& clipSpaceVertex) {
    // Transform from NDC (-1 to 1) to screen coordinates (0 to width/height)
    float x = (clipSpaceVertex.x + 1.0f) * 0.5f * screenWidth;
    float y = (1.0f - clipSpaceVertex.y) * 0.5f * screenHeight; // Flip Y for screen coordinates
    float z = clipSpaceVertex.z; // Keep depth for z-buffer
    
    return Vector3(x, y, z);
}

// Pixel operations
void Renderer::setPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < screenWidth && y >= 0 && y < screenHeight) {
        frameBuffer[y * screenWidth + x] = color;
    }
}

// Bresenham line drawing algorithm
void Renderer::drawLine_Bresenham(int x0, int y0, int x1, int y1, const Color& color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (true) {
        setPixel(x, y, color);
        
        if (x == x1 && y == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Scanline triangle filling algorithm
void Renderer::fillTriangle_Scanline(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color) {
    // Sort vertices by Y coordinate
    Vector3 vertices[3] = {v0, v1, v2};
    std::sort(vertices, vertices + 3, [](const Vector3& a, const Vector3& b) {
        return a.y < b.y;
    });
    
    Vector3 top = vertices[0];
    Vector3 mid = vertices[1];
    Vector3 bottom = vertices[2];
    
    // Handle degenerate triangles
    if (std::abs(top.y - bottom.y) < 0.5f) return;
    
    // Scanline fill
    for (int y = static_cast<int>(std::ceil(top.y)); y <= static_cast<int>(bottom.y); ++y) {
        if (y < 0 || y >= screenHeight) continue;
        
        float t1, t2;
        Vector3 p1, p2;
        
        if (y <= mid.y) {
            // Upper part of triangle
            if (std::abs(top.y - mid.y) < 0.01f) continue;
            t1 = (y - top.y) / (mid.y - top.y);
            t2 = (y - top.y) / (bottom.y - top.y);
            
            p1 = Vector3(
                top.x + t1 * (mid.x - top.x),
                static_cast<float>(y),
                top.z + t1 * (mid.z - top.z)
            );
            p2 = Vector3(
                top.x + t2 * (bottom.x - top.x),
                static_cast<float>(y),
                top.z + t2 * (bottom.z - top.z)
            );
        } else {
            // Lower part of triangle
            if (std::abs(mid.y - bottom.y) < 0.01f) continue;
            t1 = (y - mid.y) / (bottom.y - mid.y);
            t2 = (y - top.y) / (bottom.y - top.y);
            
            p1 = Vector3(
                mid.x + t1 * (bottom.x - mid.x),
                static_cast<float>(y),
                mid.z + t1 * (bottom.z - mid.z)
            );
            p2 = Vector3(
                top.x + t2 * (bottom.x - top.x),
                static_cast<float>(y),
                top.z + t2 * (bottom.z - top.z)
            );
        }
        
        // Ensure p1 is left of p2
        if (p1.x > p2.x) std::swap(p1, p2);
        
        // Draw horizontal scanline
        int startX = static_cast<int>(std::ceil(p1.x));
        int endX = static_cast<int>(p2.x);
        
        for (int x = startX; x <= endX; ++x) {
            if (x >= 0 && x < screenWidth) {
                // Interpolate depth
                float t = (p2.x == p1.x) ? 0.0f : (x - p1.x) / (p2.x - p1.x);
                float depth = p1.z + t * (p2.z - p1.z);
                
                // Depth test
                if (depthTest(x, y, depth)) {
                    setPixel(x, y, color);
                }
            }
        }
    }
}

// Culling and visibility tests
bool Renderer::isBackFace(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
    // Calculate triangle normal in screen space using cross product
    Vector3 edge1 = Vector3(v1.x - v0.x, v1.y - v0.y, 0);
    Vector3 edge2 = Vector3(v2.x - v0.x, v2.y - v0.y, 0);
    
    // Cross product Z component determines winding order
    float normalZ = edge1.x * edge2.y - edge1.y * edge2.x;
    
    // Counter-clockwise winding is front-facing (positive Z)
    return normalZ <= 0;
}

bool Renderer::isTriangleVisible(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
    // Simple bounding box check
    float minX = std::min({v0.x, v1.x, v2.x});
    float maxX = std::max({v0.x, v1.x, v2.x});
    float minY = std::min({v0.y, v1.y, v2.y});
    float maxY = std::max({v0.y, v1.y, v2.y});
    
    // Check if triangle is completely outside screen bounds
    return !(maxX < 0 || minX >= screenWidth || maxY < 0 || minY >= screenHeight);
}

// Depth buffer operations
void Renderer::initZBuffer() {
    zBuffer.resize(screenWidth * screenHeight);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::max());
}

bool Renderer::depthTest(int x, int y, float depth) {
    if (x < 0 || x >= screenWidth || y < 0 || y >= screenHeight) {
        return false;
    }
    
    int index = y * screenWidth + x;
    if (depth < zBuffer[index]) {
        zBuffer[index] = depth;
        return true;
    }
    return false;
}
