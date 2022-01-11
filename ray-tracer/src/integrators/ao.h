

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_INTEGRATORS_AO_H
#define PBRT_INTEGRATORS_AO_H

// integrators/ao.h*
#include "pbrt.h"
#include "integrator.h"

namespace pbrt {

// AOIntegrator Declarations
class AOIntegrator : public SamplerIntegrator {
  public:
    // AOIntegrator Public Methods
    AOIntegrator(bool cosSample, int nSamples,
                 std::shared_ptr<const Camera> camera,
                 std::shared_ptr<Sampler> sampler,
                 const Bounds2i &pixelBounds);
    Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;
 private:
    bool cosSample;
    int nSamples;
};

AOIntegrator *CreateAOIntegrator(const ParamSet &params,
                                 std::shared_ptr<Sampler> sampler,
                                 std::shared_ptr<const Camera> camera);

}  // namespace pbrt

#endif  // PBRT_INTEGRATORS_PATH_H
