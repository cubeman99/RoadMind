#include "DrivingApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <fstream>

static const char* SAVE_FILE_PATH = "geometry_save_file.gmt";


DrivingApp::DrivingApp()
{
}

DrivingApp::~DrivingApp()
{
}


//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

void DrivingApp::OnInitialize()
{
	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);
}


void DrivingApp::OnQuit()
{
	delete m_font;
	m_font = nullptr;
}

void DrivingApp::OnUpdate(float dt)
{
	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();
	Window* window = GetWindow();
	MouseState mouseState = mouse->GetMouseState();
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());
	Vector2f mousePosition;
	mousePosition.x = (float) mouseState.x;
	mousePosition.y = (float) mouseState.y;

	bool ctrl = (keyboard->IsKeyDown(Keys::left_control) ||
		keyboard->IsKeyDown(Keys::right_control));
	bool shift = (keyboard->IsKeyDown(Keys::left_shift) ||
		keyboard->IsKeyDown(Keys::right_shift));
	int scroll = mouseState.z - mouse->GetPrevMouseState().z;

	// ESCAPE: Quit
	if (keyboard->IsKeyPressed(Keys::escape))
	{
		Quit();
		return;
	}


}

static void DrawArrowHead(Graphics2D& g, const Vector2f& position, const Vector2f& direction, float radius, const Color& color)
{
	Radians angle = Math::PI * 0.25f;
	Vector2f end1 = -direction;
	end1 = end1.Rotate(angle);
	Vector2f end2 = -direction;
	end2 = end2.Rotate(-angle);
	g.DrawLine(position, position + (end1 * radius), color);
	g.DrawLine(position, position + (end2 * radius), color);
}

void DrivingApp::OnRender()
{
	Window* window = GetWindow();
	MouseState mouseState = GetMouse()->GetMouseState();
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDepthMask(false);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, window->GetWidth(), window->GetHeight());

	Graphics2D g(window);
	g.Clear(Color::BLACK);
	g.SetTransformation(Matrix4f::IDENTITY);
	//
	//Matrix4f projection = Matrix4f::IDENTITY;
	//Matrix4f view = m_camera.GetWorldToCameraMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf(m_camera.GetViewProjectionMatrix().m);

	//// Draw HUD
	//Matrix4f projection = Matrix4f::CreateOrthographic(0.0f,
	//	(float) window->GetWidth(), (float) window->GetHeight(),
	//	0.0f, -1.0f, 1.0f);
	//glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf(projection.m);

	//using namespace std;
	//std::stringstream ss;
	//ss << "GEOMETRY TEST" << endl;

	//g.DrawString(m_font, ss.str(), Vector2f(5, 5), Color::YELLOW);
	//

}
