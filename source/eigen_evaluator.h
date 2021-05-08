﻿#pragma once 

#include <algorithm>
#include <array>
#include <iostream>
#include <complex>
#include <vector>
#include <numeric>
#include <functional>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>
#include "legendre.h"

namespace VSTMath {

/*
 * Preparation of some definitions
 */
template <class T>
using complex = std::complex<T>;
template <class T, int N>
using array = std::array<T, N>;

template <typename T> constexpr T pi() { return static_cast<T>(3.1415926535897932384626); }

/*
 * Basic class for representing a fixed length vector (of dimension d) of type T.
 * T will probably be float or double
 */
template <class T, int d>
class Vector
{
    array<T, d> values;

public:
    Vector(const T(&elems)[d]) {
	   std::copy(std::begin(elems), std::end(elems), values.begin());
    }
    Vector(const std::initializer_list<T> elems) {
	   size_t h = elems.size() < d ? elems.size() : d;
	   std::copy(std::begin(elems), std::begin(elems) + h, values.begin());
	   std::fill(values.begin() + h, values.end(), T{});
    }
    Vector(const array<T, d>& elems) {
	   std::copy(std::begin(elems), std::end(elems), values.begin());
    }

    template<class U>
    Vector(const array<U, d>& elems) {
	   for (int i = 0; i < d; i++) {
		  values[i] = static_cast<T>(elems[i]);
	   }
    }

    Vector(const T& value = 0) {
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

    Vector operator+(const T& c) const { return Vector(*this) += c; }
    Vector operator-(const T& c) const { return Vector(*this) -= c; }
    Vector operator*(const T& c) const { return Vector(*this) *= c; }
    Vector operator/(const T& c) const { return Vector(*this) /= c; }
    Vector operator+(const Vector& v) const { return Vector(*this) += v; }
    Vector operator-(const Vector& v) const { return Vector(*this) -= v; }

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

    friend std::ostream& operator<<(std::ostream& os, const Vector<T, d>& v) {
	   os << "(";
	   for (int i = 0; i < d - 1; i++)	os << v.values[i] << ",";
	   return os << v.values[d - 1] << ")";
    }

    size_t size() const { return values.size(); }
};

template <class T, int d>
bool operator==(const Vector<T, d>& a, const Vector<T, d>& b) {
    for (int i = 0; i < d; i++) if (a[i] != b[i]) return false;
    return true;
}
template <class T, int d> bool operator!=(const Vector<T, d>& a, const Vector<T, d>& b) { return !(a == b); }

template <class T, int d> Vector<T, d> operator-(const Vector<T, d>& v) { return v * -1; }
template <class T, int d> Vector<T, d> operator+(T c, const Vector<T, d>& v) { return v + c; }
template <class T, int d> Vector<T, d> operator-(T c, const Vector<T, d>& v) { return v - c; }
template <class T, int d> Vector<T, d> operator*(T c, const Vector<T, d>& v) { return v * c; }







/*
 * (Abstract) eigenvalue problem base class that implements the main procedure with eigenfunctions and -values
 * and declares methods to evaluate the eigenfunctions, -values and weights for the i-th eigenvalue.
 */
template <class T, int d, int N>
class EigenvalueProblem
{
public:
    EigenvalueProblem() {
	   init_QM();
    }
    // Evaluate for next time step at spatial position xOut.
    T next(const Vector<T, d> xOut) {
	   evolve(deltaT);
	   return evaluate(time, xOut);
    }

    // Evaluate for next time frame and give audio input at position xIn
    T next(const Vector<T, d> xOut, const Vector<T, d> xIn, T amplitudeIn) {
	   pinchDelta(xIn, amplitudeIn);
	   return next(xOut);
    }

    // Evaluate for next time step at [channels] spatial positions for Stereo or multichannel processing.
    template <int channels>
    array<T, channels> next(const array<Vector<T, d>, channels>& xOuts) {
	   evolve(deltaT);
	   array<T, channels> out;
	   for (int i = 0; i < channels; i++)
		  out[i] = evaluate(time, xOuts[i]);
	   return out;
    }

    // Same but with external audio input
    template <int channels>
    array<T, channels> next(const array<Vector<T, d>, channels>& xOuts, const Vector<T, d> xIn, T amplitudeIn) {
	   pinchDelta(xIn, amplitudeIn);
	   return next(xOuts);
    }

    // "Pinch" at the system with delta peak.
    void pinchDelta(const Vector<T, d> x, T amount) {
	   for (int i = 0; i < N; i++) {
		  setAmplitude(i, amplitude(i) + eigenFunction(i, x) * amount);
	   }
    }

    // "Pinch" at the system by adding to all amplitudes.
    void pinch(const array<complex<T>, N>& values) {
	   for (int i = 0; i < N; i++) {
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
    void setSampleRate(T sampleRate) { this->deltaT = T{ 1. } / sampleRate; }


    void setVelocity_sq(complex<T> v_sq) { velocity_sq = v_sq; }
    complex<T> getVelocity_sq() { return velocity_sq; }

protected:
    // Evolve time and amplitudes
    //void evolve(T deltaTime) {
    //	time += deltaTime;
    //	for (int i = 0; i < N; i++) {
    //		setAmplitude(i, amplitude(i) * std::exp(complex<T>(0, 1) * /*ω=*/velocity_sq * eigenValue_sqrt(i) * deltaTime));
    //	}
    //}

    // Evolve time and amplitudes
    const bool QM_mode = false;
    const double QM_x_min = -1;
    const double QM_x_max = 1;
    static constexpr int QM_number_bins = 100;
    //double QM_h_bar = 1;
    array<double, QM_number_bins> QM_distribution{ 0 };
    std::default_random_engine generator;

    array<complex<T>, N> QM_old_amplitude;

    void init_QM() {
	   unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	   generator = std::default_random_engine(seed);
    }
    void evolve(T deltaTime) {
	   time += deltaTime;
	   if (QM_mode == false) {
		  for (int i = 0; i < N; i++) {
			 setAmplitude(i, amplitude(i) * std::exp(complex<T>(0, 1) * /*ω=*/velocity_sq * eigenValue_sqrt(i) * deltaTime));
		  }
	   }
	   else {
		  for (int i = 0; i < N; i++) {
			 // if QM mode on
			 complex<T> omega = velocity_sq * eigenValue_sqrt(i);
			 // loop over all x
			 for (int j = 0; j < QM_distribution.size(); ++j) {
				T x_ = static_cast<T>(QM_x_min + j * (QM_x_max - QM_x_min) / QM_number_bins);
				complex<T> derivative_sq = std::pow((x_ - QM_old_amplitude[i]) / deltaTime, T{ 2 });

				// with hbar = 1:
				auto a = omega * omega * amplitude(i) * amplitude(i);
				QM_distribution[j] = std::abs(std::exp(-complex<T>(0, 1) * (derivative_sq - a)).real());
			 }
			 std::discrete_distribution<int> value(QM_distribution.begin(), QM_distribution.end());
			 int number = value(generator);
			 double x = QM_x_min + number * (QM_x_max - QM_x_min) / QM_number_bins;
			 complex<T> new_amplitude = amplitude(i) * std::exp(complex<T>(0, 1) * omega * deltaTime) * (static_cast<T>(x) + QM_old_amplitude[i]);
			 QM_old_amplitude[i] = amplitude(i);
			 setAmplitude(i, new_amplitude);
		  }
	   }
    }



    T evaluate(T t, const Vector<T, d> x) {
	   complex<T> result{ 0 };
	   for (int i = 0; i < N; i++) {
		  result += amplitude(i) * eigenFunction(i, x);
	   }
	   return result.real();
    }

    virtual T eigenFunction(int i, const Vector<T, d> x) const = 0;
    virtual T eigenValue_sqrt(int i) const = 0; // Using squareroots of eigenvalues for better performance
    virtual complex<T> amplitude(int i) const = 0;
    virtual void setAmplitude(int i, complex<T> value) = 0;

    T deltaT{ 0 };   // this needs to be set to 1/(sampling rate)

private:
    T time{ 0 };     // current Time

    complex<T> velocity_sq = 1;
};

/*
 * Optimized version for cases where the listening positions don't change every step
 *
 * Eigenfunctions are evaluated in setListeningPositions() at every listening position and stored.
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
		  eigenFunctionEvaluations[0][j] = this->eigenFunction(j, listeningPosition[0]);
	   }
    }

    void setStrikingPosition(const Vector<T, d> strikingPosition) {
	   for (int j = 0; j < N; ++j) {
		  eigenFunctionEvaluation_strike[j] = this->eigenFunction(j, strikingPosition);
	   }
    }

    // "Pinch" at the system with delta peak.
    void pinchDelta(T amount) {
	   for (int i = 0; i < N; i++) {
		  this->setAmplitude(i, this->amplitude(i) + eigenFunctionEvaluation_strike[i] * amount);
	   }
    }

    array<T, numChannels> next() {
	   this->evolve(this->deltaT);
	   return evaluate(this->getTime());
    }

    // Same but with external audio input
    array<T, numChannels> next(T amplitudeIn) {
	   this->pinchDelta(amplitudeIn);
	   return next();
    }

    T nextFirstChannel() {
	   this->evolve(this->deltaT);
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
    array<complex<T>, N> eigenFunctionEvaluation_strike;

};


/*
 * As all implementation probably keep a list of complex amplitudes, this (abstract) class implements
 * this feature for actual implementations to derive from.
 */
template <class T, int d, int N, int numChannels>
class EigenvalueProblemAmplitudeBase : public FixedListenerEigenvalueProblem<T, d, N, numChannels> {
public:
    virtual complex<T> amplitude(int i) const override {
	   return amplitudes[i];
    }
    virtual void setAmplitude(int i, complex<T> value) override {
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
    StringEigenvalueProblem(T length = 1) : length(length) {}

    T eigenFunction(int i, const Vector<T, 1> x) const override {
	   return std::sin((i + 1) * pi<T>() * x[0] / length); // sin(2π·x/2L)
    }
    T eigenValue_sqrt(int i) const override {
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
 *
 * Warning: This is currently a 3D sphere. The Parameter d is just a dummy to make it compatible with
 * an n-dimensional cube etc.
 */
template <class T, int d, int N, int numChannels>
class SphereEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, d, N, numChannels>
{
public:
    SphereEigenvalueProblem() {}

    static Vector<T, 3> toCartesian(T theta, T phi) {
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
	   return(std::sqrt((2 * l + 1) / 2.f * factorial(l - m) / factorial(l + m)));
    }

    T eigenFunction(int i, const Vector<T, d> x) const override {
	   // indices:
	   auto lm = linearIndex(i);
	   int l = lm.first;
	   int m = lm.second;

	   // spherical coordinates
	   T r = x[0];
	   T theta = x[1];
	   T phi = x[2];

	   T legend = static_cast<T>(VSTMath::assoc_legendre(l, m, std::cos(theta)));

	   // return only real part 
	   return std::pow(r, l) / rsrqt2pi * normalizer(l, m) * legend * std::cos(m * phi);
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
    CubeEigenvalueProblem(int defaultActualDims = d) {
	   actualDim = defaultActualDims;
	   computeEigenvalues_and_ks();
    }

    void setDimension(int dimension) {
	   if (dimension > 10 || dimension < 1) return;
	   if (actualDim != dimension) {
		  actualDim = dimension;
		  computeEigenvalues_and_ks();
	   }
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
	   int r = getRApprox();

	   int maxNumEigenvalues = std::pow(r + 1, actualDim);
	   //if (maxNumEigenvalues < N) throw std::exception("r<N");

	   std::vector<Vector<T, d + 1>> kvecs(maxNumEigenvalues);

	   for (int i = 0; i < maxNumEigenvalues; ++i) {
		  Vector<T, d + 1> kvec{}; // last entry sums up the squares of the other entries
		  int kindex = i;
		  for (int j = 0; j < actualDim; j++) {
			 kvec[j] = kindex % (r + 1) + 1;
			 kindex /= (r + 1);
			 kvec[d] += kvec[j] * kvec[j];
		  }
		  kvec[d] = static_cast<T>(std::sqrt(kvec[d]));

		  kvecs[i] = kvec;
	   }

	   std::sort(kvecs.begin(), kvecs.end(), [](Vector<T, d + 1>& a, Vector<T, d + 1>& b) {return a[d] < b[d]; });
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
    int getRApprox() {
	   // N =!  V_sphere / 2^d
	   // with
	   //       | π^(d/2) r^d /(d/2)!               even d
	   // V =   |
	   //       | 2·(.5(d-1))!·(4π)^.5(d-1)r^d/d!   odd d 
	   T r = 0;
	   if (!(actualDim % 2)) { // even d
		  r = 2 * std::pow(N / std::pow(pi<T>(), actualDim / 2) * factorial(actualDim / 2), T{ 1 } / actualDim);
	   } // odd d
	   else {
		  r = 2 * std::pow(N / std::pow(4 * pi<T>(), (actualDim - 1) / 2) * factorial(actualDim) / factorial((actualDim - 1) / 2) / T{ 2 }, T{ 1 } / actualDim);
	   }
	   // r = largest k we will get
	   return static_cast<int>(std::ceil(r));
    }


    int factorial(int n) const {
	   return n <= 1 ? 1 : factorial(n - 1) * n;
    }

protected:
    T eigenFunction(int i, const Vector<T, d> x) const override {
	   T result{ 1 };
	   for (int j = 0; j < actualDim; ++j) {
		  result *= std::sin(ks_and_eigenvalues[i][j] * pi<T>() * x[j]);
	   }
	   return result;
    }

    T eigenValue_sqrt(int i) const override {
	   return ks_and_eigenvalues[i][d];
    }

//private:
    public:
    array<Vector<T, d + 1>, N> ks_and_eigenvalues{};
    int actualDim = d;
};

/*
 * Implementation that allows to set specific eigenfunctions and values.
 */
template <class T, int d, int N, int numChannels, class F>
class IndividualFunctionEigenvalueProblem : public EigenvalueProblemAmplitudeBase<T, d, N, numChannels>
{
public:
    IndividualFunctionEigenvalueProblem() {}

    virtual T eigenFunction(int i, const Vector<T, 1> x) override {
	   return eigenFunctions[i](x);
    }
    virtual T eigenValue_sqrt(int i) const override {
	   return eigenValues_sq[i];
    }

    array<F, N> eigenFunctions{};
    array<T, N> eigenValues_sq{};
};

//for spherical eigenvalues need spherical coordinates and mapping function for indices

}
