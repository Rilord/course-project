

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_DISK_H
#define PBRT_SHAPES_DISK_H

// shapes/disk.h*
#include "shape.h"

namespace pbrt {

// Disk Declarations
class Disk : public Shape {
  public:
    // Disk Public Methods
    Disk(const Transform *ObjectToWorld, const Transform *WorldToObject,
         bool reverseOrientation, Float height, Float radius, Float innerRadius,
         Float phiMax)
        : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
          height(height),
          radius(radius),
          innerRadius(innerRadius),
          phiMax(Radians(Clamp(phiMax, 0, 360))) {}
    Bounds3f ObjectBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture) const;
    Float Area() const;
    Interaction Sample(const Point2f &u, Float *pdf) const;

  private:
    // Disk Private Data
    const Float height, radius, innerRadius, phiMax;
};

std::shared_ptr<Disk> CreateDiskShape(const Transform *o2w,
                                      const Transform *w2o,
                                      bool reverseOrientation,
                                      const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SHAPES_DISK_H
