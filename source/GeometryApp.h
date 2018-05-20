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

struct Point;

struct Shape
{
	virtual int GetNumPoints() const = 0;
	virtual Point* GetPoint(int index) = 0;
};

struct Point : public Shape
{

	Vector2f position;
	Vector2f direction;
	bool exists;
	bool directional;
	Color color;

	Point()
		: position(Vector2f::ZERO)
		, color(Color::WHITE)
		, exists(true)
		, directional(false)
		, direction(Vector2f::UNITX)
	{
	}

	Point(const Vector2f& position, const Color& color)
		: position(position)
		, color(color)
		, exists(true)
		, directional(false)
		, direction(Vector2f::UNITX)
	{
	}

	inline int GetNumPoints() const override { return 1; }
	inline Point* GetPoint(int index) override { return this; }
};

struct PointDirection : public Shape
{
	Point point;
	Vector2f direction;
	inline int GetNumPoints() const override { return 1; }
	inline Point* GetPoint(int index) override { return &point; }
};

class GeometryApp : public Application
{
public:
	GeometryApp();
	~GeometryApp();

	void Save(const Path& path);
	void Load(const Path& path);

	Point* CreatePoint(const Vector2f& position, const Color& color = Color::WHITE);
	Point* CreatePoint(const Vector2f& position, const Vector2f& direction, const Color& color = Color::WHITE);

	void OnInitialize() override;
	void OnQuit() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

private:
	void DrawArcs(Graphics2D& g, const BiarcPair& arcs, const Color& color);

	Array<Point*> m_points;
	Array<Shape> m_shapes;
	Vector2f m_mousePosition;
	SpriteFont* m_font;
	Point* m_hoverPoint;

	Point** m_arcPoints;

	float m_pointRadius;
	bool m_isDragging;
	bool m_wireframe;
};


#endif // _GEOMETRY_APP_H_