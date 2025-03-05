#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <queue>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SFML/Graphics.hpp>

#define log(x, y) std::cout << x << y << std::endl;

int MAX_LVL = 8;

// Forward declarations - this is what was missing
class SimulationManager;
class Entity;
struct Node;

struct Node {
    static std::queue<Node*> NodePool;

    int x, y, width, height, lvl;

    Node() : x(0), y(0), width(0), height(0), lvl(0) {}

    Node(int _x, int _y, int _w, int _h, int _lvl) : x(_x), y(_y), width(_w), height(_h), lvl(_lvl)
    {
        //add entities from main function to this 
    }

    std::vector<Node*> children;
    std::vector<Entity*> entities;

    //lets us add a new entity to the quadtree
    void Add(Entity& _entity);


public:
    //tests whether an entity is inside the bounding rectangle of the node
    int Inside(Entity& _entity);

    //used to reset a node
    void Set(int _x, int _y, int _w, int _h, int _lvl);

};

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


    void ConstructQuadTree(Node* root, int part_count, std::vector<Entity*> entities, float deltaTime);

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
        : manager(manager), radius(manager->getRandRadius()), pos(manager->spawnPositionX, manager->spawnPositionY)
    {
        shape = sf::CircleShape(radius);
        shape.setOrigin(sf::Vector2f{ radius, radius });
        shape.setPosition(sf::Vector2f{ manager->spawnPositionX, manager->spawnPositionY });
        shape.setFillColor(sf::Color::Green);
        shape.setPointCount(20);
        vel.x = 1.f;
    }

    void update(float deltatime);
    void render(sf::RenderWindow& window);
    void borderCollision();
    void entityCollision(Entity* first, Entity* second, float deltaTime);


public: //should be oprivate but for speedy efficiency
    SimulationManager* manager = nullptr;
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::CircleShape shape;
    float radius;

};

std::queue<Node*> Node::NodePool;
void InitializeNodePool(int initialSize = 1000)
{
    for (int i = 0; i < initialSize; i++) {
        Node::NodePool.push(new Node());
    }
}

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

    // Initialize the node pool
    InitializeNodePool(10000);
   
    SimulationManager simManager(screenWidth, screenHeight, minRadius, maxRadius, spawnLimit, gravity, spawnPositionX, spawnPositionY);
    //====== CHANGE THIS TO MAP ORT TUPLE ===========
    std::vector<Entity*> preSpawnedEntities;
    for (int i = 0; i <= spawnLimit; i++) {
        preSpawnedEntities.push_back(new Entity(&simManager));
    }
    std::vector<Entity*> entities;
    entities.push_back(preSpawnedEntities[0]);


    float initSpawnIntervall = 0.05;
    float spawnIntervall = initSpawnIntervall;
    int indexEntityToSpawn = 1; //start at one since we already spoawned 0


    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode({ screenWidth, screenHeight }), "SFML works!");

    Node root;

    while (window.isOpen())
    {
        //restart() returns a time object which ahve asSeconds as member
        float deltaTime = clock.restart().asSeconds();
        
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                window.close();
        }

        window.clear();
        simManager.ConstructQuadTree(&root, spawnLimit, entities, deltaTime);
        for (auto entity : entities) {            

            entity->render(window);
        }
        //change the whole thing to map or tuple with <entity, bool used>
        if (spawnIntervall < 0) {
            std::cout << "somehting is spawning" << std::endl;
            spawnIntervall = initSpawnIntervall;
            if (indexEntityToSpawn  != preSpawnedEntities.size()-1) {
                entities.push_back(preSpawnedEntities[indexEntityToSpawn]);
                indexEntityToSpawn++;
                std::cout << indexEntityToSpawn << std::endl;

            }
            
        }
        spawnIntervall -= deltaTime;
        window.display();
    }

    return 0;
}

void Entity::borderCollision()
{
    // Right wall collision
    if (pos.x + radius > manager->screenWidth) {
        pos.x = manager->screenWidth - radius;  // Prevent going out of bounds
        vel.x *= -0.8f; // Reverse velocity with damping to simulate energy loss
    }
    // Left wall collision
    else if (pos.x - radius < 0) {
        pos.x = radius;
        vel.x *= -0.8f;
    }

    // Bottom wall collision
    if (pos.y + radius > manager->screenHeight) {
        pos.y = manager->screenHeight - radius;
        vel.y *= -0.8f; // Reverse velocity to simulate bouncing
    }
    // Top wall collision
    else if (pos.y - radius < 0) {
        pos.y = radius;
        vel.y *= -0.8f;
    }
}

void Entity::entityCollision(Entity* first, Entity* second, float deltaTime) {

    sf::Vector2f delta = first->pos - second->pos;
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    float overlap = (first->radius + second->radius) - distance;


 /* log("deltax is : ", delta.x);
    log("deltay is : ", delta.y);
    log("distance is : ", distance);
    log("overlap is : ", overlap);
*/

    if (distance < first->radius + second->radius) {
        // **Push apart**
        sf::Vector2f normal = delta / distance; // Normalize direction
        first->pos += normal * (overlap / 2.f);
        second->pos -= normal * (overlap / 2.f);

        // Calculate dot product of velocity and normal for each entity
        float dotProductFirst = first->vel.x * normal.x + first->vel.y * normal.y;
        float dotProductSecond = second->vel.x * normal.x + second->vel.y * normal.y;

        // Update velocities - project velocities onto the normal and reflect
        first->vel.x -= 2.0f * dotProductFirst * normal.x;
        first->vel.y -= 2.0f * dotProductFirst * normal.y;

        second->vel.x -= 2.0f * dotProductSecond * normal.x;
        second->vel.y -= 2.0f * dotProductSecond * normal.y;
    
    }

}

void SimulationManager::ConstructQuadTree(Node* root, int part_count, std::vector<Entity*> entities, float deltaTime)
{
    root->Set(0, 0, root->width, root->height, 0);

    for (int i = 0; i < entities.size(); i++)
    {
        entities[i]->borderCollision();
        //entities[i]->borderCollision();
        root->Add(*entities[i]);
        entities[i]->update(deltaTime);
        for (int j = 0; j < entities.size(); j++) {
            if (entities[i] == entities[j])
                break;
            entities[i]->entityCollision(entities[i], entities[j], deltaTime);
        }
        root->Add(*entities[i]);
    }
}

void Entity::update(float deltatime)
{
    //just gravity
    //vel.x += -cos(0) * manager->gravity * deltatime;
    vel.y += manager->gravity * deltatime * 10;

    pos.x += vel.x * deltatime;
    pos.y += vel.y * deltatime;

}

void Entity::render(sf::RenderWindow& window)
{
    //wrapAround(sf::Vector2u(1200, 900)); // call wrap function after updating position
    shape.setPosition(pos);
    window.draw(shape);

}

void Node::Add(Entity& _entity)
{
    if (entities.size() < 4 || lvl == MAX_LVL)
    {
        entities.push_back(&_entity);
    }
    else
    {
        if (children.size() == 0)
        {
            // Check if we have enough nodes
            if (NodePool.size() < 4) {
                // Add more nodes to the pool or don't subdivide
                return;
            }
            Node* n1 = NodePool.front(); 
            NodePool.pop();
            Node* n2 = NodePool.front();
            NodePool.pop();
            Node* n3 = NodePool.front();
            NodePool.pop();
            Node* n4 = NodePool.front();
            NodePool.pop();

            n1->Set(x, y, width / 2, height / 2, lvl + 1);
            children.push_back(n1);
            n2->Set(x + width / 2, y, width / 2, height / 2, lvl + 1);
            children.push_back(n2);
            n3->Set(x, y + height / 2, width / 2, height / 2, lvl + 1);
            children.push_back(n3);
            n4->Set(x + width / 2, y + height / 2, width / 2, height / 2, lvl + 1);
            children.push_back(n4);


            for (auto* entity : entities)
            {
                for (auto* childNode : children)
                {
                    if (childNode->Inside(*entity))
                    {
                        childNode->Add(*entity);
                    } 
                }
            }
        }

        for (auto* childNode : children)
        {
            if (childNode->Inside(_entity))
            {
                childNode->Add(_entity);
            }
        }

    }
}

//checks whether part of the bounding circle of the entity falls within the bounding region of the node.
int Node::Inside(Entity& _entity)
{
    float _px0 = _entity.pos.x - _entity.radius;
    float _py0 = _entity.pos.y - _entity.radius;
    float _px1 = _entity.pos.x + _entity.radius;
    float _py1 = _entity.pos.y + _entity.radius;


    if ((_px0 >= x && _px0 <= (x + width)) || (_px1 >= x && _px1 <= (x + width)) &&
        (_py0 >= y && _py0 <= (y + height)) || (_py1 >= y && _py1 <= (y + height)))
        return 1;
    else
        return 0;
}

void Node::Set(int _x, int _y, int _w, int _h, int _lvl)
{
    for (auto child : children)
        NodePool.push(child);
    children.clear();
    entities.clear();
    x = _x;
    y = _y;
    width = _w;
    height = _h;
    lvl = _lvl;
}

