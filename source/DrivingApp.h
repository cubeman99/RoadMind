#ifndef _GEOMETRY_APP_H_
#define _GEOMETRY_APP_H_

#include <cmgApplication/cmg_application.h>
#include <cmgMath/cmg_math.h>
#include <cmgPhysics/cmg_physics.h>
#include <map>
#include <vector>
#include "Biarc.h"
#include "RoadNetwork.h"
#include "Camera.h"
#include "Driver.h"
#include "ToolSelection.h"
#include "ToolDraw.h"
#include "MainApp.h"


class DrivingApp : public Application
{
public:
	DrivingApp();
	~DrivingApp();

	void OnInitialize() override;
	void OnQuit() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

private:
	SpriteFont* m_font;
};


#endif // _GEOMETRY_APP_H_