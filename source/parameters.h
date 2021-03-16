
#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "brownnoise.h"
#include <array>
#define MAX_VOICES				64
#define MAX_RELEASE_TIME_SEC	5.0
#define NUM_FILTER_TYPE			3
#define NUM_TUNING_RANGE		2 

namespace Steinberg {
class IBStream;

namespace Vst {
class NoteExpressionTypeContainer;
class IParamValueQueue;
}
}
namespace Steinberg::Vst::NoteExpressionSynth {

class Processor;

constexpr int maxDimension = 10;
//-----------------------------------------------------------------------------
// Global Parameters
//-----------------------------------------------------------------------------
enum Params : Steinberg::Vst::ParamID
{
	kBypass,
	kParamReleaseTime,
	kParamDecay,
	kParamBypassSNA,
	kParamFilterType,
	kParamFilterFreq,
	kParamFilterQ,
	kParamMasterVolume,
	kParamMasterTuning,
	kParamVelToLevel,
	kParamFilterFreqModDepth,
	kParamTuningRange,
	kParamActiveVoices,

	kParamX0,
	kParamX1,
	kParamX2,
	kParamX3,
	kParamX4,
	kParamX5,
	kParamX6,
	kParamX7,
	kParamX8,
	kParamX9,

	kParamY0,
	kParamY1,
	kParamY2,
	kParamY3,
	kParamY4,
	kParamY5,
	kParamY6,
	kParamY7,
	kParamY8,
	kParamY9,

	kParamAngle,
	kParamSize,
	kParamGlobalResonatorType,

	kNumGlobalParameters
};

// Global Parameters state as stored by Processor
struct GlobalParameterState
{
	bool bypass = false;
	BrownNoise<float>* noiseBuffer;

	ParamValue masterVolume;	// [0, +1]
	ParamValue masterTuning;	// [-1, +1]
	ParamValue velToLevel;		// [0, +1]

	ParamValue radiusStrike;	// [0, +1]
	ParamValue radiusListening;	// [0, +1]
	ParamValue thetaStrike;		// [0, +1]
	ParamValue thetaListening;	// [0, +1]
	ParamValue phiStrike;		// [0, +1]
	ParamValue phiListening;	// [0, +1]

	ParamValue releaseTime;		// [0, +1]
	ParamValue decay;			// [0, +1]
	ParamValue size;			// [0, +1]

	ParamValue filterFreq;		// [-1, +1]
	ParamValue filterQ;			// [-1, +1]
	ParamValue freqModDepth;	// [-1, +1]

	int8 filterType;			// [0, 1, 2]
	int8 tuningRange;			// [0, 1]

	int8 bypassSNA;				// [0, 1]


	// All from [0, 1]
	std::array<ParamValue, maxDimension> X; // input (striking) position in #N D
	std::array<ParamValue, maxDimension> Y; // output (listening) position in #N D

	tresult setState(IBStream* stream);
	tresult getState(IBStream* stream);

	// Set parameterState to default values;
	void defaultSettings();
};

// Add all necessary parameters to the ParameterContainer. Called from Controller
void initParameters(Steinberg::Vst::ParameterContainer& parameters);
// Add all necessary note expressions to the NoteExpressionTypeContainer. Called from Controller
//void initNoteExpressions(Steinberg::Vst::NoteExpressionTypeContainer& noteExpressionTypes);

// Read parameters from queue and write the values to a GlobalParameterState. Called from Processor
void processParameters(Steinberg::Vst::IParamValueQueue* queue, GlobalParameterState& paramState, Processor& p);


}