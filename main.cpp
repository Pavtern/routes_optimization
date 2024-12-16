#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_map>


const float OBSTACLE_RADIUS = 20;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int NUM_OBSTACLES = 40;
const float SCALE = 1.0f;
const int GRID_SIZE = 800;
const int NODE_SIZE = 3;

struct Node {
    int x = 0, y = 0;
    float f = 0, g = 0, h = 0.0f;
    Node* parent = nullptr;

    bool operator<(const Node& other) const {
        return f > other.f;
    }
};

std::vector<sf::Vector2f> obstacles;

float calculate_distance(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    float dx = point2.x - point1.x;
    float dy = point2.y - point1.y;
    return std::sqrt(dx * dx + dy * dy);
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

std::vector<sf::Vector2f> calculate_stops(sf::Vector2f a, sf::Vector2f b, float step = 5.0f) {
    std::vector<sf::Vector2f> stops;
    sf::Vector2f direction = b - a;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    sf::Vector2f unitDirection = direction / distance;

    for (float i = 0; i <= distance; i += step) {
        stops.push_back(a + unitDirection * i);
    }

    if (stops.empty() || stops.back() != b) {
        stops.push_back(b);
    }

    return stops;
}

std::vector<sf::Vector2f> generate_route(const sf::Vector2f& start, const sf::Vector2f& end) {
    std::vector<sf::Vector2f> route = calculate_stops(start, end);
    if (is_an_obstacle_too_close(route, obstacles) > OBSTACLE_RADIUS) {
        return route;
    }
    route.clear();
    int startX = static_cast<int>(start.x / NODE_SIZE);
    int startY = static_cast<int>(start.y / NODE_SIZE);
    int endX = static_cast<int>(end.x / NODE_SIZE);
    int endY = static_cast<int>(end.y / NODE_SIZE);

    auto heuristic = [&](int x, int y) -> float {
        return calculate_distance(sf::Vector2f(x, y), sf::Vector2f(endX, endY));
    };

    std::priority_queue<Node> openSet;
    std::vector closedSet(GRID_SIZE / NODE_SIZE, std::vector(GRID_SIZE / NODE_SIZE, false));

    Node startNode = { startX, startY, 0.0f, 0.0f, heuristic(startX, startY), nullptr };
    openSet.push(startNode);

    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        if (current.x == endX && current.y == endY) {
            Node* node = &current;
            while (node != nullptr) {
                route.emplace_back(node->x * NODE_SIZE + NODE_SIZE / 2, node->y * NODE_SIZE + NODE_SIZE / 2);
                node = node->parent;
            }
            std::reverse(route.begin(), route.end());
            break;
        }

        if (closedSet[current.x][current.y]) {
            continue;
        }
        closedSet[current.x][current.y] = true;

        std::vector<std::pair<int, int>> neighbors = {
            {current.x + 1, current.y}, {current.x - 1, current.y},
            {current.x, current.y + 1}, {current.x, current.y - 1},
            {current.x + 1, current.y + 1}, {current.x - 1, current.y - 1},
            {current.x + 1, current.y - 1}, {current.x - 1, current.y + 1}
        };

        for (auto& neighbor : neighbors) {
            int nx = neighbor.first;
            int ny = neighbor.second;

            if (nx < 0 || ny < 0 || nx >= GRID_SIZE / NODE_SIZE || ny >= GRID_SIZE / NODE_SIZE) {
                continue;
            }

            sf::Vector2f pos(nx * NODE_SIZE + NODE_SIZE / 2, ny * NODE_SIZE + NODE_SIZE / 2);
            bool obstacle = false;
            for (const auto& obs : obstacles) {
                if (calculate_distance(pos, obs) < OBSTACLE_RADIUS + 6) {
                    obstacle = true;
                    break;
                }
            }
            if (obstacle || closedSet[nx][ny]) {
                continue;
            }

            float gCost = current.g + calculate_distance(sf::Vector2f(current.x, current.y), sf::Vector2f(nx, ny));
            float hCost = heuristic(nx, ny);
            float fCost = gCost + hCost;

            Node neighborNode = { nx, ny, fCost, gCost, hCost, new Node(current) };
            openSet.push(neighborNode);
        }
    }

    return route;
}

void generate_random_points(std::vector<sf::Vector2f>& points, int count) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int width = WINDOW_WIDTH - 200;
    int height = WINDOW_HEIGHT - 250;
    for (int i = 0; i < count; ++i) {
        float x = std::rand() % width + 75;
        float y = std::rand() % height + 50;
        bool too_close = false;
        for (const auto& obs : obstacles) {
            if (calculate_distance(sf::Vector2f(x, y), obs) < OBSTACLE_RADIUS * 1.75f) {
                too_close = true;
                break;
            }
        }
        if (too_close) {
            i--;
            continue;
        }
        points.emplace_back(x, y);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Route Planner");
    window.clear(sf::Color::White);
    std::vector<sf::Vector2f> user_points;
    std::vector<sf::Vector2f> route_points;
    bool is_user_click_too_close = false;
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

    message.setFillColor(sf::Color::Black);
    point_shape.setFillColor(sf::Color::Red);
    obstacle_shape.setFillColor(sf::Color(0, 150, 0, 50));
    obstacle_point_shape.setFillColor(sf::Color::Black);
    obstacle_shape.setOutlineColor(sf::Color(0, 100, 0, 100));
    obstacle_shape.setOutlineThickness(2);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && user_points.size() < 2) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f user_click = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                    if (is_an_obstacle_too_close({user_click}, obstacles) > OBSTACLE_RADIUS + 10) {
                        user_points.emplace_back(user_click);
                        is_user_click_too_close = false;
                        message.setFillColor(sf::Color::Black);

                    } else {
                        message.setString("Obstacle is too close to start or to end point!");
                        message.setFillColor(sf::Color::Red);
                        is_user_click_too_close = true;
                        break;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::N) {
                    user_points.clear();
                    obstacles.clear();
                    route_points.clear();
                    generate_random_points(obstacles, NUM_OBSTACLES);
                    is_user_click_too_close = false;
                    message.setFillColor(sf::Color::Black);
                }
                if (event.key.code == sf::Keyboard::R && user_points.size() == 2) {
                    route_points = generate_route(user_points[0], user_points[1]);
                }
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
            if (route_points.empty()) {
                route_points = generate_route(user_points[0], user_points[1]);
            }
            for (size_t i = 1; i < route_points.size(); ++i) {
                sf::Vertex line[] = {
                    sf::Vertex(route_points[i - 1], sf::Color::Blue),
                    sf::Vertex(route_points[i], sf::Color::Blue)
                };
                window.draw(line, 2, sf::Lines);
                for (const auto& point : user_points) {
                    point_shape.setPosition(point.x - point_shape.getRadius(), point.y - point_shape.getRadius());
                    window.draw(point_shape);
                }
                float total_distance = calculate_distance(user_points[0], user_points[1]);
                message.setString("Route is set. Distance: " + std::to_string(static_cast<int>(total_distance * SCALE)) + " meters.");
            }
        } else if (user_points.size() == 1 && !is_user_click_too_close) {
            message.setString("Select the endpoint.");
        } else if (!is_user_click_too_close) {
            message.setString("Select the starting point.");
        }

        message.setPosition(10.f, WINDOW_HEIGHT - 40);
        window.draw(message);
        window.display();
    }

    return 0;
}
