#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "../particle.hpp"
#include "../thread.hpp"
#include "../obstacles/dot.hpp"
#include "../obstacles/box.hpp"

class Solver {
public:
    Solver(float size, float radius, Threader& threader_) 
        : window_size{size}
        , grid_size{2 * radius}
        , threader{threader_}
    {}

    virtual ~Solver() {
        for (Thread& thread : threader.threads) {
            thread.stop();
        }
    }

    Particle& addObject(sf::Vector2f position, float radius) {
        int gridx = position.x / grid_size, gridy = position.x / grid_size;
        Particle newParticle = Particle(position, radius, gridx, gridy, objects.size());
        grid[gridx][gridy].push_back(newParticle.id);
        return objects.emplace_back(newParticle);
    }

    ObstacleDot& addObstacleDot(float radius, sf::Vector2f start_position, 
            sf::Vector2f end_position = {-1.0f, -1.0f}) {
        // end position left empty signifies no movement
        if (end_position == sf::Vector2f{-1.0f, -1.0f}) end_position = start_position;
        ObstacleDot newDot = ObstacleDot(start_position, end_position, radius);
        return dot_obstacles.emplace_back(newDot);
    }

    ObstacleBox& addObstacleBox(
        sf::Vector2f dimensions, 
        sf::Vector2f start_position, 
        sf::Vector2f end_position = {-1.0f, 0.0f}) {
        if (end_position.x == -1.0f) end_position = start_position;
        ObstacleBox newBox = ObstacleBox(dimensions, start_position, end_position);
        return box_obstacles.emplace_back(newBox);
    }

    void mousePull(sf::Vector2f pos, float radius) {
        for (Particle& obj : objects) {
            sf::Vector2f dir = pos - obj.position;
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            obj.accelerate(dir * std::max(0.0f, 5 * (radius - dist)));
        }
    }

    void mousePush(sf::Vector2f pos, float radius) {
        for (Particle& obj : objects) {
            sf::Vector2f dir = pos - obj.position;
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            obj.accelerate(dir * std::min(0.0f, -5 * (radius - dist)));
        }
    }

    void update() {
        float substep_dt = step_dt / sub_steps;
        for (int i = 0; i < sub_steps; i++) {
            applyGravity();
            checkCollisions();
            checkDotCollisions();
            checkBoxCollisions();
            applyBorder();
            updateObjects(substep_dt);
            updateObstacles(substep_dt);
            updateGrid();
        }
    }

    void setObjectVelocity(Particle& object, sf::Vector2f vel) {
        object.setVelocity(vel, step_dt / sub_steps);
    }

    float                    window_size      = 1260.0f;
    sf::Vector2f             gravity          = {0.0f, 150.0f}; // 250
    std::vector<Particle>    objects;
    std::vector<ObstacleDot> dot_obstacles;

    std::vector<ObstacleBox> box_obstacles;

    float                    step_dt          = 1.0f / 60;
    int                      sub_steps        = 8;

    float                    grid_size        = 16;
    std::vector<int>         grid[350][350];

    Threader&                threader;

    void bounceOffBorder(int obj_id) {
        Particle&          obj = objects[obj_id];
        const sf::Vector2f pos = obj.position;
        sf::Vector2f      npos = obj.position;
        sf::Vector2f       vel = obj.getVelocity();

        sf::Vector2f dy = { vel.x, -vel.y};
        sf::Vector2f dx = {-vel.x,  vel.y};
        const float dampening = 1.0f;

        if (pos.x < grid_size || pos.x > window_size - grid_size) { // Bounce off left/right
            if (pos.x < grid_size) npos.x = grid_size;
            if (pos.x > window_size - grid_size) npos.x = window_size - grid_size;
            obj.position = npos;
            obj.setVelocity(dx * dampening, 1.0);
        }
        if (pos.y < grid_size || pos.y > window_size - grid_size) { // Bounce off top/bottom
            if (pos.y < grid_size) npos.y = grid_size;
            if (pos.y > window_size - grid_size) npos.y = window_size - grid_size;
            obj.position = npos;
            obj.setVelocity(dy * dampening, 1.0);
        }
    }

    void applyBorder() {
        threader.parallel(objects.size(), [&](int start, int end) {
            for (int i = start; i < end; i++) bounceOffBorder(i);
        });
    }

    void collideCells (int x1, int y1, int x2, int y2) {
        for (int id_1 : grid[x1][y1]) {
            Particle& obj_1 = objects[id_1];
            for (int id_2 : grid[x2][y2]) {
                if (id_1 == id_2) continue;

                Particle& obj_2 = objects[id_2];
                sf::Vector2f v = obj_1.position - obj_2.position;
                float dist     = v.x * v.x + v.y * v.y;
                float min_dist = grid_size;

                if (dist < min_dist * min_dist) {
                    dist = sqrt(dist);
                    float delta = 0.25f * (min_dist - dist);
                    sf::Vector2f n = v / dist * delta;
                    // Larger particle moves less
                    obj_1.position += n;
                    obj_2.position -= n;
                }
            }
        }
    }

    void checkCollisionsInSlice (int lcol, int rcol) {
        int num_cells = window_size / grid_size;
        int      dx[] = {1, 1, 0, 0, -1};
        int      dy[] = {0, 1, 0, 1, 1};
        for (int i = lcol; i < rcol; i++) {
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

    void checkCollisions () {
        
        int num_cells   = window_size / grid_size;
        int slice_count = threader.num_threads * 2;
        int slice_size  = num_cells / slice_count;

        // Left pass
        for (int i = 0; i < threader.num_threads; i++) {
            threader.t_queue.addTask([this, i, slice_size]{
                int start = 2 * i * slice_size;
                int end = start + slice_size;
                checkCollisionsInSlice(start, end);
            }); 
        }
        if (slice_count * slice_size < num_cells) {
            threader.t_queue.addTask([this, slice_count, slice_size, num_cells]{
                checkCollisionsInSlice(slice_count * slice_size, num_cells);
            }); 
        }
        threader.t_queue.waitUntilDone();
        // Right pass
        for (int i = 0; i < threader.num_threads; i++) {
            threader.t_queue.addTask([this, i, slice_size]{
                int start = (2 * i + 1) * slice_size;
                int end = start + slice_size;
                checkCollisionsInSlice(start, end);
            }); 
        }
        threader.t_queue.waitUntilDone();
    }

    void dotBounce (int obj_id, sf::Vector2f pos, float radius) {
        Particle& obj = objects[obj_id];
        const sf::Vector2f displacement = pos - obj.position;
        const float dist = displacement.x * displacement.x + displacement.y * displacement.y;
        const float min_dist = obj.radius + radius;
        if (dist < min_dist * min_dist) {
            const sf::Vector2f norm = displacement / sqrt(dist);
            const sf::Vector2f perp = {-norm.y, norm.x};
            const sf::Vector2f vel = obj.getVelocity();
            obj.position = pos - norm * min_dist;
            obj.setVelocity(2.0f * (vel.x * perp.x + vel.y * perp.y) * perp - vel, 1.0f);
        }
    }

    void checkDotCollisions () {
        int num_cells = window_size / grid_size;
        for (ObstacleDot& dot : dot_obstacles) {
            const sf::Vector2f center = dot.position;
            const float offset = dot.radius + grid_size;
            
            int left = (dot.position.x - offset) / grid_size;
            int right = (dot.position.x + offset) / grid_size;
            int top = (dot.position.y - offset) / grid_size;
            int bottom = (dot.position.y + offset) / grid_size;
            for (int i = left; i <= right; i++) {
                if (i < 0 || i >= num_cells) continue;
                for (int j = top; j <= bottom; j++) {
                    if (j < 0 || j >= num_cells) continue;
                    for (int obj_id : grid[i][j]) dotBounce(obj_id, center, dot.radius);
                }
            }
        }
    }


     void checkBoxCollisions () {
        int num_cells = window_size / grid_size;

        for (ObstacleBox& box : box_obstacles) {
            // Get limits
            sf::Transform clockwise, anticlockwise;
            clockwise.rotate(box.rotation);
            anticlockwise.rotate(-box.rotation);
            const sf::Vector2f size = box.dimensions * 0.5f;
            const sf::Vector2f center = box.position;

            sf::Vector2f top_left     = anticlockwise.transformPoint(-size.x, -size.y);
            sf::Vector2f top_right    = anticlockwise.transformPoint( size.x, -size.y);
            sf::Vector2f bottom_left  = anticlockwise.transformPoint(-size.x,  size.y);
            sf::Vector2f bottom_right = anticlockwise.transformPoint( size.x,  size.y);

            float upper_limit = std::min(std::min(top_left.y, top_right.y), std::min(bottom_left.y, bottom_right.y));
            float lower_limit = std::max(std::max(top_left.y, top_right.y), std::max(bottom_left.y, bottom_right.y));
            float left_limit  = std::min(std::min(top_left.x, top_right.x), std::min(bottom_left.x, bottom_right.x));
            float right_limit = std::max(std::max(top_left.x, top_right.x), std::max(bottom_left.x, bottom_right.x));

            upper_limit = floor((upper_limit + center.y) / grid_size) - 1;
            lower_limit =  ceil((lower_limit + center.y) / grid_size) + 1;
            left_limit  = floor((left_limit  + center.x) / grid_size) - 1;
            right_limit =  ceil((right_limit + center.x) / grid_size) + 1;

            // Check particles
            for (int i = left_limit; i <= right_limit; i++) {
                if (i < 0 || i >= num_cells) continue;
                for (int j = upper_limit; j <= lower_limit; j++) {
                    if (j < 0 || j >= num_cells) continue;
                    for (int obj_id : grid[i][j]) {

                        dotBounce(obj_id, top_left, 0.0f);
                        dotBounce(obj_id, top_right, 0.0f);
                        dotBounce(obj_id, bottom_left, 0.0f);
                        dotBounce(obj_id, bottom_right, 0.0f);

                        Particle& obj = objects[obj_id];
                        const float  radius = obj.radius;
                        sf::Vector2f pos    = obj.position;
                        sf::Vector2f vel    = obj.getVelocity();
                        sf::Vector2f rotpos = anticlockwise.transformPoint(pos - center);
                        sf::Vector2f rotvel = anticlockwise.transformPoint(vel);

                        // Top edge
                        if ((-size.y - radius < rotpos.y && rotpos.y < 0) &&
                         (-size.x < rotpos.x && rotpos.x < size.x)) {
                            rotpos.y = -size.y - radius;
                            if (rotvel.y > 0) rotvel.y *= -1;
                        }
                        // Bottom edge
                        if ((0 < rotpos.y && rotpos.y < size.y + radius) &&
                         (-size.x < rotpos.x && rotpos.x < size.x)) {
                            rotpos.y = size.y + radius;
                            if (rotvel.y < 0) rotvel.y *= -1;
                        }   
                        // Left edge
                        if ((-size.x - radius < rotpos.x && rotpos.x < 0) &&
                         (-size.y < rotpos.y && rotpos.y < size.y)) {
                            rotpos.x = -size.x - radius;
                            if (rotvel.x > 0) rotvel.x *= -1;
                        }
                        // Right edge
                        if ((0 < rotpos.x && rotpos.x < size.x + radius) &&
                         (-size.y < rotpos.y && rotpos.y < size.y)) {
                            rotpos.x = size.x + radius;
                            if (rotvel.x < 0) rotvel.x *= -1;
                        }

                        obj.position = clockwise.transformPoint(rotpos) + center;
                        obj.setVelocity(clockwise.transformPoint(rotvel), 1.0f);
                    }
                }
            }
        }
    }

    void applyGravity() {
        for (auto& obj : objects) {
            obj.accelerate(gravity);
        }
    }
    
    void updateObjectsThreaded (int start, int end, float dt) {
        for (int i = start; i < end; i++) {
            Particle& obj = objects[i];
            int cur_gridx = obj.gridx, cur_gridy = obj.gridy;
            obj.update(dt);
            obj.gridx = obj.position.x / grid_size;
            obj.gridy = obj.position.y / grid_size;
            sf::Vector2f vel = obj.getVelocity();
            if (vel.x * vel.x + vel.y * vel.y > grid_size) obj.setVelocity({0.0f, 0.0f}, 1.0);
        }
    }

    void updateObjects (float dt) {
        threader.parallel(objects.size(), [&](int start, int end) {
            updateObjectsThreaded(start, end, dt);
        });
    }

    void updateObstacles(float dt) {
        for (auto& dot : dot_obstacles) {
            dot.update(dt);
        }
        for (auto& box : box_obstacles) {
            box.update(dt);
        }
    }

    void updateGrid() {
        int num_cells   = window_size / grid_size;
        for (int i = 0; i < num_cells; i++)
            for (int j = 0; j < num_cells; j++)
                grid[i][j].clear();

        for (Particle& obj : objects) {
            if (obj.gridx < 0 || obj.gridy < 0 || obj.gridx >= num_cells && obj.gridy >= num_cells) continue;
            grid[obj.gridx][obj.gridy].push_back(obj.id);
        }
    }
};