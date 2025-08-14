#pragma once
#include <vector>
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Vector3.hpp"

// Simple RGB color structure (no SFML dependency)
struct Color {
    unsigned char r, g, b;
    Color(unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255) 
        : r(red), g(green), b(blue) {}
};

// Forward declaration for minimal SFML usage
namespace sf {
    class RenderWindow;
    class Image;
    class Texture;
    class Sprite;
}

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    void clear(const Color& clearColor = Color(0, 0, 0));
    void render(const std::vector<Mesh>& meshes, const Camera& camera);
    void present(sf::RenderWindow& window);

private:
    // Core pipeline stages
    Vector3 viewportTransform(const Vector3& clipSpaceVertex);

    // Rasterization helpers
    void drawLine_Bresenham(int x0, int y0, int x1, int y1, const Color& color);
    void fillTriangle_Scanline(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);
    
    // Pixel operations
    void setPixel(int x, int y, const Color& color);
    
    // Culling and clipping
    bool isBackFace(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    bool isTriangleVisible(const Vector3& v0, const Vector3& v1, const Vector3& v2);

    // Depth buffering
    void initZBuffer();
    bool depthTest(int x, int y, float depth);

private:
    int screenWidth;
    int screenHeight;
    
    // Frame buffer (raw pixel data)
    std::vector<Color> frameBuffer;
    
    // Depth buffer
    std::vector<float> zBuffer;
    
    // Minimal SFML objects for display only
    sf::Image* displayImage;
    sf::Texture* displayTexture;
    sf::Sprite* displaySprite;
};
