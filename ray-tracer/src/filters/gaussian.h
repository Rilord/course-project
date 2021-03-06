

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_FILTERS_GAUSSIAN_H
#define PBRT_FILTERS_GAUSSIAN_H

// filters/gaussian.h*
#include "filter.h"

namespace pbrt {

// Gaussian Filter Declarations
class GaussianFilter : public Filter {
  public:
    // GaussianFilter Public Methods
    GaussianFilter(const Vector2f &radius, Float alpha)
        : Filter(radius),
          alpha(alpha),
          expX(std::exp(-alpha * radius.x * radius.x)),
          expY(std::exp(-alpha * radius.y * radius.y)) {}
    Float Evaluate(const Point2f &p) const;

  private:
    // GaussianFilter Private Data
    const Float alpha;
    const Float expX, expY;

    // GaussianFilter Utility Functions
    Float Gaussian(Float d, Float expv) const {
        return std::max((Float)0, Float(std::exp(-alpha * d * d) - expv));
    }
};

GaussianFilter *CreateGaussianFilter(const ParamSet &ps);

}  // namespace pbrt

#endif  // PBRT_FILTERS_GAUSSIAN_H
