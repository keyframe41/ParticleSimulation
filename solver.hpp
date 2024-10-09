#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "utils/math.hpp"
struct Particle {
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float radius = 10.0f;
    sf::Color color = sf::Color::White;
    int gridx = 0, gridy = 0, id = 0;

    Particle() = default;
    Particle(sf::Vector2f position_, float radius_, int gridx_, int gridy_, int id_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
        , gridx{gridx_}
        , gridy{gridy_}
        , id{id_}
    {}

    void update(float dt) {
        // Compute how much we moved
        const sf::Vector2f displacement = position - position_last;
        // Update position
        position_last = position;
        position      = position + displacement + acceleration * (dt * dt);
        // Reset acceleration
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

    sf::Vector2f getVelocity() const {
        return position - position_last;
    }
};


class Solver {
public:
    Solver() = default;

    Particle& addObject(sf::Vector2f position, float radius) {
        int gridx = position.x / grid_size, gridy = position.y / grid_size;
        Particle newParticle = Particle(position, radius, gridx, gridy, getObjectsCount());
        grid[gridx][gridy].push_back(newParticle);
        return objects.emplace_back(newParticle);
    }

    void explode1(sf::Vector2f pos) {
        const sf::Vector2f mouse_pos = {pos.x * 1.0f, pos.y * 1.0f};
        for (auto &obj : objects) {
            sf::Vector2f dir = mouse_pos - obj.position;
            float dist = Math::magnitude(dir);
            obj.accelerate(dir * std::max(0.0f, 10 * (120 - dist)));
        }
    }
    void explode2(sf::Vector2f pos) {
        const sf::Vector2f mouse_pos = {pos.x * 1.0f, pos.y * 1.0f};
        for (auto &obj : objects) {
            float dist = Math::magnitude(mouse_pos - obj.position);
            sf::Vector2f dir = mouse_pos - obj.position;
            obj.accelerate(dir * std::min(0.0f, -10 * (120 - dist)));
        }
    }

    void update() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (uint32_t i{sub_steps}; i--;) {
            applyGravity();
            checkCollisions(step_dt);
            applyBoundary();
            // applyBorder();
            updateObjects(step_dt);
        }
    }

    void setSimulationUpdateRate(uint32_t rate) {
        frame_dt = 1.0f / static_cast<float>(rate);
    }

    void setBoundary(sf::Vector2f position, float radius) {
        boundary_center = position;
        boundary_radius = radius;
    }

    void setSubStepsCount(uint32_t sub_steps) {
        sub_steps = sub_steps;
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

    void setObjectVelocity(Particle& object, sf::Vector2f v) {
        object.setVelocity(v, getStepDt());
    }

    const std::vector<Particle>& getObjects() const {
        return objects;
    }

    sf::Vector3f getBoundary() const {
        return {boundary_center.x, boundary_center.y, boundary_radius};
    }

    uint64_t getObjectsCount() const {
        return objects.size();
    }

    float getTime() const {
        return time;
    }

    float getStepDt() const {
        return frame_dt / static_cast<float>(sub_steps);
    }


private:
    const float               window_size          = 840;
    uint32_t                  sub_steps          = 8;
    sf::Vector2f              gravity            = {0.0f, 1000.0f};
    sf::Vector2f              boundary_center;
    float                     boundary_radius    = 100.0f;
    std::vector<Particle>     objects;
    float                     time               = 0.0f;
    float                     frame_dt           = 0.0f;
    int                       grid_size            = 15;
    std::vector<Particle> grid[100][100];

    void applyGravity() {
        for (auto& obj : objects) {
            obj.accelerate(gravity);
        }
    }

    std::vector<int> getCollisionParticles (int particleID) {
        Particle p = objects[particleID];
        std::vector<int> res;
        sf::Vector2f top_left = {floor((p.position.x - p.radius) / grid_size), 
                                floor((p.position.y - p.radius) / grid_size)};
        sf::Vector2f bottom_right = {floor((p.position.x + p.radius) / grid_size), 
                                floor((p.position.y + p.radius) / grid_size)};
        for (int i = p.gridx - 1; i <= p.gridx + 1; i++)
            for (int j = p.gridy - 1; j <= p.gridy + 1; j++) {
                if (i < 0 || j < 0 || i >= 56 || j >= 56) continue;
                for (Particle& pp : grid[i][j])
                    if (pp.id != p.id) res.push_back(pp.id);
            }
        return res;
    }

    void collideCells (int x1, int y1, int x2, int y2) {
        const float    response_coef = 0.75f;
        for (Particle& object_1 : grid[x1][y1])
            for (Particle& object_2 : grid[x2][y2]) {
                if (object_1.id == object_2.id) continue;
                const sf::Vector2f v        = object_1.position - object_2.position;
                const float        dist     = Math::magnitude(v);
                const float        min_dist = object_1.radius + object_2.radius;
                // Check overlapping
                if (dist < min_dist) {
                    const sf::Vector2f n     = v / dist;
                    const float mass_ratio_1 = object_1.radius / (object_1.radius + object_2.radius);
                    const float mass_ratio_2 = object_2.radius / (object_1.radius + object_2.radius);
                    const float delta        = 0.5 * response_coef * (dist - min_dist);
                    // Update positions
                    object_1.position -= n * (mass_ratio_2 * delta);
                    object_2.position += n * (mass_ratio_1 * delta);
                }
            }
    }

    void checkCollisions (float dt) {
        const int dx[] = {1, 1, 1, 0, -1, -1, -1, 0, 0};
        const int dy[] = {1, 0, -1, -1, -1, 0, 1, 1, 0};
        // for (uint64_t i{1}; i < 56 - 1; ++i)
        //     for (uint64_t j{1}; j < 56 - 1; ++j)
        //         for (uint64_t k{0}; k <= 8; ++k) {
        //             int nx = i + dx[k], ny = j + dy[k];
        //             collideCells(i, j, nx, ny);
        //         }
        for (Particle& object_1 : objects) {
            for (int i : getCollisionParticles(object_1.id)) {
                Particle& object_2 = objects[i];
                if (object_1.id == object_2.id) continue;
                const sf::Vector2f v        = object_1.position - object_2.position;
                const float        dist     = Math::magnitude(v);
                const float        min_dist = object_1.radius + object_2.radius;
                // Check overlapping
                if (dist < min_dist) {
                    const sf::Vector2f n     = v / dist;
                    const float mass_ratio_1 = object_1.radius / (object_1.radius + object_2.radius);
                    const float mass_ratio_2 = object_2.radius / (object_1.radius + object_2.radius);
                    const float delta        = 0.5 * (dist - min_dist);
                    // Update positions
                    object_1.position -= n * (mass_ratio_2 * delta);
                    object_2.position += n * (mass_ratio_1 * delta);
                }
            }
        }
    }

    void applyBorder() {
        for (auto& obj : objects) {
            const float dampening = 0.75f;
            const sf::Vector2f pos  = obj.position;
            sf::Vector2f npos = obj.position;
            sf::Vector2f vel  = obj.getVelocity();
            // std::cout << pos.x << ' ' << pos.y << ' ' << vel.x << ' ' << vel.y << std::endl;
            sf::Vector2f dx = {vel.x * dampening, -vel.y};
            // sf::Vector2f dy = {-vel.x, vel.y * dampening};
            if (pos.x < obj.radius || pos.x + obj.radius > window_size) {
                
                if (pos.x < obj.radius) npos.x = obj.radius;
                if (pos.x + obj.radius > window_size) npos.x = window_size - obj.radius;
                // obj.accelerate(obj.position - npos);
                obj.position = npos;
                obj.setVelocity(dx, -1.0);
                vel  = obj.getVelocity();
                //1std::cout << "A " << pos.x << ' ' << pos.y << ' ' << vel.x << ' ' << vel.y << std::endl;
            }
            sf::Vector2f dy = {-vel.x, vel.y * dampening};
            if (pos.y < obj.radius || pos.y + obj.radius > window_size) {
                if (pos.y < obj.radius) npos.y = obj.radius;
                if (pos.y + obj.radius > window_size) npos.y = window_size - obj.radius;
                // obj.accelerate(npos - obj.position);
                obj.position = npos;
                obj.setVelocity(dy, -1.0);
                vel  = obj.getVelocity();
                // std::cout << "B " << pos.x << ' ' << pos.y << ' ' << vel.x << ' ' << vel.y << std::endl;
            }
            
        }
        // std::cout << std::endl;
    }

    void applyBoundary() {
        for (auto& obj : objects) {
            const sf::Vector2f    v = boundary_center - obj.position;
            const float        dist = Math::magnitude(v);
            if (dist > (boundary_radius - obj.radius)) {
                const sf::Vector2f n = v / dist;
                const sf::Vector2f vel = obj.getVelocity();
                const sf::Vector2f perp = {-n.y, n.x};
                obj.position = boundary_center - n * (boundary_radius - obj.radius);
                obj.setVelocity(2.0f * Math::dot_mag(vel, perp) * perp - vel, 1);
            }
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