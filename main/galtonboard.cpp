// Galton board demonstration
// dampening 0.4, gravity 200
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include "solvers/solver_final.hpp"
#include "renderers/renderer_fast.hpp"
#include "thread.hpp"
#include <chrono>
#include <thread>


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
    constexpr int window_width  = 1560;
    constexpr int window_height = 1380;

    const float        radius         = 2.0f;
    const int          max_objects    = 20000;
    const sf::Vector2f spawn_position = {window_width / 2.0f - 60.5f, 4.0f}; // 4, 4
    const float        spawn_velocity = 400.0f; // 400
    const float        spawn_delay    = 0.05f;
    const int          max_spawner    = 16; 
    int                num_spawner    = 16;
    int                spawned_count  = 0;
    sf::Vector2f       gravity        = {0.0f, 200.0f};

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Particles", sf::Style::Default, settings);
    const int frame_rate = 60;
    window.setFramerateLimit(frame_rate);
    
    Threader threadPool(10);
    Solver solver(window_width, window_height, radius, threadPool, gravity);
    Renderer renderer(window, threadPool, solver);
 
    sf::Clock timer, fpstimer;
    sf::Font arialFont;
    arialFont.loadFromFile("assets/Arial Unicode.ttf");

    for (int i = 1; i <= 21; i++) {
        float height = 340 + i * 20;
        float start = (i % 2 == 1 ? 0 : 10);
        for (float j = start; j <= window_width; j += 20) {
            ObstacleDot& obs = solver.addObstacleDot(2.0f, {j, height});
        }
    }
    for (float i = 40; i < window_width; i += 40) {
        ObstacleBox& obs = solver.addObstacleBox({2.0f, 600.0f}, {i, window_height - 300});
    }

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }
        float time = timer.getElapsedTime().asSeconds();
        // Spawn particles
        int num_objects = solver.objects.size();
        if (num_objects < max_objects) {
            sf::Color currentColor = getColor(time);
            spawned_count++;
            for (int i = 0; i < std::min(num_spawner, max_objects - num_objects); i++) {
                auto& new_object = solver.addObject(spawn_position + sf::Vector2f{i * 8.0f + getRandom(), 0.0f}, radius); // 8
                new_object.color = currentColor;
                solver.setObjectVelocity(new_object, spawn_velocity * sf::Vector2f{0.0, 1.0});
            }
            if (spawned_count / 50 >= num_spawner && num_spawner < max_spawner) num_spawner++;
        }
        // Detect mouse action
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            float ratio = window_width / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePull(pos, 120);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            float ratio = window_width / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePush(pos, 120);
        }
        
        // Slowdown utils
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) std::this_thread::sleep_for (std::chrono::milliseconds(60));
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) std::this_thread::sleep_for (std::chrono::milliseconds(250));

        fpstimer.restart();
        solver.update();
        float ms = 1.0 * fpstimer.getElapsedTime().asMicroseconds() / 1000;
        window.clear(sf::Color::White);
        renderer.newRender();
        ms = 1.0 * fpstimer.getElapsedTime().asMicroseconds() / 1000;
        // Render performance
        sf::Text number;
        number.setFont(arialFont);
        
        number.setString(std::to_string(ms) + "ms, " + std::to_string(solver.objects.size()) + " particles");
        number.setCharacterSize(24);
        number.setFillColor(sf::Color::White);
        window.draw(number);
        
        window.display();
    }
    return 0;
}