#pragma once
#include "solver.hpp"
#include <string>

class Renderer {
public:
    explicit
    Renderer(sf::RenderTarget& target)
        : m_target{target}
    {}

    void render(const Solver& solver) const {
        // Render boundary
        const sf::Vector3f boundary = solver.getBoundary();
        sf::CircleShape boundary_background{boundary.z};
        boundary_background.setOrigin(boundary.z, boundary.z);
        boundary_background.setFillColor(sf::Color::Black);
        boundary_background.setPosition(boundary.x, boundary.y);
        boundary_background.setPointCount(128);
        m_target.draw(boundary_background); // DRAW CIRCLE
        // m_target.clear(sf::Color::Black);
        
        sf::Font arialFont;
        arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");
        // Render objects
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto& objects = solver.getObjects();
        for (const auto& obj : objects) {
            circle.setPosition(obj.position);
            circle.setScale(obj.radius, obj.radius);
            circle.setFillColor(obj.color);
            m_target.draw(circle);
            // sf::Text number;
            // number.setFont(arialFont);
            // number.setString(std::to_string(obj.id));
            // number.setPosition(obj.position.x - obj.radius, obj.position.y - obj.radius);
            // number.setCharacterSize(24);
            // m_target.draw(number);
        }
        // for (int i = 60; i < 840; i += 60) {
        //     sf::VertexArray lines(sf::LinesStrip, 2);
        //     lines[0].position = sf::Vector2f(i, 0);
        //     lines[1].position = sf::Vector2f(i, 840);
        //     m_target.draw(lines);
        //     lines[0].position = sf::Vector2f(0, i);
        //     lines[1].position = sf::Vector2f(840, i);
        //     m_target.draw(lines);
        // }
    }

private:
    sf::RenderTarget& m_target;
};