#include "Application.h"

Application::Application(u8 tileSize) :
	mTileSize(tileSize)
{
}

void Application::go()
{
	Game game;
	u8 selectedSquare(-1);

	sf::Sprite sprite;
	sf::RectangleShape tile(sf::Vector2f(mTileSize, mTileSize));
	std::array<std::array<sf::Texture, 6>, 2> textures;
	
	std::array <sf::Color, 2> couleursDamier;
	couleursDamier[0] = sf::Color(209, 139, 71);
	couleursDamier[1] = sf::Color(255, 206, 158);

	std::list<Move> moves;

	// Load textures
	for (u8 i(0); i < 2; ++i) {
		for (u8 j(0); j < 6; ++j) {
			if (!textures[i][j].loadFromFile("pieces_sprite.png", sf::IntRect(j*mTileSize, i*mTileSize, mTileSize, mTileSize)))
				throw std::exception("cannot open sprite file");

			textures[i][j].setSmooth(true);
		}
	}

	mWindow.create(sf::VideoMode(8 * mTileSize, 8 * mTileSize), "Jeu d'échecs");

	while (mWindow.isOpen()) {
		sf::Event event;

		while (mWindow.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				mWindow.close();
				break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left) {
					u8 s = (7 - (event.mouseButton.y / mTileSize))*8 + (event.mouseButton.x / mTileSize);

					if (s == selectedSquare) {
						selectedSquare = -1;
						moves.clear();
					} else {

						std::list<Move>::const_iterator it = std::find_if(moves.begin(), moves.end(), [s](const Move& m) { return m.to() == s; });
						moves.clear();

						if (it != moves.end())
							game.makeMove(*it);
						else {
							selectedSquare = s;
							std::for_each(game.possibleMoves().begin(), game.possibleMoves().end(), [&moves, selectedSquare](const Move& m) { if (m.from() == selectedSquare) moves.push_back(m); });
						}
					}
				}
				
				break;
			}
		}

		mWindow.clear();

		// Show the game
		u64 p(1), s(0);
		u8 pieceType(0);

		for (u8 i(0); i < 8; ++i) {
			for (u8 j(0); j < 8; ++j, ++s, p <<= 1) {
				tile.setPosition(sf::Vector2f(j*mTileSize, (7-i)*mTileSize));
				sprite.setPosition(sf::Vector2f(j*mTileSize, (7-i)*mTileSize));

				tile.setFillColor(couleursDamier[(i+j)%2]);
				mWindow.draw(tile);

				std::list<Move>::const_iterator it = std::find_if(moves.begin(), moves.end(), [s](const Move& m) { return m.to() == s; });

				if (it != moves.end()) {
					Move move(*it);

					if (move.isCapture())
						tile.setFillColor(sf::Color(255, 0, 0, 196));
					else if (move.isPromotion())
						tile.setFillColor(sf::Color(255, 216, 0, 127));
					else
						tile.setFillColor(sf::Color(76, 255, 0, 127));

					mWindow.draw(tile);
				}

				if ((pieceType = game.pieceType(s)) != u8(-1)) {
					sprite.setTexture(textures[bool(game.player(Black) & p)][pieceType]);
					mWindow.draw(sprite);
				}
			}
		}

		mWindow.display();
	}
}
