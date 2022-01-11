


// filters/gaussian.cpp*
#include "filters/gaussian.h"
#include "paramset.h"

namespace pbrt {

// Gaussian Filter Method Definitions
Float GaussianFilter::Evaluate(const Point2f &p) const {
    return Gaussian(p.x, expX) * Gaussian(p.y, expY);
}

GaussianFilter *CreateGaussianFilter(const ParamSet &ps) {
    // Find common filter parameters
    Float xw = ps.FindOneFloat("xwidth", 2.f);
    Float yw = ps.FindOneFloat("ywidth", 2.f);
    Float alpha = ps.FindOneFloat("alpha", 2.f);
    return new GaussianFilter(Vector2f(xw, yw), alpha);
}

}  // namespace pbrt
