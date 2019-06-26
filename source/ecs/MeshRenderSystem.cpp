#include "MeshRenderSystem.h"

MeshRenderSystem::MeshRenderSystem(DebugDraw& debugDraw, RenderDevice& renderDevice)
	: BaseECSSystem()
	, m_debugDraw(debugDraw)
	, m_renderDevice(renderDevice)
	, m_camera(nullptr)
{
	AddComponentType<TransformComponent>();
	AddComponentType<MeshComponent>();
	AddComponentType<MaterialComponent>();
}


void MeshRenderSystem::SetCamera(Camera* camera)
{
	m_camera = camera;
}
void MeshRenderSystem::UpdateComponents(float delta, BaseECSComponent** components)
{
	TransformComponent* transform = (TransformComponent*) components[0];
	MeshComponent* meshComponent = (MeshComponent*) components[1];
	MaterialComponent* material = (MaterialComponent*) components[2];

	if (material->shader == nullptr)
	{
		m_debugDraw.DrawMesh(meshComponent->mesh,
			transform->transform.GetMatrix(),
			Color::GREEN);
	}
	else
	{

		Matrix4f mvp = m_camera->GetViewProjectionMatrix() *
			transform->transform.GetMatrix();
		m_renderDevice.SetShaderUniform(material->shader, "u_mvp", mvp);
		uint32 samplerSlot = 0;
		for (auto it = material->uniforms.begin(); it != material->uniforms.end(); it++)
		{
			String name = it->first;
			const UniformValue::Value& value = it->second.value;
			if (it->second.type == UniformType::k_texture)
			{
				m_renderDevice.SetTextureSampler(material->shader, name, value.texture, samplerSlot);
				samplerSlot++;
			}
			else if (it->second.type == UniformType::k_vec4)
				m_renderDevice.SetShaderUniform(material->shader, name, value.vec4);
			else if (it->second.type == UniformType::k_vec3)
				m_renderDevice.SetShaderUniform(material->shader, name, value.vec3);
			else if (it->second.type == UniformType::k_vec2)
				m_renderDevice.SetShaderUniform(material->shader, name, value.vec2);
			else if (it->second.type == UniformType::k_float)
				m_renderDevice.SetShaderUniform(material->shader, name, value.float32_value);
			else
				CMG_ASSERT(false);
		}
		m_renderDevice.Draw(nullptr, material->shader, meshComponent->mesh);
	}
}
