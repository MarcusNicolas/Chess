#include "Application.h"

int main()
{
	try {
		Application app(60);
		app.go();
	}
	catch (const std::exception& e) {
		std::cout << "\n\n/!\\ Exception : " << e.what() << "\n\n\n";
		system("PAUSE");
	}

	system("PAUSE");
	return 0;
}
