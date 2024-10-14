#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
#include "solver.hpp"
#include "renderer.hpp"

float getRandom() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static sf::Color getColor(float t) {
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * M_PI);
    const float b = sin(t + 0.66f * 2.0f * M_PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
}

int main() {
    // Create window
    constexpr int window_width  = 840;
    constexpr int window_height = 840;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Verlet", sf::Style::Default, settings);
    const int frame_rate = 60;
    window.setFramerateLimit(frame_rate);
    Renderer renderer{window};

    Solver solver;
    solver.setBoundary({window_width / 2, window_height / 2}, (window_width - 20.0f) / 2);

    const int          max_objects    = 3000;
    const float        spawn_delay    = 0.005f;
    const sf::Vector2f spawn_position = {420.0f, 200.0f};
    const float        min_radius     = 4.0f;
    const float        max_radius     = 8.0f;
    const float        spawn_velocity = 2000.0f;
    const float        max_angle      = M_PI * 0.5f;

    sf::Clock respawnClock, timer, fpstimer;
    sf::Font arialFont;
    arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }
        // Spawn particles
        if (solver.getObjects().size() < max_objects && respawnClock.getElapsedTime().asSeconds() >= spawn_delay) {
            float t = timer.getElapsedTime().asSeconds();
            float radius = min_radius + (max_radius - min_radius) * getRandom();
            auto& object = solver.addObject(spawn_position, radius);
            
            object.color = getColor(t);
            float angle = M_PI * 0.5f + max_angle * sin(3 * t);
            solver.setObjectVelocity(object, spawn_velocity * sf::Vector2f{cos(angle), sin(angle)});
            respawnClock.restart();
        }
        // Detect mouse action
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            float ratio = 840.0f / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePull(pos);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            float ratio = 840.0f / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePush(pos);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) solver.toggleGravityUp();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) solver.toggleGravityDown();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) solver.toggleGravityLeft();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) solver.toggleGravityRight();

        fpstimer.restart();
        solver.update();
        window.clear(sf::Color::White);
        renderer.render(solver);
        // Render performance
        sf::Text number;
        number.setFont(arialFont);
        float ms = 1.0 * fpstimer.getElapsedTime().asMicroseconds() / 1000;
        number.setString(std::to_string(ms) + "ms, " + std::to_string(solver.getObjects().size()) + " particles");
        number.setCharacterSize(24);
        number.setFillColor(sf::Color::Magenta);
        window.draw(number);
        
        window.display();
    }
    
    return 0;
}