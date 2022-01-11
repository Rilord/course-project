

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_LIGHTS_PROJECTION_H
#define PBRT_LIGHTS_PROJECTION_H

// lights/projection.h*
#include "pbrt.h"
#include "light.h"
#include "shape.h"
#include "mipmap.h"

namespace pbrt {

// ProjectionLight Declarations
class ProjectionLight : public Light {
  public:
    // ProjectionLight Public Methods
    ProjectionLight(const Transform &LightToWorld,
                    const MediumInterface &medium, const Spectrum &I,
                    const std::string &texname, Float fov);
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    Spectrum Projection(const Vector3f &w) const;
    Spectrum Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;

  private:
    // ProjectionLight Private Data
    std::unique_ptr<MIPMap<RGBSpectrum>> projectionMap;
    const Point3f pLight;
    const Spectrum I;
    Transform lightProjection;
    Float hither, yon;
    Bounds2f screenBounds;
    Float cosTotalWidth;
};

std::shared_ptr<ProjectionLight> CreateProjectionLight(
    const Transform &light2world, const Medium *medium,
    const ParamSet &paramSet);

}  // namespace pbrt

#endif  // PBRT_LIGHTS_PROJECTION_H
