// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The REC's taps (2026-10-22): asking for a tap never changes the output;
// "pre-safety" is the creative MASTER's output, before the trim and the
// safety limiter; "pre-master-criativo" is the mix before the creative
// MASTER - equal to pre-safety when the MASTER is bypassed, different when
// it acts.
#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
using Tap = aquorbium::BiomaEngine::RecordTap;

struct Run
{
    std::vector<float> out, tap;
};

Run run(Tap tap, float creativeMaster, float trimDb)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(5u);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    engine->setCreativeMasterInfluence(creativeMaster);
    engine->setOutputStageParameters({ trimDb, -1.0f, 150.0f, false });
    engine->noteOn(48, 0.8f);
    std::vector<float> left(256), right(256), tapLeft(256), tapRight(256);
    Run r;
    for (int block = 0; block < 48000 * 6 / 256; ++block)
    {
        if (tap != Tap::postSafety)
            engine->setRecordTap(tap, tapLeft.data(), tapRight.data(), tapLeft.size());
        engine->render(left.data(), right.data(), left.size());
        for (std::size_t i = 0; i < left.size(); ++i)
        {
            r.out.push_back(left[i]);
            r.tap.push_back(tapLeft[i]);
        }
    }
    return r;
}

double rms(const std::vector<float>& x, std::size_t from)
{
    double s = 0.0;
    for (std::size_t i = from; i < x.size(); ++i)
        s += static_cast<double>(x[i]) * x[i];
    return std::sqrt(s / static_cast<double>(x.size() - from));
}
}

int main()
{
    const auto plain = run(Tap::postSafety, 0.6f, -6.0f);
    const auto preSafety = run(Tap::preSafety, 0.6f, -6.0f);
    const auto preMaster = run(Tap::preMasterCreative, 0.6f, -6.0f);
    check(plain.out == preSafety.out && plain.out == preMaster.out, "a tap never changes the output, bit for bit");

    // pre-safety is the output before the -6 dB trim (and the limiter, idle
    // here): 6 dB louder, the same signal 5 ms earlier (the lookahead)
    // the output stage also removes DC (a 10 Hz high-pass) after the tap, so
    // the trim is read as the difference between two trims
    const auto from = std::size_t { 48000 };
    const auto unity = run(Tap::preSafety, 0.6f, 0.0f);
    const auto atUnity = 20.0 * std::log10(rms(unity.tap, from) / rms(unity.out, from));
    const auto atMinusSix = 20.0 * std::log10(rms(preSafety.tap, from) / rms(plain.out, from));
    std::cout << "pre-safety over the output: " << atUnity << " dB at 0 dB trim (the DC the output removes), "
              << atMinusSix << " dB at -6 dB trim\n";
    check(std::abs((atMinusSix - atUnity) - 6.0) < 0.1, "pre-safety is before the trim (6 dB more against a -6 dB trim)");

    // pre-master-criativo = pre-safety when the creative MASTER is bypassed;
    // different when it acts
    const auto bypassPreSafety = run(Tap::preSafety, 0.0f, 0.0f);
    const auto bypassPreMaster = run(Tap::preMasterCreative, 0.0f, 0.0f);
    double bypassDifference = 0.0, actingDifference = 0.0;
    for (std::size_t i = from; i < bypassPreSafety.tap.size(); ++i)
    {
        bypassDifference = std::max(bypassDifference, static_cast<double>(std::abs(bypassPreSafety.tap[i] - bypassPreMaster.tap[i])));
        actingDifference = std::max(actingDifference, static_cast<double>(std::abs(preSafety.tap[i] - preMaster.tap[i])));
    }
    std::cout << "pre-master vs pre-safety: bypassed " << bypassDifference << ", MASTER acting " << actingDifference << '\n';
    check(bypassDifference < 1.0e-6, "the creative MASTER bypassed: pre-master-criativo = pre-safety");
    check(actingDifference > 1.0e-3, "the creative MASTER acting: pre-master-criativo differs");

    // the MIDI voices carry no DC into the mix (2026-10-22: the Pulsar's
    // train left +0.0055 before the output stage removed it)
    {
        auto engine = std::make_unique<aquorbium::BiomaEngine>();
        engine->prepare(48000.0);
        engine->setSeed(5u);
        engine->setEcosystemBlendImmediately(1.0f);
        engine->setAutonomousEnabled(false);
        engine->noteOn(48, 0.8f);
        std::vector<float> left(256), right(256), tapLeft(256), tapRight(256);
        double sum = 0.0, squares = 0.0;
        long count = 0;
        for (int block = 0; block < 48000 * 10 / 256; ++block)
        {
            engine->setRecordTap(Tap::preMasterCreative, tapLeft.data(), tapRight.data(), tapLeft.size());
            engine->render(left.data(), right.data(), left.size());
            if (block > 48000 * 2 / 256)
                for (const auto x : tapLeft)
                {
                    sum += x;
                    squares += static_cast<double>(x) * x;
                    ++count;
                }
        }
        const auto mean = sum / static_cast<double>(count), rmsValue = std::sqrt(squares / static_cast<double>(count));
        std::cout << "MIDI note, before the creative MASTER: mean " << mean << ", RMS " << rmsValue << '\n';
        check(std::abs(mean) < 1.0e-4 && rmsValue > 1.0e-3, "a MIDI note leaves no DC in the mix");
    }

    // the stems (2026-10-22): five parts whose sum is the pre-master mix
    {
        auto engine = std::make_unique<aquorbium::BiomaEngine>();
        engine->prepare(48000.0);
        engine->setSeed(9u);
        engine->setEcosystemBlendImmediately(1.0f);
        engine->setAutonomousEnabled(true);
        engine->noteOn(48, 0.8f);
        aquorbium::IntegratedModuleBank::Controls bank {};
        bank.fill(0.5f);
        bank[aquorbium::IntegratedModuleBank::mix] = 0.8f;
        engine->setIntegratedModuleBankControls(bank);
        engine->setSpectralEnvironmentPerformance({ 0.8f, 0.6f, 0.5f, 0.5f, 0.45f, 0.7f });
        engine->setSyntheticTailPerformance({ 0.7f, 0.5f, 0.5f, 0.0f });
        engine->setMultiTapSpaceReturn(0.8f);
        engine->setEcologicalMusicPerformance({ 0.8f, 0.6f, 0.5f, 1.0f, 1.0f });  // boiling: bubbles
        constexpr std::size_t block = 256;
        std::vector<float> l(block), r(block), tapL(block), tapR(block);
        std::array<std::vector<float>, aquorbium::BiomaEngine::stemCount> stemL, stemR;
        for (auto& v : stemL) v.assign(block, 0.0f);
        for (auto& v : stemR) v.assign(block, 0.0f);
        std::array<double, aquorbium::BiomaEngine::stemCount> energy {};
        double worst = 0.0, total = 0.0;
        for (int b = 0; b < 48000 * 6 / static_cast<int>(block); ++b)
        {
            aquorbium::BiomaEngine::StemBuffers buffers;
            for (std::size_t s = 0; s < buffers.left.size(); ++s)
            {
                buffers.left[s] = stemL[s].data();
                buffers.right[s] = stemR[s].data();
            }
            buffers.capacity = block;
            engine->setStems(buffers);
            engine->setRecordTap(Tap::preMasterCreative, tapL.data(), tapR.data(), block);
            engine->render(l.data(), r.data(), block);
            if (b < 48000 * 2 / static_cast<int>(block))
                continue;
            for (std::size_t i = 0; i < block; ++i)
            {
                double sumL = 0.0, sumR = 0.0;
                for (std::size_t s = 0; s < stemL.size(); ++s)
                {
                    sumL += stemL[s][i];
                    sumR += stemR[s][i];
                    energy[s] += static_cast<double>(stemL[s][i]) * stemL[s][i];
                }
                worst = std::max({ worst, std::abs(sumL - tapL[i]), std::abs(sumR - tapR[i]) });
                total += static_cast<double>(tapL[i]) * tapL[i];
            }
        }
        std::cout << "stems: the sum against the pre-master mix, worst difference " << worst << "; energies";
        for (const auto e : energy)
            std::cout << ' ' << 10.0 * std::log10(std::max(e / total, 1.0e-30)) << " dB";
        std::cout << " (organisms, LAB, water, space, tail, re the mix)\n";
        check(worst < 1.0e-5, "the five stems sum to the pre-master mix");
        bool all = true;
        for (const auto e : energy)
            all = all && e / total > 1.0e-6;
        check(all, "every stem carries its part (above -60 dB of the mix)");
    }

    // the volume does not change the music (2026-10-22): the module bank's
    // feedback and the habitat read before the trim and the limiter
    {
        const auto mixAt = [](float trimDb) {
            auto engine = std::make_unique<aquorbium::BiomaEngine>();
            engine->prepare(48000.0);
            engine->setSeed(3u);
            engine->setEcosystemBlendImmediately(1.0f);
            engine->setAutonomousEnabled(true);
            aquorbium::IntegratedModuleBank::Controls bank {};
            bank.fill(0.6f);
            bank[aquorbium::IntegratedModuleBank::mix] = 0.8f;
            engine->setIntegratedModuleBankControls(bank);
            engine->setEcologicalMusicPerformance({ 0.7f, 0.6f, 0.5f, 0.5f, 1.0f });
            engine->setOutputStageParameters({ trimDb, -1.0f, 150.0f, false });
            std::vector<float> l(256), r(256), tl(256), tr(256), out;
            for (int b = 0; b < 48000 * 6 / 256; ++b)
            {
                engine->setRecordTap(Tap::preMasterCreative, tl.data(), tr.data(), tl.size());
                engine->render(l.data(), r.data(), l.size());
                out.insert(out.end(), tl.begin(), tl.end());
            }
            return out;
        };
        check(mixAt(0.0f) == mixAt(-18.0f) && mixAt(0.0f) == mixAt(6.0f),
              "the OUTPUT TRIM leaves the music unchanged, bit for bit (only its level)");
    }

    // the published feedback taps (2026-10-24): bounded envelopes; the
    // taps before the safety stage ignore the OUTPUT TRIM; the reduction
    // shows when the limiter works
    {
        const auto taps = [](float trimDb, bool pushed) {
            auto engine = std::make_unique<aquorbium::BiomaEngine>();
            engine->prepare(48000.0);
            engine->setSeed(5u);
            engine->setEcosystemBlendImmediately(1.0f);
            engine->setAutonomousEnabled(true);
            engine->setOutputStageParameters({ trimDb, pushed ? -6.0f : -1.0f, 150.0f, false });
            if (pushed)
            {
                // loud enough for the safety limiter to work
                aquorbium::IntegratedModuleBank::Controls c {};
                c.fill(0.6f);
                c[aquorbium::IntegratedModuleBank::mix] = 1.0f;
                engine->setIntegratedModuleBankControls(c);
            }
            engine->noteOn(48, 0.8f);
            std::vector<float> left(256), right(256);
            aquorbium::BiomaEngine::FeedbackTaps peak {};
            auto bounded = true;
            for (int block = 0; block < 48000 * 6 / 256; ++block)
            {
                engine->render(left.data(), right.data(), left.size());
                const auto& t = engine->feedbackTaps();
                for (const auto v : { t.preMaster, t.preSafety, t.safetyReduction, t.postSafety })
                    bounded = bounded && std::isfinite(v) && v >= 0.0f && v <= 1.0f;
                peak.preMaster = std::max(peak.preMaster, t.preMaster);
                peak.preSafety = std::max(peak.preSafety, t.preSafety);
                peak.safetyReduction = std::max(peak.safetyReduction, t.safetyReduction);
                peak.postSafety = std::max(peak.postSafety, t.postSafety);
            }
            return std::make_pair(peak, bounded);
        };
        const auto [quiet, quietBounded] = taps(-18.0f, false);
        const auto [loud, loudBounded] = taps(6.0f, false);
        const auto [pushed, pushedBounded] = taps(6.0f, true);
        std::cout << "feedback taps, trim -18 / +6 dB: pre-master " << quiet.preMaster << " / " << loud.preMaster
                  << ", pre-safety " << quiet.preSafety << " / " << loud.preSafety << ", post-safety "
                  << quiet.postSafety << " / " << loud.postSafety << ", limiter reduction " << quiet.safetyReduction
                  << " / " << loud.safetyReduction << '\n';
        std::cout << "  pushed (module bank, ceiling -6 dB): limiter reduction " << pushed.safetyReduction << '\n';
        check(quietBounded && loudBounded && pushedBounded, "feedback taps: finite and within 0..1");
        check(quiet.preMaster == loud.preMaster && quiet.preSafety == loud.preSafety,
              "feedback taps: before the safety stage the trim changes nothing");
        check(loud.postSafety > quiet.postSafety * 4.0f, "feedback taps: after it, the trim shows (+24 dB)");
        check(quiet.safetyReduction == 0.0f && pushed.safetyReduction > 0.0f,
              "feedback taps: the limiter's reduction shows only when it works");
    }

    if (failures == 0)
        std::cout << "record taps: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
