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
    std::ios::sync_with_stdio(false);
    std::cin.tie(0); std::cout.tie(0);
    // freopen("colors.txt", "r", stdin);
    freopen("positions.txt", "w", stdout);

    // Create window
    constexpr int window_width  = 1840;
    constexpr int window_height = 1380;

    const float        radius         = 5.0f;
    const int          max_objects    = 25000;
    const sf::Vector2f spawn_position = {600.0f, 4.0f}; // 4, 4
    const float        spawn_velocity = 2000.0f; // 400
    const int          max_spawner    = 24; 
    int                num_spawner    = 24;
    int                spawned_count  = 0;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Particles", sf::Style::Default, settings);
    const int frame_rate = 60;
    int frame = 0;
    window.setFramerateLimit(frame_rate);
    
    Threader threadPool(10);
    Solver solver(window_width, window_height, radius, threadPool);
    Renderer renderer(window, threadPool, solver);
 
    sf::Clock timer, fpstimer;
    int r, g, b;
    sf::Font arialFont;
    arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    // ObstacleBox& left = solver.addObstacleBox({4, 1000}, {200, window_height / 2}, 
    //     {300, window_height / 2});
    // left.cycle_speed = 120.0 / 144.0;
    // left.rotation = 15;
    // ObstacleBox& right = solver.addObstacleBox({4, 1000}, {window_width - 300, window_height / 2}, 
    //     {window_width - 400, window_height / 2});
    // right.cycle_speed = 120.0 / 144.0;
    // right.rotation = -15;
    ObstacleDot& ldot = solver.addObstacleDot(20.0, {400.0, -20.0}, {400.0, window_height + 20.0});
    ldot.cycle_speed = 240.0 / 144.0;
    ObstacleDot& rdot = solver.addObstacleDot(20.0, {window_width - 400.0, window_height + 20.0}, 
        {window_width - 400.0, -20.0});
    rdot.cycle_speed = 240.0 / 144.0;

    std::vector<std::pair<int, int>> pos[7000];
    int pos_counter = 0;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }
        float time = timer.getElapsedTime().asSeconds();

        if (frame >= 1350 && frame < 14490 && frame % 2 == 0) {
            pos_counter++;
            for (Particle& obj : solver.objects) {
                pos[pos_counter].push_back(std::make_pair((int)obj.position.x, (int)obj.position.y));
            }
        }
        
        if (frame > 14490) {
            window.close();
        }
        frame++;
        // Spawn particles
        int num_objects = solver.objects.size();
        if (num_objects < max_objects) {
            sf::Color currentColor = getColor(time);
            spawned_count++;
            for (int i = 0; i < std::min(num_spawner, max_objects - num_objects); i++) {
                auto& new_object = solver.addObject(spawn_position + sf::Vector2f{i * 10.0f, 0.0f}, radius); // 8
                // std::cin >> r >> g >> b;
                // new_object.color = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
                new_object.color = currentColor;
                solver.setObjectVelocity(new_object, spawn_velocity * sf::Vector2f{0.4, 0.9});
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
    for (int i = 1; i <= pos_counter; i++)
        for (std::pair<int, int> j : pos[i]) 
            std::cout << j.first << ' ' << j.second << std::endl;
    return 0;
}
