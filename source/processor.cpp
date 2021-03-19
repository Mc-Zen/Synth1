//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/note_expression_synth_processor.cpp
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

#include "processor.h"
#include "public.sdk/samples/vst/common/voiceprocessor.h"
#include "controller.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <algorithm>
#include "parameters.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h" // getChannelBuffersPointer()


namespace Steinberg {
namespace Vst {
namespace NoteExpressionSynth {

//-----------------------------------------------------------------------------
FUID Processor::cid(0xc7ec93ee, 0xde6644fe, 0x9337b908, 0x45b58473);

//-----------------------------------------------------------------------------
Processor::Processor() : voiceProcessor(nullptr)
{
	setControllerClass(Controller::cid);

	memset(&paramState, 0, sizeof(paramState));

	paramState.defaultSettings();
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
	tresult result = AudioEffect::initialize(context);
	if (result == kResultTrue)
	{
		addAudioInput(STR16("AudioInput"), Vst::SpeakerArr::kStereo);
		addAudioOutput(STR16("Audio Output"), SpeakerArr::kStereo);
		addEventInput(STR16("Event Input"), 1);
	}
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::notify(IMessage* message)
{
	auto msgID = message->getMessageID();
	if (strcmp(msgID, MsgIDEvent) != 0)
		return kResultFalse;
	if (auto attr = message->getAttributes())
	{
		const void* msgData;
		uint32 msgSize;
		if (attr->getBinary(MsgIDEvent, msgData, msgSize) == kResultTrue && msgSize == sizeof(Event))
		{
			auto evt = reinterpret_cast<const Event*>(msgData);
			controllerEvents.push(*evt);
		}
	}
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::setState(IBStream* state)
{
	return paramState.setState(state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::getState(IBStream* state)
{
	return paramState.getState(state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns,
	SpeakerArrangement* outputs, int32 numOuts)
{
	// we only support one stereo output bus
	if (numIns == 0 && numOuts == 1 && outputs[0] == SpeakerArr::kStereo)
	{
		return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize)
{
	if (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)
	{
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::setActive(TBool state)
{
	if (state)
	{
		if (paramState.noiseBuffer == nullptr)
			paramState.noiseBuffer = new BrownNoise<float>((int32)processSetup.sampleRate, (float)processSetup.sampleRate);
		if (voiceProcessor == nullptr)
		{
			if (processSetup.symbolicSampleSize == kSample32)
			{
				voiceProcessor =
					new VoiceProcessorImplementation<float, Voice<float>, 2, MAX_VOICES,
					GlobalParameterState>((float)processSetup.sampleRate, &paramState);
			}
			else if (processSetup.symbolicSampleSize == kSample64)
			{
				voiceProcessor =
					new VoiceProcessorImplementation<double, Voice<double>, 2, MAX_VOICES,
					GlobalParameterState>((float)processSetup.sampleRate, &paramState);
			}
			else
			{
				return kInvalidArgument;
			}
		}

		voiceProcessor->clearOutputNeeded(false);
		systemWrapper.init((float)processSetup.sampleRate);
		systemWrapper.updateStrikingPosition(paramState.X);
		systemWrapper.updateListeningPosition(paramState.Y);
	}
	else
	{
		if (voiceProcessor)
		{
			delete voiceProcessor;
		}
		voiceProcessor = nullptr;
		if (paramState.noiseBuffer)
		{
			delete paramState.noiseBuffer;
		}
		paramState.noiseBuffer = nullptr;
	}
	return AudioEffect::setActive(state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Processor::process(ProcessData& data)
{
	// TODO: maybe try to make this nearly sample accurate
	if (data.inputParameterChanges)
	{
		int32 count = data.inputParameterChanges->getParameterCount();
		for (int32 i = 0; i < count; i++)
		{
			IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);
			if (queue)
			{
				processParameters(queue, paramState, *this);
			}
		}
	}
	tresult result = kResultTrue;
	Event evt;
	while (controllerEvents.pop(evt))
	{
		voiceProcessor->processEvent(evt);
	}

	tresult resultAudio = processAudio(data);
	// initialize audio output buffers
	if (data.numOutputs < 1)
		result = kResultTrue;

	// flush mode
	if (data.numOutputs < 1)
		result = kResultTrue;
	else
		result = voiceProcessor->process(data);
	if (result == kResultTrue)
	{
		if (data.outputParameterChanges)
		{
			int32 index;
			IParamValueQueue* queue = data.outputParameterChanges->addParameterData(kParamActiveVoices, index);
			if (queue)
			{
				queue->addPoint(0, (ParamValue)voiceProcessor->getActiveVoices() / (ParamValue)MAX_VOICES, index);
			}
			if (vuPPM != vuPPMOld) {
				queue = data.outputParameterChanges->addParameterData(kParamOutputVolume, index);
				if (queue) {
					queue->addPoint(0, vuPPM, index);
				}
			}
			/*if (paramState.dimension != currDim) {
				queue = data.outputParameterChanges->addParameterData(kParamDim, index);
				if (queue) {
					float a = (paramState.dimension - 1) / 9.0;
					queue->addPoint(0, a, index);
					//FDebugPrint("asd", )
				}
				paramState.dimension = currDim;
			}*/
		}
		if (voiceProcessor->getActiveVoices() == 0 && data.numOutputs > 0)
		{
			data.outputs[0].silenceFlags = 0x11; // left and right channel are silent
		}
	}
	return result && resultAudio;
}

tresult PLUGIN_API Processor::processAudio(ProcessData& data)
{

	if (data.numInputs == 0 || data.numOutputs == 0)
	{
		return kResultOk; // nothing to do
	}



	// Wie viele Kanäle? 2 für Stereo, 1 für Mono. 
	int32 numChannels = data.inputs[0].numChannels;

	// Wie viele Bytes an Daten enthält der Buffer?
	uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
	void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
	void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);

	// Implement true bypass
	if (paramState.bypass) {
		for (int32 i = 0; i < numChannels; ++i) {
			if (in[i] != out[i]) {
				memcpy(out[i], in[i], sampleFramesSize);
			}
		}
		return kResultOk;
	}

	// Die Silence-Flags dienen nur der Optimierung. Man kann CPU sparen, wenn kein Signal anliegt
	// Achtung dieses Plugin kann Audio produzieren (nachklingen), auch wenn kein Signal anliegt. 
	{
		if (data.inputs[0].silenceFlags != 0 && vuPPM < 0.0001) {
			data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

			for (int32 i = 0; i < numChannels; ++i) {
				if (in[i] != out[i]) {
					memset(out[i], 0, sampleFramesSize);
				}
			}
			return kResultOk;
		}
		data.outputs[0].silenceFlags = 0;
	}

	systemWrapper.setVelocity_sq({ (float)paramState.size * 2000,(float)paramState.decay * 5 });

	vuPPMOld = vuPPM;
	int32 numSamples = data.numSamples;	 // Wie viele Samples hat der Buffer?
	Sample32* sInL;
	Sample32* sInR;
	VSTMath::Vector <GlobalResonatorWrapper::type, GlobalResonatorWrapper::numChannels> tmp;
	Sample32 pL, pR, avg;

	float wet = paramState.mix;
	float dry = 1.0f - wet;

	for (int32 i = 0; i < numSamples; i++) {

		// First channel is send into system
		sInL = (Sample32*)in[0] + i;
		sInR = (Sample32*)in[1] + i;

		tmp = systemWrapper.resonator->next((*sInL + *sInR) * .5f);

		pL = (Sample32)systemWrapper.filter.process(tmp[0]) * paramState.masterVolume;
		pR = (Sample32)systemWrapper.filterR.process(tmp[1]) * paramState.masterVolume;

		*((Sample32*)out[0] + i) = pL * wet + dry * (*sInL);
		*((Sample32*)out[1] + i) = pR * wet + dry * (*sInR);

		//avg = (std::abs(pL) + std::abs(pR)) * .5f;
		vuPPM += std::abs(pL);
		// response of system is sent to all output channels. 
		//for (int32 j = 0; j < numChannels; j++) {

		//	sOut = (Sample32*)out[j] + i;
		//	*sOut = tmp;

		//	if (tmp > 0) {
		//		vuPPM += tmp;
		//	}
		//	else {
		//		vuPPM -= tmp;

		//	}
		//}
	}
	vuPPM /= numSamples;
	return kResultOk;
}
void Processor::strikingPositionChanged()
{
	systemWrapper.updateStrikingPosition(paramState.X);
}
void Processor::resonatorTypeChanged()
{
	systemWrapper.setResonator(static_cast<GlobalResonatorWrapper::ResonatorType>(paramState.resonatorType));
}
void Processor::listeningPositionChanged()
{
	systemWrapper.updateListeningPosition(paramState.Y);
}
void Processor::dimensionChanged()
{
	systemWrapper.setDimension(paramState.dimension);
}
} // NoteExpressionSynth
} // Vst
} // Steinberg
