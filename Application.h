#ifndef APPLICATION_H
#define APPLICATION_H

#include "Game.h"
#include <SFML\Graphics.hpp>

class Application
{
public:
	Application(u8);

	void go();

private:
	const u8 mTileSize;

	sf::RenderWindow mWindow;
};

#endif // APPLICATION_H
