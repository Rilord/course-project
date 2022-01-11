

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_FILTERS_TRIANGLE_H
#define PBRT_FILTERS_TRIANGLE_H

// filters/triangle.h*
#include "filter.h"

namespace pbrt {

// Triangle Filter Declarations
class TriangleFilter : public Filter {
  public:
    TriangleFilter(const Vector2f &radius) : Filter(radius) {}
    Float Evaluate(const Point2f &p) const;
};

TriangleFilter *CreateTriangleFilter(const ParamSet &ps);

}  // namespace pbrt

#endif  // PBRT_FILTERS_TRIANGLE_H
