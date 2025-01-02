#pragma once
#include <vector>
#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
struct ObstacleDot {
    sf::Vector2f position;
    sf::Vector2f start_position;
    sf::Vector2f end_position;
    float radius = 10.0f;
    float time = 0.0f;
    float cycle_speed = 10.0f;
    int update_type = 1;
    sf::Color color = sf::Color::White;

    ObstacleDot() = default;
    ObstacleDot(sf::Vector2f start_position_, sf::Vector2f end_position_, float radius_)
        : position{start_position_}
        , start_position{start_position_}
        , end_position{end_position_}
        , radius{radius_}
    {}

    void update(float dt) {
        time += dt;
        switch (update_type) {
            case 1:
                oscillate();
                break;
            case 2:
                linear();
                break;
            default:
                break;
        }
    }
    
    void oscillate () {
        float between = 0.5f * (1 - cos(2 * M_PI / cycle_speed * time));
        position = start_position + (end_position - start_position) * between;
    }

    void linear () {
        float between = (time - cycle_speed * floor(time / cycle_speed)) / cycle_speed;
        position = start_position + (end_position - start_position) * between;
    }
};