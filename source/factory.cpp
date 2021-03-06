//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/factory.cpp
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

#include "public.sdk/source/main/pluginfactory.h"
#include "processor.h"
#include "controller.h"
#include "ui.h"
#include "version.h"	// for versioning

#if TARGET_OS_IPHONE
#include "public.sdk/source/vst/vstguieditor.h"
extern void* moduleHandle;
#endif

#define stringPluginName "Tesseract"


BEGIN_FACTORY_DEF("Roggenburg Technologies",
	"http://www.steinberg.net",
	"mailto:info@steinberg.de")

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::ProcessorWithUIController::cid),
		PClassInfo::kManyInstances,
		kVstAudioEffectClass,
		stringPluginName,
		Vst::kDistributable,
		"Instrument|Multidimensional",
		FULL_VERSION_STR,
		kVstVersionString,
		Steinberg::Vst::NoteExpressionSynth::ProcessorWithUIController::createInstance)

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::ControllerWithUI::cid),
		PClassInfo::kManyInstances,
		kVstComponentControllerClass,
		stringPluginName,
		0,						// not used here
		"",						// not used here
		FULL_VERSION_STR,
		kVstVersionString,
		Steinberg::Vst::NoteExpressionSynth::ControllerWithUI::createInstance)

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::ProcessorWithUIControllerFx::cid),
		PClassInfo::kManyInstances,
		kVstAudioEffectClass,
		stringPluginName " Fx",
		Vst::kDistributable,
		"Fx|Multidimensional",
		FULL_VERSION_STR,
		kVstVersionString,
		Steinberg::Vst::NoteExpressionSynth::ProcessorWithUIControllerFx::createInstance)

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::ControllerWithUIFx::cid),
		PClassInfo::kManyInstances,
		kVstComponentControllerClass,
		stringPluginName " Fx",
		0,						// not used here
		"",						// not used here
		FULL_VERSION_STR,
		kVstVersionString,
		Steinberg::Vst::NoteExpressionSynth::ControllerWithUIFx::createInstance)

	/*DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::Processor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				stringPluginName,
				Vst::kDistributable,
				Vst::PlugType::kFxInstrument,
				FULL_VERSION_STR,
				kVstVersionString,
				Steinberg::Vst::NoteExpressionSynth::Processor::createInstance)

	DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::NoteExpressionSynth::Controller::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				stringPluginName,
				0,						// not used here
				"",						// not used here
				FULL_VERSION_STR,
				kVstVersionString,
				Steinberg::Vst::NoteExpressionSynth::Controller::createInstance)*/

	END_FACTORY

	bool InitModule()
{
#if TARGET_OS_IPHONE
	Steinberg::Vst::VSTGUIEditor::setBundleRef(moduleHandle);
#endif
	return true;
}

bool DeinitModule() { return true; }
