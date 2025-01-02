#pragma once
#include "../solvers/solver_final.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include "../thread.hpp"

class Renderer {
public:

    Renderer(sf::RenderWindow& window_, Threader& threader_, Solver& solver_)
        : target{window_}
        , solver{solver_}
        , threader{threader_}
    {
        obj_texture.loadFromFile("/Users/richard/Desktop/SFMLExperiments/circle.png");
        obj_texture.generateMipmap();
        obj_texture.setSmooth(true);
    }

    void newRender() {
        target.clear(sf::Color::Black);
        updateVA();

        target.draw(box_va);
        target.draw(trail_va);
        
        sf::RenderStates states;
        states.texture = &obj_texture;
        target.draw(dot_va, states);
        target.draw(obj_va, states);
    }

    void updateVA() {
        obj_va.resize(solver.objects.size() * 4);
        const float tex_size = 1024.0f;
        const float radius = solver.objects[0].radius;
        
        threader.parallel(solver.objects.size(), [&](int start, int end) {
            for (int i = start; i < end; i++) {
                const Particle& obj = solver.objects[i];
                const int id = i * 4;
                obj_va[id    ].position = obj.position + sf::Vector2f{-radius, -radius};
                obj_va[id + 1].position = obj.position + sf::Vector2f{ radius, -radius};
                obj_va[id + 2].position = obj.position + sf::Vector2f{ radius,  radius};
                obj_va[id + 3].position = obj.position + sf::Vector2f{-radius,  radius}; 

                obj_va[id    ].texCoords = {0.0f, 0.0f};
                obj_va[id + 1].texCoords = {tex_size, 0.0f};
                obj_va[id + 2].texCoords = {tex_size, tex_size};
                obj_va[id + 3].texCoords = {0.0f, tex_size};

                const sf::Color color = obj.color;
                obj_va[id    ].color = color;
                obj_va[id + 1].color = color;
                obj_va[id + 2].color = color;
                obj_va[id + 3].color = color;

                
            }
        });

        dot_va.resize(solver.dot_obstacles.size() * 4);
        for (int i = 0; i < solver.dot_obstacles.size(); i++) {
            const int id = i * 4;
            const ObstacleDot& obj = solver.dot_obstacles[i];
            const float obj_rad = obj.radius;
            sf::Color   color   = obj.color;
            dot_va[id    ].position = obj.position + sf::Vector2f{-obj_rad, -obj_rad};
            dot_va[id + 1].position = obj.position + sf::Vector2f{ obj_rad, -obj_rad};
            dot_va[id + 2].position = obj.position + sf::Vector2f{ obj_rad,  obj_rad};
            dot_va[id + 3].position = obj.position + sf::Vector2f{-obj_rad,  obj_rad}; 

            dot_va[id    ].texCoords = {0.0f, 0.0f};
            dot_va[id + 1].texCoords = {tex_size, 0.0f};
            dot_va[id + 2].texCoords = {tex_size, tex_size};
            dot_va[id + 3].texCoords = {0.0f, tex_size};
            
            dot_va[id    ].color = color;
            dot_va[id + 1].color = color;
            dot_va[id + 2].color = color;
            dot_va[id + 3].color = color;
        }

        box_va.resize(solver.box_obstacles.size() * 4);
        for (int i = 0; i < solver.box_obstacles.size(); i++) {
            const int id = i * 4;
            const ObstacleBox& obj = solver.box_obstacles[i];
            const sf::Vector2f size = obj.dimensions * 0.5f;
            sf::Color         color = obj.color;
            sf::Transform rotation;
            rotation.rotate(obj.rotation);
            box_va[id    ].position = obj.position + rotation.transformPoint(-size.x, -size.y);
            box_va[id + 1].position = obj.position + rotation.transformPoint( size.x, -size.y);
            box_va[id + 2].position = obj.position + rotation.transformPoint( size.x,  size.y);
            box_va[id + 3].position = obj.position + rotation.transformPoint(-size.x,  size.y);

            if (obj.breakable) color.a = obj.durability * 255 / obj.total_dur;
            box_va[id    ].color = color;
            box_va[id + 1].color = color;
            box_va[id + 2].color = color;
            box_va[id + 3].color = color;
        }
    }

    void updateTrailVA() {
        trail_va.resize(solver.objects.size() * 4);
        const float radius = solver.objects[0].radius * 0.6;

        threader.parallel(solver.objects.size(), [&](int start, int end) {
            for (int i = start; i < end; i++) {
                const Particle& obj = solver.objects[i];
                const int id = i * 4;
                sf::Vector2f disp = obj.position - obj.position_last;
                sf::Vector2f back = disp * 20.0f;
                disp = {-disp.y, disp.x};
                disp /= sqrt(disp.x * disp.x + disp.y * disp.y);
                disp *= radius;

                trail_va[id    ].position = obj.position + disp;
                trail_va[id + 1].position = obj.position - disp;
                trail_va[id + 2].position = obj.position - back - disp;
                trail_va[id + 3].position = obj.position - back + disp;

                sf::Color color = obj.color;
                trail_va[id    ].color = color;
                trail_va[id + 1].color = color;
                color.a = 0;
                trail_va[id + 2].color = color;
                trail_va[id + 3].color = color;
            }
        });
    }

private:
    sf::RenderWindow&        target;
    Solver&                  solver;
    Threader&          threader;

    sf::Texture     obj_texture;
    sf::VertexArray obj_va{sf::Quads};
    sf::VertexArray trail_va{sf::Quads};
    sf::VertexArray dot_va{sf::Quads};
    sf::VertexArray box_va{sf::Quads};
};