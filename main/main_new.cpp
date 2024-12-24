#include <iostream>
#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include "solvers/solver_final.hpp"
#include "renderers/renderer_fast.hpp"
#include "thread.hpp"
#include <chrono>
#include <thread>

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
    freopen("colors.txt", "r", stdin);
    // freopen("positions.txt", "w", stdout);

    // Create window
    constexpr int window_width  = 1140;
    constexpr int window_height = 1140;

    const float        radius         = 2.0f;
    const int          max_objects    = 88888;
    const sf::Vector2f spawn_position = {4.0f, 4.0f};
    const float        spawn_velocity = 400.0f;
    const int          max_spawner    = 24;  
    int                num_spawner    = 24;
    int                spawned_count  = 0;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Particles", sf::Style::Default, settings);
    const int frame_rate = 60;
    window.setFramerateLimit(frame_rate);
    
    tp::ThreadPool threadPool(10);
    Solver solver(window_width, radius, threadPool);
    Renderer renderer(window, threadPool, solver);

    sf::Clock timer, fpstimer;
    bool done = false;
    int r, g, b;
    sf::Font arialFont;
    arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");
    /*
    for (int i = 1; i <= 7; i++) {
        ObstacleDot& obstacle = solver.addObstacleDot(15.0f, 
             {170.0f * i, window_height + 15.0f}, {170.0f * i, -15.0f});
        obstacle.color = sf::Color::White;
        obstacle.cycle_speed = 3.0f;
        obstacle.update_type = 2;
    }
    ObstacleBox& rightBox = solver.addObstacleBox({6.0f, 600.0f},
        {window_width - 35, window_height - 320});
    ObstacleBox& rightSlanted = solver.addObstacleBox({6.0f, 50.0f},
        {window_width - 15, window_height - 640});
    rightSlanted.rotation = -45.0f;
    for (int i = 0; i < 8; i++) {
        ObstacleBox& moving = solver.addObstacleBox({40.0f, 6.0f},
            {window_width - 15, window_height + 10}, {window_width - 15, window_height - 663});
        moving.update_type = 2;
        moving.cycle_speed = 5;
        moving.time = 0.625f * i;
        moving.rotation = -30.0f;
    }

    ObstacleBox& leftBox = solver.addObstacleBox({6.0f, 600.0f},
        {35, window_height - 320});
    ObstacleBox& leftSlanted = solver.addObstacleBox({6.0f, 50.0f},
        {15, window_height - 640});
    leftSlanted.rotation = 45.0f;
    for (int i = 0; i < 8; i++) {
        ObstacleBox& moving = solver.addObstacleBox({40.0f, 6.0f},
            {15, window_height + 10}, {15, window_height - 663});
        moving.update_type = 2;
        moving.cycle_speed = 5;
        moving.time = 0.625f * i;
        moving.rotation = 30.0f;
    }
    */

    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }
        float time = timer.getElapsedTime().asSeconds();
        // if (time > 90 && !done) {
        //     done = true;
        //     for (Particle& obj : solver.objects) {
        //         std::cout << obj.position.x << ' ' << obj.position.y << std::endl;
        //     }
        // }
        // Spawn particles
        int num_objects = solver.objects.size();
        if (num_objects < max_objects) {
            // sf::Color currentColor = getColor(time);
            spawned_count++;
            for (int i = 0; i < std::min(num_spawner, max_objects - num_objects); i++) {
                auto& new_object = solver.addObject(spawn_position + sf::Vector2f{0.0f, i * 8.0f}, radius);
                std::cin >> r >> g >> b;
                new_object.color = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                // new_object.color = currentColor;
                solver.setObjectVelocity(new_object, spawn_velocity * sf::Vector2f{0.8, 0.6});
            }
            // if (spawned_count / 50 >= num_spawner && num_spawner < max_spawner) num_spawner++;
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

        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) solver.toggleGravityUp();
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) solver.toggleGravityDown();
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) solver.toggleGravityLeft();
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) solver.toggleGravityRight();
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) std::this_thread::sleep_for (std::chrono::milliseconds(60));
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) std::this_thread::sleep_for (std::chrono::milliseconds(250));

        fpstimer.restart();
        solver.update();
        window.clear(sf::Color::White);
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