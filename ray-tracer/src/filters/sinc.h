

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_FILTERS_SINC_H
#define PBRT_FILTERS_SINC_H

// filters/sinc.h*
#include "filter.h"

namespace pbrt {

// Sinc Filter Declarations
class LanczosSincFilter : public Filter {
  public:
    // LanczosSincFilter Public Methods
    LanczosSincFilter(const Vector2f &radius, Float tau)
        : Filter(radius), tau(tau) {}
    Float Evaluate(const Point2f &p) const;
    Float Sinc(Float x) const {
        x = std::abs(x);
        if (x < 1e-5) return 1;
        return std::sin(Pi * x) / (Pi * x);
    }
    Float WindowedSinc(Float x, Float radius) const {
        x = std::abs(x);
        if (x > radius) return 0;
        Float lanczos = Sinc(x / tau);
        return Sinc(x) * lanczos;
    }

  private:
    const Float tau;
};

LanczosSincFilter *CreateSincFilter(const ParamSet &ps);

}  // namespace pbrt

#endif  // PBRT_FILTERS_SINC_H
