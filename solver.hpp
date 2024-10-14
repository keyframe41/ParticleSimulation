#pragma once
#include <vector>
#include <iostream>
#include <cmath>
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


class Solver {
public:
    Solver() = default;

    Particle& addObject(sf::Vector2f position, float radius) {
        int gridx = position.x / grid_size, gridy = position.x / grid_size;
        Particle newParticle = Particle(position, radius, gridx, gridy, objects.size());
        grid[gridx][gridy].push_back(newParticle);
        return objects.emplace_back(newParticle);
    }

    void mousePull(sf::Vector2f pos) {
        for (Particle& obj : objects) {
            sf::Vector2f dir = pos - obj.position;
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            obj.accelerate(dir * std::max(0.0f, 10 * (120 - dist)));
        }
    }

    void mousePush(sf::Vector2f pos) {
        for (Particle& obj : objects) {
            sf::Vector2f dir = pos - obj.position;
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            obj.accelerate(dir * std::min(0.0f, -10 * (120 - dist)));
        }
    }

    void update() {
        float substep_dt = step_dt / sub_steps;
        for (int i = 0; i < sub_steps; i++) {
            applyGravity();
            checkCollisions();
            // applyBoundary();
            applyBorder();
            updateObjects(substep_dt);
        }
    }

    std::vector<Particle>& getObjects() {
        return objects;
    }

    void setObjectVelocity(Particle& object, sf::Vector2f vel) {
        object.setVelocity(vel, step_dt / sub_steps);
    }

    void setBoundary(sf::Vector2f position, float radius) {
       boundary_center = position;
       boundary_radius = radius;
    }

    sf::Vector3f getBoundary() const {
      return {boundary_center.x, boundary_center.y, boundary_radius};
    }

    void toggleGravityUp() {
        gravity = {0.0f, -1000.0f};
    }

    void toggleGravityDown() {
        gravity = {0.0f, 1000.0f};
    }

    void toggleGravityLeft() {
        gravity = {-1000.0f, 0.0f};
    }

    void toggleGravityRight() {
        gravity = {1000.0f, 0.0f};
    }


private:
    float                  window_size      = 840.0f;
    sf::Vector2f           gravity          = {0.0f, 1000.0f};
    sf::Vector2f           boundary_center  = {420.0f, 420.0f};
    float                  boundary_radius  = 100.0f; 
    std::vector<Particle>  objects;
    float                  step_dt          = 1.0f / 60;
    int                    sub_steps        = 8;
    int                    grid_size        = 15;
    std::vector<Particle>  grid[100][100];

    

    void applyBorder() {
        for (auto& obj : objects) {
            const float dampening = 0.75f;
            const sf::Vector2f pos  = obj.position;
            sf::Vector2f npos = obj.position;
            sf::Vector2f vel  = obj.getVelocity();
            sf::Vector2f dy = {vel.x * dampening, -vel.y};
            sf::Vector2f dx = {-vel.x, vel.y * dampening};
            if (pos.x < obj.radius || pos.x + obj.radius > window_size) { // Bounce off left/right
                if (pos.x < obj.radius) npos.x = obj.radius;
                if (pos.x + obj.radius > window_size) npos.x = window_size - obj.radius;
                obj.position = npos;
                obj.setVelocity(dx, 1.0);
            }
            if (pos.y < obj.radius || pos.y + obj.radius > window_size) { // Bounce off top/bottom
                if (pos.y < obj.radius) npos.y = obj.radius;
                if (pos.y + obj.radius > window_size) npos.y = window_size - obj.radius;
                obj.position = npos;
                obj.setVelocity(dy, 1.0);
            }
        }
    }

    void applyBoundary() {
        for (auto& obj : objects) {
            const sf::Vector2f r = boundary_center - obj.position;
            const float dist = sqrt(r.x * r.x + r.y * r.y);
            if (dist > boundary_radius - obj.radius) {
                const sf::Vector2f n = r / dist;
                const sf::Vector2f perp = {-n.y, n.x};
                const sf::Vector2f vel = obj.getVelocity();
                obj.position = boundary_center - n * (boundary_radius - obj.radius);
                obj.setVelocity(2.0f * (vel.x * perp.x + vel.y * perp.y) * perp - vel, 1.0f);
            }
        }
    }

    std::vector<int> getCollisionParticles (int particleID) {
        Particle p = objects[particleID];
        std::vector<int> res;
        for (int i = p.gridx - 1; i <= p.gridx + 1; i++)
            for (int j = p.gridy - 1; j <= p.gridy + 1; j++) {
                if (i < 0 || j < 0 || i >= 56 || j >= 56) continue;
                for (Particle& pp : grid[i][j])
                    if (pp.id != p.id) res.push_back(pp.id);
            }
        return res;
    }

    void checkCollisions() {
        int num_objects = objects.size();
        for (Particle& obj_1 : objects) {
            for (int i : getCollisionParticles(obj_1.id)) {
                Particle& obj_2 = objects[i];
                sf::Vector2f v = obj_1.position - obj_2.position;
                float dist = sqrt(v.x * v.x + v.y * v.y);
                float min_dist = obj_1.radius + obj_2.radius;
                if (dist < min_dist) {
                    sf::Vector2f n = v / dist; // Normalize
                    float total_mass = obj_1.radius * obj_1.radius + obj_2.radius * obj_2.radius;
                    float mass_ratio = (obj_1.radius * obj_1.radius) / total_mass;
                    float delta = 0.5f * (min_dist - dist);
                    // Larger particle moves less
                    obj_1.position += n * (1 - mass_ratio) * delta;
                    obj_2.position -= n * mass_ratio * delta;
                }
            }
        }
    }

    void applyGravity() {
        for (auto& obj : objects) {
            obj.accelerate(gravity);
        }
    }
    
    void updateObjects(float dt) {
        for (int i = 0; i < 100; i++)
            for (int j = 0; j < 100; j++) grid[i][j].clear();
        for (auto& obj : objects) {
            obj.update(dt);
            grid[obj.gridx][obj.gridy].push_back(obj);
        }
    }
};