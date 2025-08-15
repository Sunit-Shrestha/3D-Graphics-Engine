#include "Renderer.hpp"
// #include "Light.hpp"
#include "Material.hpp"
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
    
    printf("Renderer initialized: %dx%d\n", screenWidth, screenHeight);
}

Renderer::~Renderer() {
    delete displayImage;
}

void Renderer::clear(const Color& clearColor) {
    // Clear frame buffer
    std::fill(frameBuffer.begin(), frameBuffer.end(), clearColor);
    
    // Clear depth buffer
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::max());
}

void Renderer::render_Mesh(const std::vector<Mesh>& meshes, const Camera& camera) {
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
        
        // Get mesh transformation matrix
        Matrix4 worldMatrix = mesh.getWorldTransformMatrix();
        Matrix4 mvpMatrix = viewProjMatrix * worldMatrix;
        
        // Store visible triangles for edge rendering
        std::vector<std::tuple<Vector3, Vector3, Vector3>> visibleTriangles;
        
        // First pass: Render triangle fills
        for (size_t i = 0; i < mesh.getTriangleCount(); ++i) {
            Triangle triangle = mesh.getTriangle(i);
            
            // Transform vertices to clip space
            Vector3 v0_clip = mvpMatrix.multiply(triangle.v0.position);
            Vector3 v1_clip = mvpMatrix.multiply(triangle.v1.position);
            Vector3 v2_clip = mvpMatrix.multiply(triangle.v2.position);
            
            // Perspective division (clip space to NDC)
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
            if (!isBackFace(v0_screen, v1_screen, v2_screen)) {
                continue;
            }
            
            // Rasterize triangle fill
            fillTriangle_Scanline(v0_screen, v1_screen, v2_screen, meshColor);
            
            // Store triangle for edge rendering (with slightly closer z for priority)
            Vector3 v0_edge = Vector3(v0_screen.x, v0_screen.y, v0_screen.z - 0.001f);
            Vector3 v1_edge = Vector3(v1_screen.x, v1_screen.y, v1_screen.z - 0.001f);
            Vector3 v2_edge = Vector3(v2_screen.x, v2_screen.y, v2_screen.z - 0.001f);
            visibleTriangles.push_back(std::make_tuple(v0_edge, v1_edge, v2_edge));
        }
        
        // Second pass: Render triangle edges on top
        Color edgeColor = Color(255, 255, 255); // White edges
        for (const auto& triangle : visibleTriangles) {
            Vector3 v0 = std::get<0>(triangle);
            Vector3 v1 = std::get<1>(triangle);
            Vector3 v2 = std::get<2>(triangle);
            
            // Draw the three edges of the triangle
            drawLine_Bresenham_Depth(static_cast<int>(v0.x), static_cast<int>(v0.y), 
                                    static_cast<int>(v1.x), static_cast<int>(v1.y), 
                                    v0.z, v1.z, edgeColor);
                              
            drawLine_Bresenham_Depth(static_cast<int>(v1.x), static_cast<int>(v1.y), 
                                    static_cast<int>(v2.x), static_cast<int>(v2.y), 
                                    v1.z, v2.z, edgeColor);
                              
            drawLine_Bresenham_Depth(static_cast<int>(v2.x), static_cast<int>(v2.y), 
                                    static_cast<int>(v0.x), static_cast<int>(v0.y), 
                                    v2.z, v0.z, edgeColor);
        }
    }
    
    // Simple completion message for first render only
    static bool firstRender = true;
    if (firstRender) {
        printf("Renderer: Successfully processed %zu meshes\n", meshes.size());
        firstRender = false;
    }
}

void Renderer::render_Light(const std::vector<Mesh>& meshes, const Camera& camera, 
                           const std::vector<Light>& lights, const Material& material) {
    // Get combined view-projection matrix
    Matrix4 viewProjMatrix = camera.getViewProjectionMatrix();
    
    // Render each mesh with Gouraud shading
    for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex) {
        const Mesh& mesh = meshes[meshIndex];
        
        // Get mesh transformation matrix
        Matrix4 worldMatrix = mesh.getWorldTransformMatrix();
        Matrix4 mvpMatrix = viewProjMatrix * worldMatrix;
        
        // Render triangles with Gouraud lighting (no edges)
        for (size_t i = 0; i < mesh.getTriangleCount(); ++i) {
            Triangle triangle = mesh.getTriangle(i);
            
            // Transform vertices to clip space
            Vector3 v0_clip = mvpMatrix.multiply(triangle.v0.position);
            Vector3 v1_clip = mvpMatrix.multiply(triangle.v1.position);
            Vector3 v2_clip = mvpMatrix.multiply(triangle.v2.position);
            
            // Perspective division (clip space to NDC)
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
            if (!isBackFace(v0_screen, v1_screen, v2_screen)) {
                continue;
            }
            
            // Calculate world space positions for lighting
            Vector3 v0_world = worldMatrix.multiply(triangle.v0.position);
            Vector3 v1_world = worldMatrix.multiply(triangle.v1.position);
            Vector3 v2_world = worldMatrix.multiply(triangle.v2.position);
            
            // Calculate face normal for this triangle
            Vector3 faceNormal = calculateFaceNormal(v0_world, v1_world, v2_world);
            
            // Get camera position for view direction calculation
            Vector3 viewPos = camera.position;
            
            // Calculate Gouraud lighting at each vertex using Light's computeColor method
            Color c0 = computeVertexLighting(v0_world, faceNormal, viewPos, lights, material);
            Color c1 = computeVertexLighting(v1_world, faceNormal, viewPos, lights, material);
            Color c2 = computeVertexLighting(v2_world, faceNormal, viewPos, lights, material);
            
            // Rasterize triangle with interpolated colors (Gouraud shading)
            fillTriangle_Gouraud(v0_screen, v1_screen, v2_screen, c0, c1, c2);
        }
    }
    
    // Simple completion message for first render only
    static bool firstLightRender = true;
    if (firstLightRender) {
        printf("Renderer: Successfully processed %zu meshes with lighting\n", meshes.size());
        firstLightRender = false;
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

// Depth-aware Bresenham line drawing algorithm with clipping
void Renderer::drawLine_Bresenham_Depth(int x0, int y0, int x1, int y1, float z0, float z1, const Color& color) {
    // Simple line clipping to screen bounds
    if ((x0 < 0 && x1 < 0) || (x0 >= screenWidth && x1 >= screenWidth) ||
        (y0 < 0 && y1 < 0) || (y0 >= screenHeight && y1 >= screenHeight)) {
        return; // Line completely outside screen
    }
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    // Calculate total line length for depth interpolation
    float lineLength = std::sqrt(static_cast<float>(dx * dx + dy * dy));
    
    while (true) {
        // Only draw if pixel is within screen bounds
        if (x >= 0 && x < screenWidth && y >= 0 && y < screenHeight) {
            // Interpolate depth along the line
            float t = 0.0f;
            if (lineLength > 0.0f) {
                float currentLength = std::sqrt(static_cast<float>((x - x0) * (x - x0) + (y - y0) * (y - y0)));
                t = currentLength / lineLength;
            }
            float z = z0 + t * (z1 - z0);
            
            // Use depth test before setting pixel
            if (depthTest(x, y, z)) {
                setPixel(x, y, color);
            }
        }
        
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
        
        for (int x = std::max(0, startX); x <= std::min(screenWidth - 1, endX); ++x) {
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

// Culling and visibility tests
bool Renderer::isBackFace(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
    // Calculate triangle normal in screen space using cross product
    Vector3 edge1 = Vector3(v1.x - v0.x, v1.y - v0.y, 0);
    Vector3 edge2 = Vector3(v2.x - v0.x, v2.y - v0.y, 0);
    
    // Cross product Z component determines winding order
    float normalZ = edge1.cross(edge2).z;
    
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

// Calculate face normal from three vertices
Vector3 Renderer::calculateFaceNormal(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;
    return edge1.cross(edge2).normalized();
}

// Compute vertex lighting using Light's computeColor method
Color Renderer::computeVertexLighting(const Vector3& worldPos, const Vector3& normal, 
                                     const Vector3& viewPos, const std::vector<Light>& lights, 
                                     const Material& material) {
    Color finalColor(0, 0, 0);
    Vector3 viewDir = (viewPos - worldPos).normalized();
    
    // Accumulate lighting from all lights using each Light's computeColor method
    for (const Light& light : lights) {
        Color lightContribution = light.computeColor(normal, viewDir, material);
        finalColor = finalColor + lightContribution;
    }
    
    return finalColor;
}

// Gouraud shaded triangle rasterization with color interpolation
void Renderer::fillTriangle_Gouraud(const Vector3& v0, const Vector3& v1, const Vector3& v2, 
                                   const Color& c0, const Color& c1, const Color& c2) {
    // Convert to integer coordinates
    int x0 = static_cast<int>(v0.x), y0 = static_cast<int>(v0.y);
    int x1 = static_cast<int>(v1.x), y1 = static_cast<int>(v1.y);
    int x2 = static_cast<int>(v2.x), y2 = static_cast<int>(v2.y);
    
    // Find bounding box
    int minX = std::max(0, std::min({x0, x1, x2}));
    int maxX = std::min(screenWidth - 1, std::max({x0, x1, x2}));
    int minY = std::max(0, std::min({y0, y1, y2}));
    int maxY = std::min(screenHeight - 1, std::max({y0, y1, y2}));
    
    // Precompute triangle area for barycentric coordinates
    float area = static_cast<float>((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0));
    if (std::abs(area) < 0.001f) return; // Degenerate triangle
    
    // Scanline fill with barycentric interpolation
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Calculate barycentric coordinates
            float w0 = static_cast<float>((x1 - x) * (y2 - y) - (x2 - x) * (y1 - y)) / area;
            float w1 = static_cast<float>((x2 - x) * (y0 - y) - (x0 - x) * (y2 - y)) / area;
            float w2 = 1.0f - w0 - w1;
            
            // Check if point is inside triangle
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                // Interpolate depth
                float depth = w0 * v0.z + w1 * v1.z + w2 * v2.z;
                
                // Depth test
                if (depthTest(x, y, depth)) {
                    // Interpolate color using barycentric coordinates
                    unsigned char r = static_cast<unsigned char>(
                        std::min(255.0f, w0 * c0.r + w1 * c1.r + w2 * c2.r));
                    unsigned char g = static_cast<unsigned char>(
                        std::min(255.0f, w0 * c0.g + w1 * c1.g + w2 * c2.g));
                    unsigned char b = static_cast<unsigned char>(
                        std::min(255.0f, w0 * c0.b + w1 * c1.b + w2 * c2.b));
                    
                    setPixel(x, y, Color(r, g, b));
                }
            }
        }
    }
}
