#pragma once
#include "solver.hpp"

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
        m_target.draw(boundary_background);

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
        }
    }

private:
    sf::RenderTarget& m_target;
};