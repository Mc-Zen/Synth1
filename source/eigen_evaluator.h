#pragma once 

#include <algorithm>
#include <array>
#include <iostream>
#include <complex>
#include <vector>
#include <numeric>
#include <functional>
#include <cmath>

namespace VSTMath
{

/*
 * Preparation of some definitions
 */
template <class T>
using complex = std::complex<T>;
template <class T, int N>
using array = std::array<T, N>;

template <typename T>
constexpr T pi()
{
	return static_cast<T>(3.1415926535897932384626);
}

/*
 * Basic class for representing a fixed length vector (of dimension d) of type T.
 * T will probably be float or double
 */
template <class T, int d>
class Vector
{
	array<T, d> values;

public:
	Vector(const T(&elems)[d])
	{
		std::copy(std::begin(elems), std::end(elems), values.begin());
	}
	Vector(const std::initializer_list<T> elems)
	{
		size_t h = elems.size() < d ? elems.size() : d;
		std::copy(std::begin(elems), std::begin(elems) + h, values.begin());
		std::fill(values.begin() + h, values.end(), T{});
	}
	Vector(const T& value = 0)
	{
		std::fill(values.begin(), values.end(), value);
	}

	// Unchecked access (faster)
	const T& operator[](int i) const { return values[i]; }
	T& operator[](int i) { return values[i]; }
	// Runtime checked access (saver)
	const T& at(int i) const { values.at(i); }
	T& at(int i) { return values.at(i); }

	Vector& operator+=(const T& c) { return apply(std::plus<T>(), c); }
	Vector& operator-=(const T& c) { return apply(std::minus<T>(), c); }
	Vector& operator*=(const T& c) { return apply(std::multiplies<T>(), c); }
	Vector& operator/=(const T& c) { return apply(std::divides<T>(), c); }
	Vector& operator+=(const Vector& v) { return apply(std::plus<T>(), v); }
	Vector& operator-=(const Vector& v) { return apply(std::minus<T>(), v); }

	Vector operator+(const T& c) const { Vector result(*this); return result += c; }
	Vector operator-(const T& c) const { Vector result(*this); return result -= c; }
	Vector operator*(const T& c) const { Vector result(*this); return result *= c; }
	Vector operator/(const T& c) const { Vector result(*this); return result /= c; }
	Vector operator+(const Vector& v) const { Vector result(*this); return result += v; }
	Vector operator-(const Vector& v) const { Vector result(*this); return result -= v; }

	template<class F> Vector& apply(F f) {
		std::for_each(values.begin(), values.end(), f); return *this;
	}
	template<class F> Vector& apply(F f, const T& c) {
		std::for_each(values.begin(), values.end(), [&](T& v) { v = f(v, c); }); return *this;
	}
	template<class F> Vector& apply(F f, const Vector& v) {
		std::transform(values.begin(), values.end(), v.values.begin(), values.begin(), f); return *this;
	}

	// Inner product of two vectors
	T operator*(const Vector& v) const {
		return std::inner_product(values.begin(), values.end(), v.values.begin(), T{ 0 });
	}

	friend std::ostream& operator<<(std::ostream& os, const Vector<T, d>& v)
	{
		os << "(";
		for (int i = 0; i < d - 1; i++)	os << v.values[i] << ",";
		return os << v.values[d - 1] << ")";
	}

	size_t size() const { return values.size(); }
};

/*
 * (Abstract) eigenvalue problem base class that implements the main procedure with eigenfunctions and -values
 * and declares methods to evaluate the eigenfunctions, -values and weights for the i-th eigenvalue.
 */
template <class T, int d, int N>
class EigenvalueProblem
{
public:
	// Evaluate for next time step at spatial position xOut.
	T next(const Vector<T, d> xOut)
	{
		evolve(deltaT);
		return evaluate(time, xOut);
	}

	// Evaluate for next time frame and give audio input at position xIn
	T next(const Vector<T, d> xOut, const Vector<T, d> xIn, T amplitudeIn)
	{
		pinchDelta(xIn, amplitudeIn);
		return next(xOut);
	}

	// Evaluate for next time step at [channels] spatial positions for Stereo or multichannel processing.
	template <int channels>
	array<T, channels> next(const array<Vector<T, d>, channels>& xOuts)
	{
		evolve(deltaT);
		array<T, channels> out;
		for (int i = 0; i < channels; i++)
			out[i] = evaluate(time, xOuts[i]);
		return out;
	}

	// Same but with external audio input
	template <int channels>
	array<T, channels> next(const array<Vector<T, d>, channels>& xOuts, const Vector<T, d> xIn, T amplitudeIn)
	{
		pinchDelta(xIn, amplitudeIn);
		return next(xOuts);
	}

	// "Pinch" at the system with delta peak.
	void pinchDelta(const Vector<T, d> x, T amount)
	{
		for (int i = 0; i < N; i++)
		{
			setAmplitude(i, amplitude(i) + eigenFunction(i, x) * amount);
		}
	}

	// "Pinch" at the system by adding to all amplitudes.
	void pinch(const array<complex<T>, N>& values)
	{
		for (int i = 0; i < N; i++)
		{
			setAmplitude(i, amplitude(i) + values[i]);
		}
	}

	// TODO: pinch with spatial function defined over interval -> needs to be decomposed 
	// TODO?: pinch with temporal function -> instead pass audio signal to next()

	// Set all amplitudes to zero
	void silence() {
		for (int i = 0; i < N; i++) {
			setAmplitude(i, T{ 0 });
		}
	}
	// Call silence() and set time to 0
	void reset() {
		silence();
		resetTime();
	}
	// Call silence() and set time to 0
	void resetTime() {
		time = 0;
	}
	// Get current time
	T getTime() const { return time; }
	// Set step time interval according to sampling rate
	void setTimeInterval(T deltaT) { this->deltaT = deltaT; }
	void setVelocity_sq(complex<T> v_sq) {
		velocity_sq = v_sq;
	}
	complex<T> getVelocity_sq() {
		return velocity_sq;
	}

protected:
	// Evolve time and amplitudes
	void evolve(T deltaTime)
	{
		time += deltaTime;
		for (int i = 0; i < N; i++)
		{
			setAmplitude(i, amplitude(i) * std::exp(complex<T>(0, 1) * /*ω=*/velocity_sq * eigenValue_sqrt(i) * deltaTime));
		}
	}

	T evaluate(T t, const Vector<T, d> x)
	{
		complex<T> result{ 0 };
		for (int i = 0; i < N; i++)
		{
			result += amplitude(i) * eigenFunction(i, x);
		}
		return result.real();
	}

	virtual T eigenFunction(int i, const Vector<T, d> x) const = 0;
	virtual T eigenValue_sqrt(int i) const = 0; // Using squares of eigenvalues for better performance
	virtual complex<T> amplitude(int i) const = 0;
	virtual void setAmplitude(int i, complex<T> value) = 0;

	T deltaT{ 0 };   // this needs to be set to 1/(sampling rate)

private:
	T time{ 0 };     // current Time

	/*T disp(T k_sq) const {
		return k_sq * velocity_sq;
	}*/

	complex<T> velocity_sq = 1;
};

/*
 * Optimized version for cases where the listening positions don't change every sample
 *
 * Eigenfunctions are evaluated int setListeningPositions() at every listening position and stored.
 * When asking for the next sample, the cached values are used to compute the current deflection.
 */

template <class T, int d, int N, int numChannels = 1>
class FixedListenerEigenvalueProblem : public EigenvalueProblem<T, d, N>
{
public:

	void setListeningPositions(const array<Vector<T, d>, numChannels>& listeningPositions) {
		for (int i = 0; i < numChannels; ++i) {
			for (int j = 0; j < N; ++j) {
				eigenFunctionEvaluations[i][j] = this->eigenFunction(j, listeningPositions[i]);
			}
		}
	}
	void setFirstListeningPosition(const Vector<T, d>& listeningPosition) {
		for (int j = 0; j < N; ++j) {
			eigenFunctionEvaluations[0][j] = eigenFunction(j, listeningPosition[0]);
		}

	}

	array<T, numChannels> next() {
		this->evolve(this->deltaT);
		return evaluate(this->getTime());
	}

	// Same but with external audio input
	array<T, numChannels> next(const Vector<T, d> strikePosition, T amplitudeIn)
	{
		this->pinchDelta(strikePosition, amplitudeIn);
		return next();
	}
	T nextFirstChannel() {
		evolve(this->deltaT);
		return evaluateFirstChannel(this->getTime());
	}

protected:

	array<T, numChannels> evaluate(T t) {
		array<T, numChannels> results{ 0 };
		for (int i = 0; i < numChannels; ++i) {
			for (int j = 0; j < N; ++j) {
				results[i] += (this->amplitude(j) * eigenFunctionEvaluations[i][j]).real();
			}
		}
		return results;
	}
	T evaluateFirstChannel(T t) {
		T result{ 0 };
		for (int j = 0; j < N; ++j) {
			result += (this->amplitude(j) * eigenFunctionEvaluations[0][j]).real();
		}
		return result;
	}

private:
	// Eigenfunctions evaluated at the listening positions last set through setListeningPositions()
	array<array<complex<T>, N>, numChannels> eigenFunctionEvaluations;

};

/*
 * As all implementation probably keep a list of complex amplitudes, this (abstract) class implements
 * this feature for actual implementations to derive from.
 */
template <class T, int d, int N, int numChannels>
class EigenvalueProblemAmplitudeBase : public FixedListenerEigenvalueProblem<T, d, N, numChannels> {
public:
	virtual complex<T> amplitude(int i) const override
	{
		return amplitudes[i];
	}
	virtual void setAmplitude(int i, complex<T> value) override
	{
		amplitudes[i] = value;
	};

private:
	array<complex<T>, N> amplitudes{}; // all default initialized with 0
};


/*
 * Implementation of the eigenvalue problem of a 1D string with fixed length. The eigenfunctions and
 * -values are similar and need not be declared separately. The weights are initialized with zero.
 */
template <class T, int N, int numChannels>
class StringEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, 1, N, numChannels>
{
public:
	StringEigenvalueProblem(T length=1) : length(length) {}

	T eigenFunction(int i, const Vector<T, 1> x) const override
	{
		return std::sin((i + 1) * pi<T>() * x[0] / length);
		// sin(2π·x/2L)
		// f = 1/2L
	}
	T eigenValue_sqrt(int i) const override
	{
		return (i + 1) * pi<T>() / length;
	}
	T getLength() const { return length; }

	void setLength(T length) {
		this->length = length;
	}

private:
	T length; // String length
};

/*
 * Implementation of the eigenvalue problem of a sphere. The eigenfunctions and
 * -values are similar and need not be declared separately. The weights are initialized with zero.
 */
template <class T, int N, int numChannels>
class SphereEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, 3, N, numChannels>
{
public:
	SphereEigenvalueProblem() {}

	Vector<T, 3> toCartesian(T theta, T phi) const {
		return { std::sin(theta) * std::cos(phi),std::sin(theta) * std::sin(phi),std::cos(theta) };
	}

	// Get m and l numbers from linear index i∈[0, n)
	std::pair<int, int> linearIndex(int i) const {
		// i+1 = l²+l+1+m and m from -l to l
		// solve for l: √(i+1) -1 <= l <= √i
		// Then, calc m from l and i
		int l = static_cast<int>(std::floor(std::sqrt(i)));
		int m = i - (l * l + l);
		return { l, m };
	}

	int factorial(int n) const {
		return n <= 1 ? 1 : factorial(n - 1) * n;
	}

	float normalizer(int l, int m) const {
		return(std::sqrt((2 * l + 1) / 2 * factorial(l - m) / factorial(l + m)));
	}

	T eigenFunction(int i, const Vector<T, 3> x) const override {
		// indices:
		auto lm = linearIndex(i);
		int l = lm.first;
		int m = lm.second;

		// spherical coordinates
		T r = x[0];
		T theta = x[1];
		T phi = x[2];

		T legend = std::assoc_legendre(l, m, std::cos(theta));

		// return only real part 
		return  std::pow(r, l) / rsrqt2pi * normalizer(l, m) * legend * std::cos(m * phi);
	}

	const T rsrqt2pi = std::sqrt(2 * pi<T>());

	// k = ω/c
	// k = 2π/λ    ω=2πf=2π/T

	T eigenValue_sqrt(int i) const override {
		int l = linearIndex(i).first;
		return l * (l + 1);
		//return std::sqrt(l * (l + 1));
	}

};



template <class T, int d, int N, int numChannels>
class CubeEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, d, N, numChannels> {
public:
	CubeEigenvalueProblem() {
		computeEigenvalues_and_ks();
	}
protected:
	/*

	 d
	___
	111 = λ₁ |
	211 = λ₂ |
	212 = λ₃ | N
			 |  ...

			 N Eigenwerte, aber N·d Quantenzahlen/Wellenzahlen

	*/
	void computeEigenvalues_and_ks() {
		int r = getRApprox(d, N);
		std::vector<Vector<T, d + 1>> kvecs;

		int maxNumEigenvalues = std::pow(r + 1, d);
		if (maxNumEigenvalues < N) throw std::exception("r<N");

		for (int i = 0; i < maxNumEigenvalues; ++i) {
			Vector<T, d + 1> kvec{}; // last entry sums up the squares of the other entries
			int kindex = i;
			for (int j = 0; j < d; j++) {
				kvec[j] = kindex % (r + 1) + 1;
				kindex /= (r + 1);
				kvec[d] += kvec[j] * kvec[j];
			}
			kvec[d] = static_cast<T>(std::sqrt(kvec[d]));

			kvecs.push_back(kvec);
		}

		std::sort(kvecs.begin(), kvecs.end(), [](Vector<T, d + 1>& a, Vector<T, d + 1>& b) {return a[d] < b[d]; });
		for (auto& a : kvecs) {
			//std::cout << a << "\n";
		}
		std::copy(kvecs.begin(), kvecs.begin() + N, ks_and_eigenvalues.begin());
	}
	/*
	0000..
	1000..
	0100..
	0010..
	 .
	 .
	1100
	0110
	 .
	 .
	1110
	 .
	 .
	1111

	*/
	int getRApprox(int d, int N) {
		// N =!  V_sphere / 2^d
		// with
		//       | π^(d/2) r^d /(d/2)!               even d
		// V =   |
		//       | 2·(.5(d-1))!·(4π)^.5(d-1)r^d/d!   odd d 
		T r = 0;
		if (!(d % 2)) { // even d
			r = 2 * std::pow<T>(N / std::pow(pi<T>(), d / 2) * factorial(d / 2), T{ 1 } / d);
		} // odd d
		else {
			r = 2 * std::pow<T>(N / std::pow(4 * pi<T>(), (d - 1) / 2) * factorial(d) / factorial((d - 1) / 2) / T{ 2 }, T{ 1 } / d);
		}
		// r = largest k we will get
		return static_cast<int>(std::ceil(r));
	}


	int factorial(int n) const {
		return n <= 1 ? 1 : factorial(n - 1) * n;
	}

protected:
	T eigenFunction(int i, const Vector<T, d> x) const override
	{
		T result{ 1 };
		for (int j = 0; j < d; ++j) {
			result *= std::sin(ks_and_eigenvalues[i][j] * pi<T>() * x[j]);
		}
		return result;
	}

	T eigenValue_sqrt(int i) const override
	{
		return ks_and_eigenvalues[i][d];
	}

private:
	array<Vector<T, d + 1>, N> ks_and_eigenvalues{};
};

/*
 * Implementation that allows to set specific eigenfunctions and values.
 */
template <class T, int d, int N, int numChannels, class F>
class IndividualFunctionEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, d, N, numChannels>
{
public:
	IndividualFunctionEigenvalueProblem() {}

	virtual T eigenFunction(int i, const Vector<T, 1> x) override
	{
		return eigenFunctions[i](x);
	}
	virtual T eigenValue_sqrt(int i) const override
	{
		return eigenValues_sq[i];
	}

	array<F, N> eigenFunctions{};
	array<T, N> eigenValues_sq{};
};

//for spherical eigenvalues need spherical coordinates and mapping function for indices

}
