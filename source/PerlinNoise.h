//----------------------------------------------------------------------------------------
//
//	siv::PerlinNoise
//	Perlin noise library for modern C++
//
//	Copyright (C) 2013-2018 Ryo Suzuki <reputeless@gmail.com>
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//----------------------------------------------------------------------------------------

# pragma once
#include <cmgCore/cmgBase.h>
#include <cmgMath/types/cmgVector3f.h>
# include <cstdint>
# include <numeric>
# include <algorithm>
# include <random>

namespace siv
{
	template<typename T>
	class PerlinNoise
	{
	private:

		int32 p[512];

		static T Fade(T t) noexcept
		{
			return t * t * t * (t * (t * T(6) - T(15)) + T(10));
		}

		static T Lerp(T t, T a, T b) noexcept
		{
			return a + t * (b - a);
		}

		static T Grad(int32 hash, T x, T y, T z) noexcept
		{
			const int32 h = hash & 15;
			const T u = h < 8 ? x : y;
			const T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
			return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
		}

	public:

		explicit PerlinNoise(uint32 seed = std::default_random_engine::default_seed)
		{
			reseed(seed);
		}

		template <class URNG>
		explicit PerlinNoise(URNG& urng)
		{
			reseed(urng);
		}

		void reseed(uint32 seed)
		{
			for (size_t i = 0; i < 256; ++i)
			{
				p[i] = i;
			}

			std::shuffle(std::begin(p), std::begin(p) + 256, std::default_random_engine(seed));

			for (size_t i = 0; i < 256; ++i)
			{
				p[256 + i] = p[i];
			}
		}

		template <class URNG>
		void reseed(URNG& urng)
		{
			for (size_t i = 0; i < 256; ++i)
			{
				p[i] = i;
			}

			std::shuffle(std::begin(p), std::begin(p) + 256, urng);

			for (size_t i = 0; i < 256; ++i)
			{
				p[256 + i] = p[i];
			}
		}

		T noise(T x) const
		{
			return noise(x, T(0.0), T(0.0));
		}

		T noise(T x, T y) const
		{
			return noise(x, y, T(0.0));
		}

		T noise(Vector2f point) const
		{
			return noise(T(point.x), T(point.y), T(0.0));
		}

		T noise(Vector3f point) const
		{
			return noise(T(point.x), T(point.y), T(point.z));
		}

		T noise(T x, T y, T z) const
		{
			const int32 X = static_cast<int32>(Math::Floor(x)) & 255;
			const int32 Y = static_cast<int32>(Math::Floor(y)) & 255;
			const int32 Z = static_cast<int32>(Math::Floor(z)) & 255;

			x -= Math::Floor(x);
			y -= Math::Floor(y);
			z -= Math::Floor(z);

			const T u = Fade(x);
			const T v = Fade(y);
			const T w = Fade(z);

			const int32 A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
			const int32 B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

			return Lerp(w, Lerp(v, Lerp(u, Grad(p[AA], x, y, z),
				Grad(p[BA], x - 1, y, z)),
				Lerp(u, Grad(p[AB], x, y - 1, z),
					Grad(p[BB], x - 1, y - 1, z))),
				Lerp(v, Lerp(u, Grad(p[AA + 1], x, y, z - 1),
					Grad(p[BA + 1], x - 1, y, z - 1)),
					Lerp(u, Grad(p[AB + 1], x, y - 1, z - 1),
						Grad(p[BB + 1], x - 1, y - 1, z - 1))));
		}

		T octaveNoise(T x, int32 octaves) const
		{
			T result = T(0.0);
			T amplitude = T(1.0);

			for (int32 i = 0; i < octaves; ++i)
			{
				result += noise(x) * amplitude;
				x *= T(2.0);
				amplitude *= T(0.5);
			}

			return result;
		}

		T octaveNoise(T x, T y, int32 octaves) const
		{
			T result = T(0.0);
			T amplitude = T(1.0);

			for (int32 i = 0; i < octaves; ++i)
			{
				result += noise(x, y) * amplitude;
				x *= T(2.0);
				y *= T(2.0);
				amplitude *= T(0.5);
			}

			return result;
		}

		T octaveNoise(T x, T y, T z, int32 octaves) const
		{
			T result = T(0.0);
			T amplitude = T(1.0);

			for (int32 i = 0; i < octaves; ++i)
			{
				result += noise(x, y, z) * amplitude;
				x *= T(2.0);
				y *= T(2.0);
				z *= T(2.0);
				amplitude *= T(0.5);
			}

			return result;
		}

		T noise0_1(T x) const
		{
			return noise(x) * T(0.5) + T(0.5);
		}

		T noise0_1(T x, T y) const
		{
			return noise(x, y) * T(0.5) + T(0.5);
		}

		T noise0_1(T x, T y, T z) const
		{
			return noise(x, y, z) * T(0.5) + T(0.5);
		}

		T octaveNoise0_1(T x, int32 octaves) const
		{
			return octaveNoise(x, octaves) * T(0.5) + T(0.5);
		}

		T octaveNoise0_1(T x, T y, int32 octaves) const
		{
			return octaveNoise(x, y, octaves) * T(0.5) + T(0.5);
		}

		T octaveNoise0_1(T x, T y, T z, int32 octaves) const
		{
			return octaveNoise(x, y, z, octaves) * T(0.5) + T(0.5);
		}
	};
}