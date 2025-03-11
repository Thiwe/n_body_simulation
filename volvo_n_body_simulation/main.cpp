#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <string>
#include <stdexcept> 
#include <cstdlib>
#include <algorithm>
#include <cmath>

using namespace std;

class Particle
{
public:

    sf::CircleShape pShape;
    sf::Vector2f velocity;
    sf::Vector2f position;
    float gravity;
    float widthBound;
    float heightBound;
    float mass = 1.f;

    Particle(float radius, sf::Vector2f position, float gravity, float widthBound, float heightBound, sf::Color color, sf::Vector2f velocity)
        : position(position), gravity(gravity), widthBound(widthBound), heightBound(heightBound), velocity(velocity)
    {
        pShape.setRadius(radius);
        pShape.setOrigin(sf::Vector2f(radius, radius));
        pShape.setFillColor(color);
        pShape.setPosition(position);
        mass /= radius;
    }

    void update(float deltaTime);
    void borderCheck();
    void particleCollisionCheck(Particle* other);
};

struct GridCell {
    int xPos;
    int yPos;
    vector<Particle*> particles;
};

class Grid {
public:
    int cellSize;
    int numRows;
    int numColumns;
    vector< vector<GridCell> > cells;


    Grid(int numRows, int numColumns, int value, float cellSize)
        : numRows(numRows), numColumns(numColumns), cellSize(cellSize)
    {
        cells.resize(numRows, vector<GridCell>(numColumns));

        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numColumns; col++)
            {
                cells[row][col].xPos = col * cellSize;
                cells[row][col].yPos = row * cellSize;
            }
        }

    }

    void checkCellsCollisions(GridCell& current, GridCell& other) {
        for (auto& particle : current.particles) {
            for (auto& otherParticle : other.particles) {
                if(particle != otherParticle) {
                    if()
        }
    }


    void find_collisions_grid() {
        for (int row = 1; row < numRows - 1; row++)
        {
            for (int col = 1; col < numColumns - 1; col++)
            {
                auto& currentCell = cells[row][col];
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dx) {
                        auto& otherCell = cells[row+dx][col+dy];
                        checkCellsCollisions(currentCell, otherCell);
                    }
                }
            }
        }
    }

};

//Main loop taking command arguments
int main(int argc, char* argv[])
{
    //Check if we have correct amount of arguments needed to run application
    if (argc != 7)
    {
        cerr << "Error: You must provide following arguments: 'windowWidth' 'windowHeight' 'spawnLimit' 'minParticleRadius' 'maxParticleRadius' 'gravity'" << endl;
        return 1;
    }

    //Check if arguments are convertable and valid
    for (int i = 1; i < argc; ++i)
    {
        cout << "Argument " << i << ": " << argv[i] << endl;

        try
        {
            if (i > 3)
            {
                float intArg = stof(argv[i]);
                cout << "Float argument: " << intArg << endl;
            }
            else
            {
                int intArg = stoi(argv[i]);
                cout << "Integer argument: " << intArg << endl;
            }
        }
        catch (const invalid_argument& e)
        {
            cerr << "Error: Argument" << i << "is of wrong type!" << endl;
            return 1;
        }
        catch (const out_of_range& e)
        {
            cerr << "Error: The" << i << "argument is out of range!" << endl;
            return 1;
        }
    }

    //Print string to give indication that arguments passed through correctly
    cout << "Application is running!" << endl;

    //Storing command arguments
    unsigned int windowWidth = stoi(argv[1]);
    unsigned int windowHeight = stoi(argv[2]);
    unsigned int spawnLimit = stoi(argv[3]);
    float minParticleRadius = stof(argv[4]);
    float maxParticleRadius = stof(argv[5]);
    float gravity = stof(argv[6]);

    //Initiate random varibles
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> color(0, 255);
    uniform_real_distribution<float> radius(minParticleRadius, maxParticleRadius);
    uniform_real_distribution<float> vel(0.5f, 1.0f);

    sf::RenderWindow window(sf::VideoMode({ windowWidth, windowHeight }), "Simulation window");

    //Instanciate a objects
    vector<Particle*> particles;
    for (int i = 0; i < spawnLimit; i++)
    {
        particles.push_back(new Particle(radius(gen), sf::Vector2f(maxParticleRadius * 2.f, maxParticleRadius * 2.f), gravity, static_cast<float>(windowWidth), static_cast<float>(windowHeight), sf::Color(color(gen), color(gen), color(gen)), sf::Vector2f(vel(gen), 0.0f)));
    }

    //Clock for deltaTime
    sf::Clock clock;
    int particlesProcessed = 0; //Tracks how many particles we've processed so far
    float timeSinceLastIncrease = 0.0f; //Time passed since last time we increased processed particles
    float increaseInterval = .01f; //How often to increase particlesProcessed (in seconds)

    Grid screenGrid(64, 64, 10, 10);

    //Main loop
    while (window.isOpen())
    {


        //Get time between frames and convert to second
        float deltaTime = clock.restart().asSeconds();

        //Loops if there is an queued event
        while (const optional event = window.pollEvent())
        {
            //Close application if "ESC" key is pressed
            if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                cout << "Number of particles spawned: " << particlesProcessed << endl;
                window.close();
            }
        }

        //Increment timeSinceLastIncrease
        timeSinceLastIncrease += deltaTime;

        //Every 'increaseInterval' seconds, increase the number of particles to process
        if (timeSinceLastIncrease >= increaseInterval) {
            particlesProcessed += 1; //Increase the number of particles to process by 1 every interval
            timeSinceLastIncrease = 0.0f; //Reset the timer
        }

        //Process particles incrementally
        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i)
        {
            Particle* p = particles[i];

            //Loops through pointers for particlesProcessed and checks for collision with other particles
            for (int j = 0; j <= particlesProcessed && j < particles.size(); ++j)
            {
                Particle* pOther = particles[j];
                p->particleCollisionCheck(pOther);
            }

            p->update(deltaTime);
   
        }

        //Clear the window
        window.clear();


        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i) {
            Particle* p = particles[i];
            //Draw all processed particles to screen
            window.draw(p->pShape);
        }

        //Display the window contents
        window.display();

        //Ensure we don't process more than the total number of particles
        if (particlesProcessed > particles.size())
        {
            particlesProcessed = particles.size();
        }

    }

    //Clean up the particles
    for (Particle* p : particles)
    {
        delete p;
    }

    return 0;
};

void Particle::update(float deltaTime)
{
    //Change velocity with time affected by gravity
    velocity.y += gravity * deltaTime;
    if (velocity.x != 0.0f)
        velocity.x *= 0.9999f;

    //Checks if collision with border and updates position.
    borderCheck();
    position = pShape.getPosition();
    pShape.setPosition({ position.x + velocity.x, position.y + velocity.y });
}

void Particle::borderCheck()
{
    //Check if we collide with border and change velocity accordingly
    if (position.y + pShape.getRadius() > heightBound)
    {
        pShape.setPosition({ position.x, heightBound - pShape.getRadius() });
        velocity.y *= -0.9f;
    }
    else if (position.y - pShape.getRadius() < 0.0f)
    {
        pShape.setPosition({ position.x, 0.0f + pShape.getRadius() });
        velocity.y *= -0.9f;
    }
    else if (position.x + pShape.getRadius() > widthBound)
    {
        pShape.setPosition({ widthBound - pShape.getRadius(), position.y });
        velocity.x *= -0.9f;
    }
    else if (position.x - pShape.getRadius() < 0.0f)
    {
        pShape.setPosition({ 0.0f + pShape.getRadius(), position.y });
        velocity.x *= -0.9f;
    }
}

void Particle::particleCollisionCheck(Particle* other)
{
    //Calculates the distance and overlap between particles
    sf::Vector2f delta = this->position - other->position;
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
    float overlap = (this->pShape.getRadius() + other->pShape.getRadius()) - distance;

    //Checks if distance is smaller than both particles radius combined
    if (distance < (this->pShape.getRadius() + other->pShape.getRadius()))
    {
        //Safety check so distance can not be equal to 0.0f
        if (distance == 0)
            distance += 0.1f;

        //Gets the direction
        sf::Vector2f normal = delta / distance;

        //Move both particles away from each other as a safety check
        this->position += normal * (overlap / 2.f);
        other->position -= normal * (overlap / 2.f);

        //Calculate relative velocity along the normal direction
        sf::Vector2f relativeVelocity = this->velocity - other->velocity;
        float dotProduct = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

        //If the particles are moving towards each other, apply collision response
        if (dotProduct < 0)
        {
            //This can be modified to simulate inelastic collisions (less than 1)
            float coefficientOfRestitution = .75f;

            //Calculate the force scalar
            float force = -(1 + coefficientOfRestitution) * dotProduct;
            //Taking the mass of the particles into count
            force /= (1 / this->mass) + (1 / other->mass);

            //Apply the force to to the correct direction
            sf::Vector2f forceVector = normal * force;

            //Apply force to ecah particles velocities
            this->velocity += forceVector / this->mass;
            other->velocity -= forceVector / other->mass;
        }
    }
};