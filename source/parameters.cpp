#include "parameters.h"
#include "base/source/fstreamer.h"
#include "public.sdk/samples/vst/common/logscale.h"
#include "voice.h" // For VoiceStatics
#include "controller.h"
#include "processor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <limits>
#include <algorithm>


namespace Steinberg::Vst::NoteExpressionSynth {


void GlobalParameterState::defaultSettings() {
	bypass = false;

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
	decay = .2;
	size = 0;

	filterFreq = 1;
	filterQ = 0;
	freqModDepth = 1;

	filterType = 0;
	tuningRange = 1;
	std::fill(X.begin(), X.end(), .5);
	std::fill(Y.begin(), Y.end(), .5);

	resonatorType = 0;
	dimension = 10;

	bypassSNA = 0;
}


void processParameters(Steinberg::Vst::IParamValueQueue* queue, GlobalParameterState& paramState, Processor& p) {
	int32 sampleOffset;
	ParamValue value;
	ParamID pid = queue->getParameterId();

	if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) == kResultTrue) {
		switch (pid) {

		case kBypass: paramState.bypass = (value > 0.5f); break;

		case kParamMasterVolume: paramState.masterVolume = value; break;
		case kParamMasterTuning: paramState.masterTuning = 2 * (value - 0.5); break;
		case kParamVelToLevel: paramState.velToLevel = value; break;
		case kParamFilterFreqModDepth: paramState.freqModDepth = 2 * (value - 0.5); break;

		case kParamX0: paramState.X[0] = value; p.strikingPositionChanged(); break;
		case kParamX1: paramState.X[1] = value; p.strikingPositionChanged(); break;
		case kParamX2: paramState.X[2] = value; p.strikingPositionChanged(); break;
		case kParamX3: paramState.X[3] = value; p.strikingPositionChanged(); break;
		case kParamX4: paramState.X[4] = value; p.strikingPositionChanged(); break;
		case kParamX5: paramState.X[5] = value; p.strikingPositionChanged(); break;
		case kParamX6: paramState.X[6] = value; p.strikingPositionChanged(); break;
		case kParamX7: paramState.X[7] = value; p.strikingPositionChanged(); break;
		case kParamX8: paramState.X[8] = value; p.strikingPositionChanged(); break;
		case kParamX9: paramState.X[9] = value; p.strikingPositionChanged(); break;

		case kParamY0: paramState.Y[0] = value; p.listeningPositionChanged(); break;
		case kParamY1: paramState.Y[1] = value; p.listeningPositionChanged(); break;
		case kParamY2: paramState.Y[2] = value; p.listeningPositionChanged(); break;
		case kParamY3: paramState.Y[3] = value; p.listeningPositionChanged(); break;
		case kParamY4: paramState.Y[4] = value; p.listeningPositionChanged(); break;
		case kParamY5: paramState.Y[5] = value; p.listeningPositionChanged(); break;
		case kParamY6: paramState.Y[6] = value; p.listeningPositionChanged(); break;
		case kParamY7: paramState.Y[7] = value; p.listeningPositionChanged(); break;
		case kParamY8: paramState.Y[8] = value; p.listeningPositionChanged(); break;
		case kParamY9: paramState.Y[9] = value; p.listeningPositionChanged(); break;


		case kParamReleaseTime:
			paramState.releaseTime = value; break;
		case kParamDecay:
			paramState.decay = value; break;
		case kParamSize:
			paramState.size = value; break;
		case kParamDim:
			paramState.dimension = std::min<int8>((int8)(11 * value + 1), 10);p.dimensionChanged();break;
		case kParamFilterType:
			paramState.filterType = std::min<int8>((int8)(NUM_FILTER_TYPE * value), NUM_FILTER_TYPE - 1); break;

		case kParamFilterFreq:
			paramState.filterFreq = value; break;
		case kParamFilterQ:
			paramState.filterQ = value;	break;
		case kParamBypassSNA:
			paramState.bypassSNA = (value >= 0.5) ? 1 : 0; break;
		case kParamTuningRange:
			paramState.tuningRange = std::min<int8>((int8)(NUM_TUNING_RANGE * value), NUM_TUNING_RANGE - 1);
			break;

		case kParamResonatorType:
			paramState.resonatorType = value; p.resonatorTypeChanged();  break;

		}
	}
}

static uint64 currentParamStateVersion = 6;

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
		if (!s.readDouble(decay))
			return kResultFalse;
	}
	if (version >= 5)
	{
		if (!s.readBool(bypass)) return kResultFalse;

		if (!s.readDouble(X[0])) return kResultFalse;
		if (!s.readDouble(X[1])) return kResultFalse;
		if (!s.readDouble(X[2])) return kResultFalse;
		if (!s.readDouble(X[3])) return kResultFalse;
		if (!s.readDouble(X[4])) return kResultFalse;
		if (!s.readDouble(X[5])) return kResultFalse;
		if (!s.readDouble(X[6])) return kResultFalse;
		if (!s.readDouble(X[7])) return kResultFalse;
		if (!s.readDouble(X[8])) return kResultFalse;
		if (!s.readDouble(X[9])) return kResultFalse;

		if (!s.readDouble(Y[0])) return kResultFalse;
		if (!s.readDouble(Y[1])) return kResultFalse;
		if (!s.readDouble(Y[2])) return kResultFalse;
		if (!s.readDouble(Y[3])) return kResultFalse;
		if (!s.readDouble(Y[4])) return kResultFalse;
		if (!s.readDouble(Y[5])) return kResultFalse;
		if (!s.readDouble(Y[6])) return kResultFalse;
		if (!s.readDouble(Y[7])) return kResultFalse;
		if (!s.readDouble(Y[8])) return kResultFalse;
		if (!s.readDouble(Y[9])) return kResultFalse;

		if (!s.readDouble(size)) return kResultFalse;
		if (!s.readInt8(resonatorType))	return kResultFalse;
	}

	if (version >= 6)
	{
		if (!s.readInt8(dimension))	return kResultFalse;
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

	// version 4
	if (!s.writeDouble(decay))
		return kResultFalse;

	// version 5

	if (!s.writeBool(bypass)) return kResultFalse;

	if (!s.writeDouble(X[0])) return kResultFalse;
	if (!s.writeDouble(X[1])) return kResultFalse;
	if (!s.writeDouble(X[2])) return kResultFalse;
	if (!s.writeDouble(X[3])) return kResultFalse;
	if (!s.writeDouble(X[4])) return kResultFalse;
	if (!s.writeDouble(X[5])) return kResultFalse;
	if (!s.writeDouble(X[6])) return kResultFalse;
	if (!s.writeDouble(X[7])) return kResultFalse;
	if (!s.writeDouble(X[8])) return kResultFalse;
	if (!s.writeDouble(X[9])) return kResultFalse;

	if (!s.writeDouble(Y[0])) return kResultFalse;
	if (!s.writeDouble(Y[1])) return kResultFalse;
	if (!s.writeDouble(Y[2])) return kResultFalse;
	if (!s.writeDouble(Y[3])) return kResultFalse;
	if (!s.writeDouble(Y[4])) return kResultFalse;
	if (!s.writeDouble(Y[5])) return kResultFalse;
	if (!s.writeDouble(Y[6])) return kResultFalse;
	if (!s.writeDouble(Y[7])) return kResultFalse;
	if (!s.writeDouble(Y[8])) return kResultFalse;
	if (!s.writeDouble(Y[9])) return kResultFalse;

	if (!s.writeDouble(size)) return kResultFalse;
	if (!s.writeInt8(resonatorType)) return kResultFalse;

	// version 6
	if (!s.writeInt8(dimension)) return kResultFalse;


	return kResultTrue;
}


void initParameters(Steinberg::Vst::ParameterContainer& parameters) {
	Parameter* param;

	parameters.addParameter(STR16("Bypass"), 0, 1, 0, Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, kBypass);


	auto addRangeParameter = [&](UString256 a, ParamID id, UString256 units, ParamValue min, ParamValue max, ParamValue defaultValue, int32 precision = 1) {
		Parameter* param = new RangeParameter(a, id, units, min, max, defaultValue);
		param->setPrecision(precision);
		parameters.addParameter(param);
	};

	addRangeParameter("Master Volume", Params::kParamMasterVolume, "%", 0, 100, 80, 1);
	addRangeParameter("Master Tuning", Params::kParamMasterTuning, "ct", -200, 200, 0, 0);
	addRangeParameter("Velocity To Level", Params::kParamVelToLevel, "%", 0, 100, 100, 1);

	addRangeParameter("Release Time", Params::kParamReleaseTime, "sec", 0.005, MAX_RELEASE_TIME_SEC, 0.025, 3);

	addRangeParameter("Decay", Params::kParamDecay, "%", 0, 100, 80, 1);

	addRangeParameter("X0", Params::kParamX0, "%", 0, 100, 20, 0);
	addRangeParameter("X1", Params::kParamX1, "%", 0, 100, 20, 0);
	addRangeParameter("X2", Params::kParamX2, "%", 0, 100, 20, 0);
	addRangeParameter("X3", Params::kParamX3, "%", 0, 100, 20, 0);
	addRangeParameter("X4", Params::kParamX4, "%", 0, 100, 20, 0);
	addRangeParameter("X5", Params::kParamX5, "%", 0, 100, 20, 0);
	addRangeParameter("X6", Params::kParamX6, "%", 0, 100, 20, 0);
	addRangeParameter("X7", Params::kParamX7, "%", 0, 100, 20, 0);
	addRangeParameter("X8", Params::kParamX8, "%", 0, 100, 20, 0);
	addRangeParameter("X9", Params::kParamX9, "%", 0, 100, 20, 0);

	addRangeParameter("Y0", Params::kParamY0, "%", 0, 100, 20, 0);
	addRangeParameter("Y1", Params::kParamY1, "%", 0, 100, 20, 0);
	addRangeParameter("Y2", Params::kParamY2, "%", 0, 100, 20, 0);
	addRangeParameter("Y3", Params::kParamY3, "%", 0, 100, 20, 0);
	addRangeParameter("Y4", Params::kParamY4, "%", 0, 100, 20, 0);
	addRangeParameter("Y5", Params::kParamY5, "%", 0, 100, 20, 0);
	addRangeParameter("Y6", Params::kParamY6, "%", 0, 100, 20, 0);
	addRangeParameter("Y7", Params::kParamY7, "%", 0, 100, 20, 0);
	addRangeParameter("Y8", Params::kParamY8, "%", 0, 100, 20, 0);
	addRangeParameter("Y9", Params::kParamY9, "%", 0, 100, 20, 0);
	
	addRangeParameter("Angle", Params::kParamAngle, "%", 0, 100, 0, 1);
	addRangeParameter("ResFreq", Params::kParamSize, "%", 0, 100, 0, 1);
	addRangeParameter("Dimension", Params::kParamDim, " ", 1, 10, 10, 0);

	parameters.addParameter(new RangeParameter(USTRING("Output Volume"), Params::kParamOutputVolume, nullptr, 0, 1, 0, 0, ParameterInfo::kIsReadOnly));


	auto* resonatorTypeParam = new StringListParameter(USTRING("Resonator Type"), kParamResonatorType);
	resonatorTypeParam->appendString(USTRING("Sphere"));
	resonatorTypeParam->appendString(USTRING("Cube"));
	parameters.addParameter(resonatorTypeParam);

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

tresult PLUGIN_API Controller::setComponentState(IBStream* state)
{
	GlobalParameterState gps;
	tresult result = gps.setState(state);
	if (result == kResultTrue)
	{
		setParamNormalized(kBypass, gps.bypass);

		setParamNormalized(kParamMasterVolume, gps.masterVolume);
		setParamNormalized(kParamMasterTuning, (gps.masterTuning + 1) / 2.);
		setParamNormalized(kParamVelToLevel, gps.velToLevel);
		setParamNormalized(kParamFilterFreqModDepth, (gps.freqModDepth + 1) / 2.);

		setParamNormalized(kParamReleaseTime, gps.releaseTime);
		setParamNormalized(kParamDecay, gps.decay);

		/*setParamNormalized (kParamRadiusStrike, gps.radiusStrike);
		setParamNormalized(kParamRadiusListening, gps.radiusListening);
		setParamNormalized(kParamThetaStrike, gps.thetaStrike);
		setParamNormalized(kParamThetaListening, gps.thetaListening);
		setParamNormalized(kParamPhiStrike, gps.phiStrike);
		setParamNormalized(kParamPhiListening, gps.phiListening);*/

		setParamNormalized(kParamFilterType, plainParamToNormalized(kParamFilterType, gps.filterType));
		setParamNormalized(kParamFilterFreq, gps.filterFreq);
		setParamNormalized(kParamFilterQ, gps.filterQ);

		setParamNormalized(kParamBypassSNA, gps.bypassSNA);

		setParamNormalized(kParamTuningRange, plainParamToNormalized(kParamTuningRange, gps.tuningRange));

		setParamNormalized(kParamX0, gps.X[0]);
		setParamNormalized(kParamX1, gps.X[1]);
		setParamNormalized(kParamX2, gps.X[2]);
		setParamNormalized(kParamX3, gps.X[3]);
		setParamNormalized(kParamX4, gps.X[4]);
		setParamNormalized(kParamX5, gps.X[5]);
		setParamNormalized(kParamX6, gps.X[6]);
		setParamNormalized(kParamX7, gps.X[7]);
		setParamNormalized(kParamX8, gps.X[8]);
		setParamNormalized(kParamX9, gps.X[9]);

		setParamNormalized(kParamY0, gps.Y[0]);
		setParamNormalized(kParamY1, gps.Y[1]);
		setParamNormalized(kParamY2, gps.Y[2]);
		setParamNormalized(kParamY3, gps.Y[3]);
		setParamNormalized(kParamY4, gps.Y[4]);
		setParamNormalized(kParamY5, gps.Y[5]);
		setParamNormalized(kParamY6, gps.Y[6]);
		setParamNormalized(kParamY7, gps.Y[7]);
		setParamNormalized(kParamY8, gps.Y[8]);
		setParamNormalized(kParamY9, gps.Y[9]);

		setParamNormalized(kParamSize, gps.size);
		setParamNormalized(kParamResonatorType, plainParamToNormalized(kParamResonatorType, gps.resonatorType));
		setParamNormalized(kParamDim, gps.dimension);

	}
	return result;
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