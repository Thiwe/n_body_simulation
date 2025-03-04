#include <iostream>
#include <string>
#include <vector>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SFML/Graphics.hpp>

class SimulationManager {
public:
    SimulationManager(int screenWidth, int screenHeight, float minRadius, float maxRadius, float spawnLimit, float gravity, float spawnPositionX, float spawnPositionY)
        : screenWidth(screenWidth),
        screenHeight(screenHeight),
        minRadius(minRadius),
        maxRadius(maxRadius),
        spawnLimit(spawnLimit),
        gravity(gravity),
        spawnPositionX(spawnPositionX),
        spawnPositionY(spawnPositionY) {}

    float getRandRadius() {
        // Thread-local to avoid contention in multithreaded code
        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(minRadius, maxRadius);
        return distribution(generator);
    };

public: //should ´be private
    int screenWidth;
    int screenHeight;
    float minRadius;
    float maxRadius;
    float spawnLimit;
    float gravity;
    float spawnPositionX;
    float spawnPositionY;
};

class Entity {
public:
    Entity(SimulationManager* manager) 
        : manager(manager), radius(manager->getRandRadius())
    {
        mass *= radius;
        angle = sf::Angle(sf::radians(3.14));
        shape = sf::CircleShape(radius);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(sf::Vector2f({ manager->spawnPositionX, manager->spawnPositionY }));
    }


    void update(float deltatime);
    void render(sf::RenderWindow& window);


    void borderCollision();

    void applyGravity();

private:
    SimulationManager* manager = nullptr;
    sf::Vector2f pos;
    sf::Vector2f vel;
    float radius;
    sf::Transform transform;
    sf::CircleShape shape;
    sf::Angle angle;
    float mass = 1;

};


class threadPool {

};

//Window size, min/max circle radius values, spawn limit, and gravity are given as command line arguments to the program.
int main(int argc, char* argv[]) {
    if (argc < 9) {
        std::cout << "Usage: " << argv[0] << " <window_size> <spawn_position> <input_variable>" << std::endl;
        return 1;
    }

    size_t screenWidth = std::stoi(argv[1]);
    size_t screenHeight = std::stoi(argv[2]);
    float minRadius = std::stof(argv[3]);
    float maxRadius = std::stof(argv[4]);
    float spawnLimit = std::stof(argv[5]);
    float gravity = std::stof(argv[6]);
    float spawnPositionX = std::stof(argv[7]);
    float spawnPositionY = std::stof(argv[8]);

    sf::RenderWindow window(sf::VideoMode({ screenWidth, screenHeight }), "SFML works!");
   
    sf::Clock clock;

    SimulationManager simManager(screenWidth, screenHeight, minRadius, maxRadius, spawnLimit, gravity, spawnPositionX, spawnPositionY);
    
    std::vector<Entity*> preSpawnedEntities;
    for (int i = 0; i <= spawnLimit; i++) {
        preSpawnedEntities.push_back(new Entity(&simManager));
    }

    std::vector<Entity*> entities;
    entities.push_back(preSpawnedEntities[0]);
    float initSpawnIntervall = 1;
    float spawnIntervall = initSpawnIntervall;
    int indexEntityToSpawn = 1; //start at one since we already spoawned 0

    while (window.isOpen())
    {
        //restart() returns a time object which ahve asSeconds as member
        float deltaTime = clock.restart().asSeconds();
        
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();
        for (auto entity : entities) {
            entity->update(deltaTime);
            entity->render(window);
        }
        if (spawnIntervall < 0) {
            std::cout << "somehting is spawning" << std::endl;
            spawnIntervall = initSpawnIntervall;
            if (preSpawnedEntities[indexEntityToSpawn]) {
                entities.push_back(preSpawnedEntities[indexEntityToSpawn]);
                indexEntityToSpawn++;
            }
            
        }
        spawnIntervall -= deltaTime;
        window.display();
    }

    return 0;
}

//void Entity::borderCollision()
//{
//    if (pos.x + radius > manager->screenWidth) {
//        pos.x = pos.x; 
//    }
//    else if (pos.x + radius < 0) {
//        pos.x = pos.x; 
//    }
//
//    if (pos.y + radius > screenSize_.y) {
//        pos.y = 0; 
//    }
//    else if (pos.y < 0) {
//        pos.y = screenSize_.y; 
//    }
//}

void Entity::applyGravity()
{

}

void Entity::update(float deltatime)
{
    //just gravity
    //vel.x += -cos(0) * manager->gravity * deltatime;
    vel.y += sin(M_PI_2) * manager->gravity * deltatime;


    //angle = 

    //pos.x += vel.x + cos(0) * deltatime;
    pos.y += vel.y * sin(M_PI_2) * deltatime;


}

void Entity::render(sf::RenderWindow& window)
{
    
    transform.translate(pos).rotate(angle);
    //wrapAround(sf::Vector2u(1200, 900)); // call wrap function after updating position

    window.draw(shape, transform);

}
