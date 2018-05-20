#include "GeometryApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <fstream>

static const char* SAVE_FILE_PATH = "geometry_save_file.gmt";


GeometryApp::GeometryApp()
{
	m_pointRadius = 5.0f;
	m_wireframe = false;
}

GeometryApp::~GeometryApp()
{
}

void GeometryApp::Save(const Path& path)
{
	File file = File(path);
	if (file.Open(FileAccess::WRITE, FileType::BINARY).Failed())
		return;

	unsigned int count = m_points.size();
	file.Write(&count, sizeof(unsigned int));
	for (unsigned int i = 0; i < m_points.size(); i++)
	{
		Point* point = m_points[i];
		file.Write(&point->position, sizeof(Vector2f));
		file.Write(&point->direction, sizeof(Vector2f));
		file.Write(&point->directional, sizeof(bool));
		file.Write(&point->exists, sizeof(bool));
		file.Write(&point->color, sizeof(Color));
	}
}

void GeometryApp::Load(const Path& path)
{
	File file = File(path);
	if (file.Open(FileAccess::READ, FileType::BINARY).Failed())
		return;

	unsigned int count;
	file.Read(&count, sizeof(unsigned int));
	m_points.resize(count);
	for (unsigned int i = 0; i < m_points.size(); i++)
	{
		Point* point = new Point();
		m_points[i] = point;
		file.Read(&point->position, sizeof(Vector2f));
		file.Read(&point->direction, sizeof(Vector2f));
		file.Read(&point->directional, sizeof(bool));
		file.Read(&point->exists, sizeof(bool));
		file.Read(&point->color, sizeof(Color));
	}
}

Point* GeometryApp::CreatePoint(const Vector2f& position, const Color& color)
{
	Point* p = new Point(position, color);
	m_points.push_back(p);
	return p;
}


Point* GeometryApp::CreatePoint(const Vector2f& position, const Vector2f& direction, const Color& color)
{
	Point* p = new Point(position, color);
	p->direction = Vector2f::Normalize(direction);
	p->directional = true;
	m_points.push_back(p);
	return p;
}


//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

static void DrawPoint(Graphics2D& g, const Vector2f& point, const Color& color)
{
	g.FillCircle(point, 4, color);
}

static void DrawArc(Graphics2D& g, const Biarc& arc, const Color& color)
{
	if (arc.radius == 0.0f)
	{
		g.DrawLine(arc.start, arc.end, color);
	}
	else
	{
		Vector2f v = arc.start;
		int count = (int) ((Math::Abs(arc.angle) / Math::TWO_PI) * 20) + 2;
		float angle = arc.angle / count;
		for (int j = 0; j < count; j++)
		{
			Vector2f vPrev = v;
			v.Rotate(arc.center, angle);
			g.DrawLine(vPrev, v, color);
		}
	}
}

void GeometryApp::DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color)
{
	DrawArc(g, arcs.first, color);
	DrawArc(g, arcs.second, color);
	//if (m_showDebug)
	{
		g.FillCircle(arcs.first.start, 2, color);
		g.FillCircle(arcs.first.end, 2, color);
		g.FillCircle(arcs.second.end, 2, color);
	}
}

//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

Vector2f c1, c2, c, d;
Vector2f l;
float r1 = 100.0f;
float r2 = 200.0f;

Vector2f p1, p2, t1, t2;

struct
{
	Vector2f p1, p2, t1, t2;
	BiarcPair arcs;
} a, b;

void GeometryApp::OnInitialize()
{
	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);

	m_hoverPoint = nullptr;

	a.p1 = Vector2f(100, 400);
	a.t1 = Vector2f(1, -1).Normalize();
	a.p2 = Vector2f(200, 200);
	a.t2 = Vector2f(0, -1).Normalize();

	b.p1 = Vector2f(600, 500);
	b.t1 = Vector2f(-1, -1).Normalize();
	b.p2 = Vector2f(400, 200);
	b.t2 = Vector2f(-0.8f, -1).Normalize();

	c1 = Vector2f(150, 250);
	c2 = Vector2f(480, 330);
	l = Vector2f(10, 460);

	c1 = Vector2f(100, 200);
	c2 = Vector2f(600, 200);
	p1 = Vector2f(200, 300);
	p2 = Vector2f(400, 280);

	//CreatePoint(Vector2f(127, 359), Vector2f(-0.20f, -0.98f), Color::RED);
	//CreatePoint(Vector2f(205, 225), Vector2f(0.61f, -0.79f), Color::RED);
	//CreatePoint(Vector2f(511, 307), Vector2f(-0.62f, -0.78f), Color::CYAN);
	//CreatePoint(Vector2f(327, 241), Vector2f(-0.74f, -0.67f), Color::CYAN);

	Load(SAVE_FILE_PATH);
}


void GeometryApp::OnQuit()
{
	delete m_font;
	m_font = nullptr;

	for (Point* point : m_points)
		delete point;
	m_points.clear();
}

void GeometryApp::OnUpdate(float dt)
{
	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();
	Window* window = GetWindow();
	MouseState mouseState = mouse->GetMouseState();
	Vector2f windowSize((float) window->GetWidth(), (float) window->GetHeight());
	m_mousePosition.x = (float) mouseState.x;
	m_mousePosition.y = (float) mouseState.y;

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
	if (keyboard->IsKeyPressed(Keys::i))
		m_wireframe = !m_wireframe;

	if (ctrl && keyboard->IsKeyPressed(Keys::s))
	{
		Save(SAVE_FILE_PATH);
		std::cout << "Saved to " << SAVE_FILE_PATH << std::endl;
	}
	if (ctrl && keyboard->IsKeyPressed(Keys::l))
	{
		Load(SAVE_FILE_PATH);
		std::cout << "Loaded " << SAVE_FILE_PATH << std::endl;
	}

	if (keyboard->IsKeyPressed(Keys::enter))
	{
		for (unsigned int i = 0; i < m_points.size(); i++)
		{
			Point* point = m_points[i];
			printf("m_points[%d] = CreatePoint(Vector2f(%d, %d), Vector2f(%.2f, %.2f));\n",
				i, (int) point->position.x, (int) point->position.y,
				point->direction.x, point->direction.y);
		}
	}

	if (ctrl && mouse->IsButtonPressed(MouseButtons::left))
		CreatePoint(m_mousePosition);

	if (m_isDragging)
	{
		if (ctrl)
		{
			m_hoverPoint->direction = Vector2f::Normalize(
				m_mousePosition - m_hoverPoint->position);
		}
		else
		{
			m_hoverPoint->position = m_mousePosition;
		}
		if (!mouse->IsButtonDown(MouseButtons::left))
			m_isDragging = false;
	}
	else
	{
		m_hoverPoint = nullptr;
		for (int i = (int) m_points.size() - 1; i >= 0; i--)
		{
			Point* point = m_points[i];
			if (m_mousePosition.DistTo(point->position) <= m_pointRadius * 2.0f)
			{
				m_hoverPoint = point;
				break;
			}
		}
		if (m_hoverPoint != nullptr && mouse->IsButtonPressed(MouseButtons::left))
			m_isDragging = true;
	}

	if (ctrl && mouse->IsButtonDown(MouseButtons::left))
		c1 = m_mousePosition;
	if (ctrl && mouse->IsButtonDown(MouseButtons::right))
		c2 = m_mousePosition;

	if (!ctrl && mouse->IsButtonDown(MouseButtons::left))
		p1 = m_mousePosition;
	if (!ctrl && mouse->IsButtonDown(MouseButtons::right))
		p2 = m_mousePosition;
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

void GeometryApp::OnRender()
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	Graphics2D g(window);
	g.Clear(Color::BLACK);
	g.SetTransformation(Matrix4f::IDENTITY);

	// Draw HUD
	Matrix4f projection = Matrix4f::CreateOrthographic(0.0f,
		(float) window->GetWidth(), (float) window->GetHeight(),
		0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection.m);

	using namespace std;
	std::stringstream ss;
	ss << "GEOMETRY TEST" << endl;

	g.DrawString(m_font, ss.str(), Vector2f(5, 5), Color::YELLOW);
	
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw all points
	for (Point* point : m_points)
	{
		g.FillCircle(point->position, m_pointRadius, point->color);
		g.DrawCircle(point->position, m_pointRadius, Color::WHITE);
		if (point->directional)
		{
			g.DrawLine(point->position, point->position +
				(point->direction * m_pointRadius * 3.0f), point->color);
		}
	}

	if (m_hoverPoint != nullptr)
	{
		g.DrawCircle(m_hoverPoint->position,
			m_pointRadius * 1.5f, m_hoverPoint->color);
	}

	m_arcPoints = m_points.data();

	/*
	BiarcPair arcs1 = BiarcPair::Interpolate(
		m_arcPoints[0]->position, m_arcPoints[0]->direction,
		m_arcPoints[1]->position, m_arcPoints[1]->direction);
	BiarcPair arcs2 = BiarcPair::Interpolate(
		m_arcPoints[2]->position, m_arcPoints[2]->direction,
		m_arcPoints[3]->position, m_arcPoints[3]->direction);
	DrawArcs(g, arcs1, Color::RED);
	DrawArcs(g, arcs2, Color::CYAN);

	p1 = arcs1.second.end;
	p2 = arcs2.second.end;
	c1 = arcs1.second.center;
	c2 = arcs2.second.center;
	BiarcPair web = CalcWebbedCircle(arcs1, arcs2, 30.0f);
	DrawArcs(g, web, Color::YELLOW);

	Circle2f a(arcs1.second.center, arcs1.second.radius);
	Circle2f b(arcs2.second.center, arcs2.second.radius);
	Line2f outerTangent = CalcOuterTangent(a, b);
	g.DrawLine(outerTangent.end1, outerTangent.end2, Color::MAGENTA);
	*/

	Array<Vector2f> points;
	for (Point* p : m_points)
		points.push_back(p->position);
	FillShape(g, points, Color::GREEN);
	/*
	DrawPoint(g, c1, Color::RED);
	DrawPoint(g, c2, Color::RED);
	g.DrawCircle(c1, r1, Color::RED);
	g.DrawCircle(c2, r2, Color::CYAN);

	g.DrawLine(Vector2f(0, l.y), Vector2f(800, l.y), Color::YELLOW);

	Vector2f centers[4];
	float radii[4];
	CalcCircle(c1, c2, l.y, centers, radii);
	//CalcCircle(-b, A, -B, r, R, centers[2], radii[2], centers[3], radii[3]);
	g.DrawCircle(centers[0], radii[0], Color::MAGENTA);
	g.DrawCircle(centers[1], radii[1], Color::DARK_MAGENTA);
	g.DrawCircle(centers[2], radii[2], Color::GREEN);
	g.DrawCircle(centers[3], radii[3], Color::DARK_GREEN);
	*/

	//a.arcs = BiarcPair::Interpolate(a.p1, a.t1, a.p2, a.t2);
	//b.arcs = BiarcPair::Interpolate(b.p1, b.t1, b.p2, b.t2);
	//DrawArcs(g, a.arcs, Color::RED);
	//DrawArcs(g, b.arcs, Color::BLUE);
	//g.DrawLine(a.p2, b.p2, Color::GRAY);

	/*
	float r1 = p1.DistTo(c1);
	float r2 = p2.DistTo(c2);
	g.DrawCircle(c1, r1, Color::RED);
	g.DrawCircle(c2, r2, Color::BLUE);
	g.DrawLine(p1, p2, Color::GRAY);

	Biarc arc = CalcWebbedCircle(p1, p2, c1, c2);
	DrawPoint(g, arc.start, Color::MAGENTA);
	DrawPoint(g, arc.end, Color::MAGENTA);
	DrawArc(g, arc, Color::YELLOW);
	*/
}
