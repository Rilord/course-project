

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CAMERAS_ORTHOGRAPHIC_H
#define PBRT_CAMERAS_ORTHOGRAPHIC_H

// cameras/orthographic.h*
#include "pbrt.h"
#include "camera.h"
#include "film.h"

namespace pbrt {

// OrthographicCamera Declarations
class OrthographicCamera : public ProjectiveCamera {
  public:
    // OrthographicCamera Public Methods
    OrthographicCamera(const AnimatedTransform &CameraToWorld,
                       const Bounds2f &screenWindow, Float shutterOpen,
                       Float shutterClose, Float lensRadius,
                       Float focalDistance, Film *film, const Medium *medium)
        : ProjectiveCamera(CameraToWorld, Orthographic(0, 1), screenWindow,
                           shutterOpen, shutterClose, lensRadius, focalDistance,
                           film, medium) {
        // Compute differential changes in origin for orthographic camera rays
        dxCamera = RasterToCamera(Vector3f(1, 0, 0));
        dyCamera = RasterToCamera(Vector3f(0, 1, 0));
    }
    Float GenerateRay(const CameraSample &sample, Ray *) const;
    Float GenerateRayDifferential(const CameraSample &sample,
                                  RayDifferential *) const;

  private:
    // OrthographicCamera Private Data
    Vector3f dxCamera, dyCamera;
};

OrthographicCamera *CreateOrthographicCamera(const ParamSet &params,
                                             const AnimatedTransform &cam2world,
                                             Film *film, const Medium *medium);

}  // namespace pbrt

#endif  // PBRT_CAMERAS_ORTHOGRAPHIC_H
