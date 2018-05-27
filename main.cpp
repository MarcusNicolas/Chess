#include "Application.h"

int main()
{
	try {
		Application app(60);

		char p;
		std::string s("0");
		Player player;
		
		std::cout << "\nHuman side (w/b) ?\n";

		while (p != 'w' && p != 'b')
			std::cin >> p;

		if (p == 'w')
			player = White;
		else
			player = Black;

		std::cout << "\nThinking time (in s) ?\n";

		while (std::stof(s) <= 0)
			std::cin >> s;

		app.go(player, std::stof(s) * 1000);
	} catch (const std::exception& e) {
		std::cout << "\n\n/!\\ Exception : " << e.what() << "\n\n\n";
		system("PAUSE");
	}

	system("PAUSE");
	return 0;
}
