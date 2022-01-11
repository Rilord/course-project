

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SAMPLERS_MAXMIN_H
#define PBRT_SAMPLERS_MAXMIN_H

// samplers/maxmin.h*
#include "sampler.h"
#include "lowdiscrepancy.h"

namespace pbrt {

// MaxMinDistSampler Declarations
class MaxMinDistSampler : public PixelSampler {
  public:
    // MaxMinDistSampler Public Methods
    void StartPixel(const Point2i &);
    std::unique_ptr<Sampler> Clone(int seed);
    int RoundCount(int count) const { return RoundUpPow2(count); }
    MaxMinDistSampler(int64_t samplesPerPixel, int nSampledDimensions)
        : PixelSampler([](int64_t spp) {
              int Cindex = Log2Int(spp);
              if (Cindex >= sizeof(CMaxMinDist) / sizeof(CMaxMinDist[0])) {
                  Warning(
                      "No more than %d samples per pixel are supported with "
                      "MaxMinDistSampler. Rounding down.",
                      (1 << int(sizeof(CMaxMinDist) / sizeof(CMaxMinDist[0]))) -
                          1);
                  spp =
                      (1 << (sizeof(CMaxMinDist) / sizeof(CMaxMinDist[0]))) - 1;
              }
              if (!IsPowerOf2(spp)) {
                  spp = RoundUpPow2(spp);
                  Warning("Non power-of-two sample count rounded up to %" PRId64
                          " for MaxMinDistSampler.",
                          spp);
              }
              return spp;
          }(samplesPerPixel), nSampledDimensions) {
        int Cindex = Log2Int(samplesPerPixel);
        CHECK(Cindex >= 0 &&
              Cindex < (sizeof(CMaxMinDist) / sizeof(CMaxMinDist[0])));
        CPixel = CMaxMinDist[Cindex];
    }

  private:
    // MaxMinDistSampler Private Data
    const uint32_t *CPixel;
};

MaxMinDistSampler *CreateMaxMinDistSampler(const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SAMPLERS_MAXMIN_H
