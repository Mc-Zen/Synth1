#include "parameters.h"
#include "base/source/fstreamer.h"
#include "public.sdk/samples/vst/common/logscale.h"
#include "note_expression_synth_voice.h" // For VoiceStatics
#include "note_expression_synth_controller.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <array>
#include <limits>


namespace Steinberg::Vst::NoteExpressionSynth {


void GlobalParameterState::default() {
	noiseBuffer = nullptr;
	masterVolume = .8;
	masterTuning = 0;
	velToLevel = 1.;

	radiusStrike = 1;
	radiusListening = 1;
	thetaStrike = 1;
	thetaListening = 1;
	phiStrike = 1;
	phiListening = 1;

	releaseTime = 0;
	decay = 0;

	filterFreq = 1;
	filterQ = 0;
	freqModDepth = 1;

	filterType = 0;
	tuningRange = 1;

	bypassSNA = 0;
}


void processParameters(Steinberg::Vst::IParamValueQueue* queue, GlobalParameterState& paramState) {
	int32 sampleOffset;
	ParamValue value;
	ParamID pid = queue->getParameterId();

	if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) == kResultTrue) {
		switch (pid) {
		case kParamMasterVolume:
			paramState.masterVolume = value;
			break;
		case kParamMasterTuning:
			paramState.masterTuning = 2 * (value - 0.5);
			break;
		case kParamVelToLevel:
			paramState.velToLevel = value;
			break;
		case kParamFilterFreqModDepth:
			paramState.freqModDepth = 2 * (value - 0.5);
			break;

		case kParamRadiusStrike:
			paramState.radiusStrike = value;
			break;
		case kParamRadiusListening:
			paramState.radiusListening = value;
			break;
		case kParamThetaStrike:
			paramState.thetaStrike = value;
			break;
		case kParamThetaListening:
			paramState.thetaListening = value;
			break;
		case kParamPhiStrike:
			paramState.phiStrike = value;
			break;
		case kParamPhiListening:
			paramState.phiListening = value;
			break;

		case kParamReleaseTime:
			paramState.releaseTime = value;
			break;
		case kParamDecay:
			paramState.decay = value;
			break;
		case kParamFilterType:
			paramState.filterType = std::min<int8>((int8)(NUM_FILTER_TYPE * value), NUM_FILTER_TYPE - 1);
			break;

		case kParamFilterFreq:
			paramState.filterFreq = value;
			break;
		case kParamFilterQ:
			paramState.filterQ = value;
			break;
		case kParamBypassSNA:
			paramState.bypassSNA = (value >= 0.5) ? 1 : 0;
			break;
		case kParamTuningRange:
			paramState.tuningRange = std::min<int8>((int8)(NUM_TUNING_RANGE * value), NUM_TUNING_RANGE - 1);
			break;
		}
	}
}

static uint64 currentParamStateVersion = 4;

tresult GlobalParameterState::setState(IBStream* stream)
{
	IBStreamer s(stream, kLittleEndian);
	uint64 version = 0;

	// version 0
	if (!s.readInt64u(version))
		return kResultFalse;

	if (!s.readDouble(radiusStrike))
		return kResultFalse;
	if (!s.readDouble(radiusListening))
		return kResultFalse;
	if (!s.readDouble(thetaStrike))
		return kResultFalse;
	if (!s.readDouble(thetaListening))
		return kResultFalse;
	if (!s.readDouble(phiStrike))
		return kResultFalse;
	if (!s.readDouble(phiListening))
		return kResultFalse;

	if (!s.readDouble(releaseTime))
		return kResultFalse;
	if (!s.readInt8(bypassSNA))
		return kResultFalse;

	if (version >= 1)
	{
		if (!s.readInt8(filterType))
			return kResultFalse;
		if (!s.readDouble(filterFreq))
			return kResultFalse;
		if (!s.readDouble(filterQ))
			return kResultFalse;
	}
	if (version >= 2)
	{
		if (!s.readDouble(masterVolume))
			return kResultFalse;
		if (!s.readDouble(masterTuning))
			return kResultFalse;
		if (!s.readDouble(velToLevel))
			return kResultFalse;
		if (!s.readDouble(freqModDepth))
			return kResultFalse;
		if (!s.readInt8(tuningRange))
			return kResultFalse;
	}
	if (version >= 4)
	{
		if (!s.readDouble (decay))
			return kResultFalse;
	}
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult GlobalParameterState::getState(IBStream* stream)
{
	IBStreamer s(stream, kLittleEndian);

	// version 0
	if (!s.writeInt64u(currentParamStateVersion))
		return kResultFalse;

	if (!s.writeDouble(radiusStrike))
		return kResultFalse;
	if (!s.writeDouble(radiusListening))
		return kResultFalse;
	if (!s.writeDouble(thetaStrike))
		return kResultFalse;
	if (!s.writeDouble(thetaListening))
		return kResultFalse;
	if (!s.writeDouble(phiStrike))
		return kResultFalse;
	if (!s.writeDouble(phiListening))
		return kResultFalse;

	if (!s.writeDouble(releaseTime))
		return kResultFalse;
	if (!s.writeInt8(bypassSNA))
		return kResultFalse;

	// version 1
	if (!s.writeInt8(filterType))
		return kResultFalse;
	if (!s.writeDouble(filterFreq))
		return kResultFalse;
	if (!s.writeDouble(filterQ))
		return kResultFalse;

	// version 2
	if (!s.writeDouble(masterVolume))
		return kResultFalse;
	if (!s.writeDouble(masterTuning))
		return kResultFalse;
	if (!s.writeDouble(velToLevel))
		return kResultFalse;
	if (!s.writeDouble(freqModDepth))
		return kResultFalse;
	if (!s.writeInt8(tuningRange))
		return kResultFalse;

	// version 3
	if (!s.writeDouble (decay))
		return kResultFalse;

	return kResultTrue;
}


void initParameters(Steinberg::Vst::ParameterContainer& parameters) {
	Parameter* param;

	param = new RangeParameter(USTRING("Master Volume"), kParamMasterVolume, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Master Tuning"), kParamMasterTuning, USTRING("cent"), -200, 200, 0);
	param->setPrecision(0);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Velocity To Level"), kParamVelToLevel, USTRING("%"), 0, 100, 100);
	param->setPrecision(1);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Release Time"), kParamReleaseTime, USTRING("sec"), 0.005, MAX_RELEASE_TIME_SEC, 0.025);
	param->setPrecision(3);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Decay"), kParamDecay, USTRING("%"), 0, 100, 0);
	param->setPrecision(3);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Radius Strike"), kParamRadiusStrike, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);
	param = new RangeParameter(USTRING("Radius Listening"), kParamRadiusListening, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);
	param = new RangeParameter(USTRING("Theta Strike"), kParamThetaStrike, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);
	param = new RangeParameter(USTRING("Theta Listening"), kParamThetaListening, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);
	param = new RangeParameter(USTRING("Phi Strike"), kParamPhiStrike, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);
	param = new RangeParameter(USTRING("Phi Listening"), kParamPhiListening, USTRING("%"), 0, 100, 80);
	param->setPrecision(1);
	parameters.addParameter(param);


	auto* filterTypeParam = new StringListParameter(USTRING("Filter Type"), kParamFilterType);
	filterTypeParam->appendString(USTRING("Lowpass"));
	filterTypeParam->appendString(USTRING("Highpass"));
	filterTypeParam->appendString(USTRING("Bandpass"));
	parameters.addParameter(filterTypeParam);

	param = new LogScaleParameter<ParamValue>(USTRING("Filter Frequency"), kParamFilterFreq, VoiceStatics::freqLogScale);
	param->setPrecision(1);
	parameters.addParameter(param);

	param = new RangeParameter(USTRING("Frequency Mod Depth"), kParamFilterFreqModDepth, USTRING("%"), -100, 100, 100);
	param->setPrecision(1);
	parameters.addParameter(param);

	param = parameters.addParameter(USTRING("Filter Q"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamFilterQ);
	param->setPrecision(2);

	parameters.addParameter(USTRING("Bypass SNA"), nullptr, 1, 0, ParameterInfo::kCanAutomate, kParamBypassSNA);

	parameters.addParameter(new RangeParameter(USTRING("Active Voices"), kParamActiveVoices, nullptr, 0, MAX_VOICES, 0, MAX_VOICES, ParameterInfo::kIsReadOnly));

	auto* tuningRangeParam = new StringListParameter(USTRING("Tuning Range"), kParamTuningRange, nullptr, ParameterInfo::kIsList);
	tuningRangeParam->appendString(USTRING("[-1, +1] Octave"));
	tuningRangeParam->appendString(USTRING("[-3, +2] Tunes"));
	parameters.addParameter(tuningRangeParam);
}

/*void initNoteExpressions(Steinberg::Vst::NoteExpressionTypeContainer& noteExpressionTypes ) {

	// Init Note Expression Types
	auto volumeNoteExp = new NoteExpressionType(kVolumeTypeID, String("Volume"), String("Vol"), nullptr, -1, 1., 0., 1., 0, 0);
	volumeNoteExp->setPhysicalUITypeID(PhysicalUITypeIDs::kPUIPressure);
	noteExpressionTypes.addNoteExpressionType(volumeNoteExp);
	noteExpressionTypes.addNoteExpressionType(new PanNoteExpressionType());
	NoteExpressionType* tuningNoteExpression = new RangeNoteExpressionType(kTuningTypeID, String("Tuning"), String("Tun"), String("Half Tone"), -1, 0, 120, -120, NoteExpressionTypeInfo::kIsBipolar);
	tuningNoteExpression->getInfo().valueDesc.minimum = 0.5 - VoiceStatics::kNormTuningOneOctave;
	tuningNoteExpression->getInfo().valueDesc.maximum = 0.5 + VoiceStatics::kNormTuningOneOctave;
	tuningNoteExpression->setPhysicalUITypeID(PhysicalUITypeIDs::kPUIXMovement);
	noteExpressionTypes.addNoteExpressionType(tuningNoteExpression);

	auto noteExp = new NoteExpressionType(Controller::kRadiusStrikeTypeID, String("Radius Strike"), String("Radius Strike"), String("%"), -1, getParameterObject(kParamRadiusStrike), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);
	noteExp = new NoteExpressionType(Controller::kRadiusListeningTypeID, String("Radius Listening"), String("Radius Listening"), String("%"), -1, getParameterObject(kParamRadiusListening), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);
	noteExp = new NoteExpressionType(Controller::kThetaStrikeTypeID, String("Theta Strike"), String("Theta Strike"), String("%"), -1, getParameterObject(kParamThetaStrike), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);
	noteExp = new NoteExpressionType(Controller::kThetaListeningTypeID, String("Theta Listening"), String("Theta Listening"), String("%"), -1, getParameterObject(kParamThetaListening), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);
	noteExp = new NoteExpressionType(Controller::kPhiStrikeTypeID, String("Phi Strike"), String("Phi Strike"), String("%"), -1, getParameterObject(kParamPhiStrike), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);
	noteExp = new NoteExpressionType(Controller::kPhiListeningTypeID, String("Phi Listening"), String("Phi Listening"), String("%"), -1, getParameterObject(kParamPhiListening), NoteExpressionTypeInfo::kIsAbsolute);
	noteExpressionTypes.addNoteExpressionType(noteExp);


	auto rNoteExp = new RangeNoteExpressionType(Controller::kFilterFreqModTypeID, String("Filter Frequency Modulation"), String("Freq Mod"), nullptr, -1, 0, -100, 100, NoteExpressionTypeInfo::kIsBipolar, 0);
	rNoteExp->setPhysicalUITypeID(PhysicalUITypeIDs::kPUIYMovement);
	noteExpressionTypes.addNoteExpressionType(rNoteExp);

	noteExpressionTypes.addNoteExpressionType(new RangeNoteExpressionType(Controller::kFilterQModTypeID, String("Filter Q Modulation"), String("Q Mod"), nullptr, -1, 0, -100, 100, NoteExpressionTypeInfo::kIsBipolar, 0));
	noteExpressionTypes.addNoteExpressionType(new NoteExpressionType(Controller::kFilterTypeTypeID, String("Filter Type"), String("Flt Type"), nullptr, -1, getParameterObject(kParamFilterType), NoteExpressionTypeInfo::kIsBipolar));
	noteExpressionTypes.addNoteExpressionType(new ReleaseTimeModNoteExpressionType());

}*/



}