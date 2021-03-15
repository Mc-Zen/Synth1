
#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "brownnoise.h"
#define MAX_VOICES				64
#define MAX_RELEASE_TIME_SEC	5.0
#define NUM_FILTER_TYPE			3
#define NUM_TUNING_RANGE		2 

namespace Steinberg {

class IBStream;

namespace Vst {
//class ParameterContainer;
class NoteExpressionTypeContainer;
}
}
namespace Steinberg::Vst::NoteExpressionSynth {

//-----------------------------------------------------------------------------
// Global Parameters
//-----------------------------------------------------------------------------
enum
{
	kParamReleaseTime,
	kParamRadiusStrike,
	kParamRadiusListening,
	kParamThetaStrike,
	kParamThetaListening,
	kParamPhiStrike,
	kParamPhiListening,
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

	kNumGlobalParameters
};


void initParameters(Steinberg::Vst::ParameterContainer& parameters);
//void initNoteExpressions(Steinberg::Vst::NoteExpressionTypeContainer& noteExpressionTypes);

struct GlobalParameterState
{
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

	ParamValue filterFreq;		// [-1, +1]
	ParamValue filterQ;			// [-1, +1]
	ParamValue freqModDepth;	// [-1, +1]

	int8 filterType;			// [0, 1, 2]
	int8 tuningRange;			// [0, 1]

	int8 bypassSNA;				// [0, 1]

	tresult setState(IBStream* stream);
	tresult getState(IBStream* stream);


	void default();
};

}