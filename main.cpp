#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <limits>

const float OBSTACLE_RADIUS = 20;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
std::vector<sf::Vector2f> obstacles;
const int NUM_OBSTACLES = 40;


float calculate_distance(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    float dx = point2.x - point1.x;
    float dy = point2.y - point1.y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<sf::Vector2f> generate_route(const sf::Vector2f& start, const sf::Vector2f& end) {
    std::vector<sf::Vector2f> route;
    sf::Vector2f direction = end - start;
    float length = calculate_distance(start, end);
    sf::Vector2f step = direction / length;

    for (float i = 0; i <= length; ++i) {
        sf::Vector2f point = start + step * i;
        route.push_back(point);
    }
    return route;
}

float is_an_obstacle_too_close(const std::vector<sf::Vector2f>& route, const std::vector<sf::Vector2f>& obstacles) {
    float distance = 1000.f;
    for (const auto& routePoint : route) {
        for (const auto& obstacle : obstacles) {
            float temp_distance = calculate_distance(routePoint, obstacle);
            if (distance > temp_distance) {
              distance = temp_distance;
            }
            if (distance < OBSTACLE_RADIUS) {
                return distance;
            }
        }
    }
    return distance;
}

void generate_random_points(std::vector<sf::Vector2f>& points, int count) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int width = WINDOW_WIDTH - 200;
    int height = WINDOW_HEIGHT - 250;
    for (int i = 0; i < count; ++i) {
        float x = std::rand() % width + 75;
        float y = std::rand() % height + 50;
        if (is_an_obstacle_too_close({{x,y}}, obstacles) < OBSTACLE_RADIUS * 1.75) {
            i--;
            continue;
        }
        points.emplace_back(x, y);
    }
}

std::vector<sf::Vector2f> reroute(const sf::Vector2f& start, const sf::Vector2f& end, const std::vector<sf::Vector2f>& obstacles) {
    std::vector<sf::Vector2f> route;
    sf::Vector2f currentPoint = start;

    while (calculate_distance(currentPoint, end) > 1.0f) {
        sf::Vector2f direction = end - currentPoint;
        float length = calculate_distance(currentPoint, end);
        sf::Vector2f step = direction / length;

        // Check for obstacle proximity
        bool obstacleNearby = false;
        for (const auto& obstacle : obstacles) {
            if (calculate_distance(currentPoint + step, obstacle) < OBSTACLE_RADIUS) {
                obstacleNearby = true;

                // Detour around the obstacle
                sf::Vector2f perpendicular(-step.y, step.x);
                sf::Vector2f detour = currentPoint + perpendicular * (OBSTACLE_RADIUS * 1.5f);

                route.push_back(detour);
                currentPoint = detour;
                break;
            }
        }

        // If no obstacle nearby, continue in a straight line
        if (!obstacleNearby) {
            currentPoint += step;
            route.push_back(currentPoint);
        }
    }

    return route;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Route Planner");
    window.clear(sf::Color::White);
    std::vector<sf::Vector2f> user_points;
    std::vector<sf::Vector2f> route_points;
    bool is_rerouting = false;

    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        return -1;
    }

    sf::Text message;
    message.setFont(font);
    message.setCharacterSize(24);

    generate_random_points(obstacles, NUM_OBSTACLES);

    sf::CircleShape obstacle_point_shape(3);
    sf::CircleShape point_shape(5);
    sf::CircleShape obstacle_shape(OBSTACLE_RADIUS);
    sf::CircleShape route_shape(1);

    route_shape.setFillColor(sf::Color::Black);
    message.setFillColor(sf::Color::Black);
    point_shape.setFillColor(sf::Color::Red);
    obstacle_shape.setFillColor(sf::Color(0, 150, 0, 50));
    obstacle_point_shape.setFillColor(sf::Color::Black);
    obstacle_shape.setOutlineColor(sf::Color(0,100,0,100));
    obstacle_shape.setOutlineThickness(2);


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && user_points.size() < 2) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    user_points.emplace_back(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) {
                is_rerouting = false;
                user_points.clear();
                obstacles.clear();
                route_points.clear();
                generate_random_points(obstacles, NUM_OBSTACLES);
            }
            if (event.key.code == sf::Keyboard::R && user_points.size() == 2) {
                route_points = reroute(user_points[0], user_points[1], obstacles);
                is_rerouting = true;
            }
        }

        window.clear(sf::Color::White);
        for (const auto& point : obstacles) {
            obstacle_shape.setPosition(point.x - obstacle_shape.getRadius(), point.y - obstacle_shape.getRadius());
            window.draw(obstacle_shape);
            obstacle_point_shape.setPosition(point.x - obstacle_point_shape.getRadius(), point.y - obstacle_point_shape.getRadius());
            window.draw(obstacle_point_shape);
        }
        for (const auto& point : user_points) {
            point_shape.setFillColor(sf::Color::Red);
            point_shape.setPosition(point.x - point_shape.getRadius(), point.y - point_shape.getRadius());
            window.draw(point_shape);
        }

        if (user_points.size() == 2) {
            if (is_rerouting) {
                for (size_t i = 1; i < route_points.size(); ++i) {
                    sf::Vertex line[] = {
                        sf::Vertex(route_points[i - 1], sf::Color::Blue),
                        sf::Vertex(route_points[i], sf::Color::Blue)
                    };
                    window.draw(line, 2, sf::Lines);
                }
            } else {
                route_points = generate_route(user_points[0], user_points[1]);
                sf::Vertex line[] = {
                    sf::Vertex(user_points[0], sf::Color::Blue),
                    sf::Vertex(user_points[1], sf::Color::Blue)
                };
                window.draw(line, 2, sf::Lines);
                is_rerouting = false;
            }


            if (is_an_obstacle_too_close(route_points, obstacles) < OBSTACLE_RADIUS) {
                message.setString("Obstacle too close! Press 'R' to replan route? Press 'N' to restart.");
            } else {
                float total_distance = calculate_distance(user_points[0], user_points[1]);
                message.setString("Route is set. Distance: " + std::to_string(static_cast<int>(total_distance / 10))
                    + " meters. Press 'N' to restart.");
            }
        } else if (user_points.size() == 1) {
            message.setString("Select the endpoint.");
        } else {
            message.setString("Select the starting point.");
        }

        message.setPosition(10.f, WINDOW_HEIGHT - 40);
        window.draw(message);

        window.display();
    }

    return 0;
}
