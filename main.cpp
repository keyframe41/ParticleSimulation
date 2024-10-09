#include <iostream>
#include <SFML/Graphics.hpp>
#include "solver.hpp"
#include "renderer.hpp"
#include "utils/number_generator.hpp"
#include "utils/math.hpp"

static sf::Color getRainbow(float t) {
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * Math::PI);
    const float b = sin(t + 0.66f * 2.0f * Math::PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
}


int32_t main(int32_t, char*[]) {
    // Create window
    constexpr int32_t window_width  = 840;
    constexpr int32_t window_height = 840;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Verlet", sf::Style::Default, settings);
    const uint32_t frame_rate = 60;
    window.setFramerateLimit(frame_rate);

    Solver solver;
    Renderer renderer{window};

    // Solver configuration
    solver.setBoundary({0.5f * window_width, 0.5f * window_height}, window_width / 2 - 20.0f);
    solver.setSubStepsCount(8);
    solver.setSimulationUpdateRate(frame_rate);

    // Set simulation attributes
    const float        object_spawn_delay    = 0.001f;
    const float        object_spawn_speed    = 2000.0f;
    const sf::Vector2f object_spawn_position = {420.0f, 120.0f};
    const float        object_min_radius     = 3.0f;
    const float        object_max_radius     = 7.0f;
    const uint32_t     max_objects_count     = 3000;
    const float        max_angle             = Math::PI;

    sf::Clock clock, fpstimer;
    sf::Font arialFont;
    arialFont.loadFromFile("/Library/Fonts/Arial Unicode.ttf");
    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }

        if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() >= object_spawn_delay) {
            clock.restart();
            auto&       object = solver.addObject(object_spawn_position, RNGf::getRange(object_min_radius, object_max_radius));
            const float t      = solver.getTime();
            const float angle  = max_angle * sin(3 * t) + Math::PI * 0.5f;
            solver.setObjectVelocity(object, object_spawn_speed * sf::Vector2f{cos(angle), sin(angle)});

            object.color = getRainbow(t);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            float ratio = 840.0f / window.getSize().x;
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.explode1(pos);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            float ratio = 840.0f / window.getSize().x;
            sf::Vector2f pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) * ratio;
            solver.explode2(pos);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            solver.toggleGravityUp();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            solver.toggleGravityDown();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            solver.toggleGravityLeft();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            solver.toggleGravityRight();
        }
        solver.update();
        window.clear(sf::Color::White);
        renderer.render(solver);
        sf::Text number;
        number.setFont(arialFont);
        float ms = 1.0 * fpstimer.getElapsedTime().asMicroseconds() / 1000;
        number.setString(std::to_string(ms) + "ms");
        number.setCharacterSize(24);
        window.draw(number);
		window.display();
        fpstimer.restart();
    }

    return 0;
}