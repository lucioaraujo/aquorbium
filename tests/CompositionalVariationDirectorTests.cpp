// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The compositional variation director (2026-10-20 review): the tests it
// lacked. Determinism, the A -> A' -> B -> suspension order, segment lengths
// long enough for the form to breathe, a protagonist per segment (the
// contrasts the ear needs) and bounded values for any context.
#include "core/CompositionalVariationDirector.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using Director = aquorbium::CompositionalVariationDirector;
using Segment = Director::Segment;

int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

// the plan at the first event of each of the first `segments` segments
std::vector<Director::Plan> firstEvents(Director::Context context, int segments)
{
    Director director;
    director.reset();
    std::vector<Director::Plan> out;
    for (int event = 0; event < 400 && static_cast<int>(out.size()) < segments; ++event)
    {
        const auto plan = director.advance(context);
        if (plan.eventsInSegment == 0)
            out.push_back(plan);
    }
    return out;
}
}

int main()
{
    const Director::Context neutral {};

    // determinism: the same contexts give the same plans
    {
        Director a, b;
        a.reset();
        b.reset();
        bool same = true;
        for (int event = 0; event < 200; ++event)
        {
            Director::Context c;
            c.formState = static_cast<std::size_t>(event % 8);
            c.density = static_cast<float>(event % 7) / 6.0f;
            c.calm = static_cast<float>(event % 5) / 4.0f;
            c.danger = static_cast<float>(event % 3) / 2.0f;
            const auto pa = a.advance(c), pb = b.advance(c);
            same = same && pa.segment == pb.segment && pa.segmentLength == pb.segmentLength
                && pa.pulseWeight == pb.pulseWeight && pa.rhythmicDensity == pb.rhythmicDensity;
        }
        check(same, "determinism");
    }

    // order and length: A, A', B, suspension, again; each segment lasts long
    // enough (in collective boundaries) for the form to breathe
    {
        const auto plans = firstEvents(neutral, 8);
        check(plans.size() == 8, "eight segments reached");
        const Segment order[] { Segment::establish, Segment::variation, Segment::answer, Segment::suspension };
        int cycle = 0;
        for (std::size_t i = 0; i < plans.size(); ++i)
        {
            check(plans[i].segment == order[i % 4], "segment order");
            check(plans[i].segmentLength >= 3, "segment lasts at least 3 boundaries");
            check(plans[i].segmentLength <= Director::maximumSegmentLength, "segment length bounded");
            if (i < 4)
                cycle += plans[i].segmentLength;
        }
        check(cycle >= 16, "the whole cycle lasts at least 16 boundaries");
        check(plans[0].segmentLength >= 6 && plans[2].segmentLength >= 6, "A and B are the long segments");
    }

    // a protagonist per segment (neutral context)
    {
        const auto p = firstEvents(neutral, 4);
        const auto& a = p[0];
        const auto& v = p[1];
        const auto& b = p[2];
        const auto& s = p[3];
        check(a.tonalWeight > a.pulseWeight && a.tonalWeight > a.textureWeight, "A: the melodic bodies lead");
        check(v.pulseWeight > a.pulseWeight + 0.3f && v.pulseWeight > v.textureWeight
                  && v.rhythmicDensity > a.rhythmicDensity + 0.3f && v.accentWeight > a.accentWeight,
              "A': the pulse leads, denser and accented");
        check(b.textureWeight > b.pulseWeight + 0.4f && b.movementWeight > b.pulseWeight + 0.4f && b.retrogradeContour,
              "B: texture and movement lead, retrograde");
        check(s.rhythmicDensity < a.rhythmicDensity && s.pulseWeight < 0.4f && s.spaceWeight > a.spaceWeight + 0.3f,
              "suspension: sparse, almost no pulse, the space opens");
        // the weights really differ between segments (the ear can tell them apart)
        const auto spread = [&](float Director::Plan::*m) {
            float lo = 9.0f, hi = -9.0f;
            for (const auto& x : p)
            {
                lo = std::min(lo, x.*m);
                hi = std::max(hi, x.*m);
            }
            return hi - lo;
        };
        check(spread(&Director::Plan::pulseWeight) > 0.7f, "pulse varies across segments");
        check(spread(&Director::Plan::textureWeight) > 0.6f, "texture varies across segments");
        check(spread(&Director::Plan::rhythmicDensity) > 0.6f, "density varies across segments");
    }

    // bounded for any context, including extremes
    for (float x : { 0.0f, 1.0f })
    {
        Director::Context c;
        c.density = c.calm = c.cohesion = c.danger = x;
        Director d;
        d.reset();
        for (int event = 0; event < 120; ++event)
        {
            c.formState = static_cast<std::size_t>(event % 8);
            const auto plan = d.advance(c);
            const bool ok = plan.rhythmicDensity >= 0.35f && plan.rhythmicDensity <= 1.45f
                && plan.pulseWeight >= 0.25f && plan.pulseWeight <= 1.25f
                && plan.tonalWeight >= 0.55f && plan.tonalWeight <= 1.25f
                && plan.textureWeight >= 0.30f && plan.textureWeight <= 1.25f
                && plan.movementWeight >= 0.30f && plan.movementWeight <= 1.25f
                && plan.spaceWeight >= 0.35f && plan.spaceWeight <= 1.25f
                && plan.accentWeight >= 0.25f && plan.accentWeight <= 1.40f
                && std::abs(plan.harmonicPhaseOffset) <= 0.06f;
            check(ok, "values bounded");
            if (!ok)
                break;
        }
    }

    if (failures != 0)
    {
        std::cerr << failures << " check(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "compositional variation director: all checks passed\n";
    return EXIT_SUCCESS;
}
