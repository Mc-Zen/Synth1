//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/note_expression_synth_controller.cpp
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

#include "controller.h"
#include "voice.h" // only needed for setComponentState
#include "base/source/fstring.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ustring.h"

namespace Steinberg {
namespace Vst {
namespace NoteExpressionSynth {

FUID Controller::cid (0x882ca7ff, 0x7f93430f, 0xb967e4d8, 0x9b482fc9);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::initialize (FUnknown* context)
{
	tresult result = EditController::initialize (context);
	if (result == kResultTrue)
	{
	// Init parameters

		initParameters(parameters);
		
	// Init Note Expression Types
		auto volumeNoteExp = new NoteExpressionType (kVolumeTypeID, String ("Volume"), String ("Vol"), nullptr, -1, 1., 0., 1., 0, 0);
		volumeNoteExp->setPhysicalUITypeID(PhysicalUITypeIDs::kPUIPressure);
		noteExpressionTypes.addNoteExpressionType (volumeNoteExp);
		noteExpressionTypes.addNoteExpressionType (new PanNoteExpressionType ());
		NoteExpressionType* tuningNoteExpression = new RangeNoteExpressionType (kTuningTypeID, String ("Tuning"), String ("Tun"), String ("Half Tone"), -1, 0, 120, -120, NoteExpressionTypeInfo::kIsBipolar);
		tuningNoteExpression->getInfo ().valueDesc.minimum = 0.5 - VoiceStatics::kNormTuningOneOctave;
		tuningNoteExpression->getInfo ().valueDesc.maximum = 0.5 + VoiceStatics::kNormTuningOneOctave;
		tuningNoteExpression->setPhysicalUITypeID (PhysicalUITypeIDs::kPUIXMovement);
		noteExpressionTypes.addNoteExpressionType (tuningNoteExpression);
		
		auto noteExp = new NoteExpressionType (kRadiusStrikeTypeID, String ("Radius Strike"), String ("Radius Strike"), String ("%"), -1, getParameterObject (kParamX0), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType (noteExp);
		noteExp = new NoteExpressionType(kRadiusListeningTypeID, String("Radius Listening"), String("Radius Listening"), String("%"), -1, getParameterObject(kParamY0), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType(noteExp);
		noteExp = new NoteExpressionType(kThetaStrikeTypeID, String("Theta Strike"), String("Theta Strike"), String("%"), -1, getParameterObject(kParamX1), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType(noteExp);
		noteExp = new NoteExpressionType(kThetaListeningTypeID, String("Theta Listening"), String("Theta Listening"), String("%"), -1, getParameterObject(kParamY1), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType(noteExp);
		noteExp = new NoteExpressionType(kPhiStrikeTypeID, String("Phi Strike"), String("Phi Strike"), String("%"), -1, getParameterObject(kParamX2), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType(noteExp);
		noteExp = new NoteExpressionType(kPhiListeningTypeID, String("Phi Listening"), String("Phi Listening"), String("%"), -1, getParameterObject(kParamY2), NoteExpressionTypeInfo::kIsAbsolute);
		noteExpressionTypes.addNoteExpressionType(noteExp);

		
		auto rNoteExp = new RangeNoteExpressionType (kFilterFreqModTypeID, String ("Filter Frequency Modulation"), String ("Freq Mod"), nullptr, -1, 0, -100, 100, NoteExpressionTypeInfo::kIsBipolar, 0);
		rNoteExp->setPhysicalUITypeID (PhysicalUITypeIDs::kPUIYMovement);
		noteExpressionTypes.addNoteExpressionType (rNoteExp);

		noteExpressionTypes.addNoteExpressionType (new RangeNoteExpressionType (kFilterQModTypeID, String ("Filter Q Modulation"), String ("Q Mod"), nullptr, -1, 0, -100, 100, NoteExpressionTypeInfo::kIsBipolar, 0));
		noteExpressionTypes.addNoteExpressionType (new NoteExpressionType (kFilterTypeTypeID, String ("Filter Type"), String ("Flt Type"), nullptr, -1, getParameterObject (kParamFilterType), NoteExpressionTypeInfo::kIsBipolar));
		noteExpressionTypes.addNoteExpressionType (new ReleaseTimeModNoteExpressionType ());

	// Init Default MIDI-CC Map
		std::for_each (midiCCMapping.begin (), midiCCMapping.end (), [] (ParamID& pid) { pid = InvalidParamID; });
		midiCCMapping[ControllerNumbers::kPitchBend] = kParamMasterTuning;
		midiCCMapping[ControllerNumbers::kCtrlVolume] = kParamMasterVolume;
		midiCCMapping[ControllerNumbers::kCtrlModWheel] = kParamFilterFreqModDepth;
		midiCCMapping[ControllerNumbers::kCtrlFilterCutoff] = kParamFilterFreq;
		midiCCMapping[ControllerNumbers::kCtrlFilterResonance] = kParamFilterQ;
	}
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::terminate ()
{
	noteExpressionTypes.removeAll ();
	return EditController::terminate ();
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::setComponentState (IBStream* state)
{
	GlobalParameterState gps;
	tresult result = gps.setState (state);
	if (result == kResultTrue)
	{
		setParamNormalized(kBypass, gps.bypass);

		setParamNormalized (kParamMasterVolume, gps.masterVolume);
		setParamNormalized (kParamMasterTuning, (gps.masterTuning + 1) / 2.);
		setParamNormalized (kParamVelToLevel, gps.velToLevel);
		setParamNormalized (kParamFilterFreqModDepth, (gps.freqModDepth + 1) / 2.);

		setParamNormalized(kParamReleaseTime, gps.releaseTime);
		setParamNormalized(kParamDecay, gps.decay);

		/*setParamNormalized (kParamRadiusStrike, gps.radiusStrike);
		setParamNormalized(kParamRadiusListening, gps.radiusListening);
		setParamNormalized(kParamThetaStrike, gps.thetaStrike);
		setParamNormalized(kParamThetaListening, gps.thetaListening);
		setParamNormalized(kParamPhiStrike, gps.phiStrike);
		setParamNormalized(kParamPhiListening, gps.phiListening);*/

		setParamNormalized (kParamFilterType, plainParamToNormalized (kParamFilterType, gps.filterType));
		setParamNormalized (kParamFilterFreq, gps.filterFreq);
		setParamNormalized (kParamFilterQ, gps.filterQ);

		setParamNormalized (kParamBypassSNA, gps.bypassSNA);

		setParamNormalized (kParamTuningRange, plainParamToNormalized (kParamTuningRange, gps.tuningRange));

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
	}
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::setParamNormalized (ParamID tag, ParamValue value)
{
	bool newRange = false;
	if (tag == kParamTuningRange && getParamNormalized (tag) != value)
	{
		newRange = true;
		NoteExpressionType* net = noteExpressionTypes.getNoteExpressionType (kTuningTypeID);
		if (value > 0)
		{
			//noteExpressionTypes.addNoteExpressionType (new NoteExpressionType (kTriangleSlopeTypeID, String ("Triangle Slope"), String ("Tri Slope"), String ("%"),-1, getParameterObject (kParamTriangleSlop), NoteExpressionTypeInfo::kIsAbsolute));
			if (net)
			{
				net->getInfo ().valueDesc.minimum = 0.5 - 3 * VoiceStatics::kNormTuningOneTune;
				net->getInfo ().valueDesc.maximum = 0.5 + 2 * VoiceStatics::kNormTuningOneTune;
			}
		}
		else
		{
			//noteExpressionTypes.removeNoteExpressionType (kTriangleSlopeTypeID);
			if (net)
			{
				net->getInfo ().valueDesc.minimum = 0.5 - VoiceStatics::kNormTuningOneOctave;
				net->getInfo ().valueDesc.maximum = 0.5 + VoiceStatics::kNormTuningOneOctave;
			}
		}
	}

	tresult res = EditController::setParamNormalized (tag, value);

	if (newRange && componentHandler)
		componentHandler->restartComponent (kNoteExpressionChanged);

	return res;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::getMidiControllerAssignment (int32 busIndex, int16 channel,
                                                            CtrlNumber midiControllerNumber,
                                                            ParamID& id /*out*/)
{
	if (busIndex == 0 && channel == 0 && midiControllerNumber < kCountCtrlNumber)
	{
		if (midiCCMapping[midiControllerNumber] != InvalidParamID)
		{
			id = midiCCMapping[midiControllerNumber];
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
int32 PLUGIN_API Controller::getNoteExpressionCount (int32 busIndex, int16 channel)
{
	if (busIndex == 0 && channel == 0)
	{
		return noteExpressionTypes.getNoteExpressionCount ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::getNoteExpressionInfo (int32 busIndex, int16 channel,
                                                      int32 noteExpressionIndex,
                                                      NoteExpressionTypeInfo& info /*out*/)
{
	if (busIndex == 0 && channel == 0)
	{
		return noteExpressionTypes.getNoteExpressionInfo (noteExpressionIndex, info);
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::getNoteExpressionStringByValue (
    int32 busIndex, int16 channel, NoteExpressionTypeID id,
    NoteExpressionValue valueNormalized /*in*/, String128 string /*out*/)
{
	if (busIndex == 0 && channel == 0)
	{
		return noteExpressionTypes.getNoteExpressionStringByValue (id, valueNormalized, string);
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::getNoteExpressionValueByString (
    int32 busIndex, int16 channel, NoteExpressionTypeID id, const TChar* string /*in*/,
    NoteExpressionValue& valueNormalized /*out*/)
{
	if (busIndex == 0 && channel == 0)
	{
		return noteExpressionTypes.getNoteExpressionValueByString (id, string, valueNormalized);
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API Controller::getPhysicalUIMapping (int32 busIndex, int16 channel,
                                                     PhysicalUIMapList& list)
{
	if (busIndex == 0 && channel == 0)
	{
		for (uint32 i = 0; i < list.count; ++i)
		{
			NoteExpressionTypeID type = kInvalidTypeID;
			if (noteExpressionTypes.getMappedNoteExpression (list.map[i].physicalUITypeID,
			                                                 type) == kResultTrue)
				list.map[i].noteExpressionTypeID = type;
		}
		return kResultTrue;
	}
	return kResultFalse;
}

PanNoteExpressionType::PanNoteExpressionType()
	: RangeNoteExpressionType(
		kPanTypeID, String("Pan"), String("Pan"), nullptr, -1, 0, -100, 100,
		NoteExpressionTypeInfo::kIsBipolar | NoteExpressionTypeInfo::kIsAbsolute, 0)
{
}

tresult PanNoteExpressionType::getStringByValue(NoteExpressionValue valueNormalized /*in*/, String128 string /*out*/) 
{
	if (valueNormalized == 0.5)
		UString128("C").copyTo(string, 128);
	else if (valueNormalized == 0)
		UString128("L").copyTo(string, 128);
	else if (valueNormalized == 1)
		UString128("R").copyTo(string, 128);
	else
		RangeNoteExpressionType::getStringByValue(valueNormalized, string);
	return kResultTrue;
}

tresult PanNoteExpressionType::getValueByString(const TChar* string /*in*/, NoteExpressionValue& valueNormalized /*out*/) 
{
	String str(string);
	if (str == "C")
	{
		valueNormalized = 0.5;
		return kResultTrue;
	}
	else if (str == "L")
	{
		valueNormalized = 0.;
		return kResultTrue;
	}
	else if (str == "R")
	{
		valueNormalized = 1.;
		return kResultTrue;
	}
	return RangeNoteExpressionType::getValueByString(string, valueNormalized);
}

ReleaseTimeModNoteExpressionType::ReleaseTimeModNoteExpressionType()
	: NoteExpressionType(Controller::kReleaseTimeModTypeID, String("Release Time"), String("RelTime"), String("%"),
		-1, 0.5, 0., 1., 0, NoteExpressionTypeInfo::kIsBipolar | NoteExpressionTypeInfo::kIsOneShot)
{
}
tresult ReleaseTimeModNoteExpressionType::getStringByValue(NoteExpressionValue valueNormalized /*in*/, String128 string /*out*/)
{
	UString128 wrapper;
	double timeFactor = pow(100., 2 * (valueNormalized - 0.5));
	wrapper.printFloat(timeFactor, timeFactor > 10 ? 1 : 2);
	wrapper.copyTo(string, 128);
	return kResultTrue;
}

tresult ReleaseTimeModNoteExpressionType::getValueByString(const TChar* string /*in*/, NoteExpressionValue& valueNormalized /*out*/)
{
	String wrapper((TChar*)string);
	ParamValue tmp;
	if (wrapper.scanFloat(tmp))
	{
		valueNormalized = Bound(0.0, 1.0, log10(tmp) / 4. + 0.5);
		return kResultTrue;
	}
	return kResultFalse;
}
//------------------------------------------------------------------------
} // NoteExpressionSynth
} // Vst
} // Steinberg
