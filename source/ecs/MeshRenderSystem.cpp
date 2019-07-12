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
	Material::sptr material = ((MaterialComponent*) components[2])->material;
	Shader::sptr shader = material->GetShader();

	if (shader == nullptr)
	{
		m_debugDraw.DrawMesh(meshComponent->mesh.get(),
			transform->transform.GetMatrix(),
			Color::GREEN);
	}
	else
	{
		Matrix4f mvp = m_camera->GetViewProjectionMatrix() *
			transform->transform.GetMatrix();
		if (shader->HasUniform("u_mvp"))
			m_renderDevice.SetShaderUniform(shader, "u_mvp", mvp);
		if (shader->HasUniform("u_eyePos"))
			m_renderDevice.SetShaderUniform(shader, "u_eyePos", m_camera->GetPosition());

		uint32 samplerSlot = 0;
		for (auto it = material->uniforms_begin(); it != material->uniforms_end(); it++)
		{
			String name = it->first;
			const UniformValue::Value& value = it->second.value;
			if (shader->GetUniform(name) != nullptr)
			{
				if (it->second.type == UniformType::k_texture)
				{
					m_renderDevice.SetTextureSampler(shader, name, it->second.texture, samplerSlot);
					samplerSlot++;
				}
				else if (it->second.type == UniformType::k_vec4)
					m_renderDevice.SetShaderUniform(shader, name, value.vec4);
				else if (it->second.type == UniformType::k_vec3)
					m_renderDevice.SetShaderUniform(shader, name, value.vec3);
				else if (it->second.type == UniformType::k_vec2)
					m_renderDevice.SetShaderUniform(shader, name, value.vec2);
				else if (it->second.type == UniformType::k_float)
					m_renderDevice.SetShaderUniform(shader, name, value.float32_value);
				else if (it->second.type == UniformType::k_unsigned_int)
					m_renderDevice.SetShaderUniform(shader, name, value.float32_value);
				else if (it->second.type == UniformType::k_uvec3)
					m_renderDevice.SetShaderUniform(shader, name, value.uvec3);
				else if (it->second.type == UniformType::k_uvec2)
					m_renderDevice.SetShaderUniform(shader, name, value.uvec2);
				else if (it->second.type == UniformType::k_ivec3)
					m_renderDevice.SetShaderUniform(shader, name, value.ivec3);
				else if (it->second.type == UniformType::k_ivec2)
					m_renderDevice.SetShaderUniform(shader, name, value.ivec2);
				else
					CMG_ASSERT(false);
			}
		}
		m_renderDevice.Draw(nullptr, shader, meshComponent->mesh);
	}
}
