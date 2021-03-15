//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/note_expression_synth_voice.h
// Created by  : Steinberg, 02/2010
// Description : 
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2020, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#pragma once
#include "public.sdk/samples/vst/common/voicebase.h"
#include "public.sdk/samples/vst/common/logscale.h"
#include "brownnoise.h"
#include "filter.h"
#include "note_expression_synth_controller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/base/futils.h"
#include <cmath>
#include <algorithm>

#include "eigen_evaluator.h"
#include "parameters.h"

#ifndef M_PI
#define M_PI			3.14159265358979323846
#endif
#ifndef M_PI_MUL_2
#define M_PI_MUL_2		6.28318530717958647692
#endif

namespace Steinberg {
namespace Vst {
namespace NoteExpressionSynth {

//-----------------------------------------------------------------------------

constexpr ParamValue defaultRadiusStrike = 1;
constexpr ParamValue defaultRadiusListening = 1;
constexpr ParamValue defaultThetaStrike = 0;
constexpr ParamValue defaultThetaListening = 0;
constexpr ParamValue defaultPhiStrike = 0;
constexpr ParamValue defaultPhiListening = 0;

//-----------------------------------------------------------------------------
enum VoiceParameters
{
	kVolumeMod,
	kTuningMod,
	kPanningLeft,
	kPanningRight,
	kFilterFrequencyMod,
	kFilterQMod,
	kRadiusStrike,
	kRadiusListening,
	kThetaStrike,
	kThetaListening,
	kPhiStrike,
	kPhiListening,
	kFilterType,
	kReleaseTimeMod,

	kNumParameters
};

//-----------------------------------------------------------------------------
class VoiceStatics
{
public:
	//------------------------------------------------------------------------
	static double normalizedLevel2Gain(float normalized)
	{
		double level;
		if (normalized >= 0.5)
			level = scaleHeadRoom * ::pow(10, (normalized - 0.5f) * 24 / 20.0f);
		else
			level = scaleNorm2GainC1 * ::pow(normalized, scaleNorm2GainC2);

		return level;
	}

	enum {
		kNumFrequencies = 128
	};

	static float freqTab[kNumFrequencies];
	static const float scaleHeadRoom;
	static const float scaleNorm2GainC1;
	static const float scaleNorm2GainC2;
	static LogScale<ParamValue> freqLogScale;
	static const double kNormTuningOneOctave;
	static const double kNormTuningOneTune;

};

//-----------------------------------------------------------------------------
/** Example Note Expression Synth Voice Class

\sa Steinberg::Vst::VoiceBase
*/
template<class SamplePrecision>
class Voice : public VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>
{
public:
	Voice();
	~Voice();

	void setSampleRate(ParamValue sampleRate) SMTG_OVERRIDE;
	void noteOn(int32 pitch, ParamValue velocity, float tuning, int32 sampleOffset, int32 nId) SMTG_OVERRIDE;
	void noteOff(ParamValue velocity, int32 sampleOffset) SMTG_OVERRIDE;
	bool process(SamplePrecision* outputBuffers[2], int32 numSamples);
	void reset() SMTG_OVERRIDE;

	void setNoteExpressionValue(int32 index, ParamValue value) SMTG_OVERRIDE;

protected:
	uint32 n;
	int32 noisePos;
	int32 noiseStep;

	Filter* filter;

	//SamplePrecision trianglePhase;
	//SamplePrecision sinusPhase;
	//ParamValue currentTriangleF;
	//ParamValue currentSinusF;
	ParamValue currentVolume;
	ParamValue currentPanningLeft;
	ParamValue currentPanningRight;

	ParamValue currentRadiusStrike;
	ParamValue currentRadiusListening;
	ParamValue currentThetaStrike;
	ParamValue currentThetaListening;
	ParamValue currentPhiStrike;
	ParamValue currentPhiListening;

	ParamValue currentLPFreq;
	ParamValue currentLPQ;

	ParamValue levelFromVel;
	ParamValue noteOffVolumeRamp;

	static constexpr int maxDimension = 3;
	using type = float;
	VSTMath::Vector<type, maxDimension> strikePosition{};
	VSTMath::Vector<type, maxDimension> listenerPosition{};

	//create string with length 0.01 m
	VSTMath::SphereEigenvalueProblem<type, 10, 1> system;

	type strikeAmount = 1.f;
	//VSTMath::CubeEigenvalueProblem<float, 4, 5, 1> system;

	bool noteoffFlag = false;
};

//-----------------------------------------------------------------------------
template<class SamplePrecision>
void Voice<SamplePrecision>::setNoteExpressionValue(int32 index, ParamValue value)
{
	if (this->globalParameters->bypassSNA)
		return;

	switch (index)
	{
		//------------------------------
	case Steinberg::Vst::kVolumeTypeID:
	{
		ParamValue vol = VoiceStatics::normalizedLevel2Gain((float)value);
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kVolumeMod, vol);
		break;
	}
	//------------------------------
	case Steinberg::Vst::kTuningTypeID:
	{
		if (value == 0.5)
		{
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kTuningMod, 0);
		}
		else if (value > 0.5)
		{
			if (this->globalParameters->tuningRange > 0.5)
				value = std::min<ParamValue>(0.5 + 2 * VoiceStatics::kNormTuningOneTune, value);
			else
				value = std::min<ParamValue>(0.5 + VoiceStatics::kNormTuningOneOctave, value);
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kTuningMod, (value - 0.5) * 2.);
		}
		else
		{
			if (this->globalParameters->tuningRange > 0.5)
				value = std::max<ParamValue>(0.5 - 3 * VoiceStatics::kNormTuningOneTune, value);
			else
				value = std::max<ParamValue>(0.5 - VoiceStatics::kNormTuningOneOctave, value);
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kTuningMod, (value - 0.5) * 2.);
		}
		break;
	}
	//------------------------------
	case Steinberg::Vst::kPanTypeID:
	{
		if (value == 0.5)
		{
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningLeft, 1.);
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningRight, 1.);
		}
		else if (value > 0.5)
		{
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningLeft, 1.);
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningRight, 1. + ((0.5 - value) * 2.));
		}
		else
		{
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningLeft, value * 2.);
			VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPanningRight, 1.);
		}
		break;
	}
	//------------------------------
	case Controller::kRadiusStrikeTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kRadiusStrike, value * 2.);
		break;
	}
	//------------------------------
	case Controller::kRadiusListeningTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kRadiusListening, value * 2.);
		break;
	}
	//------------------------------
	case Controller::kThetaStrikeTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kThetaStrike, value * 2.);
		break;
	}
	//------------------------------
	case Controller::kThetaListeningTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kThetaListening, value * 2.);
		break;
	}
	//------------------------------
	case Controller::kPhiStrikeTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPhiStrike, value * 2.);
		break;
	}
	//------------------------------
	case Controller::kPhiListeningTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kPhiListening, value * 2.);
		break;
	}

	//------------------------------
	case Controller::kFilterFreqModTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kFilterFrequencyMod, (value - 0.5) * 2.);
		break;
	}
	//------------------------------
	case Controller::kFilterQModTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kFilterQMod, (value - 0.5) * 2.);
		break;
	}
	//------------------------------
	case Controller::kFilterTypeTypeID:
	{
		filter->setType((Filter::Type)std::min<int32>((int32)(NUM_FILTER_TYPE * value), NUM_FILTER_TYPE - 1));
		break;
	}
	//------------------------------
	case Controller::kReleaseTimeModTypeID:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(kReleaseTimeMod, 2 * (value - 0.5));
		break;
	}
	//------------------------------
	default:
	{
		VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setNoteExpressionValue(index, value);
		break;
	}
	}
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
bool Voice<SamplePrecision>::process(SamplePrecision* outputBuffers[2], int32 numSamples)
{
	//---compute tuning-------------------------

	// main tuning
	ParamValue tuningInHz = 0.;
	if (this->values[kTuningMod] != 0. || this->globalParameters->masterTuning != 0 || this->tuning != 0)
	{
		tuningInHz = VoiceStatics::freqTab[this->pitch] * (::pow(2.0, (this->values[kTuningMod] * 10 + this->globalParameters->masterTuning * 2.0 / 12.0 + this->tuning)) - 1);
	}

	//ParamValue triangleFreq = (VoiceStatics::freqTab[this->pitch] + tuningInHz) * M_PI_MUL_2 / this->getSampleRate() / 2.;
	//if (currentTriangleF == -1)
	//	currentTriangleF = triangleFreq;
	//// check for frequency changes and update the phase so that it is crackle free
	//if (triangleFreq != currentTriangleF)
	//{
	//	// update phase
	//	trianglePhase = (SamplePrecision)((currentTriangleF - triangleFreq) * n + trianglePhase);
	//	currentTriangleF = triangleFreq;
	//}

	//// Sinus Detune
	//if (currentSinusDetune != this->values[kSinusDetune])
	//{
	//	currentSinusDetune = VoiceStatics::freqTab[this->pitch] * (::pow(2.0, this->values[kSinusDetune] * 2.0 / 12.0) - 1);
	//}
	//ParamValue sinusFreq = (VoiceStatics::freqTab[this->pitch] + tuningInHz + currentSinusDetune) * M_PI_MUL_2 / this->getSampleRate();
	//if (currentSinusF == -1)
	//	currentSinusF = sinusFreq;
	//if (sinusFreq != currentSinusF)
	//{
	//	// update phase
	//	sinusPhase = (SamplePrecision)((currentSinusF - sinusFreq) * n) + sinusPhase;
	//	currentSinusF = sinusFreq;
	//}

	//---calculate parameter ramps
	ParamValue volumeRamp = 0.;
	ParamValue panningLeftRamp = 0.;
	ParamValue panningRightRamp = 0.;

	// TODO: Johann: check if ramping is necessary
	ParamValue sinusVolumeRamp = 0.;

	ParamValue filterFreqRamp = 0.;
	ParamValue filterQRamp = 0.;
	ParamValue rampTime = std::max<ParamValue>((ParamValue)numSamples, (this->sampleRate * 0.005));

	ParamValue wantedVolume = VoiceStatics::normalizedLevel2Gain((float)Bound(0.0, 1.0, this->globalParameters->masterVolume * levelFromVel + this->values[kVolumeMod]));
	if (wantedVolume != currentVolume)
	{
		volumeRamp = (wantedVolume - currentVolume) / rampTime;
	}

	if (this->values[kPanningLeft] != currentPanningLeft)
	{
		panningLeftRamp = (this->values[kPanningLeft] - currentPanningLeft) / rampTime;
	}
	if (this->values[kPanningRight] != currentPanningRight)
	{
		panningRightRamp = (this->values[kPanningRight] - currentPanningRight) / rampTime;
	}
	if (this->values[kRadiusStrike] != currentRadiusStrike)
	{
		sinusVolumeRamp = (this->values[kRadiusStrike] - currentRadiusStrike) / rampTime;
	}


	ParamValue wantedLPFreq = Bound(0., 1., this->globalParameters->filterFreq + this->globalParameters->freqModDepth * this->values[kFilterFrequencyMod]);
	if (wantedLPFreq != currentLPFreq)
	{
		filterFreqRamp = (wantedLPFreq - currentLPFreq) / rampTime;
	}
	ParamValue wantedLPQ = Bound(0., 1., this->globalParameters->filterQ + this->values[kFilterQMod]);
	if (wantedLPQ != currentLPQ)
	{
		filterQRamp = (wantedLPQ - currentLPQ) / rampTime;
	}


	for (int32 i = 0; i < numSamples; i++)
	{
		this->noteOnSampleOffset--;
		this->noteOffSampleOffset--;

		if (this->noteOnSampleOffset <= 0)
		{
			// we are in Release
			if (this->noteOffSampleOffset == 0)
			{
				volumeRamp = 0;
				if (currentVolume > 0)
				{
					// ramp note off
					currentVolume -= noteOffVolumeRamp;
					if (currentVolume < 0.)
						currentVolume = 0.;
					this->noteOffSampleOffset++;
				}
				else
				{
					this->noteOffSampleOffset = this->noteOnSampleOffset = -1;
					return false;
				}
			}
			SamplePrecision sample;
			//auto osc = (SamplePrecision)sin(n * triangleFreq + trianglePhase);
			// square osc
			//sample = (SamplePrecision)((::floor(osc) + 0.5) * currentSquareVolume);
			// triangle osc
			//sample += (SamplePrecision)((osc - ::fabs(sin(n * triangleFreq + trianglePhase + 1 + currentTriangleSlope))) * currentTriangleVolume);
			// sinus osc
			//sample += (SamplePrecision)(sin(n * sinusFreq + sinusPhase) * currentSinusVolume);
			//sample = 0;

			//iterate the system and multiply with volume to make it attenuatable
			//the listening position is at 0.7 times the string length
			//sample = currentSquareVolume * system.next({ system.getLength() * 0.7f });
			//system.setFirstListeningPosition({ (float)currentRadiusListening, (float)(currentThetaListening * M_PI_MUL_2),  (float)(currentPhiListening * M_PI_MUL_2) ,.5 });

			const auto& pos = listenerPosition;
			constexpr type twopi = 2 * VSTMath::pi<type>();
			system.setFirstListeningPosition({ pos[0],twopi * pos[1],twopi * pos[2] });
			sample = 10 * system.nextFirstChannel();


			if (noteoffFlag) {
				// find first zero crossing
				if (std::abs(sample) < 0.0001) {
					system.silence();
				}
			}

			n++;

			// add noise
			//sample += (SamplePrecision)(this->globalParameters->noiseBuffer->at(noisePos) * currentNoiseVolume);

			// filter
			if (filterFreqRamp != 0. || filterQRamp != 0.)
			{
				filter->setFreqAndQ(VoiceStatics::freqLogScale.scale(currentLPFreq), 1. - currentLPQ);
				currentLPFreq += filterFreqRamp;
				currentLPQ += filterQRamp;
			}
			sample = (SamplePrecision)filter->process(sample);

			// store in output
			outputBuffers[0][i] += (SamplePrecision)(sample * currentPanningLeft * currentVolume);
			outputBuffers[1][i] += (SamplePrecision)(sample * currentPanningRight * currentVolume);

			//// advance noise
			//noisePos += noiseStep;
			//if (noisePos > this->globalParameters->noiseBuffer->getSize() - 2)
			//{
			//	noisePos = (int32)((float)::rand() / (float)RAND_MAX * (this->globalParameters->noiseBuffer->getSize() - 2) + 2);
			//	noiseStep = -1;
			//}
			//else if (noisePos < 2)
			//{
			//	noiseStep = 1;
			//}

			// ramp parameters
			currentVolume += volumeRamp;
			currentPanningLeft += panningLeftRamp;
			currentPanningRight += panningRightRamp;

			currentRadiusStrike += sinusVolumeRamp;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
void Voice<SamplePrecision>::noteOn(int32 _pitch, ParamValue velocity, float _tuning, int32 sampleOffset, int32 nId)
{
	currentVolume = 0;
	this->values[kVolumeMod] = 0;
	levelFromVel = 1.f + this->globalParameters->velToLevel * (velocity - 1.);

	currentRadiusStrike = this->values[kRadiusStrike] = this->globalParameters->radiusStrike;
	currentRadiusListening = this->values[kRadiusListening] = this->globalParameters->radiusListening;
	currentThetaStrike = this->values[kThetaStrike] = this->globalParameters->thetaStrike;
	currentThetaListening = this->values[kThetaListening] = this->globalParameters->thetaListening;
	currentPhiStrike = this->values[kPhiStrike] = this->globalParameters->phiStrike;
	currentPhiListening = this->values[kPhiListening] = this->globalParameters->phiListening;
	strikePosition = {
		(type)(this->globalParameters->radiusStrike) ,
		(type)(this->globalParameters->thetaStrike) ,
		(type)(this->globalParameters->phiStrike)
	};
	listenerPosition = {
		(type)(this->globalParameters->radiusListening) ,
		(type)(this->globalParameters->thetaListening) ,
		(type)(this->globalParameters->phiListening)
	};

	// filter setting
	currentLPFreq = this->globalParameters->filterFreq;
	this->values[kFilterFrequencyMod] = 0;
	currentLPQ = this->globalParameters->filterQ;
	this->values[kFilterQMod] = 0;

	filter->setType((Filter::Type)this->globalParameters->filterType);
	filter->setFreqAndQ(VoiceStatics::freqLogScale.scale(currentLPFreq), 1. - currentLPQ);

	//currentSinusDetune = 0.;
	//if (this->globalParameters->sinusDetune != 0.)
	//{
	//	currentSinusDetune = VoiceStatics::freqTab[this->pitch] * (::pow(2.0, this->globalParameters->sinusDetune * 2.0 / 12.0) - 1);
	//}
	//this->values[kSinusDetune] = currentSinusDetune;
	this->values[kTuningMod] = 0;

	VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::noteOn(_pitch, velocity, _tuning, sampleOffset, nId);
	this->noteOnSampleOffset++;


	noteoffFlag = false;
	system.resetTime(); // let's avoid a discontinuity at beginning
	system.setVelocity_sq({ VoiceStatics::freqTab[_pitch],std::max((float)this->globalParameters->decay * 5.f,0.f) });
	//system.pinchDelta({ (float)currentRadiusStrike, (float)(currentThetaStrike * M_PI_MUL_2),  (float)(currentPhiStrike * M_PI_MUL_2),.4 }, 1.f);

	const auto& pos = listenerPosition;
	constexpr type twopi = 2 * VSTMath::pi<type>();
	system.pinchDelta({ pos[0],twopi * pos[1],twopi * pos[2] }, strikeAmount);
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
void Voice<SamplePrecision>::noteOff(ParamValue velocity, int32 sampleOffset)
{
	VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::noteOff(velocity, sampleOffset);
	this->noteOffSampleOffset++;

	ParamValue timeFactor;
	if (this->values[kReleaseTimeMod] == 0)
		timeFactor = 1;
	else
		timeFactor = ::pow(100., this->values[kReleaseTimeMod]);

	noteOffVolumeRamp = 1.0 / (timeFactor * this->sampleRate * ((this->globalParameters->releaseTime * MAX_RELEASE_TIME_SEC) + 0.005));
	if (currentVolume)
		noteOffVolumeRamp *= currentVolume;

	//when note is off, set flag that system should be silenced at next zero crossing
	noteoffFlag = true;
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
void Voice<SamplePrecision>::reset()
{
	noiseStep = 1;
	noisePos = 0;
	n = 0;
	/*sinusPhase = trianglePhase = 0.;
	currentSinusF = currentTriangleF = -1.;*/
	this->values[kVolumeMod] = 0.;
	this->values[kTuningMod] = 0.;
	this->values[kFilterFrequencyMod] = 0.;
	this->values[kFilterQMod] = 0.;
	this->values[kReleaseTimeMod] = 0.;
	currentPanningLeft = this->values[kPanningLeft] = 1.;
	currentPanningRight = this->values[kPanningRight] = 1.;

	currentRadiusStrike = this->values[kRadiusStrike] = 0.5;
	currentRadiusListening = this->values[kRadiusListening] = 0.5;
	currentThetaStrike = this->values[kThetaStrike] = 0.5;
	currentThetaListening = this->values[kThetaListening] = 0.5;
	currentPhiStrike = this->values[kPhiStrike] = 0.5;
	currentPhiListening = this->values[kPhiListening] = 0.5;

	currentLPFreq = 1.;
	currentLPQ = 0.;
	filter->reset();
	noteOffVolumeRamp = 0.005;

	//when voice is reset, silence the string
	system.silence();
	noteoffFlag = false;

	VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::reset();
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
void Voice<SamplePrecision>::setSampleRate(ParamValue _sampleRate)
{
	filter->setSampleRate(_sampleRate);
	VoiceBase<kNumParameters, SamplePrecision, 2, GlobalParameterState>::setSampleRate(_sampleRate);

	//set sample rate of string
	system.setTimeInterval(1.0f / _sampleRate);
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
Voice<SamplePrecision>::Voice()
{
	filter = new Filter(Filter::kLowpass);
}

//-----------------------------------------------------------------------------
template<class SamplePrecision>
Voice<SamplePrecision>::~Voice()
{
	delete filter;
}

}
}
} // namespaces
