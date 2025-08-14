#include <iostream>
#include <SFML/Graphics.hpp>
#include "Vector3.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"

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

  cout << "\nControls:" << endl;
  cout << "- Close window or ESC: Exit" << endl;
  cout << "- Arrow Keys: Rotate cube" << endl;
  cout << "  ↑/↓: Rotate around X-axis" << endl;
  cout << "  ←/→: Rotate around Y-axis" << endl;
  cout << "\nStarting render loop..." << endl;

  // Manual rotation control variables
  float rotationX = 0.0f;
  float rotationY = 0.0f;
  float rotationZ = 0.0f;
  const float rotationSpeed = 0.05f; // Rotation increment per key press
  sf::Clock clock; // For frame timing

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
        // Arrow key controls for cube rotation
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
          rotationX -= rotationSpeed; // Rotate up (negative X rotation)
          cout << "Rotate up - X: " << rotationX << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
          rotationX += rotationSpeed; // Rotate down (positive X rotation)
          cout << "Rotate down - X: " << rotationX << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
          rotationY -= rotationSpeed; // Rotate left (negative Y rotation)
          cout << "Rotate left - Y: " << rotationY << endl;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
          rotationY += rotationSpeed; // Rotate right (positive Y rotation)
          cout << "Rotate right - Y: " << rotationY << endl;
        }
      }
    }

    // Apply user-controlled rotation to the cube
    for (Mesh& mesh : meshes) {
      mesh.setWorldRotation(rotationX, rotationY, rotationZ);
    }

    // Clear renderer
    renderer.clear(Color(20, 20, 40)); // Dark blue background
    
    // Render scene
    renderer.render(meshes, camera);
    
    // Present to window
    window.clear();
    renderer.present(window);
    window.display();
  }

  cout << "Render loop finished." << endl;
  return 0;
}