// any gravity, dampening 0.9
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
    // freopen("colors.txt", "r", stdin);
    // freopen("positions.txt", "w", stdout);

    // Create window
    constexpr int window_width  = 2560;
    constexpr int window_height = 1380;

    const float        radius         = 4.0f;
    const int          max_objects    = 2400;
    const sf::Vector2f spawn_position = {4, 8};
    const int          max_spawner    = 16;  
    int                num_spawner    = 16;
    int                spawned_count  = 0;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Particles", sf::Style::Default, settings);
    const int frame_rate = 60;
    window.setFramerateLimit(frame_rate);
    
    Threader threadPool(10);
    Solver solver(window_width, window_height, radius, threadPool);
    Renderer renderer(window, threadPool, solver);

    sf::Clock timer, fpstimer;
    sf::Font arialFont;
    arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 100; j++) {
            ObstacleBox& box = solver.addObstacleBox({10, 10}, {5.0f + i * 10, 385.0f + j * 10});
            box.breakable = true;
            box.durability = box.total_dur = 50;
            box.color = getColor((i + j) * 0.02f);
        }

    // Main loop
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
            spawned_count++;
            for (int i = 0; i < std::min(num_spawner, max_objects - num_objects); i++) {
                // sf::Color currentColor = getColor(time + i * 0.5);
                auto& new_object = solver.addObject(spawn_position + sf::Vector2f{0, i * 10.0f}, radius);
                new_object.color = sf::Color::White;
                solver.setObjectVelocity(new_object, {800, 600});
            }
            // if (spawned_count / 50 >= num_spawner && num_spawner < max_spawner) num_spawner++;
        }
        // Detect mouse action
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            float ratio = window_width / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePull(pos, 160);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            float ratio = window_width / window.getSize().x; // Correct for scaled window
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.mousePush(pos, 160);
        }

        fpstimer.restart();
        solver.update();
        window.clear(sf::Color::White);
        renderer.updateTrailVA();
        renderer.newRender();
        // Render performance
        sf::Text number;
        number.setFont(arialFont);
        float ms = 1.0 * fpstimer.getElapsedTime().asMicroseconds() / 1000;
        number.setString(std::to_string(ms) + "ms, " + std::to_string(solver.objects.size()) + " particles");
        number.setCharacterSize(24);
        number.setFillColor(sf::Color::White);
        window.draw(number);
        
        window.display();
    }
    return 0;
}