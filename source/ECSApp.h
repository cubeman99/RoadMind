#pragma once

#include <cmgApplication/cmg_application.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "Camera.h"
#include "CommonTypes.h"
#include "ecs/MeshRenderSystem.h"
#include "HeightmapTerrainManager.h"
#include "DensityTerrainManager.h"


class ECSApp : public Application
{
public:
	ECSApp();
	~ECSApp();

	void LoadResources();
	void UnloadResources();
	void Reset();
	void GenerateTerrain();
	void LoadComputeShader(Shader*& outShader, const Path& path);

	void OnInitialize() override;
	void OnQuit() override;
	void OnResizeWindow(int width, int height) override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

private:
	RenderDevice* m_renderDevice;
	DebugDraw* m_debugDraw;
	PhysicsEngine* m_physicsEngine;
	Renderer m_renderer;
	RenderParams m_renderParams;

	TerrainManager* m_world;
	DensityTerrainManager* m_worldDensity;
	HeightmapTerrainManager* m_worldHeightmap;

	bool m_wireFrame;

	// ECS
	ECS m_scene;
	ECSSystemList m_systems;
	ECSSystemList m_renderSystems;
	MeshRenderSystem* m_meshRenderSystem;
	ArcBallControlSystem m_arcBallControlSystem;

	// Entities
	EntityHandle m_cameraEntity;
	EntityHandle m_entityPlayer;
	Camera m_camera;

	// Resources
	Shader* m_shader;
	Shader* m_computeShader;
	SpriteFont* m_font;
	Texture* m_textureRoad;
	Texture* m_textureTest2;
	Texture* m_textureTerrain;
	Texture* m_textureRock;
	Texture* m_textureGrass;
	Texture* m_textureGrassColormap;
	Mesh* m_vehicleMesh;
	Mesh* m_meshWheel;
	Mesh* m_terrainMesh;
	Shader* m_shaderRenderTerrain;
};

