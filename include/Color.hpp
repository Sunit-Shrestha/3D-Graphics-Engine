#pragma once

// Simple RGB color structure (no SFML dependency)
class Color {
public:
    unsigned char r, g, b;

    Color(unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255);
    Color operator*(float factor) const;
    Color operator+(const Color& other) const;
};
