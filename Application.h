#ifndef APPLICATION_H
#define APPLICATION_H

#include "AI.h"
#include "Hashing.h"

class Application
{
public:
	Application(u8);

	void go(Player, u8);

private:
	const u8 mTileSize;

	sf::RenderWindow mWindow;
};

#endif // APPLICATION_H
