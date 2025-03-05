#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <queue>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SFML/Graphics.hpp>

#define log(x, y) std::cout << x << y << std::endl;

// A simple Rectangle class
struct Rectangle {
    float x, y, width, height;

    Rectangle(float _x, float _y, float _w, float _h)
        : x(_x), y(_y), width(_w), height(_h) {}
};

// Forward declaration of Entity
class Entity;

// The Quadtree class
class QuadTree {
public:
    // Maximum objects per node and maximum levels before stopping subdivision.
    static const int MAX_OBJECTS = 10;
    static const int MAX_LEVELS = 5;

    int level;                      // Current node level (0 is the root)
    std::vector<Entity*> objects;   // Objects stored in this node
    Rectangle bounds;               // The region of space this node occupies
    QuadTree* nodes[4];             // Pointers to four subnodes

    // Constructor: initializes level, bounds, and nulls out the subnodes.
    QuadTree(int pLevel, const Rectangle& pBounds)
        : level(pLevel), bounds(pBounds)
    {
        for (int i = 0; i < 4; i++) {
            nodes[i] = nullptr;
        }
    }

    void clear();
    void split();
    int getIndex(const Entity* entity) const;
    void insert(Entity* entity);
    void retrieve(std::vector<Entity*>& returnObjects, Entity* entity);

    // Destructor: clears the quadtree to free memory.
    ~QuadTree() {
        clear();
    }
};

class Entity {
public:

     Entity(int screenWidth, int screenHeight, float minRadius, float maxRadius, float spawnLimit, float gravity, float spawnPositionX, float spawnPositionY)
        :   screenWidth(screenWidth),
            screenHeight(screenHeight),
            minRadius(minRadius),
            maxRadius(maxRadius),
            spawnLimit(spawnLimit),
            gravity(gravity),
            spawnPositionX(spawnPositionX),
            spawnPositionY(spawnPositionY) 
     {
         radius = getRandRadius();
         pos = sf::Vector2f{ spawnPositionX, spawnPositionY };
         shape = sf::CircleShape(radius);
         shape.setOrigin(sf::Vector2f{ radius, radius });
         shape.setPosition(pos);
         shape.setFillColor(sf::Color::Green);
         shape.setPointCount(20);
         vel.x = 1.f;
     }

    float getRandRadius() {
        // Thread-local to avoid contention in multithreaded code
        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(minRadius, maxRadius);
        return distribution(generator);
    };


public: //should ´be private
    void update(float deltatime);
    void render(sf::RenderWindow& window);
    void borderCollision();
    void entityCollision(Entity* first, Entity* second, float deltaTime);

    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::CircleShape shape;
    float radius;

    int screenWidth;
    int screenHeight;
    float minRadius;
    float maxRadius;
    float spawnLimit;
    float gravity;
    float spawnPositionX;
    float spawnPositionY;

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

    ////====== CHANGE THIS TO MAP ORT TUPLE ===========
    std::vector<Entity*> preSpawnedEntities;
    for (int i = 0; i <= spawnLimit; i++) {
        preSpawnedEntities.push_back(new Entity(screenWidth, screenHeight, minRadius, maxRadius, spawnLimit, gravity, spawnPositionX, spawnPositionY));
    }
    std::vector<Entity*> entities;
    entities.push_back(preSpawnedEntities[0]);


    float initSpawnIntervall = 0.01;
    float spawnIntervall = initSpawnIntervall;
    int indexEntityToSpawn = 1; //start at one since we already spoawned 0


    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode({ screenWidth, screenHeight }), "SFML works!");
    Rectangle rootRect = Rectangle(0, 0, screenWidth, screenHeight);
    QuadTree* quadTree = new QuadTree(0, rootRect);
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
        quadTree->clear();
        for (Entity* entity : entities) {
            quadTree->insert(entity);
        }
        // 2. For each entity, retrieve potential collision candidates
//    and then run your sphere collision detection.
        for (Entity* entity : entities) {
            entity->update(deltaTime);
            entity->borderCollision();
            std::vector<Entity*> candidates;
            quadTree->retrieve(candidates, entity);
            for (Entity* other : candidates) {
                if (entity != other) {
                    // Call your sphere collision function.
                    entity->entityCollision(entity, other, deltaTime);
                }
            }
            entity->render(window);
        }

        //change the whole thing to map or tuple with <entity, bool used>
        if (spawnIntervall < 0) {
            spawnIntervall = initSpawnIntervall;
            if (indexEntityToSpawn  != preSpawnedEntities.size()-1) {
                entities.push_back(preSpawnedEntities[indexEntityToSpawn]);
                indexEntityToSpawn++;
            }
        }
        spawnIntervall -= deltaTime;
        float fps = (deltaTime > 0) ? 1.0f / deltaTime : 0.f;
        std::cout << fps << std::endl;

        window.display();
    }

    return 0;
}

void Entity::borderCollision()
{
    // Right wall collision
    if (pos.x + radius > screenWidth) {
        pos.x = screenWidth - radius;  // Prevent going out of bounds
        vel.x *= -0.8f; // Reverse velocity with damping to simulate energy loss
    }
    // Left wall collision
    else if (pos.x - radius < 0) {
        pos.x = radius;
        vel.x *= -0.8f;
    }

    // Bottom wall collision
    if (pos.y + radius > screenHeight) {
        pos.y = screenHeight - radius;
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

    if (distance < first->radius + second->radius) {
        // **Push apart**
        if (distance == 0)
            distance += 0.0001;
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


void Entity::update(float deltatime)
{
    //just gravity
    //vel.x += -cos(0) * manager->gravity * deltatime;
    vel.y += gravity * 10 * deltatime;

    pos.x += vel.x * deltatime;
    pos.y += vel.y * deltatime;

}

void Entity::render(sf::RenderWindow& window)
{
    //wrapAround(sf::Vector2u(1200, 900)); // call wrap function after updating position
    shape.setPosition(pos);
    window.draw(shape);

}

void QuadTree::clear()
{
    objects.clear();
    for (int i = 0; i < 4; i++) {
        if (nodes[i] != nullptr) {
            nodes[i]->clear();
            delete nodes[i];
            nodes[i] = nullptr;
        }
    }
}

/*
* Splits the node into 4 subnodes
*/
void QuadTree::split()
{
    int subWidth = static_cast<int>(bounds.width / 2);
    int subHeight = static_cast<int>(bounds.height / 2);
    int x = static_cast<int>(bounds.x);
    int y = static_cast<int>(bounds.y);

    // Create the four subnodes with their respective bounds.
    nodes[0] = new QuadTree(level + 1, Rectangle(x + subWidth, y, subWidth, subHeight));
    nodes[1] = new QuadTree(level + 1, Rectangle(x, y, subWidth, subHeight));
    nodes[2] = new QuadTree(level + 1, Rectangle(x, y + subHeight, subWidth, subHeight));
    nodes[3] = new QuadTree(level + 1, Rectangle(x + subWidth, y + subHeight, subWidth, subHeight));


}

// Determine which node the object belongs to.
// Returns:
//   0, 1, 2, or 3 if the object fits completely within a subnode,
//   -1 if the object cannot completely fit within a child node.
int QuadTree::getIndex(const Entity* entity) const {
    // Compute bounding box of the entity (circle)
    float ex = entity->pos.x - entity->radius;
    float ey = entity->pos.y - entity->radius;
    float ewidth = 2 * entity->radius;
    float eheight = 2 * entity->radius;

    int index = -1;
    float verticalMidpoint = bounds.x + (bounds.width / 2.0f);
    float horizontalMidpoint = bounds.y + (bounds.height / 2.0f);

    // Check if it fits completely in the top quadrants.
    bool topQuadrant = (ey < horizontalMidpoint && ey + eheight < horizontalMidpoint);
    // Check if it fits completely in the bottom quadrants.
    bool bottomQuadrant = (ey > horizontalMidpoint);

    // Check left quadrants.
    if (ex < verticalMidpoint && ex + ewidth < verticalMidpoint) {
        if (topQuadrant)
            index = 1;
        else if (bottomQuadrant)
            index = 2;
    }
    // Check right quadrants.
    else if (ex > verticalMidpoint) {
        if (topQuadrant)
            index = 0;
        else if (bottomQuadrant)
            index = 3;
    }
    return index;
}


// Insert the object into the quadtree.
// If the node already has child nodes, it tries to pass the object 
// to the appropriate child.
// Otherwise, it stores the object here and splits if necessary.
void QuadTree::insert(Entity* entity)
{
    if (nodes[0] != nullptr) {
        int index = getIndex(entity);
        if (index != -1) {
            nodes[index]->insert(entity);
            return;
        }
    }
    // If we can't fully fit in a subnode, store it here.
    objects.push_back(entity);

    // If the number of objects exceeds the capacity and we haven't reached the maximum level,
    // split the node and redistribute objects.
    if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
        if (nodes[0] == nullptr) {
            split();
        }

        // Iterate over objects and move those that fit completely into a child node.
        int i = 0;
        while (i < objects.size()) {
            int index = getIndex(objects[i]);
            if (index != -1) {
                Entity* obj = objects[i];
                objects.erase(objects.begin() + i);
                nodes[index]->insert(obj);
                // Do not increment i since the vector has shifted.
            }
            else {
                i++;
            }
        }
    }
}


void QuadTree::retrieve(std::vector<Entity*>& returnObjects, Entity* entity) {
    int index = getIndex(entity);
    if (index != -1 && nodes[0] != nullptr) {
        nodes[index]->retrieve(returnObjects, entity);
    }
    // Add objects from the current node.
    returnObjects.insert(returnObjects.end(), objects.begin(), objects.end());
}

