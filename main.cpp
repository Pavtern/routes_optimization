#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>

void generateRandomPoints(std::vector<sf::Vector2f>& points, int count, int width, int height) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < count; ++i) {
        float x = std::rand() % width + 75;
        float y = std::rand() % height + 50;
        points.emplace_back(x, y);
    }
}

float calculateDistance(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    float dx = point2.x - point1.x;
    float dy = point2.y - point1.y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<sf::Vector2f> generateRoute(const sf::Vector2f& start, const sf::Vector2f& end) {
    std::vector<sf::Vector2f> route;
    sf::Vector2f direction = end - start;
    float length = calculateDistance(start, end);
    sf::Vector2f step = direction / length;

    for (float i = 0; i <= length; ++i) {
        sf::Vector2f point = start + step * i;
        route.push_back(point);
    }
    return route;
}

float findMinimumDistance(const std::vector<sf::Vector2f>& route, const std::vector<sf::Vector2f>& obstacles) {
    float minDistance = std::numeric_limits<float>::max();
    for (const auto& routePoint : route) {
        for (const auto& obstacle : obstacles) {
            float distance = calculateDistance(routePoint, obstacle);
            if (distance < minDistance) {
                minDistance = distance;
            }
        }
    }
    return minDistance;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Route Planner");
    std::vector<sf::Vector2f> userPoints;
    std::vector<sf::Vector2f> randomPoints;
    std::vector<sf::Vector2f> routePoints;
    sf::CircleShape pointShape(5);
    pointShape.setFillColor(sf::Color::Red);

    generateRandomPoints(randomPoints, 20, 600, 550);

    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        return -1;
    }

    sf::Text message;
    message.setFont(font);
    message.setCharacterSize(24);
    message.setFillColor(sf::Color::White);

    sf::CircleShape obstacleShape(20);
    obstacleShape.setFillColor(sf::Color(0, 255, 0, 100));
    obstacleShape.setOutlineColor(sf::Color::Green);
    obstacleShape.setOutlineThickness(2);

    sf::CircleShape obstaclePointShape(5);
    obstaclePointShape.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && userPoints.size() < 2) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    userPoints.emplace_back(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) {
                userPoints.clear();
                randomPoints.clear();
                routePoints.clear();
                generateRandomPoints(randomPoints, 20, 600, 550);
            }
        }

        window.clear();
        for (const auto& point : randomPoints) {
            obstacleShape.setPosition(point.x - obstacleShape.getRadius(), point.y - obstacleShape.getRadius());
            window.draw(obstacleShape);
            obstaclePointShape.setPosition(point.x - obstaclePointShape.getRadius(), point.y - obstaclePointShape.getRadius());
            window.draw(obstaclePointShape);
        }
        for (const auto& point : userPoints) {
            pointShape.setFillColor(sf::Color::Red);
            pointShape.setPosition(point.x - pointShape.getRadius(), point.y - pointShape.getRadius());
            window.draw(pointShape);
        }

        if (userPoints.size() == 2) {
            routePoints = generateRoute(userPoints[0], userPoints[1]);
            sf::Vertex line[] = {
                sf::Vertex(userPoints[0], sf::Color::Blue),
                sf::Vertex(userPoints[1], sf::Color::Blue)
            };
            window.draw(line, 2, sf::Lines);

            float minDistance = findMinimumDistance(routePoints, randomPoints);
            if (minDistance < 20.0f) {
                message.setString("Obstacle too close! Replan route? Press 'N' to restart.");
            } else {
                float totalDistance = calculateDistance(userPoints[0], userPoints[1]);
                message.setString("Route is set. Distance: " + std::to_string(static_cast<int>(totalDistance / 10)) + " meters.");
            }
        } else if (userPoints.size() == 1) {
            message.setString("Select the endpoint.");
        } else {
            message.setString("Select the starting point.");
        }

        message.setPosition(10.f, 760.f);
        window.draw(message);

        window.display();
    }

    return 0;
}
