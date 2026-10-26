cmake_minimum_required(VERSION 3.22)
project(Aquorbium VERSION 0.0.1 LANGUAGES C CXX)

option(AQUORBIUM_BUILD_APP "Build the standalone Aquorbium instrument" ON)
option(AQUORBIUM_BUILD_TESTS "Build Aquorbium core tests" ON)
option(AQUORBIUM_BUILD_TOOLS "Build Aquorbium offline validation tools" ON)
set(AQUORBIUM_JUCE_PATH "" CACHE PATH "Path to a JUCE source checkout")

function(aquorbium_configure_own_target target_name)
    set_target_properties(${target_name} PROPERTIES CXX_EXTENSIONS OFF)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /WX)
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_library(aquorbium_core STATIC
    src/core/AutonomousEnergy.cpp
    src/core/CompositionalVariationDirector.cpp
    src/core/EcologicalMusicDirector.cpp
    src/core/TimbreBody.cpp
    src/core/ChorusPhase.cpp
    src/core/SongForm.cpp
    src/core/SpectralConsonance.cpp
    src/core/PopulationCycles.cpp
    src/core/LifeCycle.cpp
    src/core/SoundChannel.cpp
    src/core/HabitatHealth.cpp
    src/core/RoomCoupling.cpp
    src/core/CodaExchange.cpp
    src/core/BubbleField.cpp
    src/core/BinauralSource.cpp
    src/core/SynthVoice.cpp
    src/core/SynthSources.cpp
    src/core/TruePeakDetector.cpp
    src/core/TakeAnalysis.cpp
    src/core/AquariumPose.cpp
    src/core/AquariumSpatializer.cpp
    src/core/BiomaBrain.cpp
    src/core/BiomaEngine.cpp
    src/core/BiomaGranularStriker.cpp
    src/core/BiomaModalResonator.cpp
    src/core/BiomaPhysicalString.cpp
    src/core/Ecosystem.cpp
    src/core/GeometricSequencer.cpp
    src/core/MultiTapSpace.cpp
    src/core/OrganismVoiceEngine.cpp)
target_sources(aquorbium_core PRIVATE
    src/core/SynthFMVoice.cpp
    src/core/SlewRateRandomWalk.cpp
    src/core/LorenzCV.cpp
    src/core/MasterChain.cpp
    src/core/BiomaGranularCapture.cpp
    src/core/BiomaMarkov.cpp
    src/core/AquariumTail.cpp
    src/core/ControlEventExtractor.cpp
    src/core/FractionalLogicClock.cpp
    src/core/AddressableStepMemory.cpp
    src/core/HarmonicProgression.cpp
    src/core/AccumulatingTransposer.cpp
    src/core/ThresholdStepSelector.cpp
    src/core/StepPatternBank.cpp
    src/core/RandomToLoop.cpp
    src/core/BranchingStepRouter.cpp
    src/core/NotePermutationMatrix.cpp
    src/core/PatternRotationMutator.cpp
    src/core/StepMemoryLayout.cpp
    src/core/DriftCurveShaper.cpp
    src/core/SamplePercussionVoice.cpp
    src/core/HarmonicMorphOscillator.cpp
    src/core/StochasticPulseMask.cpp
    src/core/FractalCore.cpp
    src/core/VectorGenerator.cpp
    src/core/ShiftRegisterMemory.cpp
    src/core/ProbabilityDistribution.cpp
    src/core/EvolutionLoopMachine.cpp
    src/core/ConditionalStageMachine.cpp
    src/core/TripleSlowDrift.cpp
    src/core/IntegrityCrossfade.cpp
    src/core/HarmonicPhaseBank.cpp
    src/core/FluidMotion.cpp
    src/core/MotionWaveBank.cpp
    src/core/MotionGestureRecorder.cpp
    src/core/OpenInteractionModules.cpp
    src/core/CoreRefinementModules.cpp
    src/core/SchoolSpectralBank.cpp
    src/core/EcologicalNoiseBed.cpp
    src/core/PhilosophyMixEngine.cpp
    src/core/BehaviourMemory.cpp
    src/core/EmergentSoundModules.cpp
    src/core/AdvancedVoiceEcologyModules.cpp
    src/core/GenerativeInteractionModules.cpp
    src/core/RoutingResourceModules.cpp
    src/core/IntegratedModuleBank.cpp)
target_sources(aquorbium_core PRIVATE
    src/core/LoudnessMeter.cpp
    src/core/OutputStage.cpp)
target_include_directories(aquorbium_core PUBLIC src)
target_compile_features(aquorbium_core PUBLIC cxx_std_20)
aquorbium_configure_own_target(aquorbium_core)

if (AQUORBIUM_BUILD_TESTS)
    enable_testing()
    find_package(Threads REQUIRED)
    add_executable(aquorbium_core_tests tests/BiomaEngineTests.cpp)
    target_link_libraries(aquorbium_core_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_core_tests)
    add_test(NAME aquorbium_core_tests COMMAND aquorbium_core_tests)

    add_executable(aquorbium_ecosystem_tests tests/EcosystemTests.cpp)
    target_link_libraries(aquorbium_ecosystem_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ecosystem_tests)
    add_test(NAME aquorbium_ecosystem_tests COMMAND aquorbium_ecosystem_tests)

    add_executable(aquorbium_lens_audio_tests tests/LensAudioTests.cpp)
    target_link_libraries(aquorbium_lens_audio_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_lens_audio_tests)
    add_test(NAME aquorbium_lens_audio_tests COMMAND aquorbium_lens_audio_tests)
    set_tests_properties(aquorbium_lens_audio_tests PROPERTIES TIMEOUT 30)

    add_executable(aquorbium_control_response_tests tests/ControlResponseTests.cpp)
    target_link_libraries(aquorbium_control_response_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_control_response_tests)
    add_test(NAME aquorbium_control_response_tests COMMAND aquorbium_control_response_tests)
    # The exhaustive response matrix now includes the sixteen module-bank
    # controls and is intentionally serial; 60 s had become a harness limit,
    # not an audio assertion. Real-time cost remains covered separately.
    set_tests_properties(aquorbium_control_response_tests PROPERTIES TIMEOUT 120)

    add_executable(aquorbium_diagnostic_capture_tests tests/DiagnosticAudioCaptureTests.cpp)
    target_include_directories(aquorbium_diagnostic_capture_tests PRIVATE src)
    target_link_libraries(aquorbium_diagnostic_capture_tests PRIVATE aquorbium_core Threads::Threads)
    target_compile_features(aquorbium_diagnostic_capture_tests PRIVATE cxx_std_20)
    aquorbium_configure_own_target(aquorbium_diagnostic_capture_tests)
    add_test(NAME aquorbium_diagnostic_capture_tests COMMAND aquorbium_diagnostic_capture_tests)
    set_tests_properties(aquorbium_diagnostic_capture_tests PROPERTIES TIMEOUT 15)

    # the session's world drawn from its seed (2026-10-24)
    add_executable(aquorbium_session_genesis_tests tests/SessionGenesisTests.cpp)
    target_include_directories(aquorbium_session_genesis_tests PRIVATE src)
    target_link_libraries(aquorbium_session_genesis_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_session_genesis_tests)
    add_test(NAME aquorbium_session_genesis_tests COMMAND aquorbium_session_genesis_tests)
    set_tests_properties(aquorbium_session_genesis_tests PROPERTIES TIMEOUT 400)

    # the opening seed and the VARIA (2026-10-24)
    add_executable(aquorbium_opening_seed_tests tests/OpeningSeedTests.cpp)
    target_include_directories(aquorbium_opening_seed_tests PRIVATE src)
    target_compile_definitions(aquorbium_opening_seed_tests PRIVATE AQUORBIUM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_features(aquorbium_opening_seed_tests PRIVATE cxx_std_20)
    aquorbium_configure_own_target(aquorbium_opening_seed_tests)
    add_test(NAME aquorbium_opening_seed_tests COMMAND aquorbium_opening_seed_tests)
    set_tests_properties(aquorbium_opening_seed_tests PROPERTIES TIMEOUT 60)

    # the LUPA, MASTER CRIATIVO and CÉREBRO controls (2026-10-24)
    add_executable(aquorbium_card_controls_tests tests/CardControlsTests.cpp)
    target_include_directories(aquorbium_card_controls_tests PRIVATE src)
    target_link_libraries(aquorbium_card_controls_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_card_controls_tests)
    add_test(NAME aquorbium_card_controls_tests COMMAND aquorbium_card_controls_tests)
    set_tests_properties(aquorbium_card_controls_tests PROPERTIES TIMEOUT 120)

    # the emotion of the form: arc, dynamics, breath, suspensions (2026-10-24)
    add_executable(aquorbium_form_emotion_tests tests/FormEmotionTests.cpp)
    target_include_directories(aquorbium_form_emotion_tests PRIVATE src)
    target_link_libraries(aquorbium_form_emotion_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_form_emotion_tests)
    add_test(NAME aquorbium_form_emotion_tests COMMAND aquorbium_form_emotion_tests)
    set_tests_properties(aquorbium_form_emotion_tests PROPERTIES TIMEOUT 300)

    # first-order Ambisonics, AmbiX (2026-10-23)
    add_executable(aquorbium_ambisonic_tests tests/AmbisonicTests.cpp)
    target_include_directories(aquorbium_ambisonic_tests PRIVATE src)
    target_link_libraries(aquorbium_ambisonic_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ambisonic_tests)
    add_test(NAME aquorbium_ambisonic_tests COMMAND aquorbium_ambisonic_tests)
    set_tests_properties(aquorbium_ambisonic_tests PROPERTIES TIMEOUT 60)

    # aliasing of the non-linear stages, measured on a prime FFT grid (2026-10-23)
    add_executable(aquorbium_aliasing_measure_tests tests/AliasingMeasureTests.cpp)
    target_include_directories(aquorbium_aliasing_measure_tests PRIVATE src)
    target_link_libraries(aquorbium_aliasing_measure_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_aliasing_measure_tests)
    add_test(NAME aquorbium_aliasing_measure_tests COMMAND aquorbium_aliasing_measure_tests)
    set_tests_properties(aquorbium_aliasing_measure_tests PROPERTIES TIMEOUT 60)

    add_executable(aquorbium_take_analysis_tests tests/TakeAnalysisTests.cpp)
    target_include_directories(aquorbium_take_analysis_tests PRIVATE src)
    target_link_libraries(aquorbium_take_analysis_tests PRIVATE aquorbium_core Threads::Threads)
    aquorbium_configure_own_target(aquorbium_take_analysis_tests)
    add_test(NAME aquorbium_take_analysis_tests COMMAND aquorbium_take_analysis_tests)
    set_tests_properties(aquorbium_take_analysis_tests PROPERTIES TIMEOUT 30)

    add_executable(aquorbium_realtime_tests tests/RealtimeSafetyTests.cpp)
    target_link_libraries(aquorbium_realtime_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_realtime_tests)
    add_test(NAME aquorbium_realtime_tests COMMAND aquorbium_realtime_tests)

    add_executable(aquorbium_mpsc_queue_tests tests/FixedMpscQueueTests.cpp)
    target_link_libraries(aquorbium_mpsc_queue_tests PRIVATE aquorbium_core Threads::Threads)
    aquorbium_configure_own_target(aquorbium_mpsc_queue_tests)
    add_test(NAME aquorbium_mpsc_queue_tests COMMAND aquorbium_mpsc_queue_tests)

    add_executable(aquorbium_snapshot_tests tests/FixedLatestSnapshotTests.cpp)
    target_link_libraries(aquorbium_snapshot_tests PRIVATE aquorbium_core Threads::Threads)
    aquorbium_configure_own_target(aquorbium_snapshot_tests)
    add_test(NAME aquorbium_snapshot_tests COMMAND aquorbium_snapshot_tests)
    set_tests_properties(aquorbium_snapshot_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_numeric_safety_tests tests/NumericSafetyTests.cpp)
    target_link_libraries(aquorbium_numeric_safety_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_numeric_safety_tests)
    add_test(NAME aquorbium_numeric_safety_tests COMMAND aquorbium_numeric_safety_tests)

    add_executable(aquorbium_output_stage_tests tests/OutputStageTests.cpp)
    target_link_libraries(aquorbium_output_stage_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_output_stage_tests)
    add_test(NAME aquorbium_output_stage_tests COMMAND aquorbium_output_stage_tests)

    add_executable(aquorbium_loudness_tests tests/LoudnessMeterTests.cpp)
    target_link_libraries(aquorbium_loudness_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_loudness_tests)
    add_test(NAME aquorbium_loudness_tests COMMAND aquorbium_loudness_tests)
    set_tests_properties(aquorbium_loudness_tests PROPERTIES TIMEOUT 30)

    add_executable(aquorbium_open_interaction_tests
        tests/OpenInteractionModulesTests.cpp)
    target_link_libraries(aquorbium_open_interaction_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_open_interaction_tests)
    add_test(NAME aquorbium_open_interaction_tests
        COMMAND aquorbium_open_interaction_tests)
    set_tests_properties(aquorbium_open_interaction_tests PROPERTIES TIMEOUT 30)

    add_executable(aquorbium_core_refinement_tests
        tests/CoreRefinementModulesTests.cpp)
    target_link_libraries(aquorbium_core_refinement_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_core_refinement_tests)
    add_test(NAME aquorbium_core_refinement_tests
        COMMAND aquorbium_core_refinement_tests)
    set_tests_properties(aquorbium_core_refinement_tests PROPERTIES TIMEOUT 30)

    add_executable(aquorbium_autonomous_energy_tests tests/AutonomousEnergyTests.cpp)
    target_link_libraries(aquorbium_autonomous_energy_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_autonomous_energy_tests)
    add_test(NAME aquorbium_autonomous_energy_tests COMMAND aquorbium_autonomous_energy_tests)

    add_executable(aquorbium_geometric_sequencer_tests tests/GeometricSequencerTests.cpp)
    target_link_libraries(aquorbium_geometric_sequencer_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_geometric_sequencer_tests)
    add_test(NAME aquorbium_geometric_sequencer_tests COMMAND aquorbium_geometric_sequencer_tests)

    add_executable(aquorbium_compositional_variation_director_tests tests/CompositionalVariationDirectorTests.cpp)
    target_link_libraries(aquorbium_compositional_variation_director_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_compositional_variation_director_tests)
    add_test(NAME aquorbium_compositional_variation_director_tests COMMAND aquorbium_compositional_variation_director_tests)

    add_executable(aquorbium_synth_patch_codec_tests tests/SynthPatchCodecTests.cpp)
    target_link_libraries(aquorbium_synth_patch_codec_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_synth_patch_codec_tests)
    add_test(NAME aquorbium_synth_patch_codec_tests COMMAND aquorbium_synth_patch_codec_tests)

    add_executable(aquorbium_synth_proposal_tests tests/SynthProposalTests.cpp)
    target_link_libraries(aquorbium_synth_proposal_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_synth_proposal_tests)
    add_test(NAME aquorbium_synth_proposal_tests COMMAND aquorbium_synth_proposal_tests)

    add_executable(aquorbium_variation_route_tests tests/VariationRouteTests.cpp)
    target_link_libraries(aquorbium_variation_route_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_variation_route_tests)
    add_test(NAME aquorbium_variation_route_tests COMMAND aquorbium_variation_route_tests)

    add_executable(aquorbium_ebu_true_peak_tests tests/EbuTruePeakTests.cpp)
    target_link_libraries(aquorbium_ebu_true_peak_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ebu_true_peak_tests)
    add_test(NAME aquorbium_ebu_true_peak_tests COMMAND aquorbium_ebu_true_peak_tests)

    add_executable(aquorbium_ebu_loudness_tests tests/EbuLoudnessTests.cpp)
    target_link_libraries(aquorbium_ebu_loudness_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ebu_loudness_tests)
    add_test(NAME aquorbium_ebu_loudness_tests COMMAND aquorbium_ebu_loudness_tests)

    add_executable(aquorbium_record_tap_tests tests/RecordTapTests.cpp)
    target_link_libraries(aquorbium_record_tap_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_record_tap_tests)
    add_test(NAME aquorbium_record_tap_tests COMMAND aquorbium_record_tap_tests)

    add_executable(aquorbium_gain_budget_tests tests/GainBudgetTests.cpp)
    target_link_libraries(aquorbium_gain_budget_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_gain_budget_tests)
    add_test(NAME aquorbium_gain_budget_tests COMMAND aquorbium_gain_budget_tests)
    set_tests_properties(aquorbium_gain_budget_tests PROPERTIES TIMEOUT 300)

    add_executable(aquorbium_midi_binaural_tests tests/MidiBinauralTests.cpp)
    target_link_libraries(aquorbium_midi_binaural_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_midi_binaural_tests)
    add_test(NAME aquorbium_midi_binaural_tests COMMAND aquorbium_midi_binaural_tests)

    add_executable(aquorbium_golden_render_tests tests/GoldenRenderTests.cpp)
    target_link_libraries(aquorbium_golden_render_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_golden_render_tests)
    add_test(NAME aquorbium_golden_render_tests COMMAND aquorbium_golden_render_tests)

    # 30 minutes of audio, ~2 min of CPU: ctest -LE slow skips it
    add_executable(aquorbium_engine_soak_tests tests/EngineSoakTests.cpp)
    target_link_libraries(aquorbium_engine_soak_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_engine_soak_tests)
    add_test(NAME aquorbium_engine_soak_tests COMMAND aquorbium_engine_soak_tests)
    set_tests_properties(aquorbium_engine_soak_tests PROPERTIES TIMEOUT 1800 LABELS slow)

    # every source carries its licence header (2026-10-22)
    add_test(NAME aquorbium_license_headers
        COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/LicenseHeaders.cmake)

    add_executable(aquorbium_synth_voice_tests tests/SynthVoiceTests.cpp)
    target_link_libraries(aquorbium_synth_voice_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_synth_voice_tests)
    add_test(NAME aquorbium_synth_voice_tests COMMAND aquorbium_synth_voice_tests)

    add_executable(aquorbium_binaural_source_tests tests/BinauralSourceTests.cpp)
    target_link_libraries(aquorbium_binaural_source_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_binaural_source_tests)
    add_test(NAME aquorbium_binaural_source_tests COMMAND aquorbium_binaural_source_tests)

    add_executable(aquorbium_bubble_field_tests tests/BubbleFieldTests.cpp)
    target_link_libraries(aquorbium_bubble_field_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_bubble_field_tests)
    add_test(NAME aquorbium_bubble_field_tests COMMAND aquorbium_bubble_field_tests)

    add_executable(aquorbium_coda_exchange_tests tests/CodaExchangeTests.cpp)
    target_link_libraries(aquorbium_coda_exchange_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_coda_exchange_tests)
    add_test(NAME aquorbium_coda_exchange_tests COMMAND aquorbium_coda_exchange_tests)

    add_executable(aquorbium_room_coupling_tests tests/RoomCouplingTests.cpp)
    target_link_libraries(aquorbium_room_coupling_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_room_coupling_tests)
    add_test(NAME aquorbium_room_coupling_tests COMMAND aquorbium_room_coupling_tests)

    add_executable(aquorbium_habitat_health_tests tests/HabitatHealthTests.cpp)
    target_link_libraries(aquorbium_habitat_health_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_habitat_health_tests)
    add_test(NAME aquorbium_habitat_health_tests COMMAND aquorbium_habitat_health_tests)

    add_executable(aquorbium_sound_channel_tests tests/SoundChannelTests.cpp)
    target_link_libraries(aquorbium_sound_channel_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_sound_channel_tests)
    add_test(NAME aquorbium_sound_channel_tests COMMAND aquorbium_sound_channel_tests)

    add_executable(aquorbium_life_cycle_tests tests/LifeCycleTests.cpp)
    target_link_libraries(aquorbium_life_cycle_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_life_cycle_tests)
    add_test(NAME aquorbium_life_cycle_tests COMMAND aquorbium_life_cycle_tests)

    add_executable(aquorbium_population_cycles_tests tests/PopulationCyclesTests.cpp)
    target_link_libraries(aquorbium_population_cycles_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_population_cycles_tests)
    add_test(NAME aquorbium_population_cycles_tests COMMAND aquorbium_population_cycles_tests)

    add_executable(aquorbium_spectral_consonance_tests tests/SpectralConsonanceTests.cpp)
    target_link_libraries(aquorbium_spectral_consonance_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_spectral_consonance_tests)
    add_test(NAME aquorbium_spectral_consonance_tests COMMAND aquorbium_spectral_consonance_tests)

    add_executable(aquorbium_song_form_tests tests/SongFormTests.cpp)
    target_link_libraries(aquorbium_song_form_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_song_form_tests)
    add_test(NAME aquorbium_song_form_tests COMMAND aquorbium_song_form_tests)

    add_executable(aquorbium_chorus_phase_tests tests/ChorusPhaseTests.cpp)
    target_link_libraries(aquorbium_chorus_phase_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_chorus_phase_tests)
    add_test(NAME aquorbium_chorus_phase_tests COMMAND aquorbium_chorus_phase_tests)

    add_executable(aquorbium_timbre_body_tests tests/TimbreBodyTests.cpp)
    target_link_libraries(aquorbium_timbre_body_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_timbre_body_tests)
    add_test(NAME aquorbium_timbre_body_tests COMMAND aquorbium_timbre_body_tests)

    add_executable(aquorbium_ecological_music_director_tests tests/EcologicalMusicDirectorTests.cpp)
    target_link_libraries(aquorbium_ecological_music_director_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ecological_music_director_tests)
    add_test(NAME aquorbium_ecological_music_director_tests COMMAND aquorbium_ecological_music_director_tests)

    add_executable(aquorbium_modal_resonator_tests tests/ModalResonatorTests.cpp)
    target_link_libraries(aquorbium_modal_resonator_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_modal_resonator_tests)
    add_test(NAME aquorbium_modal_resonator_tests COMMAND aquorbium_modal_resonator_tests)

    add_executable(aquorbium_granular_striker_tests tests/GranularStrikerTests.cpp)
    target_link_libraries(aquorbium_granular_striker_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_granular_striker_tests)
    add_test(NAME aquorbium_granular_striker_tests COMMAND aquorbium_granular_striker_tests)

    add_executable(aquorbium_physical_string_tests tests/PhysicalStringTests.cpp)
    target_link_libraries(aquorbium_physical_string_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_physical_string_tests)
    add_test(NAME aquorbium_physical_string_tests COMMAND aquorbium_physical_string_tests)

    add_executable(aquorbium_space_tests tests/AquariumSpaceTests.cpp)
    target_link_libraries(aquorbium_space_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_space_tests)
    add_test(NAME aquorbium_space_tests COMMAND aquorbium_space_tests)

    add_executable(aquorbium_multitap_space_tests tests/MultiTapSpaceTests.cpp)
    target_link_libraries(aquorbium_multitap_space_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_multitap_space_tests)
    add_test(NAME aquorbium_multitap_space_tests COMMAND aquorbium_multitap_space_tests)

    add_executable(aquorbium_window_layout_tests tests/WindowLayoutPolicyTests.cpp)
    target_include_directories(aquorbium_window_layout_tests PRIVATE src)
    target_compile_features(aquorbium_window_layout_tests PRIVATE cxx_std_20)
    aquorbium_configure_own_target(aquorbium_window_layout_tests)
    add_test(NAME aquorbium_window_layout_tests COMMAND aquorbium_window_layout_tests)

    add_executable(aquorbium_organism_voice_tests tests/OrganismVoiceEngineTests.cpp)
    target_link_libraries(aquorbium_organism_voice_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_organism_voice_tests)
    add_test(NAME aquorbium_organism_voice_tests COMMAND aquorbium_organism_voice_tests)

    add_executable(aquorbium_pending_modules_tests tests/PendingModulesTests.cpp)
    target_link_libraries(aquorbium_pending_modules_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_pending_modules_tests)
    add_test(NAME aquorbium_pending_modules_tests COMMAND aquorbium_pending_modules_tests)
    set_tests_properties(aquorbium_pending_modules_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_lorenz_capture_tests tests/LorenzCaptureTests.cpp)
    target_link_libraries(aquorbium_lorenz_capture_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_lorenz_capture_tests)
    add_test(NAME aquorbium_lorenz_capture_tests COMMAND aquorbium_lorenz_capture_tests)
    set_tests_properties(aquorbium_lorenz_capture_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_markov_tail_tests tests/MarkovTailTests.cpp)
    target_link_libraries(aquorbium_markov_tail_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_markov_tail_tests)
    add_test(NAME aquorbium_markov_tail_tests COMMAND aquorbium_markov_tail_tests)
    set_tests_properties(aquorbium_markov_tail_tests PROPERTIES TIMEOUT 15)

    add_executable(aquorbium_event_clock_tests tests/EventClockTests.cpp)
    target_link_libraries(aquorbium_event_clock_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_event_clock_tests)
    add_test(NAME aquorbium_event_clock_tests COMMAND aquorbium_event_clock_tests)
    set_tests_properties(aquorbium_event_clock_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_sequencer_memory_tests
        tests/SequencerMemoryTests.cpp)
    target_link_libraries(aquorbium_sequencer_memory_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_sequencer_memory_tests)
    add_test(NAME aquorbium_sequencer_memory_tests
        COMMAND aquorbium_sequencer_memory_tests)
    set_tests_properties(aquorbium_sequencer_memory_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_sequencer_navigation_tests
        tests/SequencerNavigationTests.cpp)
    target_link_libraries(aquorbium_sequencer_navigation_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_sequencer_navigation_tests)
    add_test(NAME aquorbium_sequencer_navigation_tests
        COMMAND aquorbium_sequencer_navigation_tests)
    set_tests_properties(aquorbium_sequencer_navigation_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_pattern_evolution_tests
        tests/PatternEvolutionTests.cpp)
    target_link_libraries(aquorbium_pattern_evolution_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_pattern_evolution_tests)
    add_test(NAME aquorbium_pattern_evolution_tests
        COMMAND aquorbium_pattern_evolution_tests)
    set_tests_properties(aquorbium_pattern_evolution_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_decision_permutation_tests
        tests/DecisionPermutationTests.cpp)
    target_link_libraries(aquorbium_decision_permutation_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_decision_permutation_tests)
    add_test(NAME aquorbium_decision_permutation_tests
        COMMAND aquorbium_decision_permutation_tests)
    set_tests_properties(aquorbium_decision_permutation_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_geometry_evolution_tests
        tests/GeometryEvolutionTests.cpp)
    target_link_libraries(aquorbium_geometry_evolution_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_geometry_evolution_tests)
    add_test(NAME aquorbium_geometry_evolution_tests
        COMMAND aquorbium_geometry_evolution_tests)
    set_tests_properties(aquorbium_geometry_evolution_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_percussion_morph_tests
        tests/PercussionMorphTests.cpp)
    target_link_libraries(aquorbium_percussion_morph_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_percussion_morph_tests)
    add_test(NAME aquorbium_percussion_morph_tests
        COMMAND aquorbium_percussion_morph_tests)
    set_tests_properties(aquorbium_percussion_morph_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_multirate_generator_tests
        tests/MultirateGeneratorTests.cpp)
    target_link_libraries(aquorbium_multirate_generator_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_multirate_generator_tests)
    add_test(NAME aquorbium_multirate_generator_tests
        COMMAND aquorbium_multirate_generator_tests)
    set_tests_properties(aquorbium_multirate_generator_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_probabilistic_memory_tests
        tests/ProbabilisticMemoryTests.cpp)
    target_link_libraries(aquorbium_probabilistic_memory_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_probabilistic_memory_tests)
    add_test(NAME aquorbium_probabilistic_memory_tests
        COMMAND aquorbium_probabilistic_memory_tests)
    set_tests_properties(aquorbium_probabilistic_memory_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_structural_control_tests
        tests/StructuralControlTests.cpp)
    target_link_libraries(aquorbium_structural_control_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_structural_control_tests)
    add_test(NAME aquorbium_structural_control_tests
        COMMAND aquorbium_structural_control_tests)
    set_tests_properties(aquorbium_structural_control_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_school_motion_tests
        tests/SchoolMotionTests.cpp)
    target_link_libraries(aquorbium_school_motion_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_school_motion_tests)
    add_test(NAME aquorbium_school_motion_tests
        COMMAND aquorbium_school_motion_tests)
    set_tests_properties(aquorbium_school_motion_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_motion_gesture_tests
        tests/MotionGestureTests.cpp)
    target_link_libraries(aquorbium_motion_gesture_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_motion_gesture_tests)
    add_test(NAME aquorbium_motion_gesture_tests
        COMMAND aquorbium_motion_gesture_tests)
    set_tests_properties(aquorbium_motion_gesture_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_spectral_environment_tests
        tests/SpectralEnvironmentTests.cpp)
    target_link_libraries(aquorbium_spectral_environment_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_spectral_environment_tests)
    add_test(NAME aquorbium_spectral_environment_tests
        COMMAND aquorbium_spectral_environment_tests)
    set_tests_properties(aquorbium_spectral_environment_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_philosophy_mix_tests
        tests/PhilosophyMixTests.cpp)
    target_link_libraries(aquorbium_philosophy_mix_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_philosophy_mix_tests)
    add_test(NAME aquorbium_philosophy_mix_tests
        COMMAND aquorbium_philosophy_mix_tests)
    set_tests_properties(aquorbium_philosophy_mix_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_behaviour_memory_tests
        tests/BehaviourMemoryTests.cpp)
    target_link_libraries(aquorbium_behaviour_memory_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_behaviour_memory_tests)
    add_test(NAME aquorbium_behaviour_memory_tests
        COMMAND aquorbium_behaviour_memory_tests)
    set_tests_properties(aquorbium_behaviour_memory_tests PROPERTIES TIMEOUT 10)

    add_executable(aquorbium_integrated_module_bank_tests
        tests/IntegratedModuleBankTests.cpp)
    target_link_libraries(aquorbium_integrated_module_bank_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_integrated_module_bank_tests)
    add_test(NAME aquorbium_integrated_module_bank_tests
        COMMAND aquorbium_integrated_module_bank_tests)
    set_tests_properties(aquorbium_integrated_module_bank_tests PROPERTIES TIMEOUT 20)

    add_executable(aquorbium_emergent_module_cohort_tests
        tests/EmergentModuleCohortTests.cpp)
    target_link_libraries(aquorbium_emergent_module_cohort_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_emergent_module_cohort_tests)
    add_test(NAME aquorbium_emergent_module_cohort_tests
        COMMAND aquorbium_emergent_module_cohort_tests)
    set_tests_properties(aquorbium_emergent_module_cohort_tests PROPERTIES TIMEOUT 20)

    add_executable(aquorbium_advanced_voice_ecology_tests
        tests/AdvancedVoiceEcologyTests.cpp)
    target_link_libraries(aquorbium_advanced_voice_ecology_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_advanced_voice_ecology_tests)
    add_test(NAME aquorbium_advanced_voice_ecology_tests
        COMMAND aquorbium_advanced_voice_ecology_tests)
    set_tests_properties(aquorbium_advanced_voice_ecology_tests PROPERTIES TIMEOUT 20)

    add_executable(aquorbium_generative_interaction_tests
        tests/GenerativeInteractionTests.cpp)
    target_link_libraries(aquorbium_generative_interaction_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_generative_interaction_tests)
    add_test(NAME aquorbium_generative_interaction_tests
        COMMAND aquorbium_generative_interaction_tests)
    set_tests_properties(aquorbium_generative_interaction_tests PROPERTIES TIMEOUT 20)

    add_executable(aquorbium_master_chain_tests tests/MasterChainTests.cpp)
    target_link_libraries(aquorbium_master_chain_tests PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_master_chain_tests)
    add_test(NAME aquorbium_master_chain_tests COMMAND aquorbium_master_chain_tests)
    set_tests_properties(aquorbium_master_chain_tests PROPERTIES TIMEOUT 20)
endif()

if (AQUORBIUM_BUILD_TOOLS)
    add_executable(aquorbium_ab_render tools/AquorbiumABRender.cpp)
    target_link_libraries(aquorbium_ab_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ab_render)

    add_executable(aquorbium_core_benchmark tools/AquorbiumCoreBenchmark.cpp)
    target_link_libraries(aquorbium_core_benchmark PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_core_benchmark)

    add_executable(aquorbium_granular_striker_benchmark
        tools/GranularStrikerBenchmark.cpp)
    target_link_libraries(aquorbium_granular_striker_benchmark PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_granular_striker_benchmark)

    add_executable(aquorbium_granular_striker_render
        tools/GranularStrikerRender.cpp)
    target_link_libraries(aquorbium_granular_striker_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_granular_striker_render)

    add_executable(aquorbium_physical_string_benchmark
        tools/PhysicalStringBenchmark.cpp)
    target_link_libraries(aquorbium_physical_string_benchmark PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_physical_string_benchmark)

    add_executable(aquorbium_physical_string_render
        tools/PhysicalStringRender.cpp)
    target_link_libraries(aquorbium_physical_string_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_physical_string_render)

    add_executable(aquorbium_physical_bodies_blind_render
        tools/PhysicalBodiesBlindRender.cpp)
    target_link_libraries(aquorbium_physical_bodies_blind_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_physical_bodies_blind_render)

    add_executable(aquorbium_autonomous_render tools/AquorbiumAutonomousRender.cpp)
    target_link_libraries(aquorbium_autonomous_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_autonomous_render)

    add_executable(aquorbium_multitap_space_render tools/MultiTapSpaceRender.cpp)
    target_link_libraries(aquorbium_multitap_space_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_multitap_space_render)

    add_executable(aquorbium_multitap_space_benchmark
        tools/MultiTapSpaceBenchmark.cpp)
    target_link_libraries(aquorbium_multitap_space_benchmark PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_multitap_space_benchmark)

    add_executable(aquorbium_lorenz_capture_benchmark
        tools/LorenzCaptureBenchmark.cpp)
    target_link_libraries(aquorbium_lorenz_capture_benchmark PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_lorenz_capture_benchmark)

    add_executable(aquorbium_spectral_environment_render
        tools/SpectralEnvironmentRender.cpp)
    target_link_libraries(aquorbium_spectral_environment_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_spectral_environment_render)

    add_executable(aquorbium_synthetic_tail_render
        tools/SyntheticTailRender.cpp)
    target_link_libraries(aquorbium_synthetic_tail_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_synthetic_tail_render)

    add_executable(aquorbium_form_memory_render
        tools/FormMemoryRender.cpp)
    target_link_libraries(aquorbium_form_memory_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_form_memory_render)

    add_executable(aquorbium_compositional_variation_render
        tools/CompositionalVariationRender.cpp)
    target_link_libraries(aquorbium_compositional_variation_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_compositional_variation_render)

    add_executable(aquorbium_ecological_music_render
        tools/EcologicalMusicRender.cpp)
    target_link_libraries(aquorbium_ecological_music_render PRIVATE aquorbium_core)
    aquorbium_configure_own_target(aquorbium_ecological_music_render)
endif()

if (AQUORBIUM_BUILD_APP)
    if (AQUORBIUM_JUCE_PATH AND EXISTS "${AQUORBIUM_JUCE_PATH}/CMakeLists.txt")
        add_subdirectory("${AQUORBIUM_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
        add_subdirectory(src/app)
        if (AQUORBIUM_BUILD_TESTS)
            add_executable(aquorbium_ecology_parameter_tests tests/EcologyParameterStateTests.cpp
                tests/PerformanceSceneFileTests.cpp tests/PerformanceScenePlaybackTests.cpp
                tests/PerformancePresetBankTests.cpp tests/EngineStressTests.cpp
                tests/AutomationClickTests.cpp tests/BypassSweepTests.cpp)
            target_include_directories(aquorbium_ecology_parameter_tests PRIVATE src)
            target_link_libraries(aquorbium_ecology_parameter_tests PRIVATE
                aquorbium_core
                juce::juce_data_structures
                juce::juce_recommended_config_flags)
            target_compile_features(aquorbium_ecology_parameter_tests PRIVATE cxx_std_20)
            target_compile_definitions(aquorbium_ecology_parameter_tests PRIVATE
                JUCE_USE_CURL=0
                JUCE_WEB_BROWSER=0)
            aquorbium_configure_own_target(aquorbium_ecology_parameter_tests)
            add_test(NAME aquorbium_ecology_parameter_tests COMMAND aquorbium_ecology_parameter_tests)
            set_tests_properties(aquorbium_ecology_parameter_tests PROPERTIES TIMEOUT 15)
            add_test(NAME aquorbium_scene_playback_tests
                COMMAND aquorbium_ecology_parameter_tests --scene-playback)
            set_tests_properties(aquorbium_scene_playback_tests PROPERTIES TIMEOUT 15)
            add_test(NAME aquorbium_preset_bank_tests
                COMMAND aquorbium_ecology_parameter_tests --preset-bank)
            set_tests_properties(aquorbium_preset_bank_tests PROPERTIES TIMEOUT 10)
            # the engine under stress (2026-10-22): worst case, stimuli, blocks, rates
            add_test(NAME aquorbium_engine_stress_tests
                COMMAND aquorbium_ecology_parameter_tests --engine-stress)
            set_tests_properties(aquorbium_engine_stress_tests PROPERTIES TIMEOUT 900 LABELS slow)
            # fast automation without clicks, mono compatibility (2026-10-22)
            add_test(NAME aquorbium_automation_click_tests
                COMMAND aquorbium_ecology_parameter_tests --automation)
            set_tests_properties(aquorbium_automation_click_tests PROPERTIES TIMEOUT 600)
            add_test(NAME aquorbium_bypass_sweep_tests
                COMMAND aquorbium_ecology_parameter_tests --bypass-sweep)
            set_tests_properties(aquorbium_bypass_sweep_tests PROPERTIES TIMEOUT 600)  # ~2 min: ctest -LE slow skips it
        endif()
    else()
        message(WARNING "Set AQUORBIUM_JUCE_PATH to build the standalone app")
    endif()
endif()
