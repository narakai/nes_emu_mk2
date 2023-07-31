//
// Created by lai leon on 7/7/2023.
//

#include <VirtualScreen.h>

/*
 *  SMFL(Simple and Fast Multimedia Library)
 *  API 文档： https://www.sfml-dev.org/documentation/2.5.1/
*/

void VirtualScreen::Create(unsigned int width, unsigned int height, float pixel_size, sf::Color color) {
//    每个像素由两个三角形组成，每个三角形有三个顶点，因此每个像素需要六个顶点来表示。
//    乘以6是为了保证容器的大小足够存储所有像素的顶点信息。

//    +---+
//    |\  |
//    | \ |
//    |  \|
//    +---+

    m_vertices.resize(width * height * 6);
    m_vertices.setPrimitiveType(sf::Triangles);
    // 二元组
    m_screenSize = {width, height};
    // 像素大小
    m_pixelSize = pixel_size;

    for (std::size_t x = 0; x < width; ++x) {
        for (std::size_t y = 0; y < height; ++y) {
            // 将一个二维数组的元素转换为一维数组中的索引，从而在一维数组中存储和处理二维数组的数据
            // x * m_screenSize.y 计算出当前行的起始位置在一维数组中的索引
            // x * m_screenSize.y + y 计算出当前元素在一维数组中的索引
            auto index = (x * m_screenSize.y + y) * 6;
            // float 型 二元组
            sf::Vector2f coord2d(x * m_pixelSize, y * m_pixelSize);

            //Triangle-1 (右半部分三角形)
            // 上左
            m_vertices[index].position = coord2d;
            m_vertices[index].color = color;
            // 下右
            m_vertices[index + 2].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
            m_vertices[index + 2].color = color;
            //上右
            m_vertices[index + 1].position = coord2d + sf::Vector2f{m_pixelSize, 0};
            m_vertices[index + 1].color = color;

            //Triangle-2 (左半部分三角形)
            //bottom-right
            m_vertices[index + 3].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
            m_vertices[index + 3].color = color;

            //bottom-left
            m_vertices[index + 4].position = coord2d + sf::Vector2f{0, m_pixelSize};
            m_vertices[index + 4].color = color;

            //top-left
            m_vertices[index + 5].position = coord2d;
            m_vertices[index + 5].color = color;
        }
    }
}

void VirtualScreen::SetPixel(std::size_t x, std::size_t y, sf::Color color) {
//    每个像素（二维屏幕坐标 (x, y)）被展开成了一个由两个三角形组成的结构，而每个三角形都有三个顶点，
//    所以每个像素对应的索引需要乘以 6 才能表示六个顶点的索引
    auto index = (x * m_screenSize.y + y) * 6;
    if (index >= m_vertices.getVertexCount())
        return;

    //Triangle-1
    //top-left
    m_vertices[index].color = color;

    //top-right
    m_vertices[index + 1].color = color;

    //bottom-right
    m_vertices[index + 2].color = color;

    //Triangle-2
    //bottom-right
    m_vertices[index + 3].color = color;

    //bottom-left
    m_vertices[index + 4].color = color;

    //top-left
    m_vertices[index + 5].color = color;
}

void VirtualScreen::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // Draw primitives defined by a vertex buffer.
    target.draw(m_vertices, states);
}
