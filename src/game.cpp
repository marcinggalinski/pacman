#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "headers/game.hpp"
#include "headers/config.hpp"
#include "headers/tile.hpp"
#include "headers/map.hpp"
#include "headers/pacman.hpp"

void processGameEvents(sf::RenderWindow& window, Pacman& player)
{
    sf::Event event;
    while(window.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            window.close();
            return;
        }
        if(event.type == sf::Event::KeyPressed)
        {
            switch(event.key.code)
            {
            case sf::Keyboard::Escape:
                window.close();
                return;
            case sf::Keyboard::W:
                player.turn(Direction::up);
                break;
            case sf::Keyboard::S:
                player.turn(Direction::down);
                break;
            case sf::Keyboard::A:
                player.turn(Direction::left);
                break;
            case sf::Keyboard::D:
                player.turn(Direction::right);
                break;
            case sf::Keyboard::Up:
                player.turn(Direction::up);
                break;
            case sf::Keyboard::Down:
                player.turn(Direction::down);
                break;
            case sf::Keyboard::Left:
                player.turn(Direction::left);
                break;
            case sf::Keyboard::Right:
                player.turn(Direction::right);
                break;
            }
        }
    }
}

int game(sf::RenderWindow& window)
{
    if(loadMap() == false)
        return 1;

    std::shared_ptr<Map> map = std::make_shared<Map>(CONFIG.mapData);
    Pacman player(CONFIG.mapData->pacmanX, CONFIG.mapData->pacmanY, DEFINES.PACMAN_SIZE, PACMAN_SPEED, map);
    while(window.isOpen())
    {
        processGameEvents(window, player);

        window.clear();

        map->draw(window);
        player.move();
        player.draw(window);

        window.display();
    }
    return 0;
}