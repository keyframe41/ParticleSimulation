// rectangular box obstacle
#pragma once
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>
struct ObstacleBox {
    sf::Vector2f position;
    sf::Vector2f dimensions;
    sf::Vector2f start_position;
    sf::Vector2f end_position;
    float time = 0.0f, rotation = 0.0f;
    float cycle_speed = 2.0f * M_PI; // number of seconds per movement period
    float rotation_speed = 0.0f; // optional rotation (degrees / second)
    int update_type = 1;
    bool breakable = false;
    float durability = 2000, total_dur = 2000; // breakable box durability
    sf::Color color = sf::Color::White;

    ObstacleBox() = default;
    ObstacleBox(sf::Vector2f dimensions_, sf::Vector2f start_position_, sf::Vector2f end_position_)
        : position{start_position_}
        , dimensions{dimensions_}        
        , start_position{start_position_}
        , end_position{end_position_}
    {}

    void update (float dt) {
        time += dt;
        rotation += rotation_speed * dt;
        switch (update_type) {
        case 1: 
            oscillate(); // cosine between start and end position
            break;
        case 2:
            linear(); // Loop interpolate between start and end position
        default:
            break;
        }
    }

    void oscillate () {
        float between = 0.5f * (1.0f + sin(2 * M_PI / cycle_speed * time));
        position = start_position + (end_position - start_position) * between;
    }

    void linear () {
        float between = (time - cycle_speed * floor(time / cycle_speed)) / cycle_speed;
        position = start_position + (end_position - start_position) * between;
    }
};