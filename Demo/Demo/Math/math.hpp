#pragma once
namespace Demo
{
	/**
	 * @brief Just a very basic math class.
	 */
	class Math
	{
	public:

		template<typename T>
		static constexpr T PI = static_cast<T>(3.14159265358979323846264338327950288L);

		template <typename T = float>
		static constexpr auto Lerp(const T& start, const T& end, const T& t)
		{
			return start + t * (end - start);
		}

		template <typename T = float>
		static constexpr auto LerpCos(const T& start, const T& end, const T& t)
		{
			return start + ((float)-cos((float)3.14159265358979323846264f * t) / (float)2.0f + (float)0.5f) * (end - start);
		}

		template <typename T = float>
		static constexpr auto SmoothStep(const T& start, const T& end, const T& t)
		{
			return start + ((t * t) * (3 - (t + t)) ) * (end - start);
		}

		template <typename T = float>
		static constexpr auto Clamp(const T& x, const T& lowerlimit, const T& upperlimit)
		{
			float y = 0.0f;
			if (x < lowerlimit)
				y = lowerlimit;
			if (x > upperlimit)
				y = upperlimit;
			return y;
		}

		template <typename T = float>
		static constexpr auto Acceleration(const T& start, const T& end, const T& t)
		{
			return start + (t*t) * (end - start);
		}

		template <typename T = float>
		static constexpr auto Deceleration(const T& start, const T& end, const T& t)
		{
			return start + (1.0f - (1.0f - t)*2.0f) * (end - start);
		}

		template <typename T = float>
		static constexpr auto Map(const T& x, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
		{
			return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		}

	};
}
