
#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include <cmgGraphics/cmg_graphics.h>
#include <cmgInput/cmg_input.h>
#include <cmgApplication/cmg_application.h>
#include <gl/GL.h>
#include <ctime>
#include <stdio.h>
#include "MainApp.h"


int main(int argc, char* argv[])
{
	srand((unsigned int) time(nullptr));

	//TestApp app;
	MainApp app;
	app.Initialize("Road Mind", 800, 600);
	app.Run();

	return 0;
}
