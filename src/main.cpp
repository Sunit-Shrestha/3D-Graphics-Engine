#include <iostream>
#include <SFML/Graphics.hpp>
#include "Vector3.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "Light.hpp"
#include "Material.hpp"

using namespace std;

int main() {
  cout << "3D Graphics Engine - Simple Renderer Test" << endl;

  // Create SFML window
  const int WINDOW_WIDTH = 800;
  const int WINDOW_HEIGHT = 600;
  sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "3D Renderer");
  
  // Create renderer
  Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
  
  // Create camera - positioned to clearly see the cube
  Camera camera;
  camera.position = Vector3(0.0f, 0.0f, 3.0f);   // Very close to origin
  camera.target = Vector3(0.0f, 0.0f, 0.0f);     // Looking at the origin where cube will be
  camera.fieldOfView = 3.14159f / 4.0f; // 45 degrees
  camera.aspectRatio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
  camera.nearPlane = 0.1f;
  camera.farPlane = 100.0f;
  
  // Load meshes
  std::vector<Mesh> meshes;
  
  // Load cube - positioned at center, visible and red
  Mesh cubeMesh;
  if (cubeMesh.loadFromOBJ("assets/cube.obj")) {
    cubeMesh.rotateWorldX(2);
    cubeMesh.rotateWorldY(3);
    cubeMesh.setWorldPosition(0.0f, 0.0f, -2.0f);  // Just 2 units in front of camera
    cubeMesh.setWorldScale(1.0f);                   // Normal size first
    meshes.push_back(cubeMesh);
    cout << "✓ Red cube loaded: " << cubeMesh.getVertexCount() << " vertices, " 
         << cubeMesh.getTriangleCount() << " triangles" << endl;
    cout << "  Position: (0, 0, -2), Scale: 1.0" << endl;
    cout << "  Camera position: (0, 0, 3), looking at: (0, 0, 0)" << endl;
  }

  if (meshes.empty()) {
    cout << "✗ No meshes loaded!" << endl;
    return -1;
  }

  // Create lighting setup for Gouraud shading
  std::vector<Light> lights;
 
  // // Add a main directional light from the front-top-right
  Light mainLight(
    Vector3(0, 0, 0),                           // position not used for directional light
    Vector3(-0.7f, 0.8f, -1.0f).normalized(),   // direction from top-left toward cube
    Color(40, 40, 40),                          // ambient
    Color(200, 200, 180),                       // bright warm diffuse
    Color(180, 180, 180)                        // slightly reduced specular to avoid harsh spot
  );
  lights.push_back(mainLight);
  
  // Add a fill light from the left to illuminate shadowed areas
  Light fillLight(
    Vector3(0, 0, 0),                           // position (not used for directional)
    Vector3(1.0f, 0.2f, -0.5f).normalized(),   // direction from left side
    Color(20, 20, 20),                          // low ambient
    Color(80, 80, 120),                         // softer cool diffuse for fill
    Color(100, 100, 100)                        // lower specular
  );
  lights.push_back(fillLight);
  
  // Add a rim light from behind to separate the cube from background
  Light rimLight(
    Vector3(0, 0, 0),                           // position (not used for directional)
    Vector3(0.2f, 0.5f, 1.0f).normalized(),    // direction from behind
    Color(10, 10, 10),                          // minimal ambient
    Color(60, 70, 80),                          // subtle cool diffuse
    Color(150, 150, 150)                        // moderate specular
  );
  lights.push_back(rimLight);
  
  // Create material for the cube with higher ambient for better visibility
  Material cubeMaterial(
    0.4f,   // higher ambient coefficient for minimum lighting
    0.7f,   // diffuse coefficient
    0.3f,   // specular coefficient
    32.0f   // shininess
  );
  
  cout << "✓ Lighting setup: " << lights.size() << " lights created (main + fill + rim)" << endl;

  cout << "\nControls:" << endl;
  cout << "- Close window or ESC: Exit" << endl;
  cout << "- Arrow Keys: Move camera position" << endl;
  cout << "  ↑/↓: Move camera forward/backward" << endl;
  cout << "  ←/→: Move camera left/right" << endl;
  cout << "- H/L: Rotate cube around Y-axis (left/right)" << endl;
  cout << "- J/K: Rotate cube around X-axis (down/up)" << endl;
  cout << "- SPACE: Toggle between Mesh and Lighting rendering" << endl;
  cout << "\nStarting render loop..." << endl;

  // Manual rotation control variables
  float rotationX = 0.0f;
  float rotationY = 0.0f;
  float rotationZ = 0.0f;
  float positionX = 0.0f;
  float positionY = 0.0f;
  float positionZ = -5.0f;
  const float rotationSpeed = 0.05f; // Rotation increment per key press
  const float movementSpeed = 0.2f; // Camera movement speed
  sf::Clock clock; // For frame timing
  bool useLighting = true; // Start with lighting rendering

  // Main render loop - continues until window is closed
  while (window.isOpen()) {
    // Process events
    while (auto event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        cout << "Window close requested." << endl;
        window.close();
      }
      else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
          cout << "ESC key pressed - exiting." << endl;
          window.close();
        }
        // Toggle between mesh and lighting rendering
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
          useLighting = !useLighting;
          cout << "Switched to " << (useLighting ? "Lighting" : "Mesh") << " rendering" << endl;
        }
        // Arrow key controls for camera movement
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
          // camera.position.z -= cameraSpeed; // Move forward
          positionY -= movementSpeed;
          cout << "Camera forward - Z: " << camera.position.z << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
          // camera.position.z += cameraSpeed; // Move backward
          positionY += movementSpeed;
          cout << "Camera backward - Z: " << camera.position.z << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
          // camera.position.x -= cameraSpeed; // Move left
          positionX -= movementSpeed;
          cout << "Camera left - X: " << camera.position.x << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
          // camera.position.x += cameraSpeed; // Move right
          positionX += movementSpeed;
          cout << "Camera right - X: " << camera.position.x << endl;
        }
        // H/L keys for cube rotation around Y-axis
        else if (keyPressed->scancode == sf::Keyboard::Scancode::H) {
          rotationY -= rotationSpeed; // Rotate left around Y-axis
          cout << "Cube rotate left - Y: " << rotationY << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::L) {
          rotationY += rotationSpeed; // Rotate right around Y-axis
          cout << "Cube rotate right - Y: " << rotationY << endl;
        }
        // J/K keys for cube rotation around X-axis
        else if (keyPressed->scancode == sf::Keyboard::Scancode::J) {
          rotationX += rotationSpeed; // Rotate down around X-axis
          cout << "Cube rotate down - X: " << rotationX << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::K) {
          rotationX -= rotationSpeed; // Rotate up around X-axis
          cout << "Cube rotate up - X: " << rotationX << endl;
        }
      }
    }

    // Apply user-controlled rotation to the cube
    for (Mesh& mesh : meshes) {
      mesh.setWorldRotation(rotationX, rotationY, rotationZ);
      mesh.setWorldPosition(positionX, positionY, positionZ);
    }

    // Clear renderer
    renderer.clear(Color(20, 20, 40)); // Dark blue background
    
    // Render scene with either lighting or mesh rendering
    if (useLighting) {
      renderer.render_Light(meshes, camera, lights, cubeMaterial);
    } else {
      renderer.render_Mesh(meshes, camera);
    }
    
    // Present to window
    window.clear();
    renderer.present(window);
    window.display();
  }

  cout << "Render loop finished." << endl;
  return 0;
}