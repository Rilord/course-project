

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CAMERAS_REALISTIC_H
#define PBRT_CAMERAS_REALISTIC_H

// cameras/realistic.h*
#include "pbrt.h"
#include "camera.h"
#include "film.h"

namespace pbrt {

// RealisticCamera Declarations
class RealisticCamera : public Camera {
  public:
    // RealisticCamera Public Methods
    RealisticCamera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
                    Float shutterClose, Float apertureDiameter,
                    Float focusDistance, bool simpleWeighting,
                    std::vector<Float> &lensData, Film *film,
                    const Medium *medium);
    Float GenerateRay(const CameraSample &sample, Ray *) const;

  private:
    // RealisticCamera Private Declarations
    struct LensElementInterface {
        Float curvatureRadius;
        Float thickness;
        Float eta;
        Float apertureRadius;
    };

    // RealisticCamera Private Data
    const bool simpleWeighting;
    std::vector<LensElementInterface> elementInterfaces;
    std::vector<Bounds2f> exitPupilBounds;

    // RealisticCamera Private Methods
    Float LensRearZ() const { return elementInterfaces.back().thickness; }
    Float LensFrontZ() const {
        Float zSum = 0;
        for (const LensElementInterface &element : elementInterfaces)
            zSum += element.thickness;
        return zSum;
    }
    Float RearElementRadius() const {
        return elementInterfaces.back().apertureRadius;
    }
    bool TraceLensesFromFilm(const Ray &ray, Ray *rOut) const;
    static bool IntersectSphericalElement(Float radius, Float zCenter,
                                          const Ray &ray, Float *t,
                                          Normal3f *n);
    bool TraceLensesFromScene(const Ray &rCamera, Ray *rOut) const;
    void DrawLensSystem() const;
    void DrawRayPathFromFilm(const Ray &r, bool arrow,
                             bool toOpticalIntercept) const;
    void DrawRayPathFromScene(const Ray &r, bool arrow,
                              bool toOpticalIntercept) const;
    static void ComputeCardinalPoints(const Ray &rIn, const Ray &rOut, Float *p,
                                      Float *f);
    void ComputeThickLensApproximation(Float pz[2], Float f[2]) const;
    Float FocusThickLens(Float focusDistance);
    Float FocusBinarySearch(Float focusDistance);
    Float FocusDistance(Float filmDist);
    Bounds2f BoundExitPupil(Float pFilmX0, Float pFilmX1) const;
    void RenderExitPupil(Float sx, Float sy, const char *filename) const;
    Point3f SampleExitPupil(const Point2f &pFilm, const Point2f &lensSample,
                            Float *sampleBoundsArea) const;
    void TestExitPupilBounds() const;
};

RealisticCamera *CreateRealisticCamera(const ParamSet &params,
                                       const AnimatedTransform &cam2world,
                                       Film *film, const Medium *medium);

}  // namespace pbrt

#endif  // PBRT_CAMERAS_REALISTIC_H
