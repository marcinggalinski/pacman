#ifndef MAP_HPP
#define MAP_HPP

#include <memory>

class Tile;
struct MapData;

class Map
{
    int width;
    int height;
    Tile*** tiles;
    std::shared_ptr<sf::Texture> backgroundImage;
    std::shared_ptr<sf::Sprite> background;

public:
    Map(std::unique_ptr<MapData>& map);
    ~Map();
    int getWidth();
    int getHeight();
    void draw(sf::RenderWindow& window);

    Tile& operator()(int c, int r);
};

#endif
