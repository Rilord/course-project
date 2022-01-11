

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_FILTERS_MITCHELL_H
#define PBRT_FILTERS_MITCHELL_H

// filters/mitchell.h*
#include "filter.h"

namespace pbrt {

// Mitchell Filter Declarations
class MitchellFilter : public Filter {
  public:
    // MitchellFilter Public Methods
    MitchellFilter(const Vector2f &radius, Float B, Float C)
        : Filter(radius), B(B), C(C) {}
    Float Evaluate(const Point2f &p) const;
    Float Mitchell1D(Float x) const {
        x = std::abs(2 * x);
        if (x > 1)
            return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
                    (-12 * B - 48 * C) * x + (8 * B + 24 * C)) *
                   (1.f / 6.f);
        else
            return ((12 - 9 * B - 6 * C) * x * x * x +
                    (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) *
                   (1.f / 6.f);
    }

  private:
    const Float B, C;
};

MitchellFilter *CreateMitchellFilter(const ParamSet &ps);

}  // namespace pbrt

#endif  // PBRT_FILTERS_MITCHELL_H
