#pragma once


/*
 * Fast Trigonometry functions
 *
 * Implemented by Mc-Zen
 * https://github.com/Mc-Zen
 */


#ifndef __FAST_TRIG_H__
#define __FAST_TRIG_H__

#include <cmath>



namespace VSTMath {




constexpr int sin_lookup_size = 2000;

double sin_lookup[sin_lookup_size + 1];
double cos_lookup[sin_lookup_size + 1];

void init_cos_lookup() {
	constexpr double twopi = 2 * pi<double>();
	for (int i = 0; i < sin_lookup_size + 1; i++) {
		cos_lookup[i] = std::cos(i * twopi / sin_lookup_size);
	}
}

// cos lookup table. Takes optimized about 2/3 time of std::cos() but twice as long in Debug mode
double cos_lut(double x) {
	constexpr double factor = sin_lookup_size / (2 * pi<double>());
	return cos_lookup[std::abs((int)(x * factor) % sin_lookup_size)];
}

double sin_lut(double x) {
	constexpr double pi_halfs = pi<double>() / 2;
	return cos_lut(x - pi_halfs);
}

/*template<typename T>
inline T cos_approx2(T x) noexcept
{
	constexpr T r_twopi = 1. / (2. * 3.14159265358);
	T a = std::abs(x);
	// (x/2π - 0.25 + [x + 0.25])·16·(|x| - 0.5) + 0.225x(|x| - 1)
	return (x * r_twopi - T(.25) + std::floor(x + T(.25))) * T(16.) * (a - T(.5)) + T(.225) * x * (a - T(1.));
}*/
// Really good and smooth approximation. About twice to three times as fast as std::cos when optimized but
// 3 times slower in Debug mode
template<typename T>
inline T cos_approx(T x) noexcept {
	constexpr T tp = 1. / (2. * 3.14159265358);
	x *= tp;
	x -= T(.25) + std::floor(x + T(.25));
	x *= T(16.) * (std::abs(x) - T(.5));
	x += T(.225) * x * (std::abs(x) - T(1.));

	return x;
}
template<typename T>
constexpr T csin(T x) {
	auto z = x * x;
	return x * (1 - z * (1 / 6.0 - z * (1 / 120.0 - z * (1 / 5040.0 - z * (1 / 362880.0)))));
}
template<typename T>
constexpr T ccos(T x) {
	T sign = 1;
	if (x > pi<T>()) {
		sign = -1;
		x -= pi<T>();
	}
	auto z = x * x;
	return sign * (1 - z * (1.0 / 2.0 - z * (1.0 / 24.0 - z * (1.0 / 720.0 - z * (1.0 / 40320.0 - z * (1.0 / 3628800.0 - z * (1.0 / 479001600.0 - z*(1.0 / (13*14*479001600.0)))))))));
}
}
#endif
