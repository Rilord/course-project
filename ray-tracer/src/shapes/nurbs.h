

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_NURBS_H
#define PBRT_SHAPES_NURBS_H

// shapes/nurbs.h*
#include "pbrt.h"
#include "shape.h"
#include "geometry.h"

namespace pbrt {

std::vector<std::shared_ptr<Shape>> CreateNURBS(const Transform *o2w,
                                                const Transform *w2o,
                                                bool reverseOrientation,
                                                const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SHAPES_NURBS_H
