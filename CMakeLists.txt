
if(SMTG_ADD_VSTGUI)
    set(noteexpressionsynth_sources
        source/brownnoise.h
        source/factory.cpp
        source/filter.h
        source/controller.cpp
        source/controller.h
        source/processor.cpp
        source/processor.h
        source/parameters.cpp
        source/parameters.h
        source/ui.cpp
        source/ui.h
        source/voice.cpp
        source/voice.h
        source/eigen_evaluator.h
        source/note_touch_controller.cpp
        source/note_touch_controller.h
        source/version.h
        ${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/contrib/keyboardview.cpp
        ${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/contrib/keyboardview.h
        resource/note_expression_synth.uidesc
     )
     
    set(target synth1)
    #set(SDK_IDE_PLUGIN_EXAMPLES_FOLDER FOLDER "PlugInExamples") # added this

    smtg_add_vst3plugin(${target} ${noteexpressionsynth_sources})
    #set_target_properties(${target} PROPERTIES ${SDK_IDE_PLUGIN_EXAMPLES_FOLDER})
    set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
    target_link_libraries(${target} PRIVATE sdk vstgui_support)

    smtg_add_vst3_resource(${target} "resource/note_expression_synth.uidesc")
    smtg_add_vst3_resource(${target} "resource/about.png")
    smtg_add_vst3_resource(${target} "resource/background.png")
    smtg_add_vst3_resource(${target} "resource/groupframe.png")
    smtg_add_vst3_resource(${target} "resource/knob.png")
    smtg_add_vst3_resource(${target} "resource/knob2.png")
    smtg_add_vst3_resource(${target} "resource/knob big.png")
    smtg_add_vst3_resource(${target} "resource/vst3_logo_small.png")
    smtg_add_vst3_resource(${target} "resource/studstift_logo.png")
    smtg_add_vst3_resource(${target} "resource/vu_on.png")
    smtg_add_vst3_resource(${target} "resource/vu_off.png")
    
    smtg_add_vst3_snapshot(${target} "resource/4fa505e70d314866b2cf10c7781d4858_snapshot.png")
    smtg_add_vst3_snapshot(${target} "resource/4fa505e70d314866b2cf10c7781d4858_snapshot_2.0x.png")

    if(SMTG_MAC)
        smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
    elseif(SMTG_WIN)
        target_sources(${target} PRIVATE resource/note_expression_synth.rc)
    endif()

    if(SMTG_MAC AND XCODE AND SMTG_IOS_DEVELOPMENT_TEAM)
        set(target noteexpressionsynth_ios)
        smtg_add_ios_vst3plugin("${SMTG_CODE_SIGN_IDENTITY_IOS}" ${target} "${target}" "${noteexpressionsynth_sources}")

        set_target_properties(${target} PROPERTIES ${SDK_IDE_PLUGIN_EXAMPLES_FOLDER})
        target_include_directories(${target} PUBLIC ${SMTG_VSTGUI_ROOT}/vstgui4)
        target_link_libraries(${target} PRIVATE
            base_ios 
            sdk_ios
            "-framework UIKit"
            "-framework CoreGraphics" 
            "-framework QuartzCore" 
            "-framework CoreText" 
            "-framework Accelerate" 
            "-framework ImageIO" 
            "-framework MobileCoreServices" 
        )

        smtg_add_vst3_resource(${target} "resource/note_expression_synth.uidesc")
        smtg_add_vst3_resource(${target} "resource/about.png")
        smtg_add_vst3_resource(${target} "resource/background.png")
        smtg_add_vst3_resource(${target} "resource/groupframe.png")
        smtg_add_vst3_resource(${target} "resource/knob.png")
        smtg_add_vst3_resource(${target} "resource/knob2.png")
        smtg_add_vst3_resource(${target} "resource/knob big.png")
        smtg_add_vst3_resource(${target} "resource/vst3_logo_small.png")
        smtg_add_vst3_resource(${target} "resource/studstift_logo.png")
        smtg_add_vst3_resource(${target} "resource/vu_on.png")
        smtg_add_vst3_resource(${target} "resource/vu_off.png")
    
        smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
        target_sources(${target} PRIVATE 
            "${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription.cpp"
            "${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/vstgui_ios.mm"
            "${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.cpp"
            "${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3groupcontroller.cpp"
            "${SMTG_VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3padcontroller.cpp"
            "${SDK_ROOT}/public.sdk/source/vst/vstguieditor.cpp"
        )
    endif()

    
    target_compile_features(${target} PUBLIC cxx_std_17)

endif(SMTG_ADD_VSTGUI)
