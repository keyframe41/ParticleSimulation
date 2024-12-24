// Solver at the end of part 1

#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "../particle.hpp"

class Solver {
public:
    Solver() = default;

    Particle& addObject(sf::Vector2f position, float radius) {
        int gridx = position.x / grid_size, gridy = position.x / grid_size;
        Particle newParticle = Particle(position, radius, gridx, gridy, objects.size());
        grid[gridx][gridy].push_back(objects.size());
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


private:
    float                  window_size      = 840.0f;
    sf::Vector2f           gravity          = {0.0f, 1000.0f};
    sf::Vector2f           boundary_center  = {420.0f, 420.0f};
    float                  boundary_radius  = 100.0f; 
    std::vector<Particle>  objects;
    float                  step_dt          = 1.0f / 60;
    int                    sub_steps        = 8;
    int                    grid_size        = 12;
    std::vector<int>  grid[350][350];

    void applyBorder() {
        for (auto & obj : objects) {
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

    void collideCells (int x1, int y1, int x2, int y2) {
        for (int id_1 : grid[x1][y1]) {
            Particle& obj_1 = objects[id_1];
            for (int id_2 : grid[x2][y2]) {
                if (id_1 == id_2) continue;

                Particle& obj_2 = objects[id_2];
                sf::Vector2f v = obj_1.position - obj_2.position;
                float dist = sqrt(v.x * v.x + v.y * v.y);
                float min_dist = obj_1.radius + obj_2.radius;
                if (dist < min_dist) {
                    sf::Vector2f n = v / dist; // Normalize
                    float delta = 0.5f * (min_dist - dist);
                    // Larger particle moves less
                    obj_1.position += n * 0.5f * delta;
                    obj_2.position -= n * 0.5f * delta;
                }
            }
        }
    }

    void checkCollisions() {
        int num_cells = window_size / grid_size;
        int dx[] = {1, 1, 0, 0, -1};
        int dy[] = {0, 1, 0, 1, 1};
        for (int i = 0; i < num_cells; i++) {
            for (int j = 0; j < num_cells; j++) {
                if (!grid[i][j].size()) continue;
                for (int k = 0; k < 5; k++) {
                    int nx = i + dx[k], ny = j + dy[k];
                    if (nx < 0 || ny < 0 || nx >= num_cells || ny >= num_cells) continue;
                    collideCells(i, j, nx, ny);
                }
            }
        }
    }

    void applyGravity() {
        for (auto& obj : objects) {
            obj.accelerate(gravity);
        }
    }
    
    // void updateObjects(float dt) {
    //     for (int i = 0; i < 100; i++)
    //         for (int j = 0; j < 100; j++) grid[i][j].clear();
    //     for (auto& obj : objects) {
    //         obj.update(dt);
    //         grid[obj.gridx][obj.gridy].push_back(obj.id);
    //     }
    // }
    
    void updateObjects(float dt) {
        for (auto& obj : objects) {
            int cur_gridx = obj.gridx, cur_gridy = obj.gridy;
            obj.update(dt);
            obj.gridx = obj.position.x / grid_size;
            obj.gridy = obj.position.y / grid_size;

            if (cur_gridx != obj.gridx || cur_gridy != obj.gridy) {
                auto pos = find(grid[cur_gridx][cur_gridy].begin(), grid[cur_gridx][cur_gridy].end(), obj.id);
                grid[cur_gridx][cur_gridy].erase(pos);
                grid[obj.gridx][obj.gridy].push_back(obj.id);
            }
        }
    }
};