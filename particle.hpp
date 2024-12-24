#pragma once
#include <vector>
#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
struct Particle {
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float radius = 10.0f;
    sf::Color color = sf::Color::Magenta;
    int gridx = 0, gridy = 0, id = 0;

    Particle() = default;
    Particle(sf::Vector2f position_, float radius_, int gx_, int gy_, int id_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
        , gridx{gx_}
        , gridy{gy_}
        , id{id_}
    {}

    void update(float dt) {
        sf::Vector2f displacement = position - position_last;
        position_last = position;
        position      = position + displacement + acceleration * (dt * dt);
        acceleration  = {};
        
        gridx = position.x / 15;
        gridy = position.y / 15;
    }

    void accelerate(sf::Vector2f a) {
        acceleration += a;
    }

    void setVelocity(sf::Vector2f v, float dt) {
        position_last = position - (v * dt);
    }

    void addVelocity(sf::Vector2f v, float dt) {
        position_last -= v * dt;
    }

    sf::Vector2f getVelocity() {
        return position - position_last;
    }
};