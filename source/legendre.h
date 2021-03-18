#pragma once


/*
 * C++ functions to compute Legendre and associated Legendre Polynoms (without Phase term (-1)^m)
 *
 * Implemented by Mc-Zen
 * https://github.com/Mc-Zen
 *
 * with help of James A. Chappells Project
 * http://www.storage-b.com/math-numerical-analysis/1
 *
 */


#ifndef __LEGENDRE_H__
#define __LEGENDRE_H__

#include <cmath>


namespace VSTMath {


template<class T>
inline auto legendre_p0(const T& x) {
	return static_cast<T>(1);
}

template<class T>
inline auto legendre_p1(const T& x) {
	return x;
}

template<class T>
inline auto legendre_p2(const T& x) {
	return ((static_cast<T>(3) * x * x) - static_cast<T>(1)) / static_cast<T>(2);
}

// Legendre polynom
template<class T>
inline auto legendre(unsigned int n, const T& x) {
	switch (n) {
	case 0:
		return legendre_p0<T>(x);

	case 1:
		return legendre_p1<T>(x);

	case 2:
		return legendre_p2<T>(x);

	default:
		break;
	}

	/*  We could simply do this:
		  return (static_cast<T>(((2 * n) - 1)) * x * Pn(n - 1, x) -
			  (static_cast<T>(n - 1)) * Pn(n - 2, x)) / static_cast<T>(n);
		but it could be slow for large n */

	auto pnm1(legendre_p2<T>(x));
	auto pnm2(legendre_p1<T>(x));
	T pn;

	for (auto m = 3u; m <= n; ++m) {
		pn = ((static_cast<T>((2 * m) - 1)) * x * pnm1 - (static_cast<T>(m - 1) * pnm2)) / static_cast<T>(m);
		pnm2 = pnm1;
		pnm1 = pn;
	}

	return pn;
}

// Product of all integers from 1 to n with same parity (odd/even) as n
inline unsigned int doublefactorial(unsigned int n) {
	if (n == 0 || n == 1)
		return 1;
	return n * doublefactorial(n - 2);
}

// Recursive property of associated legendre polynoms
template<class T>
inline T assoc_legendre_next(int l, int m, T x, T Pl, T Plm1) {
	return ((2 * l + 1) * (x) * (Pl)-(l + m) * (Plm1)) / static_cast<T>(l + 1 - m);
}

template<class T>
inline T assoc_legendre_impl(int l, int m, T x, T sin_theta_power) {
	if (l < 0)
		return assoc_legendre_impl(-l - 1, m, x, sin_theta_power);

	if (m < 0)
	{
		int sign = (m & 1) ? -1 : 1;
		return sign * std::tgamma(static_cast<T>(l + m + 1)) / std::tgamma(static_cast<T>(l + 1 - m)) * assoc_legendre_impl(l, -m, x, sin_theta_power);
	}
	if (m > l) {
		return T{ 0 };
	}
	if (m == 0) {
		return legendre(l, x);
	}

	T p0 = static_cast<T>(doublefactorial(2 * m - 1)) * sin_theta_power;

	if (m & 1)
		p0 *= -1;
	if (m == l)
		return p0;
	T p1 = x * (2 * m + 1) * p0;
	int n = m + 1;

	while (n < l) {
		std::swap(p0, p1);
		p1 = assoc_legendre_next(n, m, x, p0, p1);
		++n;
	}

	if ((m & 1u) != 0) {
		p1 = -p1;
	}
	return p1;
}

// Associated Legendre polynom
template<class T>
inline auto assoc_legendre(int l, int m, T x) noexcept {
	auto d = static_cast<T>(pow(1 - x * x, T(abs(m)) / 2));
	return  assoc_legendre_impl(l, m, x, d);
}


}
#endif
