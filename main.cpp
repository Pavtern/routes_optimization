#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

void generateRandomPoints(std::vector<sf::Vector2f>& points, int count, int width, int height) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < count; ++i) {
        float x = std::rand() % width;
        float y = std::rand() % height;
        points.emplace_back(x, y);
    }
}

float calculateDistance(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    float dx = point2.x - point1.x;
    float dy = point2.y - point1.y;
    return std::sqrt(dx * dx + dy * dy);
}

float calculateDistanceToLine(const sf::Vector2f& point, const sf::Vector2f& lineStart, const sf::Vector2f& lineEnd) {
    float dx = lineEnd.x - lineStart.x;
    float dy = lineEnd.y - lineStart.y;
    float numerator = std::abs(dy * point.x - dx * point.y + lineEnd.x * lineStart.y - lineEnd.y * lineStart.x);
    float denominator = std::sqrt(dy * dy + dx * dx);
    return numerator / denominator;
}

bool checkRouteProximity(const sf::Vector2f& start, const sf::Vector2f& end, const std::vector<sf::Vector2f>& obstacles, float threshold) {
    const int numSteps = 100;
    for (int i = 0; i <= numSteps; ++i) {
        float t = i / static_cast<float>(numSteps);
        sf::Vector2f pointOnLine = start + t * (end - start);

        for (const auto& obstacle : obstacles) {
            float distToLine = calculateDistanceToLine(obstacle, start, end);
            if (distToLine < threshold) {
                return true;  // Obstacle is too close to the route
            }
        }
    }
    return false;
}

void restartProgram() {
    std::cout << "Press 'N' to restart the program." << std::endl;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Route Planner");
    std::vector<sf::Vector2f> userPoints;
    std::vector<sf::Vector2f> randomPoints;
    sf::CircleShape pointShape(5);
    pointShape.setFillColor(sf::Color::Red);

    generateRandomPoints(randomPoints, 20, 700, 650);

    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
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

    sf::CircleShape obstaclePointShape(3);
    obstaclePointShape.setFillColor(sf::Color::Red);

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
                generateRandomPoints(randomPoints, 20, 700, 650);
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
            sf::Vertex line[] = {
                sf::Vertex(userPoints[0], sf::Color::Blue),
                sf::Vertex(userPoints[1], sf::Color::Blue)
            };
            window.draw(line, 2, sf::Lines);

            float distance = calculateDistance(userPoints[0], userPoints[1]);
            message.setString("Route is set. Distance: " + std::to_string(static_cast<int>(distance / 10)) + " meters.");

            bool needReplan = checkRouteProximity(userPoints[0], userPoints[1], randomPoints, 20.0f);

            if (needReplan) {
                message.setString("Obstacle too close! Distance < 200 meters. Replan the route?");
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
