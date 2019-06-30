#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "CommonTypes.h"


struct UniformValue
{
	UniformValue() {}
	~UniformValue() {}
	UniformValue(Texture* texture) { this->value.texture = texture; type = UniformType::k_texture; }
	UniformValue(float32 value) { this->value.float32_value = value; type = UniformType::k_float; }
	UniformValue(uint32 value) { this->value.uint32_value = value; type = UniformType::k_integer; }
	UniformValue(const Matrix3f& mat3) { this->value.mat4 = Matrix4f(mat3); type = UniformType::k_matrix; }
	UniformValue(const Matrix4f& mat4) { this->value.mat4 = mat4; type = UniformType::k_matrix; }
	UniformValue(const Vector2f& vec2) { this->value.vec2 = vec2; type = UniformType::k_vec2; }
	UniformValue(const Vector3f& vec3) { this->value.vec3 = vec3; type = UniformType::k_vec3; }
	UniformValue(const Vector4f& vec4) { this->value.vec4 = vec4; type = UniformType::k_vec4; }
	UniformValue(const Color& color) { this->value.vec4 = color.ToVector4f(); type = UniformType::k_vec4; }
	UniformValue(const Vector2ui& uvec2) { this->value.uvec2 = uvec2; type = UniformType::k_uvec2; }
	UniformValue(const Vector3ui& uvec3) { this->value.uvec3 = uvec3; type = UniformType::k_uvec3; }
	UniformValue(const Vector2i& ivec2) { this->value.ivec2 = ivec2; type = UniformType::k_ivec2; }
	UniformValue(const Vector3i& ivec3) { this->value.ivec3 = ivec3; type = UniformType::k_ivec3; }

	union Value
	{
		Value()
		{
			memset(this, 0, sizeof(Value));
		}
		Value(const Value& copy)
		{
			*this = copy;
		}
		~Value() {}
		void operator=(const Value& copy)
		{
			memcpy(this, &copy, sizeof(Value));
		}

		Texture* texture;
		float32 float32_value;
		uint32 uint32_value;
		Vector2f vec2;
		Vector3f vec3;
		Vector4f vec4;
		Vector2i ivec2;
		Vector3i ivec3;
		Vector2ui uvec2;
		Vector3ui uvec3;
		Matrix3f mat3;
		Matrix4f mat4;
	};

	Value value;
	UniformType::value_type type;
};


struct MaterialComponent : public ECSComponent<MaterialComponent>
{
	Shader* shader = nullptr;
	Map<String, UniformValue> uniforms;

	void SetShader(Shader* shader)
	{
		this->shader = shader;
	}

	template <typename T>
	void SetUniform(const String& name, T value)
	{
		uniforms[name] = UniformValue(value);
	}
};

