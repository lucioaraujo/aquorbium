# the identity (assets/identity, approved by the author on 2026-10-25): the
# window/taskbar icon, and in the header the sonar symbol and the original
# vector lettering, drawn from the SVGs themselves
juce_add_gui_app(AquorbiumApp PRODUCT_NAME "Aquorbium" COMPANY_NAME "RASGO" BUNDLE_ID "org.rasgo.aquorbium"
    ICON_BIG "${PROJECT_SOURCE_DIR}/assets/identity/aquorbium-512.png"
    ICON_SMALL "${PROJECT_SOURCE_DIR}/assets/identity/aquorbium-32.png")
juce_add_binary_data(AquorbiumIdentity SOURCES "${PROJECT_SOURCE_DIR}/assets/identity/aquorbium-symbol.svg"
    "${PROJECT_SOURCE_DIR}/assets/identity/aquorbium-wordmark.svg"
    "${PROJECT_SOURCE_DIR}/assets/identity/aquorbium-256.png")
juce_generate_juce_header(AquorbiumApp)
target_sources(AquorbiumApp PRIVATE Main.cpp WindowLayoutPolicy.h EcologyParameterState.h
    PerformanceParameterState.h AudioControlFrame.h PerformancePresetBank.h EcologyPanelLayout.h
    PerformanceSceneFile.h PerformanceSceneControls.h ParameterXml.h MacroPanelLayout.h OrganismLabelLayout.h
    RuntimeAuditProtocol.h DiagnosticAudioCapture.h)
target_compile_features(AquorbiumApp PRIVATE cxx_std_20)
if (MSVC)
    target_compile_options(AquorbiumApp PRIVATE /utf-8)
else()
    target_compile_options(AquorbiumApp PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
target_link_libraries(AquorbiumApp PRIVATE
    aquorbium_core
    AquorbiumIdentity
    juce::juce_audio_utils
    juce::juce_data_structures
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
target_compile_definitions(AquorbiumApp PRIVATE
    JUCE_APPLICATION_NAME_STRING="Aquorbium"
    JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)
