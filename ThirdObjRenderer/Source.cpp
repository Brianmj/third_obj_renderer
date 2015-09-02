#include <Windows.h>
#include "app.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	App app;
	return app.run();
}