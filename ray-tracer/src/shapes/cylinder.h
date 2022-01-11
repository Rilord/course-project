

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_CYLINDER_H
#define PBRT_SHAPES_CYLINDER_H

// shapes/cylinder.h*
#include "shape.h"

namespace pbrt {

// Cylinder Declarations
class Cylinder : public Shape {
  public:
    // Cylinder Public Methods
    Cylinder(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation, Float radius, Float zMin, Float zMax,
             Float phiMax)
        : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
          radius(radius),
          zMin(std::min(zMin, zMax)),
          zMax(std::max(zMin, zMax)),
          phiMax(Radians(Clamp(phiMax, 0, 360))) {}
    Bounds3f ObjectBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture) const;
    Float Area() const;
    Interaction Sample(const Point2f &u, Float *pdf) const;

  protected:
    // Cylinder Private Data
    const Float radius, zMin, zMax, phiMax;
};

std::shared_ptr<Cylinder> CreateCylinderShape(const Transform *o2w,
                                              const Transform *w2o,
                                              bool reverseOrientation,
                                              const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SHAPES_CYLINDER_H
