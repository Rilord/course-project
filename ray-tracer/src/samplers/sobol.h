

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SAMPLERS_SOBOL_H
#define PBRT_SAMPLERS_SOBOL_H

// samplers/sobol.h*
#include "sampler.h"

namespace pbrt {

// SobolSampler Declarations
class SobolSampler : public GlobalSampler {
  public:
    // SobolSampler Public Methods
    std::unique_ptr<Sampler> Clone(int seed);
    SobolSampler(int64_t samplesPerPixel, const Bounds2i &sampleBounds)
        : GlobalSampler(RoundUpPow2(samplesPerPixel)),
          sampleBounds(sampleBounds) {
        if (!IsPowerOf2(samplesPerPixel))
            Warning("Non power-of-two sample count rounded up to %" PRId64
                    " for SobolSampler.",
                    this->samplesPerPixel);
        resolution = RoundUpPow2(
            std::max(sampleBounds.Diagonal().x, sampleBounds.Diagonal().y));
        log2Resolution = Log2Int(resolution);
        if (resolution > 0) CHECK_EQ(1 << log2Resolution, resolution);
    }
    int64_t GetIndexForSample(int64_t sampleNum) const;
    Float SampleDimension(int64_t index, int dimension) const;

  private:
    // SobolSampler Private Data
    const Bounds2i sampleBounds;
    int resolution, log2Resolution;
};

SobolSampler *CreateSobolSampler(const ParamSet &params,
                                 const Bounds2i &sampleBounds);

}  // namespace pbrt

#endif  // PBRT_SAMPLERS_SOBOL_H
