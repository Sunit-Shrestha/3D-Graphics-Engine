#pragma once
#include <vector>
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Vector3.hpp"
#include "Color.hpp"
#include "Light.hpp"
#include "Material.hpp"

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
    void render_Mesh(const std::vector<Mesh>& meshes, const Camera& camera);
    void render_Light(const std::vector<Mesh>& meshes, const Camera& camera, 
                      const std::vector<Light>& lights, const Material& material);
    void present(sf::RenderWindow& window);

private:
    // Core pipeline stages
    Vector3 viewportTransform(const Vector3& clipSpaceVertex);

    // Rasterization helpers
    void drawLine_Bresenham(int x0, int y0, int x1, int y1, const Color& color);
    void drawLine_Bresenham_Depth(int x0, int y0, int x1, int y1, float z0, float z1, const Color& color);
    void fillTriangle_Scanline(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);
    void fillTriangle_Gouraud(const Vector3& v0, const Vector3& v1, const Vector3& v2, 
                              const Color& c0, const Color& c1, const Color& c2);
    
    // Lighting helpers
    Vector3 calculateFaceNormal(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    Color computeVertexLighting(const Vector3& worldPos, const Vector3& normal, 
                                const Vector3& viewPos, const std::vector<Light>& lights, 
                                const Material& material);
    
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
