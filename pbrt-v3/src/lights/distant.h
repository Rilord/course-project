

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_LIGHTS_DISTANT_H
#define PBRT_LIGHTS_DISTANT_H

// lights/distant.h*
#include "pbrt.h"
#include "light.h"
#include "shape.h"
#include "scene.h"

namespace pbrt {

// DistantLight Declarations
class DistantLight : public Light {
  public:
    // DistantLight Public Methods
    DistantLight(const Transform &LightToWorld, const Spectrum &L,
                 const Vector3f &w);
    void Preprocess(const Scene &scene) {
        scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
    }
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    Spectrum Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;

  private:
    // DistantLight Private Data
    const Spectrum L;
    const Vector3f wLight;
    Point3f worldCenter;
    Float worldRadius;
};

std::shared_ptr<DistantLight> CreateDistantLight(const Transform &light2world,
                                                 const ParamSet &paramSet);

}  // namespace pbrt

#endif  // PBRT_LIGHTS_DISTANT_H
