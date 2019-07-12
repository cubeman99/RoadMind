#include "ECSApp.h"
#include <cmgGraphics/cmgOpenGLIncludes.h>
#include <cmgGraphics/cmg_graphics.h>
#include <process.h>
#include <sstream>
#include <fstream>

#define ASSETS_PATH "C:/workspace/c++/cmg/RoadMind/assets/"

using namespace std;
using namespace cmg;


ECSApp::ECSApp()
{
	m_renderParams.SetPolygonMode(PolygonMode::k_fill);
	m_renderParams.EnableDepthTest(true);
	m_renderParams.EnableDepthBufferWrite(true);
	m_renderParams.EnableCullFace(true);
	m_renderParams.SetCullFace(CullFace::k_back);

	ResourceManager* resourceManager = GetResourceManager();
	resourceManager->AddPath(Path(ASSETS_PATH));
	resourceManager->AddShaderIncludePath(Path(ASSETS_PATH "shaders"));
}

ECSApp::~ECSApp()
{
}


//-----------------------------------------------------------------------------
// Overridden Methods
//-----------------------------------------------------------------------------

void ECSApp::OnInitialize()
{
	m_renderDevice = new RenderDevice(GetWindow());
	m_debugDraw = new DebugDraw();

	LoadResources();

	Material::sptr material = std::make_shared<Material>();
	material->SetShader(m_shaderRenderTerrain);
	material->SetUniform("s_textureGrass", m_textureGrass);
	material->SetUniform("s_textureRock", m_textureRock);
	material->SetUniform("s_textureGrassColormap", m_textureGrassColormap);
	material->SetUniform("u_textureScaleInv", 1.0f / 20.0f);
	material->SetUniform("u_color", Color::WHITE);
	MaterialComponent materialComponent;
	materialComponent.material = material;

	m_worldHeightmap = new HeightmapTerrainManager(m_renderDevice, materialComponent, m_scene);
	m_worldDensity = new DensityTerrainManager(m_renderDevice, materialComponent, m_scene);
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

	m_camera = Camera();
	m_camera.SetPosition(Vector3f::ZERO);
	m_camera.SetPerspective(
		GetWindow()->GetAspectRatio(), 1.4f, 0.1f, 1000.0f);

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
	material = std::make_shared<Material>();
	material->SetShader(m_shader);
	material->SetUniform("u_color", Color::WHITE);
	material->SetUniform("s_diffuse", m_textureTest2);
	materialComponent.material = material;
	m_entityPlayer = m_scene.CreateEntity(transform, mesh, materialComponent);

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
	
	m_renderer.SetRenderParams(m_renderParams);
	m_renderer.ApplyRenderSettings(true);
	glPointSize(8);
	
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

	m_debugDraw->BeginImmediate();

	m_meshRenderSystem->SetCamera(&m_camera);
	m_scene.UpdateSystems(m_renderSystems, 0.0f);
}


//-----------------------------------------------------------------------------
// App Methods
//-----------------------------------------------------------------------------

void ECSApp::LoadResources()
{
	ResourceManager* resourceManager = GetResourceManager();

	// Fonts
	m_font = SpriteFont::LoadBuiltInFont(BuiltInFonts::FONT_CONSOLE);

	// Textures
	TextureParams texParams;
	texParams.SetWrap(TextureWrap::REPEAT);
	resourceManager->LoadTexture(m_textureRoad, "textures/test1.png", texParams);
	resourceManager->LoadTexture(m_textureTest2, "textures/test2.png", texParams);
	resourceManager->LoadTexture(m_textureTerrain, "textures/white.png", texParams);
	resourceManager->LoadTexture(m_textureRock, "textures/rock.png", texParams);
	resourceManager->LoadTexture(m_textureGrass, "textures/grass.png", texParams);
	texParams.SetWrap(TextureWrap::CLAMP_TO_EDGE);
	resourceManager->LoadTexture(m_textureGrassColormap, "textures/grass_colormap.png", texParams);
	
	// Meshes
	m_vehicleMesh = Mesh::sptr(Primitives::CreateCube());
	
	// Shaders
	resourceManager->LoadShader(m_shader,
		"shader",
		"shaders/shader_vs.glsl",
		"shaders/shader_fs.glsl");
	resourceManager->LoadShader(m_shaderRenderTerrain,
		"render_terrain",
		"shaders/render_terrain_vs.glsl",
		"shaders/render_terrain_fs.glsl");
}

void ECSApp::UnloadResources()
{
	delete m_font;
	m_font = nullptr;
}

void ECSApp::Reset()
{
}

void ECSApp::GenerateTerrain()
{
	ResourceManager* resourceManager = GetResourceManager();

	resourceManager->LoadComputeShader(m_worldHeightmap->m_shaderGenerateVertices,
		"shaders/generate_heightmap_vertices_cs.glsl");
	resourceManager->LoadComputeShader(m_worldHeightmap->m_shaderGenerateNormals,
		"shaders/generate_heightmap_normals_cs.glsl");

	resourceManager->LoadComputeShader(m_worldDensity->m_shaderGenerateDensity,
		"shaders/1_build_densities.glsl");
	resourceManager->LoadComputeShader(m_worldDensity->m_shaderListNonEmptyCells,
		"shaders/2_list_nonempty_cells.glsl");
	resourceManager->LoadComputeShader(m_worldDensity->m_shaderListVertices,
		"shaders/3_list_verts_to_generate.glsl");
	resourceManager->LoadComputeShader(m_worldDensity->m_shaderGenerateVertices,
		"shaders/4_gen_vertices.glsl");
	resourceManager->LoadComputeShader(m_worldDensity->m_shaderGenerateIndices,
		"shaders/5_gen_indices.glsl");

	m_world->Clear();
}
