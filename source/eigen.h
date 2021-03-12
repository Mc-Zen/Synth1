
#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <iostream>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


namespace Roggenburg {

	template<class T, int d>
	class Vector {
		std::array<T, d> values;
	public:
		/*Vector(const std::initializer_list<T>& elems) {
			std::copy(elems.begin(), elems.end(), values);
		}*/
		Vector(const T(&elems)[d]) {
			std::copy(std::begin(elems), std::end(elems), values);
		}
		Vector(const T& value) {
			std::fill(values.begin(), values.end(), value);
		}

		template<class T, int d>
		friend std::ostream& operator<<(std::ostream& os, const Vector<T, d>& v) {
			os << "(";
			for (const T& a : v.values) os << "a";
			return os << ")";
		}
	};
	template<class T, class F, int n>
	class EigenvalueProblem {

		std::array<F, n> eigenFunctions;
		std::array<T, n> eigenValues_sq; // Always use square of ev because taking sqrt is expensive
		std::array<T, n> weights;

		T evaluate(T time, T x) {
			T result{ 0 };
			int i = 0;
			for (int i = 0; i < n; i++) {
				T omega = eigenValues[i];
				result += weights[i] * eigenFunctions[i](x) * std::sin(omega * t);
			}
			return result;
		}
	};

	/*void f() {

		Vector<float, 3> a({ 4,5 });
		typedef Vector<float, 3> Vec;
		Vec b(4);
	}*/

	template<class T, int d, int n>
	class EigenvalueProblemBase {
		T operator()(T t, Vector<T, d> x) = 0;
	};

	template<class T, int n>
	class EigenvalueProblemBase1D : public EigenvalueProblemBase<T, 1, n> {
	};
	template<class T, int n>
	class EigenvalueProblemBase2D : public EigenvalueProblemBase<T, 2, n> {
	};


}
