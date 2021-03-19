//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/note_expression_synth_processor.h
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

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/utility/ringbuffer.h"
#include "voice.h"

namespace Steinberg {
namespace Vst {
class VoiceProcessor;

namespace NoteExpressionSynth {


/*
 * Wrapper class for a global eigenvalue problem system.
 */
class GlobalResonatorWrapper {
public:
	GlobalResonatorWrapper() {
		setResonator(ResonatorType::Cube);
	}

	enum class ResonatorType {
		Sphere, Cube
	};

	// Set the object that the sound is fed into
	void setResonator(ResonatorType ot) {
		switch (ot) {
		case ResonatorType::Cube:
			resonator = &cube; break;
		case ResonatorType::Sphere:
			resonator = &sphere; break;
		}
	}

	using type = float;
	static constexpr int k = 7;
	static constexpr int numChannels = 2;
	static constexpr int dim = maxDimension;
	static constexpr int defaultStartDim = 5;
	VSTMath::CubeEigenvalueProblem<type, dim, k, numChannels> cube{ defaultStartDim };
	VSTMath::SphereEigenvalueProblem<type, dim, k, numChannels> sphere;
	Filter filter{ Filter::kHighpass }; // we need a fucking filter to keep our speakers from exploding because of the ultra low mega-bass
	Filter filterR{ Filter::kHighpass }; 

	VSTMath::FixedListenerEigenvalueProblem<type, dim, k, numChannels>* resonator;

	void setDimension(int dimension) {
		cube.setDimension(dimension);
	}

	void init(float sampleRate) {
		cube.setSampleRate(sampleRate);
		sphere.setSampleRate(sampleRate);
		cube.setVelocity_sq({ 100,1 });
		sphere.setVelocity_sq({ 100,1 });
		filter.setSampleRate(sampleRate);
		filterR.setSampleRate(sampleRate);
		filter.setFreqAndQ(VoiceStatics::freqLogScale.scale(.2), .8);
		filterR.setFreqAndQ(VoiceStatics::freqLogScale.scale(.2), .8);
	}

	inline void updateStrikingPosition(const std::array<ParamValue, maxDimension>& X) {
		cube.setStrikingPosition({ X });
		sphere.setStrikingPosition({ X });
		//cube.setStrikingPosition({ (float)X[0], (float)X[1], (float)X[2] , (float)X[3] });
		//sphere.setStrikingPosition({ (float)X[0], (float)X[1], (float)X[2] , (float)X[3] });
	}
	inline void updateListeningPosition(const std::array<ParamValue, maxDimension>& Y) {

		const VSTMath::Vector<type, maxDimension> y = Y;
		const VSTMath::Vector<type, maxDimension> center(.5);
		cube.setListeningPositions({ y, center * 2 - y });
		sphere.setListeningPositions({ y, y * -1 });
		//cube.setFirstListeningPosition({ Y });
		//sphere.setFirstListeningPosition({ Y });
		//cube.setFirstListeningPosition({ (float)Y[0],  (float)Y[1], (float)Y[2], (float)Y[3] });
		//sphere.setFirstListeningPosition({ (float)Y[0],  (float)Y[1], (float)Y[2], (float)Y[3] });
	}

	inline void setVelocity_sq(std::complex<float> vel) {
		cube.setVelocity_sq(vel);
		sphere.setVelocity_sq(vel);
	}
};


//-----------------------------------------------------------------------------
/** Example Note Expression Audio Processor

\sa Steinberg::Vst::VoiceProcessor
\sa Steinberg::Vst::VoiceBase
*/
class Processor : public AudioEffect
{
public:
	Processor();

	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

	tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

	tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) SMTG_OVERRIDE;
	tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE;
	tresult PLUGIN_API processAudio(ProcessData& data);
	void listeningPositionChanged();
	void strikingPositionChanged();
	void resonatorTypeChanged();
	void dimensionChanged();

	tresult PLUGIN_API notify(IMessage* message) SMTG_OVERRIDE;

	static FUnknown* createInstance(void*) { return (IAudioProcessor*)new Processor(); }

	static FUID cid;
protected:
	VoiceProcessor* voiceProcessor;
	GlobalParameterState paramState;
	OneReaderOneWriter::RingBuffer<Event> controllerEvents{ 16 };

	GlobalResonatorWrapper systemWrapper;

	double vuPPM = 0;
	double vuPPMOld = 0;
	int currDim = 10;
};


}
}
} // namespaces
