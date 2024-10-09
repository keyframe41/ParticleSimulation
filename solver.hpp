#pragma once
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "utils/math.hpp"

struct Particle {
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float radius = 10.0f;
    sf::Color color = sf::Color::White;

    Particle() = default;
    Particle(sf::Vector2f position_, float radius_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
    {}

    void update(float dt) {
        // Compute how much we moved
        const sf::Vector2f displacement = position - position_last;
        // Update position
        position_last = position;
        position      = position + displacement + acceleration * (dt * dt);
        // Reset acceleration
        acceleration  = {};
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
        return m_objects.emplace_back(position, radius);
    }

    void update() {
        m_time += m_frame_dt;
        const float step_dt = getStepDt();
        for (uint32_t i{m_sub_steps}; i--;) {
            applyGravity();
            checkCollisions(step_dt);
            applyBoundary();
            updateObjects(step_dt);
        }
    }

    void setSimulationUpdateRate(uint32_t rate) {
        m_frame_dt = 1.0f / static_cast<float>(rate);
    }

    void setBoundary(sf::Vector2f position, float radius) {
        m_boundary_center = position;
        m_boundary_radius = radius;
    }

    void setSubStepsCount(uint32_t sub_steps) {
        m_sub_steps = sub_steps;
    }

    void setObjectVelocity(Particle& object, sf::Vector2f v) {
        object.setVelocity(v, getStepDt());
    }

    const std::vector<Particle>& getObjects() const {
        return m_objects;
    }

    sf::Vector3f getBoundary() const {
        return {m_boundary_center.x, m_boundary_center.y, m_boundary_radius};
    }

    uint64_t getObjectsCount() const {
        return m_objects.size();
    }

    float getTime() const {
        return m_time;
    }

    float getStepDt() const {
        return m_frame_dt / static_cast<float>(m_sub_steps);
    }

private:
    uint32_t                  m_sub_steps          = 1;
    sf::Vector2f              m_gravity            = {0.0f, 800.0f};
    sf::Vector2f              m_boundary_center;
    float                     m_boundary_radius  = 100.0f;
    std::vector<Particle> m_objects;
    float                     m_time               = 0.0f;
    float                     m_frame_dt           = 0.0f;

    void applyGravity() {
        for (auto& obj : m_objects) {
            obj.accelerate(m_gravity);
        }
    }

    void checkCollisions(float dt) {
        const float    response_coef = 0.75f;
        const uint64_t objects_count = m_objects.size();
        // Iterate on all objects
        for (uint64_t i{0}; i < objects_count; ++i) {
            Particle& object_1 = m_objects[i];
            // Iterate on object involved in new collision pairs
            for (uint64_t k{i + 1}; k < objects_count; ++k) {
                Particle&      object_2 = m_objects[k];
                const sf::Vector2f v        = object_1.position - object_2.position;
                const float        dist    = Math::magnitude(v);
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
    }

    void applyBoundary() {
        for (auto& obj : m_objects) {
            const sf::Vector2f    v = m_boundary_center - obj.position;
            const float        dist = Math::magnitude(v);
            if (dist > (m_boundary_radius - obj.radius)) {
                const sf::Vector2f n = v / dist;
                const sf::Vector2f vel = obj.getVelocity();
                const sf::Vector2f perp = {-n.y, n.x};
                obj.position = m_boundary_center - n * (m_boundary_radius - obj.radius);
                obj.setVelocity(vel - 2 * Math::dot_mag(vel, perp) * perp, -1);
            }
        }
    }

    void updateObjects(float dt) {
        for (auto& obj : m_objects) {
            obj.update(dt);
        }
    }
};