// gravity
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
    const sf::Vector2f spawn_position = {24, 8};
    const int          max_spawner    = 230;  
    int                num_spawner    = 230;
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

    ObstacleBox& top = solver.addObstacleBox({window_width * 2, 10}, {0, 100});
    top.breakable = true;
    top.durability = 600;

    ObstacleBox& right = solver.addObstacleBox({10, window_height}, {window_width - 205, window_height / 2});
    ObstacleBox& finish = solver.addObstacleBox({window_width - 215, 10}, {(window_width - 205) / 2, window_height - 5});
    finish.color = sf::Color::Green;

    for (int i = 0; i < 8; i++) {
        int start = (i % 2 == 0 ? 40 : 200);
        for (float j = start; j <= window_width - 200; j += 320) {
            // ObstacleDot& obs = solver.addObstacleDot(30, {j, window_height + 30}, {j, 200});
            // obs.cycle_speed = 6;
            // obs.update_type = 2;
            // obs.time = i * 0.5;
            ObstacleBox& box = solver.addObstacleBox({160, 10}, {j, window_height + 30}, {j, 200});
            box.cycle_speed = 10;
            box.update_type = 2;
            box.time = i * 10.0 / 8;
            if (getRandom() < 0.05) box.color = sf::Color::Red;
        }
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
                sf::Color currentColor = getColor(time + i * 0.02);
                auto& new_object = solver.addObject(spawn_position + sf::Vector2f{i * 10.0f + getRandom(), 0}, radius);
                new_object.color = currentColor;
                // solver.setObjectVelocity(new_object, {800, 600});
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