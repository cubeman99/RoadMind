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

	// ECS
	ECS m_scene;
	ECSSystemList m_systems;
	ECSSystemList m_renderSystems;
	MeshRenderSystem* m_meshRenderSystem;
	ArcBallControlSystem m_arcBallControlSystem;

	// Entities
	ECSEntity* m_cameraEntity;
	ECSEntity* m_entityPlayer;
	Camera m_camera;

	// Resources
	SpriteFont::sptr m_font;
	Shader::sptr m_shader;
	Shader::sptr m_computeShader;
	Shader::sptr m_shaderRenderTerrain;
	Texture::sptr m_textureRoad;
	Texture::sptr m_textureTest2;
	Texture::sptr m_textureTerrain;
	Texture::sptr m_textureRock;
	Texture::sptr m_textureGrass;
	Texture::sptr m_textureGrassColormap;
	Mesh::sptr m_vehicleMesh;
	Mesh::sptr m_meshWheel;
};

