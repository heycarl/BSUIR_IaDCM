#include "Battery.hpp"

void after_action_handler() {
	system("CLS");
	std::cout << "Ctrl" << std::endl
			  << "> Press Q to Quit" << std::endl
			  << "> Press S to Suspend" << std::endl
			  << "> Press H to Hibernate" << std::endl
			  << "> Press P to Print stats" << std::endl;
	Sleep(200);
}

int main()
{
	after_action_handler();
	while (!(GetKeyState('Q') & 0x8000)) {
		if (GetKeyState('S') & 0x8000) {
			SetSuspendState(FALSE, FALSE, FALSE); // Sleep
		}
		else if (GetKeyState('H') & 0x8000) {
			SetSuspendState(TRUE, FALSE, FALSE); // Hibernate
		}
		else if (GetKeyState('P') & 0x8000) {
			Battery battery;
			std::cout << battery << std::endl;
		}
	}
	return 0;
}