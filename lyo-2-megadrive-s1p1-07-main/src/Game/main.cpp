#include "pch.h"
#include "main.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int cmdShow)
{
	// RETRO FULLSCREEN
	//CPU_RUN(512, 256, true, true);

	// MODERN
	CPU_RUN(1024, 576, true, true);

	//FULL-HD (Release uniqument)
	//CPU_RUN(1920, 1080, true);

    return 0;
}