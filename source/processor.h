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
class GlobalPhysicalSystemWrapper {
public:
	VSTMath::SphereEigenvalueProblem<float, 10, 1> system;
	Filter filter{ Filter::kHighpass }; // we need a fucking filter to keep our speakers from exploding because of the ultra low mega-bass


	void init(float sampleRate) {
		system.setSampleRate(sampleRate);
		system.setFirstListeningPosition({ .2,.1,.34 });
		system.setStrikingPosition({ .8,.6,.09 });
		system.setVelocity_sq({ 1,1 });
		filter.setSampleRate(sampleRate);
		filter.setFreqAndQ(VoiceStatics::freqLogScale.scale(.2), .8);
		
	}

	inline void updateStrikingPosition(const std::array<ParamValue, maxDimension>& X) {
		system.setStrikingPosition({ (float)X[0], (float)X[1], (float)X[2] });
	}
	inline void updateListeningPosition(const std::array<ParamValue, maxDimension>& Y) {
		system.setFirstListeningPosition({ (float)Y[0],  (float)Y[1], (float)Y[2] });
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
	Processor ();
	
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

	tresult PLUGIN_API canProcessSampleSize (int32 symbolicSampleSize) SMTG_OVERRIDE;
	tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API process (ProcessData& data) SMTG_OVERRIDE;
	tresult PLUGIN_API processAudio(ProcessData& data);
	void listeningPositionChanged();
	void strikingPositionChanged();

	tresult PLUGIN_API notify (IMessage* message) SMTG_OVERRIDE;

	static FUnknown* createInstance (void*) { return (IAudioProcessor*)new Processor (); }

	static FUID cid;
protected:
	VoiceProcessor* voiceProcessor;
	GlobalParameterState paramState;
	OneReaderOneWriter::RingBuffer<Event> controllerEvents {16};

	//VSTMath::SphereEigenvalueProblem<float, maxDimension, 1> globalSystem;
	GlobalPhysicalSystemWrapper systemWrapper;
};


}}} // namespaces
