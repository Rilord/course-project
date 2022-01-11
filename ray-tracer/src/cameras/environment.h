

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CAMERAS_ENVIRONMENT_H
#define PBRT_CAMERAS_ENVIRONMENT_H

// cameras/environment.h*
#include "camera.h"
#include "film.h"

namespace pbrt {

// EnvironmentCamera Declarations
class EnvironmentCamera : public Camera {
  public:
    // EnvironmentCamera Public Methods
    EnvironmentCamera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
                      Float shutterClose, Film *film, const Medium *medium)
        : Camera(CameraToWorld, shutterOpen, shutterClose, film, medium) {}
    Float GenerateRay(const CameraSample &sample, Ray *) const;
};

EnvironmentCamera *CreateEnvironmentCamera(const ParamSet &params,
                                           const AnimatedTransform &cam2world,
                                           Film *film, const Medium *medium);

}  // namespace pbrt

#endif  // PBRT_CAMERAS_ENVIRONMENT_H
