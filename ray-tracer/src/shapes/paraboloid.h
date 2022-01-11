

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_PARABOLOID_H
#define PBRT_SHAPES_PARABOLOID_H

// shapes/paraboloid.h*
#include "shape.h"

namespace pbrt {

// Paraboloid Declarations
class Paraboloid : public Shape {
  public:
    // Paraboloid Public Methods
    Paraboloid(const Transform *o2w, const Transform *w2o,
               bool reverseOrientation, Float radius, Float z0, Float z1,
               Float phiMax);
    Bounds3f ObjectBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture) const;
    Float Area() const;
    Interaction Sample(const Point2f &u, Float *pdf) const;

  protected:
    // Paraboloid Private Data
    const Float radius, zMin, zMax, phiMax;
};

std::shared_ptr<Paraboloid> CreateParaboloidShape(const Transform *o2w,
                                                  const Transform *w2o,
                                                  bool reverseOrientation,
                                                  const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SHAPES_PARABOLOID_H
