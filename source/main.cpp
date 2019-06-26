
#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>
#include <cmgGraphics/cmg_graphics.h>
#include <cmgInput/cmg_input.h>
#include <cmgApplication/cmg_application.h>
#include <gl/GL.h>
#include <ctime>
#include <stdio.h>
#include "MainApp.h"
#include "GeometryApp.h"
#include "DrivingApp.h"
#include "ECSApp.h"

int main(int argc, char* argv[])
{
	srand((unsigned int) time(nullptr));

	//MainApp app;
	ECSApp app;
	//GeometryApp app;
	//DrivingApp app;
	app.Initialize("Road Mind", 800, 600);
	app.Run();

	return 0;
}
