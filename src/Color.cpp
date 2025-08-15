#include "Color.hpp"

// Constructor definition
Color::Color(unsigned char red, unsigned char green, unsigned char blue)
    : r(red), g(green), b(blue) {}
// Multiply color by a float (0.0 - 1.0)
Color Color::operator*(float factor) const {
    auto clamp = [](int value) -> unsigned char {
        if (value < 0) return 0;
        if (value > 255) return 255;
        return static_cast<unsigned char>(value);
    };
    return Color(
        clamp(static_cast<int>(r * factor)),
        clamp(static_cast<int>(g * factor)),
        clamp(static_cast<int>(b * factor))
    );
}

Color Color::operator+(const Color& other) const {
  return Color(
    r+other.r, g+other.g, b+other.b
  );
}
