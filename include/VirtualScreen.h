//
// Created by lai leon on 7/7/2023.
//

#ifndef NES_EMU_VIRTUALSCREEN_H
#define NES_EMU_VIRTUALSCREEN_H

#include <SFML/Graphics.hpp>

class VirtualScreen : public sf::Drawable {
public:
    void Create(unsigned int width, unsigned int height, float pixel_size, sf::Color color);

    void SetPixel(std::size_t x, std::size_t y, sf::Color color);

private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const;

    // unsigned vector
    sf::Vector2u m_screenSize;
    float m_pixelSize;
    // 顶点 class
    sf::VertexArray m_vertices;
};


#endif //NES_EMU_VIRTUALSCREEN_H
