#include "ECSApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <fstream>

#define ASSETS_PATH "C:/workspace/c++/cmg/RoadMind/assets/"

using namespace std;

ECSApp::ECSApp()
{
	m_renderParams.SetPolygonMode(PolygonMode::k_fill);
	m_renderParams.EnableDepthTest(true);
	m_renderParams.EnableDepthBufferWrite(true);
	m_renderParams.EnableCullFace(true);
	m_renderParams.SetCullFace(CullFace::k_back);
}

ECSApp::~ECSApp()
{
	delete m_terrainMesh;
}


//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

void ECSApp::OnInitialize()
{
	m_renderDevice = new RenderDevice(GetWindow());
	m_debugDraw = new DebugDraw();
	m_terrainMesh = new Mesh();

	LoadResources();

	MaterialComponent material;
	material.SetShader(m_shaderRenderTerrain);
	material.SetUniform("s_textureGrass", m_textureGrass);
	material.SetUniform("s_textureRock", m_textureRock);
	material.SetUniform("s_textureGrassColormap", m_textureGrassColormap);
	material.SetUniform("u_textureScaleInv", 1.0f / 20.0f);
	material.SetUniform("u_color", Color::WHITE);

	m_worldHeightmap = new HeightmapTerrainManager(m_renderDevice, material, m_scene);
	m_worldDensity = new DensityTerrainManager(m_renderDevice, material, m_scene);
	m_world = m_worldDensity;

	GenerateTerrain();

	// Create systems
	m_meshRenderSystem = new MeshRenderSystem(*m_debugDraw, *m_renderDevice);
	m_arcBallControlSystem.SetMouse(GetMouse());
	m_arcBallControlSystem.SetUpAxis(Vector3f::UNITZ);
	m_systems.AddSystem(m_arcBallControlSystem);
	m_renderSystems.AddSystem(*m_meshRenderSystem);
	m_renderSystems.AddSystem(*m_worldDensity);
	m_renderSystems.AddSystem(*m_worldHeightmap);
	

	// Box 1
	/*MeshComponent meshComponent;
	TransformComponent transform;
	meshComponent.mesh = m_terrainMesh;
	transform.position = Vector3f::ZERO;
	m_scene.CreateEntity(transform, meshComponent, material);*/
	
	/*
	// Box 2
	transform.rotation = Quaternion(Vector3f::UNITZ, Math::HALF_PI);
	transform.position = Vector3f(-5.0f, 0.0f, 0.0f);
	material.SetShader(m_shader);
	material.SetUniform("u_color", Color::MAGENTA);
	material.SetUniform("s_diffuse", m_textureTest2);
	m_scene.CreateEntity(transform, meshComponent, material);

	// Box 3
	transform.rotation = Quaternion(Vector3f::UNITZ, -1.0f);
	transform.position = Vector3f(5.0f, 0.0f, 0.0f);
	transform.scale = Vector3f(0.5f);
	material.SetShader(m_shader);
	material.SetUniform("u_color", Color::RED);
	material.SetUniform("s_diffuse", m_textureTest2);
	m_scene.CreateEntity(transform, meshComponent, material);
	*/

	// Create the camera entity
	ArcBall arcBall;
	arcBall.SetSensitivity(0.005f);
	arcBall.SetFocus(Vector3f::ZERO);
	arcBall.SetDistance(50.0f);
	TransformComponent transform;
	transform.transform.rotation = Quaternion::LookAtRotation(
		Vector3f::UNITY, Vector3f::UNITZ);
	transform.transform.rotation.Rotate(Vector3f::UNITX, -0.7f);
	m_cameraEntity = m_scene.CreateEntity(transform, arcBall);

	MeshComponent mesh;
	mesh.mesh = m_vehicleMesh;
	transform = TransformComponent();
	transform.position = Vector3f(0.0f, 0.0f, 0.0f);
	material.SetShader(m_shader);
	material.SetUniform("u_color", Color::WHITE);
	material.SetUniform("s_diffuse", m_textureTest2);
	m_entityPlayer = m_scene.CreateEntity(transform, mesh, material);

	Reset();
}


void ECSApp::OnQuit()
{
	delete m_world;
	m_world = nullptr;
	delete m_renderDevice;
	m_renderDevice = nullptr;
	UnloadResources();
}

void ECSApp::OnResizeWindow(int width, int height)
{
	m_camera.SetAspectRatio(GetWindow()->GetAspectRatio());
}

void ECSApp::OnUpdate(float dt)
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

	// Ctrl+R
	if (ctrl && keyboard->IsKeyPressed(Keys::r))
	{
		Reset();
		printf("Reset\n");
	}

	// L: Reload resources
	if (keyboard->IsKeyPressed(Keys::l))
	{
		UnloadResources();
		LoadResources();
		printf("Reloaded resources\n");
	}

	// F4: Toggle Fullscreen Mode
	if (keyboard->IsKeyPressed(Keys::f4))
		GetWindow()->SetFullscreen(!GetWindow()->IsFullscreen());

	// ESCAPE: Quit
	if (keyboard->IsKeyPressed(Keys::escape))
	{
		Quit();
		return;
	}

	if (keyboard->IsKeyPressed(Keys::i))
	{
		PolygonMode::value_type mode = m_renderParams.GetPolygonMode();
		if (mode == PolygonMode::k_fill)
			mode = PolygonMode::k_line;
		else if (mode == PolygonMode::k_line)
			mode = PolygonMode::k_point;
		else if (mode == PolygonMode::k_point)
			mode = PolygonMode::k_fill;
		m_renderParams.SetPolygonMode(mode);
	}
	if (keyboard->IsKeyPressed(Keys::g))
		GenerateTerrain();
	if (ctrl && keyboard->IsKeyPressed(Keys::g))
	{
		m_world->Clear();
		if (m_world == m_worldDensity)
			m_world = m_worldHeightmap;
		else
			m_world = m_worldDensity;
	}

	TransformComponent* transform = m_scene.GetComponent<TransformComponent>(m_entityPlayer);
	float speed = 50.0f * dt;
	Vector3f forward = m_scene.GetComponent<TransformComponent>(m_cameraEntity)->rotation.GetForward();
	Vector3f up = Vector3f::UNITZ;
	Vector3f right = Vector3f::Normalize(Vector3f::Cross(forward, up));
	forward = Vector3f::Normalize(Vector3f::Cross(up, right));
	if (keyboard->IsKeyDown(Keys::w))
		transform->position += forward * speed;
	if (keyboard->IsKeyDown(Keys::s))
		transform->position -= forward * speed;
	if (keyboard->IsKeyDown(Keys::a))
		transform->position -= right * speed;
	if (keyboard->IsKeyDown(Keys::d))
		transform->position += right * speed;
	if (keyboard->IsKeyDown(Keys::e))
		transform->position += up * speed;
	if (keyboard->IsKeyDown(Keys::q))
		transform->position -= up * speed;
	m_scene.GetComponent<ArcBall>(m_cameraEntity)->SetFocus(m_scene.GetComponent<TransformComponent>(m_entityPlayer)->position);

	// Update ECS
	m_world->SetFocus(transform->position);
	m_world->UpdateChunks(dt);
	m_scene.UpdateSystems(m_systems, dt);
}

void ECSApp::OnRender()
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

	m_renderer.SetRenderParams(m_renderParams);
	m_renderer.ApplyRenderSettings(true);
	glPointSize(8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/*
	Graphics2D g(window);
	g.Clear(Color::BLACK);
	g.SetTransformation(Matrix4f::IDENTITY);*/

	// Set camera transform
	auto transform = m_scene.GetComponent<TransformComponent>(m_cameraEntity);
	auto arcBall = m_scene.GetComponent<ArcBall>(m_cameraEntity);
	m_camera.SetOrientation(transform->transform.rotation);
	m_camera.SetPosition(transform->transform.position);

	Matrix4f viewProjection = m_camera.GetViewProjectionMatrix();
	m_debugDraw->SetViewProjection(viewProjection);
	m_debugDraw->SetShaded(true);
	m_meshRenderSystem->SetCamera(&m_camera);

	/*Meters gridRadius = arcBall->distance * 2.0f;
	Color gridColor[3];
	gridColor[0] = Color(10, 10, 10);
	gridColor[1] = Color(50, 50, 50);
	gridColor[2] = Color(120, 120, 120);
	m_debugDraw->DrawGrid(
		Matrix4f::CreateRotation(Vector3f::UNITX, Math::HALF_PI),
		Vector3f(arcBall->GetFocus().x, 0.0f, -arcBall->GetFocus().y),
		1.0f, 10, 1, 2, gridColor[0], gridColor[1], gridRadius);*/
	m_debugDraw->BeginImmediate();

	m_scene.UpdateSystems(m_renderSystems, 0.0f);
}


//-----------------------------------------------------------------------------
// App Methods
//-----------------------------------------------------------------------------

void ECSApp::LoadResources()
{
	TextureParams texParams;
	texParams.SetWrap(TextureWrap::REPEAT);
	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);
	Texture::LoadTexture(m_textureRoad, ASSETS_PATH "textures/test1.png", texParams);
	Texture::LoadTexture(m_textureTest2, ASSETS_PATH "textures/test2.png", texParams);
	Texture::LoadTexture(m_textureTerrain, ASSETS_PATH "textures/white.png", texParams);
	Texture::LoadTexture(m_textureRock, ASSETS_PATH "textures/rock.png", texParams);
	Texture::LoadTexture(m_textureGrass, ASSETS_PATH "textures/grass.png", texParams);
	texParams.SetWrap(TextureWrap::CLAMP_TO_EDGE);
	Texture::LoadTexture(m_textureGrassColormap, ASSETS_PATH "textures/grass_colormap.png", texParams);
	//Mesh::Load(ASSETS_PATH "toyota_ae86.obj", m_vehicleMesh);
	//Mesh::Load(ASSETS_PATH "wheel.obj", m_meshWheel);
	m_vehicleMesh = Primitives::CreateCube();
	
	Shader::LoadShader(m_shader,
		ASSETS_PATH "shaders/shader_vs.glsl",
		ASSETS_PATH "shaders/shader_fs.glsl");
	Shader::LoadShader(m_shaderRenderTerrain,
		ASSETS_PATH "shaders/render_terrain_vs.glsl",
		ASSETS_PATH "shaders/render_terrain_fs.glsl");
}

void ECSApp::UnloadResources()
{
	delete m_font;
	m_font = nullptr;
	delete m_vehicleMesh;
	m_vehicleMesh = nullptr;
	delete m_meshWheel;
	m_meshWheel = nullptr;
	delete m_textureTest2;
	m_textureTest2 = nullptr;
	delete m_computeShader;
	m_computeShader = nullptr;
	delete m_shader;
	m_shader = nullptr;
}

void ECSApp::Reset()
{
	m_camera = Camera();
	m_camera.SetPosition(Vector3f::ZERO);
	m_camera.SetPerspective(
	GetWindow()->GetAspectRatio(), 1.4f, 0.1f, 1000.0f);
}

void ECSApp::GenerateTerrain()
{
	LoadComputeShader(m_worldHeightmap->m_shaderGenerateVertices,
		ASSETS_PATH "shaders/generate_heightmap_vertices_cs.glsl");
	LoadComputeShader(m_worldHeightmap->m_shaderGenerateNormals,
		ASSETS_PATH "shaders/generate_heightmap_normals_cs.glsl");

	LoadComputeShader(m_worldDensity->m_shaderGenerateDensity,
		ASSETS_PATH "shaders/1_build_densities.glsl");
	LoadComputeShader(m_worldDensity->m_shaderListNonEmptyCells,
		ASSETS_PATH "shaders/2_list_nonempty_cells.glsl");
	LoadComputeShader(m_worldDensity->m_shaderListVertices,
		ASSETS_PATH "shaders/3_list_verts_to_generate.glsl");
	LoadComputeShader(m_worldDensity->m_shaderGenerateVertices,
		ASSETS_PATH "shaders/4_gen_vertices.glsl");
	LoadComputeShader(m_worldDensity->m_shaderGenerateIndices,
		ASSETS_PATH "shaders/5_gen_indices.glsl");

	m_world->Clear();
}

void ECSApp::LoadComputeShader(Shader*& outShader, const Path & path)
{
	std::cout << "Loading shader: " << path.GetPath() << std::endl;
	Shader* shader;
	Error error = Shader::LoadComputeShader(shader, path);
	if (error.Passed())
	{
		if (outShader != nullptr)
			delete outShader;
		outShader = shader;
	}
	else
		std::cerr << error.GetText() << std::endl;
}
