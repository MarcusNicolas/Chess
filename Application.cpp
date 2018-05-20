#include "Application.h"

Application::Application(u8 tileSize) :
	mTileSize(tileSize)
{
}

void Application::go()
{
	Game game;
	u8 selectedSquare(-1), hoveredSquare(-1), promoCol(0), promoDelta(0);
	int e(0);

	AI ai;

	bool promoSelection(false);

	std::map<u8, u8> promoIndex;
	promoIndex[Queen] = 0;
	promoIndex[Rook] = 1;
	promoIndex[Bishop] = 2;
	promoIndex[Knight] = 3;

	std::array<PieceType, 4> promoPiece;
	promoPiece[0] = Queen;
	promoPiece[1] = Rook;
	promoPiece[2] = Bishop;
	promoPiece[3] = Knight;

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

		hoveredSquare = (7 - (sf::Mouse::getPosition(mWindow).y / mTileSize)) * 8 + (sf::Mouse::getPosition(mWindow).x / mTileSize);;

		if (game.activePlayer() == Black)
			game.makeMove(ai.bestMove(game, 4));
		else {
			while (mWindow.pollEvent(event)) {
				switch (event.type) {
				case sf::Event::Closed:
					mWindow.close();
					break;

				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Left) {
						u8 s = (7 - (event.mouseButton.y / mTileSize)) * 8 + (event.mouseButton.x / mTileSize);

						if (promoSelection) {
							if (s % 8 == promoCol) {
								u8 i(e * (int(s / 8) - promoDelta));

								if (i >= 0 && i <= 3) {
									PieceType t(promoPiece[i]);
									std::list<Move>::const_iterator it = std::find_if(moves.begin(), moves.end(), [t](const Move& m) { return m.promotionType() == t; });

									game.makeMove(*it);
									promoSelection = false;
									moves.clear();
								}
							}
						}
						else {
							if (s == selectedSquare) {
								selectedSquare = -1;
								moves.clear();
							}
							else {
								std::list<Move>::const_iterator it = std::find_if(moves.begin(), moves.end(), [s](const Move& m) { return m.to() == s; });

								if (it != moves.end()) {
									if (it->isPromotion()) {
										promoSelection = true;
										promoDelta = 7 * (1 - game.activePlayer());
										e = game.activePlayer() * 2 - 1;

										promoCol = it->to() % 8;

										std::list<Move> tmp;

										while (it != moves.end()) {
											tmp.push_back(*it);
											tmp.erase(it);
											it = std::find_if(moves.begin(), moves.end(), [s](const Move& m) { return m.to() == s; });
										}

										moves = tmp;
									}
									else {
										moves.clear();
										game.makeMove(*it);
									}
								}
								else {
									moves.clear();

									selectedSquare = s;
									std::for_each(game.possibleMoves().begin(), game.possibleMoves().end(), [&moves, selectedSquare](const Move& m) { if (m.from() == selectedSquare) moves.push_back(m); });
								}
							}
						}
					}

					break;
				}
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

		// Show the selection of a promotion
		if (promoSelection) {
			for (u8 i(Knight); i <= Queen; ++i) {
				s = (promoDelta + e * promoIndex[i]) * 8 + promoCol;
				sf::Vector2f pos((s % 8) * mTileSize, (7 - int(s / 8)) * mTileSize);

				if (s == hoveredSquare)
					tile.setFillColor(sf::Color(255, 216, 0));
				else
					tile.setFillColor(sf::Color::White);
				
				sprite.setTexture(textures[game.activePlayer()][i]);

				tile.setPosition(pos);
				sprite.setPosition(pos);
				
				mWindow.draw(tile);
				mWindow.draw(sprite);
			}
		}

		mWindow.display();
	}
}
