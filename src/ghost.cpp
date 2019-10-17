// standard c++ lib
#include <iostream>
#include <iomanip>
#include <queue>

// additional libs
#include <SFML/Graphics.hpp>

// project headers
#include "headers/ghost.hpp"
#include "headers/config.hpp"
#include "headers/map.hpp"
#include "headers/tile.hpp"
#include "headers/pacman.hpp"


// ctor
Ghost::Ghost(MapData& mapData, Map& map, Pacman& pacman, sf::Texture* texture) : map(map), pacman(pacman)
{
    this->bMap = mapData.map;
    this->iMap = new int*[this->map.getHeight()];
    for(uint c = 0; c < this->map.getHeight(); c++)
        this->iMap[c] = new int[this->map.getWidth()];

    this->sprite = new sf::Sprite(*texture);
    this->sprite->setScale(float(DEFINES.TILE_SIZE) / texture->getSize().x, float(DEFINES.TILE_SIZE) / texture->getSize().y);

    this->speed = DEFINES.GHOST_SPEED * this->sprite->getScale().x;
    this->moveDirection = Direction::none;
    this->hasMoved = false;
}


// dtor
Ghost::~Ghost()
{
    for(uint c = 0; c < this->map.getWidth(); c++)
    {
        delete[] this->iMap[c];
        delete[] this->bMap[c];
    }
    delete[] this->iMap;
    delete[] this->bMap;

    delete this->sprite;
}


// getters
Coords Ghost::getCoords() const
{
    return this->coords;
}

Position Ghost::getPosition() const
{
    return this->position;
}

Direction Ghost::getMoveDirection() const
{
    return this->moveDirection;
}


// methods
void Ghost::move()
{
    if(!this->pacman.hasMoved())
        return;
    
    if(!this->hasMoved)
        this->findTilePosition(this->coords + sf::Vector2f(1, 0));

    // to shorten conditions at least a bit
    Tile& currTile = this->map(this->position);
    Direction dir = this->moveDirection = this->chooseDirection();
    std::cerr << "\t" << int(dir) << std::endl;

    // move
    // if there's a tunel, go through it
    if(this->isAtBorder() && currTile.isTunel())
    {
        // change coords
        this->coords += Directions[int(dir)] * this->speed;

        // if center of ghost has passed tile (and map) edge, teleport it to the other side
        Coords center(this->coords + Coords(DEFINES.TILE_SIZE / 2));
        if(center.x < this->map(0, 0).getCoords().x
            || center.x > this->map(this->map.getWidth() - 1, 0).getCoords().x + DEFINES.TILE_SIZE
            || center.y < this->map(0, 0).getCoords().y
            || center.y > this->map(0, this->map.getHeight() - 1).getCoords().y + DEFINES.TILE_SIZE)
        {
            this->position = (this->position + Position(this->map.getWidth(), this->map.getHeight()) + Directions[int(dir)])
                                % sf::Vector2i(this->map.getWidth(), this->map.getHeight());
            this->coords = this->map(this->position).getCoords() + currTile.getCoords() - this->coords;
        }

        // if moving towards center of the map and passed tile edge, update position
        else if(center.x < currTile.getCoords().x
            || center.x > currTile.getCoords().x + DEFINES.TILE_SIZE
            || center.y < currTile.getCoords().y
            || center.y > currTile.getCoords().y + DEFINES.TILE_SIZE)
        {
            this->position += Directions[int(dir)];
        }
    }
    // if about to hit the wall, STOP
    else if(this->map(this->position + sf::Vector2f(Directions[int(dir)])).isWall()
        && this->coords.x + Directions[int(dir)].x * this->speed < currTile.getCoords().x + 2
        && this->coords.x + Directions[int(dir)].x * this->speed > currTile.getCoords().x - 2
        && this->coords.y + Directions[int(dir)].y * this->speed < currTile.getCoords().y + 2
        && this->coords.y + Directions[int(dir)].y * this->speed > currTile.getCoords().y - 2)
    {
        this->coords = currTile.getCoords();
    }
    // otherwise, go ahead
    else
    {
        this->coords += Directions[int(dir)] * this->speed;
        if(this->coords.x + DEFINES.TILE_SIZE / 2 < currTile.getCoords().x
            || this->coords.x + DEFINES.TILE_SIZE / 2 > currTile.getCoords().x + DEFINES.TILE_SIZE
            || this->coords.y + DEFINES.TILE_SIZE / 2 < currTile.getCoords().y
            || this->coords.y + DEFINES.TILE_SIZE / 2 > currTile.getCoords().y + DEFINES.TILE_SIZE)
        {
            this->position += Directions[int(dir)];
        }
    }

    // updating currTile
    Tile& newCurrTile = this->map(this->position);

    if(&currTile != &newCurrTile)
    {
        currTile.setGhost(false);
        newCurrTile.setGhost(true);
    }

    this->sprite->setPosition(this->coords.x, this->coords.y);
}

bool Ghost::findTilePosition(Coords coords)
{
    // coords of center of ghost
    Coords center(coords + Coords(DEFINES.TILE_SIZE / 2));

    for(uint c = 0; c < this->map.getWidth(); c++)
    {
        for(uint r = 0; r < this->map.getHeight(); r++)
        {
            //coords of top left corner of tile
            Coords corner(this->map(c, r).getCoords().x, this->map(c, r).getCoords().y);

            if(center.x > corner.x && center.x < corner.x + DEFINES.TILE_SIZE
                && center.y > corner.y && center.y < corner.y + DEFINES.TILE_SIZE)
            {
                if(this->map(c, r).isWall())
                {
                    return false;
                }
                else
                {
                    this->position = Position(c, r);
                    this->hasMoved = true;
                    return true;
                }
            }
        }
    }
    return false;
}

bool Ghost::isAtBorder() const
{
    Tile& tile = this->map(this->position);
    return tile.getPosition().c == 0 || tile.getPosition().c == this->map.getWidth() - 1
            || tile.getPosition().r == 0 || tile.getPosition().r == this->map.getHeight() - 1;
}

void Ghost::zeroiMap()
{
    for(uint c = 0; c < this->map.getWidth(); c++)
        for(uint r = 0; r < this->map.getHeight(); r++)
            this->iMap[c][r] = INT32_MAX;
}

void Ghost::dijkstra(Position position)
{
    this->zeroiMap();

    Position pPosition = position;
    std::queue<std::pair<Position, int> > q;
    q.push(std::make_pair(pPosition, 0));
    
    while(q.size())
    {
        auto pos = q.front();
        q.pop();

        if(pos.second < this->iMap[pos.first.c][pos.first.r])
        {
            this->iMap[pos.first.c][pos.first.r] = pos.second;
            for(auto dir : Directions)
            {
                try
                {
                    if(this->map(pos.first).isTunel()
                        && (pos.first.c + dir.x < 0
                                || pos.first.c + dir.x > this->map.getWidth() - 1
                                || pos.first.r + dir.y < 0
                                || pos.first.r + dir.y > this->map.getHeight() - 1))
                    {
                        Position p = (pos.first + Position(this->map.getWidth(), this->map.getHeight()) + dir)
                                        % sf::Vector2i(this->map.getWidth(), this->map.getHeight());
                        q.push(std::make_pair(p, pos.second + 1));
                    }
                    // if(!this->map(pos.first + dir).isWall())
                    if(pos.first.c + dir.x < 0
                        || pos.first.c + dir.x > this->map.getWidth() - 1
                        || pos.first.r + dir.y < 0
                        || pos.first.r + dir.y > this->map.getHeight() - 1)
                        continue;
                    if(this->bMap[pos.first.c + int(dir.x)][pos.first.r + int(dir.y)])
                    {
                        q.push(std::make_pair(pos.first + dir, pos.second + 1));
                    }
                }
                catch(...)
                {
                    //
                }
            }
        }
    }
}

Direction Ghost::chooseDirection()
{
    Tile& currTile = this->map(this->position);
    Direction shortestDir = this->moveDirection;
    int shortestLen = INT32_MAX;
    Position backPos = (this->position + Position(this->map.getWidth(), this->map.getHeight()) + -Directions[int(shortestDir)])
                        % sf::Vector2i(this->map.getWidth(), this->map.getHeight());
    if(this->map(backPos).isWall())
        this->dijkstra(this->getDestination());
    else
    {
        this->bMap[backPos.c][backPos.r] = false;
        // this->bMap[this->position.c - int(Directions[int(shortestDir)].x)][this->position.r - int(Directions[int(shortestDir)].y)] = false;
        this->dijkstra(this->getDestination());
        this->bMap[backPos.c][backPos.r] = true;
        // this->bMap[this->position.c - int(Directions[int(shortestDir)].x)][this->position.r - int(Directions[int(shortestDir)].y)] = true;
    }

    std::cerr << "vvvvvvvvvvvvvvvvvvvv" << std::endl;
    for(uint r = 0; r < this->map.getHeight(); r++)
    {
        for(uint c = 0; c < this->map.getWidth(); c++)
        {
            std::cerr << std::setw(3);
            if(this->iMap[c][r] == INT32_MAX)
                std::cerr << '#';
            else
                std::cerr << this->iMap[c][r];
        }
        std::cerr << std::endl;
    }

    for(int i = 1; i < int(Direction::nOfDirections); i++)
    {
        try
        {
            auto dir = Directions[i];

            if(dir == -Directions[int(this->moveDirection)])
                continue;

            Position pos = (this->position + Position(this->map.getWidth(), this->map.getHeight()) + dir)
                                % sf::Vector2i(this->map.getWidth(), this->map.getHeight());

            if(this->iMap[pos.c][pos.r] < shortestLen
                && this->coords.x < currTile.getCoords().x + 2
                && this->coords.x > currTile.getCoords().x - 2
                && this->coords.y < currTile.getCoords().y + 2
                && this->coords.y > currTile.getCoords().y - 2)
            {
                shortestLen = this->iMap[pos.c][pos.r];
                shortestDir = Direction(i);
            }
        }
        catch(...)
        {
            //
        }
    }
    std::cerr << int(shortestDir) << std::endl;
    std::cerr << "^^^^^^^^^^^^^^^^^^^^" << std::endl;
    return shortestDir;
}

void Ghost::draw(sf::RenderWindow& window) const
{
    window.draw(*this->sprite);
}